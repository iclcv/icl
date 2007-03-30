#ifndef ICLFILEWRITER_H
#define ICLFILEWRITER_H

#include <iclWriter.h>
#include <iclImg.h>
/*
  FileWrite.h

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

   class FileWriter : public Writer {
   public: 
      /// Constructor
      FileWriter(const std::string& sFileName) {setFileName (sFileName); m_bCsvExtendFilename=false; m_bCsvSplitFiles=false;m_bCsvHeader_set=false;}
      void CSVsetFileName (std::string& sFileName, const ImgBase *poSrc);

      void setFileName (const std::string& sFileName);
      void setCounter (int iID) {nCounter = iID;}

      void write(const ImgBase* poSrc);
      enum csvFlag{
        csvSplitFiles,
        csvExtendFilename
      };
      void setCSVFlag(csvFlag f,bool value);
    
   private:
      bool m_bCsvSplitFiles;
      bool m_bCsvExtendFilename;
      bool m_bCsvHeader_set;
      void writePNM (const ImgBase *poSrc, const FileInfo& oInfo);
      void writeJPG (const Img<icl8u> *poSrc, 
                     const FileInfo& oInfo, int iQuality=85);
      void writeCSV (const ImgBase *poSrc, FileInfo& oInfo);
      void writeICL (const ImgBase *poSrc, const FileInfo& oInfo);
      template<class T,class R>
      std::string writeCSVTmpl(const Img<T> *poSrc,int ch) ;
      template<class T,class R>
      void writeCSVTmpl(const Img<T> *poSrc, FileInfo& oInfo);
      std::string buildFileName ();
      
      std::string sFilePrefix, sFileSuffix;
      unsigned int  nCounterDigits;
      unsigned int  nCounter;
      Img<icl8u>    m_oImg8u;
   }; //class
   
} //namespace icl
#endif //ICLFILEWRITER_H
