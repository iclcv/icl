#include <iclCsvWriter.h>
#include <iclIO.h>
#include <zlib.h>
#include <sstream>
/*
  iclCsvWriter.cpp

  Written by: Michael Götting, Robert Haschke, Andre Justus (2007)
  University of Bielefeld
  AG Neuroinformatik
  mgoettin@techfak.uni-bielefeld.de
*/


using namespace std;

namespace icl {
  CsvWriter::CsvWriter(const std::string& sFileName) : 
    FileWriter(sFileName), 
    m_bCsvSplitFiles(false), 
    m_bCsvExtendFilename(false) {
  } 
  
 
  void CsvWriter::setCSVFlag(csvFlag f,bool value) {
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
  
  void CsvWriter::setFileNameCSV (string& sFilePrefix, const ImgBase *poSrc) { 
    // {{{ open 
      char result[100];
      sprintf(result,"-ICL:%dx%dx%d:%s:%s",
              poSrc->getSize().width,poSrc->getSize().height,
              poSrc->getChannels(), 
              translateDepth(poSrc->getDepth()).c_str(), 
              translateFormat(poSrc->getFormat()).c_str());
      sFilePrefix+=result;
   }

// }}}   
  
  
  void CsvWriter::write(const ImgBase *poSrc) {
    // {{{ open
    
    if (m_bCsvExtendFilename) setFileNameCSV (sFilePrefix, poSrc);
    
    string test = buildFileName();
    FileInfo oInfo (test); // create file info
    openFile (oInfo, "wb"); // open file for writing
    
    const ImgBase *poImg = poSrc;
    // may be image needs to be converted to depth8u
    if (poSrc->getDepth () != depth8u && 
        oInfo.eFileFormat != ioFormatICL &&
        oInfo.eFileFormat != ioFormatCSV) {
      poImg = poSrc->convert<icl8u>(&m_oImg8u);
    }
    poImg->print("inWrite");
    
    try {
      // write file
      switch (oInfo.eFileFormat) {
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
  
 
  template<class T,class R>
  string CsvWriter::writeCSVTmpl(const Img<T> *poSrc,int ch) {
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
     oss << endl;
     
     return (oss.str());
   }
// }}}
  
  template<class T,class R>
  void CsvWriter::writeCSVTmpl(const Img<T> *poSrc, FileInfo& oInfo) {
    // {{{ open
    int (*pWrite)(void *fp, const void *pData, unsigned int len) 
      = oInfo.bGzipped ? gzwrite : plainWrite;
    ICLException writeError ("Error writing file.");       
    int iNumImages = poSrc->getChannels();

    // write channel consecutively
    if (!m_bCsvSplitFiles) { 
      ostringstream oss;
      
      for (int i=0;i<iNumImages-1;i++) {
        oss << writeCSVTmpl<T,R>(poSrc,i) << "," << endl;
      }
      
      oss << writeCSVTmpl<T,R>(poSrc,iNumImages-1) << endl;
      if (!pWrite (oInfo.fp, (oss.str()).c_str(), 
                   strlen((oss.str()).c_str()))) { 
        throw writeError;
      }
    } else {
      for (int i=0;i<iNumImages-1;i++) {
        printf("Split %d\n",i);
        ostringstream oss;
        oss << writeCSVTmpl<T,R>(poSrc,i) << endl;
        if (!pWrite (oInfo.fp, (oss.str()).c_str(), 
                     strlen((oss.str()).c_str()))) 
          throw writeError;
        closeFile (oInfo);
        oInfo.sFileName = buildFileName(); 
        openFile (oInfo, "wb");
      }
      
      ostringstream oss;
      oss<<writeCSVTmpl<T,R>(poSrc,iNumImages-1)<<endl;
      if (!pWrite (oInfo.fp, (oss.str()).c_str(), strlen((oss.str()).c_str()))) 
        throw writeError;
    }
  }
// }}}
  
  void CsvWriter::writeCSV(const ImgBase *poSrc, FileInfo& oInfo) {
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
  
} //namespace 
