#ifndef ICLFILEWRITER_H
#define ICLFILEWRITER_H

#include <iclWriter.h>
#include <iclImg.h>
/*
  FileWriter.h

  Written by: Michael Götting (2006)
  University of Bielefeld
  AG Neuroinformatik
  mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {
  struct FileInfo;
   
  /// write ICL images to file
  /** The FileWriter class supports writing to PNM, JPG and the proprietary
      ICL image format. The format is choosen according to the file type
      extension of the file name. The proprietary ICL format supports writing
      image depths other than depth8u. In all other cases the image is
      converted to depth8u first.
      If the filename as the pattern prefix##.filetype, the hashes (##)
      are replaced by a number continously incremented on every write
      operation. To create correctly ordered file names uses as many hashes
      as you need digits, i.e. #### goes to 0001, 0002, ...
  */
  int plainWrite (void *fp, const void *pData, unsigned int len);
  class FileWriter : public Writer {
  public: 

    /// Constructor
    FileWriter(const std::string& sFileName);
    
    /// Set Functions
    void setFileName (const std::string& sFileName);
    void setCounter (int iID) {nCounter = iID;}
    void write(const ImgBase* poSrc);
    
  protected:
    void writePNMICL (const ImgBase *poSrc, const FileInfo& oInfo);
    void writeJPG (const Img<icl8u> *poSrc, 
                   const FileInfo& oInfo, int iQuality=85);
    std::string buildFileName();
    std::string sFilePrefix, sFileSuffix;
    unsigned int  nCounterDigits;
    unsigned int  nCounter;
    Img<icl8u>    m_oImg8u;
  }; //class
   
} //namespace icl
#endif //ICLFILEWRITER_H
