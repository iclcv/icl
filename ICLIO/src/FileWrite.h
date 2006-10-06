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
#include <Writer.h>
#include <Converter.h>
#include <IO.h>

namespace icl {

class FileWrite : public Writer
{
 public:
  FileWrite(std::string sPrefix, std::string sDir, std::string sType, int iObjNum = 1);
  FileWrite(std::string sFileName);
  
  void write(ImgI* poSrc);
  
  void setObjID(int iID) { m_iCurrObj = iID;}
  
 private:
  void writeAsPGM(ImgI *poSrc, std::ofstream &streamOutputImage);
  void writeAsPPM(ImgI *poSrc, std::ofstream &streamOutputImage);
  void writeAsMatrix(ImgI *poSrc, std::ofstream &streamOutputImage);
  std::string buildFileName();
  
  Converter m_oConverter;
  info m_oInfo;
  int m_iWriteMode;
  int m_iCurrObj;
  
}; //class

} //namespace icl
#endif //ICLFILEWRITE_H
