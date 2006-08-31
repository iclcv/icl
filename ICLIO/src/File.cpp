/*
  File.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "File.h"

namespace icl{
  
  // {{{ Konstructor/ Destructor

  //--------------------------------------------------------------------------
  File::File(string sObjPrefix, string sFileType,
             int iObjStart, int iObjEnd,
             int iImageStart, int iImageEnd) :
    m_sObjPrefix(sObjPrefix),
    m_sFileType(sFileType),
    m_iObjStart(iObjStart),
    m_iObjEnd(iObjEnd),
    m_iImageStart(iImageStart),
    m_iImageEnd(iImageEnd)
  {
    // {{{ open
    FUNCTION_LOG("(string, string, int, int ,int ,int)");
    
    //---- Initialise variables ----
    m_eReaderMode = objectImageMode;
    m_iCurrObj = iObjStart;
    m_iCurrImageNum = iImageStart;
  }

  // }}}
  
  //--------------------------------------------------------------------------
  File::File(string sFileName)
  {
    // {{{ open

    FUNCTION_LOG("(string)");
    
    //---- Initialise variables ----
    ifstream streamInputImage;
    char cTmp[255];
    vector<string> vecSubStrings;
    
    m_iCurrObj = 0;
    m_iCurrImageNum = 0;
    m_sSingleFileName = sFileName;
    
    //---- Determine reader mode ----
    splitString(sFileName,".",vecSubStrings);
    
    if(vecSubStrings.back() == "seq")
    {
      SECTION_LOG("Read sequence file");
      m_eReaderMode = sequenceMode;
      
      //---- Read sequence file ----
      streamInputImage.open(sFileName.c_str(),ios::in);
      if(!streamInputImage)
      {
        ERROR_LOG("Can't open sequence file: " << sFileName);
      }
      
      while(streamInputImage)
      {
        streamInputImage.getline(cTmp, 255);
        m_vecSeqContent.push_back(cTmp);
      }
      
      streamInputImage.close();   
      m_iObjEnd = m_vecSeqContent.size()-1;
    }
    else
    {
      m_eReaderMode = singleMode;
      m_sSingleFileName = sFileName;
    }
  }
  
  // }}}

  //--------------------------------------------------------------------------
  File::~File()
  {
    // {{{ open
    FUNCTION_LOG("");
  }

// }}}
  
  // }}}
  
  // {{{ Grabbing

  //--------------------------------------------------------------------------
  ImgI* File::grab(ImgI* poDst) {     
    // {{{ open

    FUNCTION_LOG("(ImgI*)");

    //---- Variable definition ----
    ifstream streamInputImage;
    string sFileName;
    Converter oConv;
    ImgI *poInImg = 0;
    clear();
    
    //---- Build file name ----
    sFileName = buildFileName();
    
    //---- Determine file format ----
    checkFileType(sFileName, m_eFileFormat, m_eFormat);
    
    //---- Open input stream ----
    SECTION_LOG("Load image: " << sFileName);
    streamInputImage.open(sFileName.c_str(),ios::in | ios::binary);
    
    if(!streamInputImage)
    {
      //ERROR_LOG("Can't open file: " << sFileName);
      throw ICLException ("Can't open file");
    }
    
    //---- Analyse file header ----
    switch (m_eFileFormat)
    {
      case ioFormatPGM:
      case ioFormatPPM:
        //---- Read image information ----
        parseImageHeader(streamInputImage);
        break;
        
      default:
        break;
    }
            
    //---- Read image data ----
    SECTION_LOG("Start reading data");
    streamInputImage.seekg(m_streamImageStartPos);

    if(m_eFileFormat == ioFormatPGM)
    {
      if (m_iNumChannels == 1) { 
        m_eFormat = formatGray;
        poInImg = imgNew(m_eDepth,m_oImgSize,m_eFormat,m_iNumChannels,m_oROI);
        readFromPGM(poInImg, streamInputImage);
      }
      else if(m_iNumChannels != 1)
      {        
        m_eFormat = formatMatrix;
        poInImg = imgNew(m_eDepth,m_oImgSize,m_eFormat,m_iNumChannels,m_oROI);
        readFromPGM(poInImg, streamInputImage);
      }
    }

    if (m_eFileFormat == ioFormatPPM)
    { 
      if (m_iNumChannels == 3) {
        m_eFormat = formatRGB;
        poInImg = imgNew(m_eDepth,m_oImgSize,m_eFormat,m_iNumChannels,m_oROI);
        readFromPPM(poInImg, streamInputImage);
      }
      else if(m_iNumChannels != 3)
      {
        m_eFormat = formatMatrix;
        poInImg = imgNew(m_eDepth,m_oImgSize,m_eFormat,m_iNumChannels,m_oROI);
        readFromPPM(poInImg, streamInputImage);
      }
    }
    
    if (m_eFileFormat == ioFormatICL)
    {
      m_eFormat = formatMatrix;
      poInImg = imgNew(m_eDepth,m_oImgSize,m_eFormat,m_iNumChannels,m_oROI);
      //readFromPPM(poInImg, streamInputImage);
    }
    
    streamInputImage.close();
        
    //---- Convert to output format ----
    if (poDst) {
      oConv.convert(poDst, poInImg);
    }
    
    return poInImg;
  }

  // }}}

  //--------------------------------------------------------------------------
  void File::readFromPGM(ImgI* poDst, ifstream &streamInputImage) {     
    // {{{ open

    FUNCTION_LOG("(ICLBase*)");
    
    //---- Variable definition ----
    int iDim = m_oImgSize.getDim();
    
    //---- Read data ----
    for (int i=0;i<m_iNumImages;i++)
    {
      streamInputImage.read((char*) poDst->getDataPtr(i), 
                            iDim*getSizeOf(poDst->getDepth()));
    }
  }

    // }}}

  //--------------------------------------------------------------------------
  void File::readFromPPM(ImgI* poDst, ifstream &streamInputImage) {     
    // {{{ open

    FUNCTION_LOG("(ICLBase*)");
    
    //---- Read data ---
    Rect oTmpROI = poDst->getROI();
    poDst->setFullROI();
        
    Img8u::iterator itR;
    Img8u::iterator itG;
    Img8u::iterator itB;
    
    for (int i=0;i<m_iNumImages;i++)
    {
      itR = poDst->asImg<icl8u>()->getIterator(i*3);
      itG = poDst->asImg<icl8u>()->getIterator(i*3+1);
      itB = poDst->asImg<icl8u>()->getIterator(i*3+2);
      
      for(; itR.inRegion(); itR++,itG++,itB++)
      {
        *itR = streamInputImage.get();
        *itG = streamInputImage.get();
        *itB = streamInputImage.get();
      }
    }  
    poDst->setROI(oTmpROI);
  }

  // }}}

  // }}}

  // {{{ Writing

  //--------------------------------------------------------------------------
  void File::write(ImgI *poSrc) {
    // {{{ open

    FUNCTION_LOG("(ICLBase*)");
    
    //---- Initialise variables ----
    ofstream streamOutputImage;
      
    //----Build file name ----
    string sFileName = buildFileName();
    
    //---- Open output stream ----
    SECTION_LOG("Save image: " << sFileName);
    streamOutputImage.open(sFileName.c_str(),ios::out | ios::binary);
    
    if(!streamOutputImage)
    {
      ERROR_LOG ("Can't write file: " << sFileName);
    }
    
    //---- Determine file format ----
    checkFileType(sFileName, m_eFileFormat, m_eFormat);
    
    //---- Write data ----
    switch (m_eFormat)
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
  void File::writeAsPGM(ImgI *poSrc, ofstream &streamOutputImage) {
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
    streamOutputImage << "# Format " << translateFormat(m_eFormat) << endl;    
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
  void File::writeAsPPM(ImgI *poSrc, ofstream &streamOutputImage) {
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
    streamOutputImage << "# Format " << translateFormat(m_eFormat) << endl;
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
  void File::writeAsMatrix(ImgI *poSrc, ofstream &streamOutputImage) {
    // {{{ open
    FUNCTION_LOG("");
    
    //---- Initialise variables ----
    int iNumImages = poSrc->getChannels();       
    int iDim = poSrc->getDim();
    
    //---- Write header ----
    streamOutputImage << "P5" << endl;    
    streamOutputImage << "# Format " << translateFormat(m_eFormat) << endl;    
    streamOutputImage << "# NumFeatures " << iNumImages << endl;
    
    switch(m_eDepth)
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

  // }}}
  
  // {{{ Misc. functions

  //--------------------------------------------------------------------------
  void File::clear() {
    // {{{ open

    FUNCTION_LOG("");
    
    m_iOriginalMin = 0;
    m_iOriginalMax = 0;
    m_iNumChannels = 0;
    m_iNumImages = 0; 
    m_oROI = Rect();
  }

// }}}
  
  //--------------------------------------------------------------------------
  string File::buildFileName() {
    // {{{ open

    FUNCTION_LOG("");
    
    //---- Variable initialisation----
    string sFileName;
    
    //---- Build file name ----
    switch(m_eReaderMode)
    {
      case objectImageMode:
        if (m_iCurrImageNum > m_iImageEnd)
        {
          //---- Take next object ----
          m_iCurrObj++;
          
          if (m_iCurrObj > m_iObjEnd)
          {
            m_iCurrObj = m_iObjStart;
          }
          
          m_iCurrImageNum = m_iImageStart;
        }
        
        sFileName = m_sObjPrefix + number2String(m_iCurrObj) + 
          "__" + number2String(m_iCurrImageNum) + "." + m_sFileType;
        
        m_iCurrImageNum++;
        break;
        
      case objectMode:
        
        break;
        
      case singleMode:
        sFileName = m_sSingleFileName; 
        break;
        
      case sequenceMode:
        sFileName = m_vecSeqContent[m_iCurrObj];
        if(m_iCurrObj < m_iObjEnd-1)
        {
          m_iCurrObj++;
        }
        else
        {
          m_iCurrObj = 0;
        }
        break;
        
      default:
        ERROR_LOG("Unsupported reader mode");
    }
    
    return sFileName;
  }

// }}}

  //--------------------------------------------------------------------------
  void File::parseImageHeader(ifstream &streamInputImage) {
    // {{{ open
    FUNCTION_LOG("");
    
    //---- Variable initialisation ----
    vector<string> vecSubString;
    vector<string>::iterator vecSubStringIter;
    vector<string> headerParameter;
    string sTmpLine;
    int iNumFeature = 0;
    
    enum headerInfo{
      original_min,
      original_max,
      NumFeatures,
      ROI,
      ImageDepth,
      Format
    };
    
    headerParameter.push_back("original_min");
    headerParameter.push_back("original_max");
    headerParameter.push_back("NumFeatures");
    headerParameter.push_back("ROI");
    headerParameter.push_back("ImageDepth");
    headerParameter.push_back("Format");
    m_iNumChannels = 1;
    m_eDepth = depth8u;
    
    //---- Read the first line of the header ----
    getline(streamInputImage, sTmpLine);
    
    // {{{ Read special header info

    getline(streamInputImage, sTmpLine);
    
    do
    {
      //---- Analyse comment line in header ----
      SECTION_LOG("Check and analyse comment");
      splitString(sTmpLine," ",vecSubString);
      
      for (unsigned int i=0;i<headerParameter.size();i++)
      {
        vecSubStringIter = find(vecSubString.begin(), 
                                vecSubString.end(),
                                headerParameter[i]);
      
        if (vecSubStringIter != vecSubString.end())
        {
          switch(i)
          {
            case original_min:
              m_iOriginalMin = atof((*(++vecSubStringIter)).c_str());
              LOOP_LOG("Set OriginalMin to: " << m_iOriginalMin);
              break;
              
            case original_max:
              m_iOriginalMax = atof((*(++vecSubStringIter)).c_str());
              LOOP_LOG("Set OriginalMax to: " << m_iOriginalMax);
              break;
              
            case NumFeatures:
              iNumFeature = atoi((*(++vecSubStringIter)).c_str());
              switch (m_eFormat)
              {
                case formatRGB:
                  m_iNumChannels = iNumFeature*3;
                  m_iNumImages = iNumFeature;
                  break;
                  
                case formatGray:
                case formatMatrix: 
                  m_iNumChannels = m_iNumImages = iNumFeature;
                  break;
                
                default:
                  ERROR_LOG("Currently HLS, LAB, YUV not supported");
              }
              
              LOOP_LOG("Set NumChannels to: " << m_iNumChannels);
              LOOP_LOG("Set NumImages to  : " << m_iNumImages);
            break;    
            
            case ROI:
              m_oROI.x = atoi(vecSubString[1].c_str());
              m_oROI.y = atoi(vecSubString[2].c_str());
              m_oROI.width = atoi(vecSubString[3].c_str());
              m_oROI.height = atoi(vecSubString[4].c_str());
              
              LOOP_LOG("Set ROIOffset (x): "<< m_oROI.x);
              LOOP_LOG("Set ROIOffset (y): "<< m_oROI.y);
              LOOP_LOG("Set ROISize   (W): "<< m_oROI.width);
              LOOP_LOG("Set ROISize   (H): "<< m_oROI.height);
              
              break;
              
            case ImageDepth:
              if (vecSubString[1].find("depth8u",0) != string::npos) {
                m_eDepth = depth8u;
                LOOP_LOG("ImageDepth: depth8u");
              }
              else if (vecSubString[1].find("depth32f",0) != string::npos) {
                m_eDepth = depth32f;
                LOOP_LOG("ImageDepth: depth32f");
              }
              else {
                ERROR_LOG("Unknown image depth (Trying default depth)");
                m_eDepth = depth8u;
              }
              break;
              
            case Format:
              m_eFormat = translateFormat(vecSubString[1].c_str());
              LOOP_LOG("Set format to: << translateFormat(m_eFormat)");
                            
          } // switch
        } // if
      } // for 
      
      getline(streamInputImage,sTmpLine);
    } while(sTmpLine[0] == '#'); 

    // }}}

    // {{{ Read image size
    
    splitString(sTmpLine," ",vecSubString);    
    m_oImgSize.width = atoi(vecSubString[0].c_str());;
    m_oImgSize.height = atoi(vecSubString[1].c_str()) / m_iNumImages;
    LOOP_LOG("Image width set to : " << m_oImgSize.width);
    LOOP_LOG("Image height set to: " << m_oImgSize.height);
    
    getline(streamInputImage,sTmpLine);
    splitString(sTmpLine," ",vecSubString);
    LOOP_LOG("Maximum pixel read");

    // }}}
        
    //---- Get the current position in stream ----
    m_streamImageStartPos = streamInputImage.tellg();
  }

  // }}}

// }}}

} //namespace
