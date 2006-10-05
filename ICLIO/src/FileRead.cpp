/*
  FileRead.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <stdlib.h>
#include <wordexp.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>

#include <FileRead.h>
#include <Converter.h>

using namespace std;

namespace icl {

  inline void replace_newline (string::value_type& c) {
     if (c == '\n') c = ' ';
  }

  //--------------------------------------------------------------------------
  FileRead::FileRead(string sPattern, bool bBuffer)
    // {{{ open
    : m_bBufferImages(bBuffer), 
      m_iImgCnt (0)
  {
    FUNCTION_LOG("(string, bool)");
    
    wordexp_t match;
    char **ppcFiles;
    char *pcType;

    // remove newlines from sPattern
    std::for_each (sPattern.begin(), sPattern.end(), replace_newline);

    // search for file matching the pattern(s)
    switch (wordexp (sPattern.c_str(), &match, WRDE_UNDEF)) {
       case 0: break;
       case WRDE_BADCHAR: 
          throw ICLException ("illegal chars in pattern (|, &, ;, <, >, (, ), {, }");
          break;
       case WRDE_BADVAL:
          throw ICLException ("encountered undefined shell variable");
          break;
       case WRDE_NOSPACE:
          throw ICLException ("out of memory");
          break;
       case WRDE_SYNTAX:
          throw ICLException ("syntax error, e.g. unbalanced parentheses or quotes");
          break;
    }

    ppcFiles = match.we_wordv;
    for (unsigned int i=0; i < match.we_wordc; ++i) {
       LOOP_LOG(ppcFiles[i]);
       // retrieve file type
       if ( !(pcType = strrchr (ppcFiles[i], '.')) ) continue;
       if (strcmp (++pcType, "seq") == 0) readSequenceFile (ppcFiles[i]);
       else m_vecFileName.push_back(ppcFiles[i]);
    }
    wordfree(&match);
    if (!m_vecFileName.size ()) throw ICLException ("empty file list");

    //---- buffer images ----
    if (m_bBufferImages) bufferImages();
  }
// }}}

  //--------------------------------------------------------------------------
  FileRead::FileRead(const string& sFilePrefix, string sDir, 
                     const string& sFilter, bool bBuffer)
    // {{{ open
    : m_bBufferImages(bBuffer), 
      m_iImgCnt (0)
  {
    FUNCTION_LOG("(string sFileName, string sDir, string sFilter)");
    
    //---- Initialise variables ----
    struct dirent *oEntry;
    
    if (sDir.size() == 0) {
      sDir = ".";
    }

    //---- Read directory content ----
    DIR *oDir = opendir(sDir.c_str());
    if(oDir) {
      do
      {
        oEntry = readdir(oDir);
        if (oEntry) {
          string sTmpName(oEntry->d_name);
          string sFileName;
          
          if (sTmpName.rfind(sFilter,sTmpName.size()) != string::npos &&
              sTmpName.find(sFilePrefix,0) != string::npos) {
            sFileName = sDir + "/" + sTmpName;
            LOOP_LOG ("File: " << sFileName);
          }
          
          //---- add file(s) ----
          if (sFilter == "seq") readSequenceFile (sFileName);
          else m_vecFileName.push_back(sFileName);
        }
      } while (oEntry);
      closedir(oDir);
      
      if (!m_vecFileName.size()) throw ICLException ("empty file list");
      
      //---- Buffer images ----
      if (m_bBufferImages) {
        bufferImages();
      }
    }
    else {
      throw ICLException("Can't find directory");
    }
  }
    
// }}}

  //--------------------------------------------------------------------------
  FileRead::FileRead(const string& sObjPrefix, const string& sFileType, 
                     string sDir, int iObjStart, int iObjEnd,
                     int iImageStart, int iImageEnd, bool bBuffer)
    // {{{ open 
    : m_bBufferImages(bBuffer), 
    m_iImgCnt (0)
  {
    FUNCTION_LOG("(string, string, string, int, int ,int ,int, bool)");
   
    //---- Initialise variables ----
    ifstream streamInputImage;
    
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
        
        //---- Check for existing file ----
        streamInputImage.open(m_vecFileName.back().c_str(),ios::in);
        if(!streamInputImage)
        {
          throw ICLException ("Image file not found!");
        }
        streamInputImage.close();
      }
    }
    if (!m_vecFileName.size ()) throw ICLException ("Empty file list");
    
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
    info oInfo;
    vector<string> sSubStr;
    
    //---- Grab image ----
    if (m_bBufferImages) {
        m_poInImg = &(*m_iterImgBuffer);
        if (++m_iterImgBuffer == m_vecImgBuffer.end()) {
          m_iterImgBuffer = m_vecImgBuffer.begin();
        }
    }
    else {
      oInfo.sFileName = m_vecFileName[m_iImgCnt];
      SECTION_LOG("Grab image:" << oInfo.sFileName);
            
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

      ensureCompatible(&m_poInImg, oInfo.eDepth, oInfo.oImgSize, 
                       oInfo.eFormat, oInfo.iNumChannels, oInfo.oROI);
                
      switch (oInfo.eDepth)
      {
        case depth8u:
          readData(*(m_poInImg->asImg<icl8u>()), oInfo);
          break;
          
        case depth32f:
          readData(*(m_poInImg->asImg<icl32f>()), oInfo);
          break;

        default:
          readData(*(m_poInImg->asImg<icl8u>()), oInfo);
      }
    }

    //---- Convert to output format ----
    if (poDst) {
      oConv.convert(poDst, m_poInImg);
      return poDst;
    }
    else {
      return m_poInImg;
    }
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
        string sTmpLine, sMagicNumber;
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
        
         //---- Open file ----
        oImageStream.open(oImgInfo.sFileName.c_str(),ios::in | ios::binary);
        if(!oImageStream)
        {
          throw ICLException ("Can't open file");
        }

        //---- Read the magic number  ----
        getline(oImageStream, sMagicNumber);
        cout << sMagicNumber << endl;

        //---- Set image format ----
        if (sMagicNumber.find("P6",0) != string::npos) {
          SECTION_LOG("Set format RGB");
          oImgInfo.eFormat = formatRGB;
        }
        else if (sMagicNumber.find("P5",0) != string::npos) {
          SECTION_LOG("Set format GRAY");
          oImgInfo.eFormat = formatGray;
        }
        else {
          SECTION_LOG("Set format MATRIX");
          oImgInfo.eFormat = formatMatrix;
        }

        //---- Set default values ----
        oImgInfo.iNumChannels = getChannelsOfFormat(oImgInfo.eFormat);
        oImgInfo.iNumImages = 1;
        oImgInfo.eDepth = depth8u;
        
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
                      
                      //---- Is num channels in depence to the format ----
                      if (getChannelsOfFormat(oImgInfo.eFormat) != 
                          oImgInfo.iNumChannels) {
                        oImgInfo.eFormat = formatMatrix;
                      }
                      
                      break;
                      
                    case formatGray:
                    case formatMatrix: 
                      oImgInfo.iNumChannels = iNumFeature;
                      oImgInfo.iNumImages = iNumFeature;
                      
                      //---- Is num channels in depence to the format ----
                      if (getChannelsOfFormat(oImgInfo.eFormat) != 
                          oImgInfo.iNumChannels) {
                        oImgInfo.eFormat = formatMatrix;
                      }
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
    
    ifstream oImageStream;
      
    switch (oImgInfo.eFileFormat)
    {
      case ioFormatPGM:
      {
        SECTION_LOG("Start reading image:");
        // {{{ Read data

        //---- Open file ----
        oImageStream.open(oImgInfo.sFileName.c_str(),
                                   ios::in | ios::binary);
        if(!oImageStream)
        {
          throw ICLException ("Can't open file");
        }
        
        //---- Read data ----
        oImageStream.seekg(oImgInfo.streamPos);
        int iDim = oImgInfo.oImgSize.getDim();
        
        for (int i=0;i<oImgInfo.iNumImages;i++)
        {
          oImageStream.read((char*) oImg.getData(i), 
                            iDim * sizeof(Type));
        }
        
        oImageStream.close();

        // }}}
        break;
      }
      
      case ioFormatPPM:
      {
        SECTION_LOG("Start reading image:");
        // {{{ Read data

        //---- Open file ----
        oImageStream.open(oImgInfo.sFileName.c_str(),
                                   ios::in | ios::binary);
        if(!oImageStream)
        {
          throw ICLException ("Can't open file");
        }
        
        //---- Read data ---
        oImageStream.seekg(oImgInfo.streamPos);
        
        typename Img<Type>::iterator itR;
        typename Img<Type>::iterator itG;
        typename Img<Type>::iterator itB;
        
        for (int i=0;i<oImgInfo.iNumImages;i++)
        {
          itR = oImg.getIterator(i*3);
          itG = oImg.getIterator(i*3+1);
          itB = oImg.getIterator(i*3+2);
          
          for(; itR.inRegion(); itR++,itG++,itB++)
          {
            *itR = oImageStream.get();
            *itG = oImageStream.get();
            *itB = oImageStream.get();
          }
        }  

// }}}
        break;
      }
      
      case ioFormatICL:
        SECTION_LOG("Start reading image:");
        ERROR_LOG("Implementation missing");
        break;
        
      default:
        ERROR_LOG("Could not read data");
    } 
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
  
  //--------------------------------------------------------------------------
  void FileRead::readSequenceFile (const std::string& sFileName) 
     // {{{ open
  {
     char cTmp[255];
     ifstream streamSeq (sFileName.c_str(),ios::in);

     if(!streamSeq)
     {
        ERROR_LOG("Can't open sequence file: " << sFileName);
     }
      
     while(streamSeq)
     {
        streamSeq.getline(cTmp, 255);
        m_vecFileName.push_back(cTmp);
     }      
     streamSeq.close();
  }
// }}}

} // namespace icl
