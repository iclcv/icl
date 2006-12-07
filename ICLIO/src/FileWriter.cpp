/*
  FileWriter.cpp

  Written by: Michael Götting, Robert Haschke (2006)
  University of Bielefeld
  AG Neuroinformatik
  mgoettin@techfak.uni-bielefeld.de
*/

#include <FileWriter.h>
#include <IO.h>
#include <zlib.h>
#include <sstream>

using namespace std;

namespace icl {

   //--------------------------------------------------------------------------
   void FileWriter::setFileName (const string& sFileName) throw (ICLException)
      // {{{ open 

   {
      string::size_type iSuffixPos;
      string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == string::npos) 
         throw ICLException ("cannot detect file type");

      string sType = sFileName.substr (iTmpPos);
      if (sType == ".gz" && iTmpPos > 0) { // search further for file type
         iSuffixPos = sFileName.rfind ('.', iTmpPos-1);
         sType = sFileName.substr (iSuffixPos);
      } else iSuffixPos = iTmpPos;

      // check for supported file type
      bool bGzipped;
      if (getFileType (sType, bGzipped) < 0) {
         throw ICLException ("not supported file type: " + sType);
      }

      ICLASSERT (iSuffixPos < sFileName.size());

      // check for hashes      
      unsigned int nHashes = 0;
      for (string::const_reverse_iterator start (sFileName.begin() + iSuffixPos),
              it = start, end = sFileName.rend(); it != end; ++it) {
         if (*it != '#') {
            // first pos without hash, count hashes
            nHashes = it - start;
            break;
         }
      }

      // set variables
      nCounterDigits = nHashes;
      nCounter = 1;
      sFileSuffix = sType;
      sFilePrefix = nCounterDigits ? sFileName.substr (0, iSuffixPos-nHashes) : sFileName;
   }

// }}}

   //--------------------------------------------------------------------------
   string FileWriter::buildFileName()
      // {{{ open
   {
      // if counting is disabled, sFilePrefix contains the whole file name
      if (nCounterDigits == 0) return sFilePrefix;
      
      ostringstream oss; oss.fill('0'); oss.width(nCounterDigits);
      oss << sFilePrefix << nCounter << sFileSuffix; 
      nCounter++;
      return oss.str ();
   }
// }}}

   //--------------------------------------------------------------------------
   void FileWriter::write(ImgBase *poSrc) throw (FileOpenException, ICLException) {
      // {{{ open

      FileInfo oInfo (buildFileName()); // create file info
      openFile (oInfo, "wb"); // open file for writing
      
      ImgBase *poImg = poSrc;
      if (poSrc->getDepth () != depth8u && oInfo.eFileFormat != ioFormatICL) { //TODO_depth
         // image needs to be converted to depth8u
        poImg = poSrc->convertTo<icl8u> (&m_oImg8u);
      } // otherwise, use poSrc directly

      try {
         // write file
         switch (oInfo.eFileFormat) {
           case ioFormatPNM: 
           case ioFormatICL:
              writePNM (poImg, oInfo);
              break;
           case ioFormatJPG:
              writeJPG (poImg->asImg<icl8u>(), oInfo);
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
   void FileWriter::writePNM(ImgBase *poSrc, const FileInfo& oInfo) {
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
      sprintf (acBuf, "%s\n", bPPM ? "P6" : "P5");
      if (!pWrite (oInfo.fp, acBuf, strlen(acBuf))) throw writeError;

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
         Img8u *poImg8u = poSrc->asImg<icl8u>();
         const Size& size = poSrc->getSize();
         int iDim   = 3 * size.width;
         icl8u *pcBuf = new icl8u[iDim];
         icl8u *pc;
        
         for (int i=0;i<iNumImages;i++) {
            icl8u *pcR = poImg8u->getData (i*3);
            icl8u *pcG = poImg8u->getData (i*3+1);
            icl8u *pcB = poImg8u->getData (i*3+2);
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
   void FileWriter::writeJPG(Img<icl8u> *poSrc, const FileInfo& oInfo, int iQuality) {
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
            icl8u *pcBuf = poSrc->getData (0) + jpgCinfo.next_scanline*iLineStep;
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
            icl8u *pcR = poSrc->getData (i*3);
            icl8u *pcG = poSrc->getData (i*3+1);
            icl8u *pcB = poSrc->getData (i*3+2);
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
