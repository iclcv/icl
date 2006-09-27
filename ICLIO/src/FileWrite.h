/*
  FileWrite.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLFILEWRITE_H
#define ICLFILEWRITE_H

#include <fstream>
#include "Converter.h"
#include "Writer.h"
#include <ImgI.h>
#include <IO.h>

using namespace std;

namespace icl {

class FileWrite : public Writer
{
 public:
  FileWrite(string sPrefix, string sDir, string sType, int iObjNum = 1);
  FileWrite(string sFileName);
  
  void write(ImgI* poSrc);
  
  void setObjID(int iID) { m_iCurrObj = iID;}
  
 private:
  void writeAsPGM(ImgI *poSrc, ofstream &streamOutputImage);
  void writeAsPPM(ImgI *poSrc, ofstream &streamOutputImage);
  void writeAsMatrix(ImgI *poSrc, ofstream &streamOutputImage);
  string buildFileName();
  
  Converter m_oConverter;
  info m_oInfo;
  int m_iWriteMode;
  int m_iCurrObj;
  
}; //class

} //namespace icl
#endif //ICLFILEWRITE_H
