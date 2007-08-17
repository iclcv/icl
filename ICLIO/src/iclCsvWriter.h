#ifndef ICLCSVWRITER_H
#define ICLCSVWRITER_H

#include <iclWriter.h>
#include <iclImg.h>
#include <iclFileWriter.h>
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
  enum csvFlag{
    csvSplitFiles,
    csvExtendFilename
  };
  
  class CsvWriter : public FileWriter {
  public: 

    /// Constructor
    CsvWriter(const std::string& sFileName);

    void write(const ImgBase* poSrc);
    /// Set Functions
    void setFileNameCSV (std::string& sFileName, const ImgBase *poSrc);
    void setCSVFlag(csvFlag f,bool value);  
  private:

    void writeCSV (const ImgBase *poSrc, FileInfo& oInfo);
    template<class T,class R>
      std::string writeCSVTmpl(const Img<T> *poSrc,int ch) ;
    template<class T,class R>
      void writeCSVTmpl(const Img<T> *poSrc, FileInfo& oInfo);

    bool m_bCsvSplitFiles;
    bool m_bCsvExtendFilename;
  }; //class
   
} //namespace icl
#endif //ICLFILEWRITER_H
