/*
  File.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLFILE_H
#define ICLFILE_H

#include <fstream>
#include "Converter.h"
#include "Grabber.h"
#include "Writer.h"

namespace icl {

//-------- ICLFileIO class definition --------
class File : public Grabber, public Writer
{
 private:
  void parseImageHeader(ifstream &streamInputImage);
  void clear();
  void adaptImg(ImgI **poImg);
  string buildFileName();
  void writeAsPGM(ImgI *poSrc, ofstream &streamOutputImage);
  void writeAsPPM(ImgI *poSrc, ofstream &streamOutputImage);
  void writeAsMatrix(ImgI *poSrc, ofstream &streamOutputImage);
  void readFromPGM(ImgI* poDst, ifstream &streamInputImage);
  void readFromPPM(ImgI* poDst, ifstream &streamInputImage);
  
  //---- Variables for file information ----
  Converter m_oConverter;
  ioformat m_eFileFormat;
  format m_eFormat;
  depth m_eDepth;
  readermode m_eReaderMode; 
  streampos m_streamImageStartPos;
  string m_sSingleFileName;
  vector<string> m_vecSeqContent;
  
  //---- Variables for read functions ----
  Rect m_oROI;
  Size m_oImgSize;
  float m_iOriginalMin, m_iOriginalMax;
  int m_iNumChannels, m_iNumImages;
  
  //---- Variables for filename builder  ----
  string m_sObjPrefix, m_sFileType;
  int m_iObjStart, m_iObjEnd, m_iImageStart, m_iImageEnd;
  int m_iCurrObj, m_iCurrImageNum;

 public:
  File(string sObjPrefix, string sFileType,
       int iObjStart, int iObjBegin,
       int iImageStart, int iImageEnd);
  
  File(string sFileName);
  ~File();
  
  ImgI* grab(ImgI* poDst=0);
  void write(ImgI* poSrc);
};
 
}//namespace icl

#endif
