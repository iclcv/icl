/*
  FileRead.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "FileRead.h"

namespace icl {

  //--------------------------------------------------------------------------
  FileRead::FileRead(string sFileName, string sDir, 
                     string sFilter, bool bBuffer) :
    m_bBufferImages(bBuffer) {
    // {{{ open

    FUNCTION_LOG("(string sFileName, string sDir, string sFilter)");
    
    //---- Initialise variables ----
    ifstream streamInputImage;
    struct dirent *oEntry;
    char cTmp[255];
    m_iImgCnt = 0;
    
    if (sDir.size() == 0) {
      sDir = ".";
    }
    //---- Read content ----
    DIR *oDir = opendir(sDir.c_str());
    cout << sDir.c_str() << endl;
            
    do
    {
      oEntry = readdir(oDir);
      cout << oEntry << endl;
      if (oEntry) {
        string sTmpName(oEntry->d_name);
                
        if (sTmpName.rfind(sFilter,sTmpName.size()) != string::npos) {
          if(sTmpName.find(sFileName,0) != string::npos) {
            m_vecFileName.push_back(sDir+"/"+sTmpName);
            LOOP_LOG ("File: " << m_vecFileName.back());
          }
        }
        
        if (sFilter == "seq") {
          streamInputImage.open(sFileName.c_str(),ios::in);
          if(!streamInputImage)
          {
            ERROR_LOG("Can't open sequence file: " << sFileName);
          }
          
          while(streamInputImage)
          {
            streamInputImage.getline(cTmp, 255);
            m_vecFileName.push_back(cTmp);
          }      
          streamInputImage.close();
        }
      }
    } while (oEntry);
    closedir(oDir);
    
    //---- Buffer images ----
    if (m_bBufferImages) {
      bufferImages();
    }
  }

// }}}

  //--------------------------------------------------------------------------
  FileRead::FileRead(string sObjPrefix, string sFileType, string sDir,
                     int iObjStart, int iObjEnd,
                     int iImageStart, int iImageEnd, bool bBuffer) :
    m_bBufferImages(bBuffer) {    
    // {{{ open 

    FUNCTION_LOG("(string, string, string, int, int ,int ,int, bool)");
    
    //---- Initialise variables ----
    m_iImgCnt = 0;
    
    if (sDir.size() == 0) {
      sDir = ".";
    }
    
    //---- Build filename ----
    for (int i=iObjStart;i<=iObjEnd;i++) {
      for (int j=iImageStart;j<=iImageEnd;j++) {
        m_vecFileName.push_back(sDir +"/"+ sObjPrefix + 
                                number2String(i) + 
                                "__" + number2String(j) + 
                                "." + sFileType);
        LOOP_LOG ("File: " << m_vecFileName.back());
      }
    }
    
    //---- Buffer images ----
    if (m_bBufferImages) {
      bufferImages();
    }
  }
  
// }}}
  
  //--------------------------------------------------------------------------
  ImgI* FileRead::grab(ImgI* poDst) {
    // {{{ open 

    FUNCTION_LOG("(ImgI*)");

    //---- Variable definition ----
    Converter oConv;
    ImgI *poInImg = 0;
    info oInfo;
    vector<string> sSubStr;
    
    //---- Grab image ----
    if (m_bBufferImages) {
        poInImg = &*m_iterImgBuffer;
        if (m_iterImgBuffer != m_vecImgBuffer.end()-1 ) {
          m_iterImgBuffer++;
        }
        else {
          m_iterImgBuffer = m_vecImgBuffer.begin();
        }
    }
    else {
      oInfo.sFileName = m_vecFileName[m_iImgCnt];
      
      if (m_iImgCnt == m_vecFileName.size()-1) {
        m_iImgCnt = 0;
      }
      else {
        m_iImgCnt++;
      }
      
      splitString(oInfo.sFileName, ".", sSubStr);
      oInfo.sFileType = sSubStr.back();
      checkFileType(oInfo);
      readHeader(oInfo);
      cout << oInfo.iNumChannels << endl;
      poInImg = imgNew(oInfo.eDepth,oInfo.oImgSize,oInfo.eFormat,
                       oInfo.iNumChannels,oInfo.oROI);

      switch (oInfo.eDepth)
      {
        case depth8u:
          readData(*(poInImg->asImg<icl8u>()), oInfo);
          break;
          
        case depth32f:
          readData(*(poInImg->asImg<icl32f>()), oInfo);
          break;

        default:
          readData(*(poInImg->asImg<icl8u>()), oInfo);
      }
    }

    //---- Convert to output format ----
    if (poDst) {
      oConv.convert(poDst, poInImg);
    }

    return poInImg;
  }

// }}}
  
  //--------------------------------------------------------------------------
  void FileRead::readHeader(info &oImgInfo) {
    // {{{ open
    FUNCTION_LOG("");
    
    switch (oImgInfo.eFileFormat) {
      case ioFormatPPM:
      case ioFormatPGM:
      {
        // {{{ Read PPM/ PGM header

        //---- Variable initialisation ----
        ifstream oImageStream;
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
        
        //---- Set default values ----
        oImgInfo.iNumChannels = 1;
        oImgInfo.iNumImages = 1;
        oImgInfo.eDepth = depth8u;
        
        //---- Open file ----
        oImageStream.open(oImgInfo.sFileName.c_str(),ios::in | ios::binary);
        if(!oImageStream)
        {
          throw ICLException ("Can't open file");
        }
        
        //---- Read the first line of the header ----
        getline(oImageStream, sTmpLine);
        
        // {{{ Read special header info

        do
        {
          getline(oImageStream, sTmpLine);
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
                  oImgInfo.iOriginalMin=atof((*(++vecSubStringIter)).c_str());
                  LOOP_LOG("Set OriginalMin to: " << oImgInfo.iOriginalMin);
                  break;
                  
                case original_max:
                  oImgInfo.iOriginalMax=atof((*(++vecSubStringIter)).c_str());
                  LOOP_LOG("Set OriginalMax to: " << oImgInfo.iOriginalMax);
                  break;
                  
                case NumFeatures:
                  iNumFeature = atoi((*(++vecSubStringIter)).c_str());
                  switch (oImgInfo.eFormat)
                  {
                    case formatRGB:
                      oImgInfo.iNumChannels = iNumFeature*3;
                      oImgInfo.iNumImages = iNumFeature;
                      break;
                      
                    case formatGray:
                    case formatMatrix: 
                      oImgInfo.iNumChannels = iNumFeature;
                      oImgInfo.iNumImages = iNumFeature;
                      break;
                      
                    default:
                      ERROR_LOG("Currently HLS, LAB, YUV not supported");
                  }
                  
                  LOOP_LOG("Set NumChannels to: " << oImgInfo.iNumChannels);
                  LOOP_LOG("Set NumImages to  : " << oImgInfo.iNumImages);
                  break;    
                  
                case ROI:
                  oImgInfo.oROI.x = atoi(vecSubString[1].c_str());
                  oImgInfo.oROI.y = atoi(vecSubString[2].c_str());
                  oImgInfo.oROI.width = atoi(vecSubString[3].c_str());
                  oImgInfo.oROI.height = atoi(vecSubString[4].c_str());
                  
                  LOOP_LOG("Set ROIOffset (x): "<< oImgInfo.oROI.x);
                  LOOP_LOG("Set ROIOffset (y): "<< oImgInfo.oROI.y);
                  LOOP_LOG("Set ROISize   (W): "<< oImgInfo.oROI.width);
                  LOOP_LOG("Set ROISize   (H): "<< oImgInfo.oROI.height);
                  
                  break;
                  
                case ImageDepth:
                  if (vecSubString[1].find("depth8u",0) != string::npos) {
                    oImgInfo.eDepth = depth8u;
                    LOOP_LOG("ImageDepth: depth8u");
                  }
                  else if (vecSubString[1].find("depth32f",0)!=string::npos) {
                    oImgInfo.eDepth = depth32f;
                    LOOP_LOG("ImageDepth: depth32f");
                  }
                  else {
                    ERROR_LOG("Unknown image depth (Trying default depth)");
                    oImgInfo.eDepth = depth8u;
                  }
                  break;
                  
                case Format:
                  oImgInfo.eFormat = translateFormat(vecSubString[1].c_str());
                  LOOP_LOG("Set format to: << translateFormat(oImgInfo.eFormat)");
                  
              } // switch
            } // if
          } // for 
        } while(sTmpLine[0] == '#'); 

        // }}}
        
        // {{{ Read image size
        
        splitString(sTmpLine," ",vecSubString);    
        oImgInfo.oImgSize.width = atoi(vecSubString[0].c_str());;
        oImgInfo.oImgSize.height = atoi(vecSubString[1].c_str()) /
          oImgInfo.iNumImages;
        LOOP_LOG("Image width set to : " << oImgInfo.oImgSize.width);
        LOOP_LOG("Image height set to: " << oImgInfo.oImgSize.height);
        
        getline(oImageStream,sTmpLine);
        splitString(sTmpLine," ",vecSubString);
        LOOP_LOG("Maximum pixel read");
        
        // }}}
        
        //---- Change format ? ----
        if (oImgInfo.iNumChannels > 1) {
          oImgInfo.eFormat = formatMatrix;
        }
        
        //---- Save stream position ----
        oImgInfo.streamPos = oImageStream.tellg();
        oImageStream.close();
      }
      // }}}
        break;
        
      case ioFormatJPG:
        
        break;

      default:
        ERROR_LOG("Unsupported image format");
    }
  }

  // }}}

  //--------------------------------------------------------------------------
  template <class Type>
  void FileRead::readData(Img<Type> &oImg, info &oImgInfo) {
    // {{{ open
    FUNCTION_LOG("(Img<icl8u>, string, info &)");
    
    switch (oImgInfo.eFileFormat)
    {
      case ioFormatPGM:
        SECTION_LOG("Start reading image:");
        readPGM(oImg, oImgInfo);
        break;
        
      case ioFormatPPM:
        SECTION_LOG("Start reading image:");
        readPPM(oImg, oImgInfo);    
        break;
        
      case ioFormatICL:
        SECTION_LOG("Start reading image:");
        break;
        
      default:
        ERROR_LOG("Could not read data");
    } 
  }


  // }}}

  //--------------------------------------------------------------------------
  template <class Type>
  void FileRead::readPGM(Img<Type> &oDst, info &oImgInfo) {     
    // {{{ open
    
    FUNCTION_LOG("(Img<Type>, info &)");
    
    //---- Variable definition ----
    ifstream oImageStream;
    int iDim = oImgInfo.oImgSize.getDim();
    
    //---- Open file ----
    oImageStream.open(oImgInfo.sFileName.c_str(),ios::in | ios::binary);

    if(!oImageStream)
    {
      throw ICLException ("Can't open file");
    }
    
    //---- Read data ----
    oImageStream.seekg(oImgInfo.streamPos);
    for (int i=0;i<oImgInfo.iNumImages;i++)
    {
      oImageStream.read((char*) oDst.getData(i), 
                        iDim * sizeof(Type));
    }

    oImageStream.close();
  }
  
    // }}}
  
  //--------------------------------------------------------------------------
  template <class Type>
  void FileRead::readPPM(Img<Type> &oDst, info &oImgInfo) {     
    // {{{ open

    FUNCTION_LOG("(Img<Type>, string, info &)");
    
    //---- Variable definition ----
    ifstream oImageStream;
    
    //---- Open file ----
    oImageStream.open(oImgInfo.sFileName.c_str(),ios::in | ios::binary);
    if(!oImageStream)
    {
      throw ICLException ("Can't open file");
    }
    
    //---- Read data ---
    oImageStream.seekg(oImgInfo.streamPos);
    Rect oTmpROI = oDst.getROI();
    oDst.setFullROI();
        
    typename Img<Type>::iterator itR;
    typename Img<Type>::iterator itG;
    typename Img<Type>::iterator itB;
    
    for (int i=0;i<oImgInfo.iNumImages;i++)
    {
      itR = oDst.getIterator(i*3);
      itG = oDst.getIterator(i*3+1);
      itB = oDst.getIterator(i*3+2);
      
      for(; itR.inRegion(); itR++,itG++,itB++)
      {
        *itR = oImageStream.get();
        *itG = oImageStream.get();
        *itB = oImageStream.get();
      }
    }  
    oDst.setROI(oTmpROI);
  }

  // }}}

  //--------------------------------------------------------------------------
  void FileRead::bufferImages() {     
    // {{{ open

    FUNCTION_LOG("()");
    
    //---- Variable definition ----
    vector<string> sSubStr;
    info oInfo;
        
    //---- Buffer images ----
    if (m_bBufferImages) {
      m_vecImgBuffer.resize(m_vecFileName.size());
      m_iterImgBuffer = m_vecImgBuffer.begin();
      
      for (unsigned int i=0;i<m_vecImgBuffer.size();i++) {
        //---- Read image header ----
        oInfo.sFileName = m_vecFileName[i];
        splitString(oInfo.sFileName, ".", sSubStr);
        oInfo.sFileType = sSubStr.back();
        checkFileType(oInfo);
        readHeader(oInfo);
        
        //---- Resize image ----
        m_vecImgBuffer[i].setChannels(oInfo.iNumChannels);
        m_vecImgBuffer[i].resize(oInfo.oImgSize);
        //---- Read image data ----
        readData(m_vecImgBuffer[i], oInfo);
      }
    }
  }

  // }}}

} // namespace icl
