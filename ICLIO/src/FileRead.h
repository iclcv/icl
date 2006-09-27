/*
  FileRead.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLFILEREAD_H
#define ICLFILEREAD_H

#include "Grabber.h"
#include "Writer.h"
#include "stdlib.h"
#include "Converter.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>

using namespace std;

namespace icl {
 
/// The FileRead class implements the interface for reading images from file
/**
   @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de) 
**/
class FileRead : public Grabber, public Writer
{
 public:
  // @{ @name constructors / destructor
  ///Load images from file at specific location
  /** @param sFileName The filename
      @param sDir The directory to read the files from
      @param sFilter The file type (ppm, pgm)
      @param bBuffer Switch image buffering On/ Off
  **/
  FileRead(string sFileName, string sDir, string sFilter, bool bBuffer);
  
  ///Load objects and images in an specific range from file 
  /** @param sObjPrefix The filename prefix
      @param sFilter The file type (ppm, pgm)
      @param sDir The directory to read the files from 
      @param iObjStart Start with object iObjStart
      @param iObjStart End with object iObjEnd
      @param iImageStart Start with image iImageStart
      @param iImageEnd End with object iImageEnd
      @param bBuffer Switch image buffering On/ Off
  **/
  FileRead(string sObjPrefix, string sFileType, string sDir,
       int iObjStart, int iObjEnd,
       int iImageStart, int iImageEnd, bool bBuffer);

  ///Grab the next image from file/ buffer
  ImgI* grab(ImgI* poDst=0);
  
  ///Write the current image to file
  void write(ImgI* poSrc) {};
  
  ///Set image buffering on/ off. 
  /**
     If image buffering is on, all images pre-loaded and buffered in memory.
     This may cause a slower initialisation but speed up the image access
     during runtime.
  **/
  
 protected:
  void readHeader(info &oImgInfo);

  template <class Type>
  void readData(Img<Type> &oImg, info &oImgInfo);
  
 private:
  template <class Type>
  void readPGM(Img<Type> &oDst, info &oImgInfo);

  template <class Type>
  void readPPM(Img<Type> &oDst, info &oImgInfo);
  
  void bufferImages();
  vector <string> m_vecFileName;
  vector<Img<icl8u> > m_vecImgBuffer;
  vector<Img<icl8u> >::iterator m_iterImgBuffer;
  
  bool m_bBufferImages;
  unsigned int m_iImgCnt;
  
}; // class FileRead
 
} // namespace icl

#endif //ICLFILE_H
