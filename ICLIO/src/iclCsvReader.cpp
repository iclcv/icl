#include <stdlib.h>
#ifndef WIN32
#include <wordexp.h>
#endif
#include <zlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iclCsvReader.h>
#include <iclConverter.h>
#include <iclStrTok.h>
/*
  iclCsvReader.cpp

  Written by: Michael Götting, Robert Haschke, Andre Justus (2007)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

using namespace std;

namespace icl {

  inline void replace_newline (string::value_type& c) {
     if (c == '\n') c = ' ';
  }

  CsvReader::CsvReader(string sPattern): 
    FileReader(sPattern) {
  }


  //--------------------------------------------------------------------------
  CsvReader::CsvReader(const string& sPrefix, const string& sType, 
                         int iObjStart, int iObjEnd, 
                         int iImageStart, int iImageEnd):
    FileReader(sPrefix,sType,iObjStart,iObjEnd,iImageStart,iImageEnd) {
  }

  
  //--------------------------------------------------------------------------
  CsvReader::CsvReader(const FileReader& other) : FileReader(other) {
  }
  
 
  //--------------------------------------------------------------------------

  void CsvReader::readImage(const string& sFileName, ImgBase** ppoDst) {
    // {{{ open 

    FUNCTION_LOG("");

    //---- Variable definition ----
    FileInfo oInfo (sFileName);

    // set some defaults
    oInfo.oROI = Rect(); // full ROI

    try {
       switch (oInfo.eFileFormat) {
         case ioFormatCSV:
            readHeaderCSV (oInfo);
            ensureCompatible (ppoDst, oInfo.eDepth, oInfo.oImgSize, 
                              oInfo.iNumChannels, oInfo.eFormat, oInfo.oROI);
            (*ppoDst)->setTime(oInfo.timeStamp);
            readDataCSV (*ppoDst, oInfo);
            break;           

         default:
            throw ICLException (string("not supported file type"));
       }
    } catch (ICLException &e) {
       if (oInfo.fp) closeFile (oInfo);
       throw;
    }
    closeFile (oInfo);
  }

  // }}}
  

  //--------------------------------------------------------------------------
  void CsvReader::setCSVHeader (depth d, const ImgParams& p) {
    // {{{ open

     m_CSVDepth = d;
     m_CSVParams = p;
  }

  // }}}
  
  void CsvReader::readHeaderCSV (FileInfo &oInfo) {
    // {{{ open

    // use m_CSV parameters as default settings
    oInfo.eDepth = m_CSVDepth;
    oInfo.oImgSize = m_CSVParams.getSize();
    oInfo.oROI=m_CSVParams.getROI();
    oInfo.iNumChannels = m_CSVParams.getChannels();
    oInfo.eFormat = m_CSVParams.getFormat();
    //oInfo.timeStamp = 0;

    // overwrite these settings by file name contents
    int pos=oInfo.sFileName.find("-ICL:",0)+5;
    StrTok tok(oInfo.sFileName.substr(pos),":");
    if(tok.hasMoreTokens()){
      StrTok tok2(tok.nextToken(),"x");// WxHxC
      if(tok2.hasMoreTokens()){
        oInfo.oImgSize.width=atoi((tok2.nextToken()).c_str()); // W
      }
      if(tok2.hasMoreTokens()){
        oInfo.oImgSize.height=atoi((tok2.nextToken()).c_str()); // H
      }
      if(tok2.hasMoreTokens()){
        oInfo.iNumChannels=atoi((tok2.nextToken()).c_str()); //C
      }      
    }
    if(tok.hasMoreTokens()){
      oInfo.eDepth=translateDepth(tok.nextToken()); //D
    }
    if(tok.hasMoreTokens()){
      StrTok tok2(tok.nextToken(),".");
      if(tok2.hasMoreTokens()){
        oInfo.eFormat=translateFormat(tok2.nextToken()); //F
      }
    }
  }

// }}}

  template<class T>
  void CsvReader::readCSVTmpl(Img<T>* poImg, FileInfo &oInfo) {
    // {{{ open

    FUNCTION_LOG("");

    //Img16s *poImg16s = poImg->asImg<icl16s>();
    T *pc2Buf[3];
    pc2Buf[0] = poImg->getData (0);
    pc2Buf[1] = poImg->getData (1);
    pc2Buf[2] = poImg->getData (2);

    openFile (oInfo, "rb"); // open file for reading
    char *pcBuf=0;
    int fsize=20*oInfo.oImgSize.width; //20 = max char length of double (should be 14) +1 (komma) + 5 bonus    
    pcBuf=(char*)malloc(fsize*sizeof(char));
    for (int c=0;c<oInfo.iNumChannels;c++) {
      for (int i=0;i<oInfo.oImgSize.height;i++) {
        char *result = NULL;
        fgets (pcBuf, fsize,(FILE*)oInfo.fp);
        result = strtok( pcBuf, ",");
        for (int j=0;j<oInfo.oImgSize.width;j++) {
            *pc2Buf[c]=atoi(result);pc2Buf[c]++;
            result = strtok( NULL, ",");
        }
      }
    }
    free (pcBuf);
  }

  // }}}

  void CsvReader::readDataCSV(ImgBase* poImg, FileInfo &oInfo) {
    // {{{ open

    FUNCTION_LOG("");

    switch(poImg->getDepth()) {
      case depth8u: readCSVTmpl<icl8u>(poImg->asImg<icl8u>(),oInfo);
        break;
      case depth16s: readCSVTmpl<icl16s>(poImg->asImg<icl16s>(),oInfo);
        break;
      case depth32s: readCSVTmpl<icl32s>(poImg->asImg<icl32s>(),oInfo);
        break;
      case depth32f: readCSVTmpl<icl32f>(poImg->asImg<icl32f>(),oInfo);
        break;
      case depth64f: readCSVTmpl<icl64f>(poImg->asImg<icl64f>(),oInfo);
        break;
      default: ICL_INVALID_DEPTH; break;
    }
  }

  // }}}

  
} // namespace icl
