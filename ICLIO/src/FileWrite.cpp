/*
  FileWrite.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "FileWrite.h"
#include "Converter.h"
#include <ImgI.h>

using namespace std;

namespace icl {

  //--------------------------------------------------------------------------
  FileWrite::FileWrite(string sPrefix,string sDir, 
                       string sType, int iObjNum) {
    // {{{ open 

    FUNCTION_LOG("(string, string, string)");

    //---- Set write mode ----
    m_iWriteMode = 0;
    m_iCurrObj = iObjNum;
    m_oInfo.sFileName = sDir + "/" + sPrefix;
    m_oInfo.sFileType = sType;
  }

// }}}

  //--------------------------------------------------------------------------
  FileWrite::FileWrite(string sFileName) {
    // {{{ open 

    FUNCTION_LOG("(string, string, string)");
    
    //---- Set write mode ----
    vector<string> vecSubs;

    m_iWriteMode = 1;
    splitString(sFileName,".",vecSubs);

    int iTmpPos = sFileName.rfind(".",sFileName.size());
    
    //sFileName.copy(m_oInfo.sFileName, iTmpPos);

    //sFileName.copy(m_oInfo.sFileType.c_str(),
    //             3,
    //             sFileName.find_last_of(".",sFileName.size()));

    cout << m_oInfo.sFileName << endl;
    cout << m_oInfo.sFileType << endl;
  }

// }}}
  
  //--------------------------------------------------------------------------
  void FileWrite::write(ImgI *poSrc) {
    // {{{ open

    FUNCTION_LOG("(ImgI*)");
    
    //---- Initialise variables ----
    ofstream streamOutputImage;
    
    //----Build file name ----
    string sFileName = buildFileName();
    cout << sFileName << endl;
    
    //---- Open output stream ----
    SECTION_LOG("Save image: " << m_oInfo.sFileName<<"."<<m_oInfo.sFileType);
    streamOutputImage.open(sFileName.c_str(),ios::out | ios::binary);
    
    if(!streamOutputImage)
    {
      ERROR_LOG ("Can't write file: " << m_oInfo.sFileName);
    }
    
    //---- Determine file format ----
    checkFileType(m_oInfo);
    
    //---- Write data ----
    switch (m_oInfo.eFormat)
    {
      case formatGray:
        writeAsPGM(poSrc, streamOutputImage);
        break;
        
      case formatRGB:
        writeAsPPM(poSrc, streamOutputImage);
        break;
        
      case formatMatrix:
        writeAsMatrix(poSrc, streamOutputImage);
        break;
        
      default:
        ERROR_LOG("This file format is not supported by the ICL");
    }
    
    streamOutputImage.close();
  }

// }}}
  
  //--------------------------------------------------------------------------
  void FileWrite::writeAsPGM(ImgI *poSrc, ofstream &streamOutputImage) {
    // {{{ open

    FUNCTION_LOG("");
    
    //---- Initialise variables ----
    ImgI* poTmpImg;
    int iDim = poSrc->getDim();
    int iNumImages = poSrc->getChannels();       
    
    //---- Convert to Gray format ----
    if (poSrc->getFormat() != formatGray)
    { 
      poTmpImg = imgNew(depth8u, poSrc->getSize(),
                        formatGray, poSrc->getChannels());
      m_oConverter.convert(poTmpImg, poSrc);
    } 
    else
    {
      poTmpImg = poSrc;
    }
    
    //---- Write header ----
    streamOutputImage << "P5" << endl;    
    streamOutputImage << "# Format " << translateFormat(m_oInfo.eFormat) << 
      endl;
    streamOutputImage << "# NumFeatures " << iNumImages << endl;
    streamOutputImage << "# ImageDepth depth8u" << endl;
    
    streamOutputImage << poTmpImg->getSize().width << " " 
                        << poTmpImg->getSize().height * iNumImages << endl;
    //streamOutputImage << poTmpImg->asImg<icl8u>()->getMax() << endl;
    streamOutputImage << "255" << endl;
    
    //---- Write data ----
    for (int i=0;i<iNumImages;i++)
    {
      streamOutputImage.write ((char*) poSrc->getDataPtr(i),
                               iDim*getSizeOf(poSrc->getDepth()));
    }
  }

// }}}

  //--------------------------------------------------------------------------
  void FileWrite::writeAsPPM(ImgI *poSrc, ofstream &streamOutputImage) {
    // {{{ open
    FUNCTION_LOG("");
    
    //---- Initialise variables ----
    ImgI* poTmpImg;
    int iNumImages =  poSrc->getChannels()/3;
    
    //---- Convert to Gray format ----
    if (poSrc->getFormat() != formatRGB)
    { 
      poTmpImg = imgNew(depth8u, poSrc->getSize(),
                        formatRGB, poSrc->getChannels());
      m_oConverter.convert(poTmpImg, poSrc);
    } 
    else
    {
      poTmpImg = poSrc;
    }
    
    //---- Write file header ----
    streamOutputImage << "P6" << endl;    
    streamOutputImage << "# Format " << translateFormat(m_oInfo.eFormat) << 
      endl;
    streamOutputImage << "# NumFeatures " << iNumImages << endl;
    streamOutputImage << "# ImageDepth depth8u" << endl;
        
    streamOutputImage << poTmpImg->getSize().width << " " 
                        << poTmpImg->getSize().height * iNumImages << endl;;
    //streamOutputImage << poTmpImg->asImg<icl8u>()->getMax() << endl;
    streamOutputImage << "255" << endl;
    
    //---- Write data ----
    poTmpImg->setFullROI();
    
    for (int i=0;i<iNumImages;i++)
    {
      ImgIterator<icl8u> itR=poTmpImg->asImg<icl8u>()->getIterator(i*3);
      ImgIterator<icl8u> itG=poTmpImg->asImg<icl8u>()->getIterator(i*3+1);
      ImgIterator<icl8u> itB=poTmpImg->asImg<icl8u>()->getIterator(i*3+2);
      
      for(; itR.inRegion(); itR++,itG++,itB++)
      {
        streamOutputImage.put(*itR);
        streamOutputImage.put(*itG);
        streamOutputImage.put(*itB);
      }
    }
  }
  // }}}

  //--------------------------------------------------------------------------
  void FileWrite::writeAsMatrix(ImgI *poSrc, ofstream &streamOutputImage) {
    // {{{ open
    FUNCTION_LOG("");
    
    //---- Initialise variables ----
    int iNumImages = poSrc->getChannels();       
    int iDim = poSrc->getDim();
    
    //---- Write header ----
    streamOutputImage << "P5" << endl;    
    streamOutputImage << "# Format " << translateFormat(m_oInfo.eFormat) << 
      endl;
    streamOutputImage << "# NumFeatures " << iNumImages << endl;
    
    switch(m_oInfo.eDepth)
    {
      case depth8u:
        streamOutputImage << "# ImageDepth depth8u" << endl;
        break;
      case depth32f:
        streamOutputImage << "# ImageDepth depth32f" << endl;
        break;
    }

    streamOutputImage << poSrc->getSize().width << " " 
                        << poSrc->getSize().height * iNumImages << endl;
    //streamOutputImage << poSrc->asImg<icl8u>()->getMax() << endl;
    streamOutputImage << "255" << endl;
    
    //---- Write data ----
    for (int i=0;i<iNumImages;i++)
    {
      streamOutputImage.write ((char*) poSrc->getDataPtr(i),
                               iDim*getSizeOf(poSrc->getDepth()));
    } 
  }

// }}}

  //--------------------------------------------------------------------------
  string FileWrite::buildFileName() {
    // {{{ open

    FUNCTION_LOG("");
    
    //---- Variable initialisation----
    string sFileName;
    
    //---- Build file name ----
    switch(m_iWriteMode)
    {
      case 0:
        sFileName = m_oInfo.sFileName + number2String(m_iCurrObj) + 
          "." + m_oInfo.sFileType;
        m_iCurrObj++;
        break;

      case 1:
        sFileName = m_oInfo.sFileName + "." + m_oInfo.sFileType;
        break;
        
      default:
        ERROR_LOG("Unsupported reader mode");
    }
    
    return sFileName;
  }
// }}}

} //namespace 
