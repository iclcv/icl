#include <ICLIO/JPEGDecoder.h>
#include <ICLIO/JPEGHandle.h>
#include <ICLUtils/Macros.h>
#include <ICLIO/FileGrabberPlugin.h>
#include <ICLUtils/StrTok.h>

namespace icl{

  struct DataSourceManager : public jpeg_source_mgr {
    JOCTET *given_data;
    //const JOCTET * next_input_byte; /* => next byte to read from buffer */
    //size_t bytes_in_buffer;	/* # of bytes remaining in buffer */

    DataSourceManager(j_decompress_ptr cinfo, JOCTET *data, int maxDataLen):given_data(data){
      cinfo->client_data = data;                    // save client data [???]
      
      // initialize parent jpeg_source_mgr function pointers
      init_source = s_init_source;                  
      fill_input_buffer = s_fill_input_buffer;
      skip_input_data = s_skip_input_data;
      resync_to_restart = jpeg_resync_to_restart; // jpeg default
      term_source = s_term_source;

      // initialize parant jpeg_source_mgr members
      bytes_in_buffer = maxDataLen;
      next_input_byte = data;
    }
    static void s_init_source(j_decompress_ptr){}
    static void s_term_source(j_decompress_ptr){}

    static boolean s_fill_input_buffer(j_decompress_ptr cinfo){
      // for secure abort!
      DataSourceManager *dsm = reinterpret_cast<DataSourceManager*>(cinfo->src);
      
      static JOCTET EOI[] = { 0xFF, JPEG_EOI };
      
      // warning
      WARNMS(cinfo, JWRN_JPEG_EOF);
      
      // set data pointer to artificial eoi-marker
      dsm->next_input_byte = EOI;
      dsm->bytes_in_buffer = 2;
      
      return TRUE;
    }
    static void s_skip_input_data(j_decompress_ptr cinfo, long num_bytes){
      DataSourceManager *dsm = reinterpret_cast<DataSourceManager*>(cinfo->src);
      
      if(num_bytes >= (long)dsm->bytes_in_buffer)
        {
          s_fill_input_buffer(cinfo);
          return;
        }
      
      dsm->bytes_in_buffer -= num_bytes;
      dsm->next_input_byte += num_bytes;
    }
  };


  void JPEGDecoder::decode(const unsigned char *data, unsigned int maxDataLen, ImgBase **dest){
    decode_internal(0,data,maxDataLen,dest);
  }

  void JPEGDecoder::decode(File &file, ImgBase **dest) throw (InvalidFileFormatException){
    decode_internal(&file,0,0,dest);
    return;
  }

  void JPEGDecoder::decode_internal(File *file,const unsigned char *data,unsigned int maxDataLen, ImgBase **dest) throw (InvalidFileFormatException){
    ICLASSERT_RETURN(!(file&&data));
    ICLASSERT_RETURN(!(!file&&!data));
    ICLASSERT_RETURN(dest);
    
    if(file && !file->isOpen()){
      file->open(File::readBinary);
      ICLASSERT_RETURN(file->isOpen());    
    }
    
    JPEGDataHandle jpegHandle;
    
    // setup the jpeg error routine once
    if (setjmp(jpegHandle.em.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
          * We need to clean up the JPEG object and signal the error to the caller */
      jpeg_destroy_decompress(&jpegHandle.info);
      throw InvalidFileFormatException();
    }
    
    /* Step 1: Initialize the JPEG decompression object. */
    jpeg_create_decompress(&jpegHandle.info);
    
    /* Step 2: specify data source (eg, a file) */
    if(file){
      jpeg_stdio_src(&jpegHandle.info, (FILE*)file->getHandle());
    }else{
      DataSourceManager dsm(&jpegHandle.info,const_cast<JOCTET*>(data),maxDataLen);
      jpegHandle.info.src = &dsm;  
    }
    
    /* request to save comments */
    jpeg_save_markers (&jpegHandle.info, JPEG_COM, 1024);

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&jpegHandle.info, TRUE);
    
    FileGrabberPlugin::HeaderInfo oInfo;

    /* evaluate markers, i.e. comments */
    for (jpeg_saved_marker_ptr m = jpegHandle.info.marker_list; m; m = m->next){
      if (m->marker != JPEG_COM) continue;
      std::vector<std::string> ts = StrTok(std::string((char*)m->data,(char*)m->data+m->data_length)," ").allTokens();
      if(ts.size() < 2) continue;
      //      char acBuf[1025] = "";
      //memcpy (acBuf, m->data, m->data_length);
      //acBuf[m->data_length] = '\0'; // terminating null
      
      //  istringstream iss (acBuf);
      // string sKey, sValue;
      // iss >> sKey;

       if (ts[0] == "TimeStamp") {
         oInfo.time = Time::microSeconds(atoi(ts[1].c_str()));
       } else if (ts[0] == "ROI") {
         oInfo.roi = Rect(atoi(ts[1].c_str()),atoi(ts[2].c_str()),atoi(ts[3].c_str()),atoi(ts[4].c_str()));
       }
    }

    /* Step 4: set parameters for decompression */
    
    /* Step 5: Start decompressor */
    jpeg_start_decompress(&jpegHandle.info);
    
  
    /* After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     */
    
    oInfo.imageDepth = depth8u; // can only read depth8u
    oInfo.size.width  = jpegHandle.info.output_width;
    oInfo.size.height = jpegHandle.info.output_height;
    switch (jpegHandle.info.out_color_space) {
      case JCS_GRAYSCALE: oInfo.imageFormat = formatGray; break;
      case JCS_RGB: oInfo.imageFormat = formatRGB; break;
      case JCS_YCbCr: oInfo.imageFormat = formatYUV; break;
      default: throw ICLException("unknown color space");
    }
    oInfo.channelCount = getChannelsOfFormat (oInfo.imageFormat);
    
    icl8u *pcBuf = 0;
    //////////////////////////////////////////////////////////////////////
    /// ADAPT THE DESTINATION IMAGE //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    ensureCompatible (dest, oInfo.imageDepth, oInfo.size,
                      oInfo.channelCount,oInfo.imageFormat, oInfo.roi);

    ImgBase *poImg = *dest;
    poImg->setTime(oInfo.time);
    
    ////////////////////////////////////////////////////////////////////////////
    ///// READ IMAGE DATA //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    // update jump context to allow proper throw
    if (setjmp(jpegHandle.em.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object and signal the error to the caller */
      jpeg_destroy_decompress(&jpegHandle.info);
      if (oInfo.channelCount == 3) ICL_DELETE_ARRAY( pcBuf );
      throw InvalidFileFormatException();
    }
    
    ICLASSERT_THROW ( jpegHandle.info.output_components == oInfo.channelCount ,InvalidFileFormatException());
    int iRowDim = jpegHandle.info.output_width * jpegHandle.info.output_components;
    icl8u *pcR=0, *pcG=0, *pcB=0;
    
    if (oInfo.channelCount == 1) pcBuf = poImg->asImg<icl8u>()->getData (0);
    else if (oInfo.channelCount == 3) {
      pcBuf = new icl8u[iRowDim];
      pcR = poImg->asImg<icl8u>()->getData (0);
      pcG = poImg->asImg<icl8u>()->getData (1);
      pcB = poImg->asImg<icl8u>()->getData (2);
    }
    else {ERROR_LOG ("This should not happen."); return;}
    
    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */
    while (jpegHandle.info.output_scanline < jpegHandle.info.output_height) {
      /* jpeg_read_scanlines expects an array of pointers to scanlines.
       * Here the array is only one element long, but you could ask for
       * more than one scanline at a time if that's more convenient.
       */
      (void) jpeg_read_scanlines(&jpegHandle.info, &pcBuf, 1);
      
      if (oInfo.channelCount == 1) pcBuf += iRowDim;
      else { // deinterleave three channel data
        icl8u *pc = pcBuf;
        for (int c=0; c<oInfo.size.width; ++c, ++pcR, ++pcG, ++pcB) {
          *pcR = *pc++;
          *pcG = *pc++;
          *pcB = *pc++;
        }
      }
    }
    
    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&jpegHandle.info);
    
    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jpgErr.pub.num_warnings is nonzero).
     */
    
    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress(&jpegHandle.info);
    if (oInfo.channelCount == 3) ICL_DELETE_ARRAY( pcBuf );    
  }
}



