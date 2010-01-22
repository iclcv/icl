#include <ICLIO/FileWriterPluginJPEG.h>
#include <ICLCore/Types.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/IOUtils.h>
#include <cstring>
using std::strlen;

#ifdef HAVE_LIBJPEG
#include <ICLIO/JPEGHandle.h>
#endif

using namespace std;

namespace icl{
  void FileWriterPluginJPEG::setQuality(int value){
    s_iQuality = value;
  }
  int FileWriterPluginJPEG::s_iQuality = 90;
  
  Img8u FileWriterPluginJPEG::s_oBufferImage;
  
  Mutex FileWriterPluginJPEG::s_oBufferImageMutex;
 
  
#ifdef HAVE_LIBJPEG
  void FileWriterPluginJPEG::write(File &file, const ImgBase *image){
    ICLASSERT_RETURN(image);
    format fmt = image->getFormat();

    if(fmt != formatGray && fmt != formatRGB && fmt != formatYUV){
      throw ICLException (str(fmt)+" not supported by jpeg");
    }
    
    Mutex::Locker _locker(s_oBufferImageMutex);
    
    const Img8u *poSrc = 0;
    if(image->getDepth()!= depth8u){
      image->convert<icl8u>(&s_oBufferImage);
      poSrc = &s_oBufferImage;
    }else{
      poSrc = image->asImg<icl8u>();
    }
    
    file.open(File::writeBinary);
    ICLASSERT_RETURN(file.isOpen());

    
    //////////////////////////////////////////////////////////////////////
    /// WRITE HEADER DATA ////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    J_COLOR_SPACE jCS;
    switch (poSrc->getFormat ()) {
      case formatGray: jCS = JCS_GRAYSCALE; break;
      case formatYUV:  jCS = JCS_YCbCr; break;
      case formatRGB:  jCS = JCS_RGB; break;
      default: 
        throw ICLException (str(poSrc->getFormat()) + 
                            string (" not supported by jpeg"));
    }
    
    ICLException writeError ("Error writing file.");

    struct jpeg_compress_struct jpgCinfo;
    struct icl_jpeg_error_mgr   jpgErr;
    
    icl8u *pcBuf=0;
    
    // Step 1: Set up the error handler first, in case initialization fails
    jpgCinfo.err = jpeg_std_error(&jpgErr);
    if (setjmp(jpgErr.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
          * We need to clean up the JPEG object and signal the error to the caller */
      ICL_DELETE( pcBuf );
      jpeg_destroy_compress(&jpgCinfo);
      throw writeError;
    }
    
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&jpgCinfo);
    
    // Step 2: specify data destination
    jpeg_stdio_dest(&jpgCinfo, (FILE*)file.getHandle());
    
    /* Step 3: set parameters for compression */
    jpgCinfo.image_width  = poSrc->getSize().width;
    jpgCinfo.image_height = poSrc->getSize().height;
    jpgCinfo.input_components = poSrc->getChannels(); // # of color components 
    jpgCinfo.in_color_space = jCS; 	/* colorspace of input image */
    
    /* Now use the library's routine to set default compression parameters.
        * (You must set at least jpgCinfo.in_color_space before calling this,
        * since the defaults depend on the source color space.) */
    jpeg_set_defaults(&jpgCinfo);
    
    /* Now you can set any non-default parameters you wish to.
        * Here we just illustrate the use of quality (quantization table) scaling: */
    jpeg_set_quality(&jpgCinfo, s_iQuality, TRUE /* limit to baseline-JPEG values */);
    
    /* Step 4: Start compressor */
    /* TRUE ensures that we will write a complete interchange-JPEG file.
        * Pass TRUE unless you are very sure of what you're doing. */
    jpeg_start_compress(&jpgCinfo, TRUE);
    
    /* Step 5: Write comments */
    char acBuf[1024];
    // timestamp
    sprintf (acBuf, "TimeStamp %lld", poSrc->getTime().toMicroSeconds());
    jpeg_write_marker(&jpgCinfo, JPEG_COM, (JOCTET*) acBuf, strlen(acBuf));
    
    // ROI
    Rect roi = poSrc->getROI ();
    sprintf (acBuf, "ROI %d %d %d %d", roi.x, roi.y, roi.width, roi.height);
    jpeg_write_marker(&jpgCinfo, JPEG_COM, (JOCTET*) acBuf, strlen(acBuf));
    

    //////////////////////////////////////////////////////////////////////
    /// WRITE IMAGE DATA  ////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    
    /* Step 6: while (scan lines remain to be written) */
    if (poSrc->getChannels () == 1) {
      int iLineStep = poSrc->getSize().width;
      // grayscale image, can handover image channels directly
      while (jpgCinfo.next_scanline < jpgCinfo.image_height) {
        icl8u *pcBuf = const_cast<icl8u*>(poSrc->getData (0)) + jpgCinfo.next_scanline*iLineStep;
        (void) jpeg_write_scanlines(&jpgCinfo, &pcBuf, 1);
      }
    } else {
      // file format is interleaved, i.e. RGB or something similar
      const Size& size = poSrc->getSize();
      int iNumImages = 1;
      int iDim       = 3 * size.width;
      icl8u *pc;
      
      pcBuf = new icl8u[iDim];
      for (int i=0;i<iNumImages;i++) {
        const icl8u *pcR = poSrc->getData (i*3);
        const icl8u *pcG = poSrc->getData (i*3+1);
        const icl8u *pcB = poSrc->getData (i*3+2);
        for (int l=0; l<size.height; l++) {
          pc=pcBuf;
          for (int c=0; c<size.width; ++c, ++pcR, ++pcG, ++pcB) {
            *pc++ = *pcR;
            *pc++ = *pcG;
            *pc++ = *pcB;
          } // for rows (interleave)
          (void) jpeg_write_scanlines(&jpgCinfo, &pcBuf, 1);
        } // for lines
      } // for images
      delete[] pcBuf; pcBuf = 0;
    }
    
    /* Step 7: Finish compression */
    jpeg_finish_compress(&jpgCinfo);
    
    /* Step 8: release JPEG compression object */
    jpeg_destroy_compress(&jpgCinfo);
    
    
    
    
  }

#else // no JPEG_SUPPORT
  /// empty implementation with warning message!
  void FileWriterPluginJPEG::write(File &file, const ImgBase *poSrc){
    // {{{ open

    ERROR_LOG("JPEG support currently not available! \n" << 
              "To enabled JPEG support: you have to compile the ICLIO package\n" <<
              "with -DHAVE_LIBJPEG compiler flag AND with a valid\n" << 
              "LIBJPEG_ROOT set.");    
    (void) file;
    (void) poSrc;
  }  

  // }}}
#endif

  
  
}

