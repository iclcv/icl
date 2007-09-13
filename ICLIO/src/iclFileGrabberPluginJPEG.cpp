#include <iclFileGrabberPluginJPEG.h>

#ifdef WITH_JPEG_SUPPORT
#include <iclStrTok.h>
#include <iclJPEGHandle.h>
#endif

using namespace std;

namespace icl{
#ifdef WITH_JPEG_SUPPORT
  void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
    // {{{ open 
    ICLASSERT_RETURN(dest);
    file.open(File::readBinary);
    ICLASSERT_RETURN(file.isOpen());    
    
    // setup the jpeg error routine once
    if (setjmp(m_poJPEGHandle->em.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
          * We need to clean up the JPEG object and signal the error to the caller */
      jpeg_destroy_decompress(&m_poJPEGHandle->info);
      throw InvalidFileFormatException();
    }
    
    /* Step 1: Initialize the JPEG decompression object. */
    jpeg_create_decompress(&m_poJPEGHandle->info);
    
    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&m_poJPEGHandle->info, (FILE*)file.getHandle());
    
    /* request to save comments */
    jpeg_save_markers (&m_poJPEGHandle->info, JPEG_COM, 1024);

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&m_poJPEGHandle->info, TRUE);
    
    FileGrabberPlugin::HeaderInfo oInfo;

    /* evaluate markers, i.e. comments */
    for (jpeg_saved_marker_ptr m = m_poJPEGHandle->info.marker_list; m; m = m->next){
      if (m->marker != JPEG_COM) continue;
      vector<string> ts = StrTok(string((char*)m->data,(char*)m->data+m->data_length)," ").allTokens();
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
    jpeg_start_decompress(&m_poJPEGHandle->info);
    
    /* After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     */
    
    oInfo.imageDepth = depth8u; // can only read depth8u
    oInfo.size.width  = m_poJPEGHandle->info.output_width;
    oInfo.size.height = m_poJPEGHandle->info.output_height;
    switch (m_poJPEGHandle->info.out_color_space) {
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
    if (setjmp(m_poJPEGHandle->em.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object and signal the error to the caller */
      jpeg_destroy_decompress(&m_poJPEGHandle->info);
      if (oInfo.channelCount == 3) ICL_DELETE_ARRAY( pcBuf );
      throw InvalidFileFormatException();
    }
    
    ICLASSERT_THROW ( m_poJPEGHandle->info.output_components == oInfo.channelCount ,InvalidFileFormatException());
    int iRowDim = m_poJPEGHandle->info.output_width * m_poJPEGHandle->info.output_components;
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
    while (m_poJPEGHandle->info.output_scanline < m_poJPEGHandle->info.output_height) {
      /* jpeg_read_scanlines expects an array of pointers to scanlines.
       * Here the array is only one element long, but you could ask for
       * more than one scanline at a time if that's more convenient.
       */
      (void) jpeg_read_scanlines(&m_poJPEGHandle->info, &pcBuf, 1);
      
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
    (void) jpeg_finish_decompress(&m_poJPEGHandle->info);
    
    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jpgErr.pub.num_warnings is nonzero).
     */
    
    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress(&m_poJPEGHandle->info);
    if (oInfo.channelCount == 3) ICL_DELETE_ARRAY( pcBuf );
  }
  // }}}
#else
  void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
    ERROR_LOG("JPEG support currently not available! \n" << 
              "To enabled JPEG support: you have to compile the ICLIO package\n" <<
              "with -DWITH_JPEG_SUPPORT compiler flag AND with a valid\n" << 
              "LIBJPEG_ROOT set.");    
    ERROR_LOG("Destination image is set to NULL, which may cause further errors!");
    (void) file;
    ICL_DELETE( *dest );
  }
#endif
  FileGrabberPluginJPEG::FileGrabberPluginJPEG(){
    // {{{ open

    m_poJPEGHandle = new JPEGDataHandle;
  }

  // }}}
  FileGrabberPluginJPEG::~FileGrabberPluginJPEG(){
    // {{{ open

    ICL_DELETE(m_poJPEGHandle);
  }

  // }}}

}// end of the namespace icl
