/*
  FileRead.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLFILEREAD_H
#define ICLFILEREAD_H

#include <Grabber.h>
#include <string>
#include <vector>

namespace icl {
 
/// The FileRead class implements the interface for reading images from file
/**
   @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de) 
**/
class FileRead : public Grabber
{
 public:
  // @{ @name constructors / destructor
  ///Load images from files specified with shell-like regular expression
  /** @param sPattern shell expression describing file location(s)
      @param bBuffer Switch image buffering On/ Off [default = OFF]
  **/
   FileRead(std::string sPattern, bool bBuffer = 0);
  
  ///Load images from file at specific location
  /** @param sFilePrefix a prefix for the filenames to search
      @param sDir The directory to read the files from
      @param sFilter The file type (ppm, pgm)
      @param bBuffer Switch image buffering On/ Off [default = OFF]
  **/
  FileRead(const std::string& sFilePrefix, std::string sDir, 
           const std::string& sFilter, bool bBuffer = 0);
  
  ///Load objects and images in an specific range from file 
  /** @param sObjPrefix The filename prefix
      @param sFilter The file type (ppm, pgm)
      @param sDir The directory to read the files from 
      @param iObjStart Start with object iObjStart
      @param iObjStart End with object iObjEnd
      @param iImageStart Start with image iImageStart
      @param iImageEnd End with object iImageEnd
      @param bBuffer Switch image buffering On/ Off [default = OFF]
  **/
  FileRead(const std::string& sObjPrefix, const std::string& sFileType, 
           std::string sDir,
           int iObjStart, int iObjEnd,
           int iImageStart, int iImageEnd, 
           bool bBuffer = 0);

  ///Destructor for FileRead
  ~FileRead() throw() {}
  
  ///Grab the next image from file/ buffer
  ImgI* grab(ImgI* poDst=0);
  
  
 protected:
  void readSequenceFile(const std::string& sFileName);

  void readHeader(info &oImgInfo);

  template <class Type>
  void readData(Img<Type> &oImg, info &oImgInfo);
  
 private:
  void bufferImages();
  std::vector <std::string> m_vecFileName;
  std::vector<Img<icl8u> > m_vecImgBuffer;
  std::vector<Img<icl8u> >::iterator m_iterImgBuffer;
  
  bool m_bBufferImages;
  unsigned int m_iImgCnt;
  ImgI *m_poInImg;
  
}; // class FileRead
 
} // namespace icl

#endif //ICLFILE_H
