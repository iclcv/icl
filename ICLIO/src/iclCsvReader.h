#ifndef ICLCSVREADER_H
#define ICLCSVREADER_H

#include <iclFileReader.h>
#include <iclException.h>
#include <iclIO.h>
#include <string>
#include <vector>
/*
    FileReader.h

    Written by: Michael Götting, Robert Haschke (2006)
    University of Bielefeld
    AG Neuroinformatik
    mgoettin@techfak.uni-bielefeld.de
    */


namespace icl {
 
  //! The CsvReader class implements the garbber interface to read images from csv file
  /*!
      @author Robert Haschke (rhaschke@TechFak.Uni-Bielefeld.de)
      @brief The ICL

      */
  class CsvReader : public FileReader{
    public:
    CsvReader(std::string sPattern);
    
    ///Load images from files having the pattern sPrefix[obj]__[img].sType 
    ///where [obj] and [img] are replaced by numbers in a given range
    /** @param sPrefix The filename prefix, including directory
        @param sType  The file type (ppm, pgm, pnm.gz, jpg, icl)
        @param iObjStart Start with object iObjStart
        @param iObjEnd End with object iObjEnd
        @param iImageStart Start with image iImageStart
        @param iImageEnd End with object iImageEnd
        **/
    CsvReader(const std::string& sPrefix, const std::string& sType, 
               int iObjStart, int iObjEnd, int iImageStart, int iImageEnd);

    CsvReader(const FileReader& other);
    
    /// DEPRECATED todo:: redesign
    void setCSVHeader (depth depth, const ImgParams& p);

    private:
    virtual void readImage (const std::string& sFileName, ImgBase** ppoDst);
    void readHeaderCSV (FileInfo &oInfo);         
    template<class T>
    void readCSVTmpl(Img<T>* poImg, FileInfo &oInfo);
    void readDataCSV(ImgBase* poImg, FileInfo &oInfo);
    /// used for CSV file format which doesn't store image info in file
    ImgParams       m_CSVParams;
    depth           m_CSVDepth;
  }; // class FileReader
 
} // namespace icl

#endif //ICLFILE_H
