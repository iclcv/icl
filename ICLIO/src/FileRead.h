/*
  FileRead.h

  Written by: Michael Götting, Robert Haschke (2006)
  University of Bielefeld
  AG Neuroinformatik
  mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLFILEREAD_H
#define ICLFILEREAD_H

#include <Grabber.h>
#include <Exception.h>
#include <IO.h>

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
         typedef std::vector<std::string> FileList;
         typedef std::vector<ImgBase*>       ImageBuffer;

         // @{ @name constructors / destructor
         ///Load images from files specified with shell-like regular expression
         /** @param sPattern shell expression describing file location(s)
          **/
         FileRead(std::string sPattern) throw (ICLException);
  
         ///Load images from files having the pattern sPrefix[obj]__[img].sType 
         ///where [obj] and [img] are replaced by numbers in a given range
         /** @param srefix The filename prefix, including directory
             @param sType  The file type (ppm, pgm, pnm.gz, jpg, icl)
             @param iObjStart Start with object iObjStart
             @param iObjStart End with object iObjEnd
             @param iImageStart Start with image iImageStart
             @param iImageEnd End with object iImageEnd
         **/
         FileRead(const std::string& sPrefix, const std::string& sType, 
                  int iObjStart, int iObjEnd, int iImageStart, int iImageEnd)
            throw (ICLException);

         FileRead(const FileRead& other);
         FileRead& operator=(const FileRead& other);

         ///Destructor
         virtual ~FileRead ();

         ///Grab the next image from file/ buffer
         virtual ImgBase* grab(ImgBase* poDst=0) 
            throw (ICLInvalidFileFormat, FileOpenException, ICLException) ;
  
         /// Load all images from current file list into an internal buffer
         /** @return a list of failed image files */
         FileList bufferImages (bool bStopOnError=false) throw ();

         /// retrieve name of next grabbed file
         const std::string& getNextFileName () const 
            {return m_vecFileName[m_iCurImg];}

         /// skip next file and directly jump to next one
         void next ()
            { if (++m_iCurImg == m_vecFileName.size()) m_iCurImg=0; }
         /// jump back to previous file
         void prev () {
            if (m_iCurImg == 0) m_iCurImg = m_vecFileName.size()-1;
            else --m_iCurImg;
         }

         /// remove given files from internal list of files
         /// and additionally remove buffered images, if neccessary
         void removeFiles (const FileList& vecFiles);
           
      private:
         void init () throw (ICLException);
         bool findFile (const std::string& sFile, FileList::iterator& itList);

         void readSequenceFile(const std::string& sFileName);
         void readImage (const std::string& sFileName, ImgBase** ppoDst)
            throw (ICLInvalidFileFormat, FileOpenException, ICLException);

         void readHeaderPNM (FileInfo &oImgBasenfo);
         void readHeaderJPG (FileInfo &oImgBasenfo);

         void readDataPNM(ImgBase* poImg, FileInfo &oImgBasenfo);
         void readDataJPG(Img<icl8u>* poImg, FileInfo &oImgBasenfo);
  

         FileList     m_vecFileName;  //< list of files to load
         ImageBuffer  m_vecImgBuffer; //< vector of buffered images
  
         bool m_bBuffered;         //< flag indicating buffering of images
         unsigned int m_iCurImg;   //< image number to be read next
         ImgBase        *m_poCurImg;  //< recently read image

         struct jpeg_decompress_struct jpgCinfo;
         struct icl_jpeg_error_mgr     jpgErr;
      }; // class FileRead
 
} // namespace icl

#endif //ICLFILE_H
