#include <iclFileWriter.h>
#include <iclIO.h>
#include <zlib.h>
#include <sstream>
/*
  FileWriter.cpp

  Written by: Michael Götting, Robert Haschke (2006)
  University of Bielefeld
  AG Neuroinformatik
  mgoettin@techfak.uni-bielefeld.de
*/


using namespace std;

namespace icl {

   //--------------------------------------------------------------------------
   void FileWriter::setFileName (const string& sFileName) 
      // {{{ open 

   {
      // check for supported file type
      bool bGzipped;
      if (getFileType (sFileName, bGzipped) < 0) {
         throw ICLException ("not supported file type.");
      }

      // check for hashes (directly before the file suffix only)
      string::size_type iSuffixPos;
      analyseHashes (sFileName, nCounterDigits, iSuffixPos);

      // set variables
      nCounter = 1;
      sFileSuffix = sFileName.substr (iSuffixPos);
      sFilePrefix = sFileName.substr (0, iSuffixPos-nCounterDigits);
   }

// }}}

     //--------------------------------------------------------------------------

  void FileWriter::setCSVFlag(csvFlag f,bool value){
      // {{{ open 
    switch(f){
      case csvSplitFiles:
        m_bCsvSplitFiles=value;
      break;
      case csvExtendFilename:
        m_bCsvExtendFilename=value;
      break;
    }
  }

  // }}}

   
   
   //--------------------------------------------------------------------------
   void FileWriter::setFileNameCSV (string& sFilePrefix, const ImgBase *poSrc) 
      // {{{ open 

   {
      char result[100];
      sprintf(result,"-ICL:%dx%dx%d:%s:%s",
              poSrc->getSize().width,poSrc->getSize().height,
              poSrc->getChannels(), 
              translateDepth(poSrc->getDepth()).c_str(), 
              translateFormat(poSrc->getFormat()).c_str());
      sFilePrefix+=result;
   }

// }}}   
   
   
   
   //--------------------------------------------------------------------------
   string FileWriter::buildFileName()
      // {{{ open
   {
      if (nCounterDigits == 0) return sFilePrefix+sFileSuffix;
      
      ostringstream oss; 
      oss << sFilePrefix;
      oss.fill('0'); 
      oss.width(nCounterDigits); 
      oss << nCounter << sFileSuffix; 
      nCounter++;
      return oss.str ();
   }
// }}}

   //--------------------------------------------------------------------------
   void FileWriter::write(const ImgBase *poSrc) {
      // {{{ open
      if (m_bCsvExtendFilename) setFileNameCSV (sFilePrefix, poSrc);

      FileInfo oInfo (buildFileName()); // create file info
      openFile (oInfo, "wb"); // open file for writing
      
      const ImgBase *poImg = poSrc;
      if (poSrc->getDepth () != depth8u && 
          oInfo.eFileFormat != ioFormatICL &&
          oInfo.eFileFormat != ioFormatCSV) {
        // image needs to be converted to depth8u
        poImg = poSrc->convert<icl8u>(&m_oImg8u);
      } // otherwise, use poSrc directly
      
      try {
         // write file
         switch (oInfo.eFileFormat) {
           case ioFormatICL:
           case ioFormatPNM: 
             writePNMICL (poImg, oInfo);
             break;
           case ioFormatJPG:
             writeJPG (poImg->asImg<icl8u>(), oInfo);
             break;
           case ioFormatCSV:
             writeCSV (poImg, oInfo);
             break;
           default: break;
         }
         closeFile (oInfo);
      } catch (ICLException &e) {
         closeFile (oInfo);
         throw;
      }
   }

// }}}
  
   int plainWrite (void *fp, const void *pData, size_t len) {
     // {{{ open

      return fwrite (pData, 1, len, (FILE*) fp);
   }

// }}}

   //--------------------------------------------------------------------------
   void FileWriter::writePNMICL(const ImgBase *poSrc, const FileInfo& oInfo) {
      // {{{ open

      int (*pWrite)(void *fp, const void *pData, size_t len) 
         = oInfo.bGzipped ? gzwrite : plainWrite;

      // check exact file type first:
      // pgm: write separate channels below each other as pgm image (P5)
      // ppm: require a multiple of 3 of channels, write as ppm image (P6)
      // pnm: write 3-channel color images as pnm image (P6),
      //      all other formats as pnm (P5)
      // icl: write channels consecutively

      bool bPPM=false;
      int  iNumImages = poSrc->getChannels ();
      string sType = sFileSuffix.substr (1, 3);
      if (sType == "ppm") {
         bPPM = (poSrc->getChannels () % 3 == 0);
         if (!bPPM) throw ICLException ("Image cannot be written as ppm.");
      } else if (sType == "pnm") {
         bPPM = (poSrc->getChannels () == 3 && 
                 getChannelsOfFormat (poSrc->getFormat()) == 3);
      }
      if (bPPM) iNumImages = iNumImages / 3;
        
      ICLException writeError ("Error writing file.");

      //---- Write header ----
      char acBuf[1024];
      // magic number
      if (oInfo.eFileFormat != ioFormatICL) {
         sprintf (acBuf, "%s\n", bPPM ? "P6" : "P5");
         if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;
      }

      // format
      sprintf (acBuf, "# Format %s\n", translateFormat(poSrc->getFormat()).c_str());
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;

      // timestamp
      sprintf (acBuf, "# TimeStamp %lld\n", poSrc->getTime().toMicroSeconds());
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;

      // number of images
      sprintf (acBuf, "# NumFeatures %d\n", iNumImages);
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;

      // image depth
      sprintf (acBuf, "# ImageDepth %s\n", translateDepth(poSrc->getDepth()).c_str());
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;

      // ROI
      Rect roi = poSrc->getROI ();
      sprintf (acBuf, "# ROI %d %d %d %d\n", roi.x, roi.y, roi.width, roi.height);
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;
    
      // image size
      sprintf (acBuf, "%d %d\n%d\n", 
               poSrc->getSize().width, poSrc->getSize().height * iNumImages, 255);
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;


      // write image data
      if (bPPM) { // file format is interleaved, i.e. RGB or something similar
         const Img8u *poImg8u = poSrc->asImg<icl8u>();
         const Size& size = poSrc->getSize();
         int iDim   = 3 * size.width;
         icl8u *pcBuf = new icl8u[iDim];
         icl8u *pc;
        
         for (int i=0;i<iNumImages;i++) {
            const icl8u *pcR = poImg8u->getData (i*3);
            const icl8u *pcG = poImg8u->getData (i*3+1);
            const icl8u *pcB = poImg8u->getData (i*3+2);
            for (int l=0; l<size.height; l++) {
               pc=pcBuf;
               for (int c=0; c<size.width; ++c, ++pcR, ++pcG, ++pcB) {
                  *pc++ = *pcR;
                  *pc++ = *pcG;
                  *pc++ = *pcB;
               } // for rows (interleave)

               if (pWrite (oInfo.fp, pcBuf, iDim) != iDim)
                  throw writeError;
            } // for lines
         } // for images
      } else { // write all channels separately
         int iDim = poSrc->getDim () * getSizeOf(poSrc->getDepth());
         for (int i=0;i<iNumImages;i++) {
            if (pWrite (oInfo.fp, poSrc->getDataPtr (i), iDim) != iDim) 
               throw writeError;
         }
      }
   }

// }}}
  //--------------------------------------------------------------------------
   

     template<class T,class R>
     string FileWriter::writeCSVTmpl(const Img<T> *poSrc,int ch) {
       // {{{ open
       const Size& size = poSrc->getSize();
       ostringstream oss;
       // write channel consecutively
      const T *pDat = poSrc->getData(ch);
      for (int j=0;j<size.height-1;j++) {
        for (int k=0;k<size.width;k++, pDat++){
         oss << (R)*pDat << ",";
        }
        oss << endl;
      }
      for (int k=0;k<size.width-1;k++, pDat++){
       oss << (R)*pDat << ",";
      }
      oss << (R)*pDat;
//      oss << endl;
      return (oss.str());
     }
// }}}


   //--------------------------------------------------------------------------

     template<class T,class R>
     void FileWriter::writeCSVTmpl(const Img<T> *poSrc, FileInfo& oInfo) {

    int (*pWrite)(void *fp, const void *pData, size_t len) 
      = oInfo.bGzipped ? gzwrite : plainWrite;
       
    ICLException writeError ("Error writing file.");       
       // {{{ open
       int iNumImages = poSrc->getChannels();
       // write channel consecutively
       if (!m_bCsvSplitFiles){ //every channel in its own .csv file, if splitFiles is enabled
        ostringstream oss;
        for (int i=0;i<iNumImages-1;i++) {
            oss<<writeCSVTmpl<T,R>(poSrc,i)<<","<<endl;
        }
        oss<<writeCSVTmpl<T,R>(poSrc,iNumImages-1)<<endl;
        if (!pWrite (oInfo.fp, (oss.str()).c_str(), strlen((oss.str()).c_str()))) 
          throw writeError;

      }
      else{
        for (int i=0;i<iNumImages-1;i++) {
          ostringstream oss;
          oss<<writeCSVTmpl<T,R>(poSrc,i)<<endl;
          if (!pWrite (oInfo.fp, (oss.str()).c_str(), strlen((oss.str()).c_str()))) 
          throw writeError;
          closeFile (oInfo);
          oInfo.sFileName=buildFileName(); //set the filename, increasing the count value
          openFile (oInfo, "wb"); // open file for writing
        }
        ostringstream oss;
        oss<<writeCSVTmpl<T,R>(poSrc,iNumImages-1)<<endl;
        if (!pWrite (oInfo.fp, (oss.str()).c_str(), strlen((oss.str()).c_str()))) 
        throw writeError;
        
      }
     }
// }}}


  //--------------------------------------------------------------------------
   
   
   
  void FileWriter::writeCSV(const ImgBase *poSrc, FileInfo& oInfo) {
    // {{{ open
    switch(poSrc->getDepth()) {
      case depth8u: writeCSVTmpl<icl8u,int>(poSrc->asImg<icl8u>(),oInfo);
        break;
      case depth16s: writeCSVTmpl<icl16s,int>(poSrc->asImg<icl16s>(),oInfo);
        break;
      case depth32s: writeCSVTmpl<icl32s,int>(poSrc->asImg<icl32s>(),oInfo);
        break;
      case depth32f: writeCSVTmpl<icl32f,float>(poSrc->asImg<icl32f>(),oInfo);
        break;
      case depth64f: writeCSVTmpl<icl64f,double>(poSrc->asImg<icl64f>(),oInfo);
        break;
      default: ICL_INVALID_DEPTH; break;
    }
}
  
// }}}

   //--------------------------------------------------------------------------
   void FileWriter::writeJPG(const Img<icl8u> *poSrc, 
                             const FileInfo& oInfo, 
                             int iQuality) {
      // {{{ open

      J_COLOR_SPACE jCS;
      switch (poSrc->getFormat ()) {
        case formatGray: jCS = JCS_GRAYSCALE; break;
        case formatYUV:  jCS = JCS_YCbCr; break;
        case formatRGB:  jCS = JCS_RGB; break;
        default: 
           throw ICLException (translateFormat (poSrc->getFormat()) + 
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
         if (pcBuf) delete[] pcBuf;
         jpeg_destroy_compress(&jpgCinfo);
         throw writeError;
      }

      /* Now we can initialize the JPEG compression object. */
      jpeg_create_compress(&jpgCinfo);

      // Step 2: specify data destination
      jpeg_stdio_dest(&jpgCinfo, (FILE*) oInfo.fp);

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
      jpeg_set_quality(&jpgCinfo, iQuality, TRUE /* limit to baseline-JPEG values */);

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

// }}}

} //namespace 
