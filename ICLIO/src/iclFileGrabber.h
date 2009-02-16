#ifndef ICL_FILE_GRABBER_H
#define ICL_FILE_GRABBER_H

#include <string>
#include <map>
#include <iclFileGrabberPlugin.h>
#include <iclFileList.h>
#include <iclGrabber.h>

namespace icl{
  
  /// Grabber implementation to grab from files \ingroup FILEIO_G \ingroup GRABBER_G
  /** This implementation of a file grabber class provides an internally used
      and expendable plugin-based interface for reading files of different types.
      
      \section EX Example
      \code
      FileGrabber g("image/image*.jpg"); // 
      while(true){
        const ImgBase *image = g.grab();
        ...
      }
      \endcode
  **/
  class FileGrabber : public Grabber{
    public:
    
    /// for the internal plugin concept
    friend class FileGrabberPluginMapInitializer;

    /// Create a NULL FileGrabber
    FileGrabber();
    
    /// Create a file grabber with given pattern and parameters
    /** \section BUF Buffering
        Images are buffered internally using the original image format i.e. most images are
        stored as icl8u images. At first during the grab(..) call, the images are converted
        automatically to the desired parameters (dependent on the desired params flag)

        @param pattern file pattern (e.g. images/ *.jpg ...) 
        @param buffer flag to determine, whether images should be pre-buffered. If the flag is
                      set to true, images are buffered immediately in the constructor. If
                      exceptions that occur during this buffering procedure are <b>not</b>
                      caught internally (i.e. these exceptions are thrown). This is necessary
                      because, the FileGrabber can become inconsistent in this case, which 
                      must be reported explicitly using an exception.\n
                      
                      To pre-buffer images in a error-tolerant way, use the constructor without
                      a set buffer flag and call bufferImages(false) after the FileGrabber
                      instantiation.
        @param ignoreDesiredParams In some cases, grabbed images should not be adapted to the
                                   given parameters using a converter, but the original image 
                                   parameters stored in the file header should be restored. In 
                                   this case the ignoreDesiredParams flag must be set to true
                                   in the constructor or by using the setIgnoreDesiredParams(bool)
                                   function. To ensure compatibility to other grabbers, the 
                                   "ignore desired params" flag is set to false by default.

        */
    FileGrabber(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false) 
      throw(FileNotFoundException);

    /// Destructor
    virtual ~FileGrabber();
    
    /// grab implementation
    virtual const ImgBase *grabUD(ImgBase **ppoDst=0);
    
    /// returns the count of files that are available
    unsigned int getFileCount() const;
    
    /// returns the next to-be-grabbed file name
    const std::string &getNextFileName() const;
    
    /// pre-buffers all images internally
    /** This function is called automatically,if the "buffer"-flag of the constructor is set.
        The function internally pre-buffers all images, that are contained in the current
        FileList. During this procedure, some exception may occur:
        - <b>FileNotFoundException</b>, if a file list entry references a non-existing file
        - <b>InvalidFileException</b>, if no FileGrabberPlugin can be found to read files
          of the given format (determined by the file postfix e.g. ".icl" or ".ppm")
        - <b>InvalidFileFormatException</b>, if the FileGrabberPlugin encounters a file section
          with invalid contents (e.g. if a file header is corrupted, or if the file contains
          not enough data elements to fill an image with size determined by the file header)
        - <b>ICLException</b> some "not-more-specified" errors (e.g if a file could not be read
          due to missing permissions)
        
        To skip these exceptions by just leaving out files that could not be read by any reason,
        the omitExceptions flag can be set (default). In this case, exceptions mentioned above 
        are caught implicitly. \n
        However if not even a single file could be read, a FileNotFoundException is thrown, to indicate,
        that the FileGrabber's grab/prev and next function will not work properly. \n

        To indicate which files could be buffered, a reference to the internal file list is
        returned.
        */
    const FileList &bufferImages(bool omitExceptions=true);

    /// internally skips the next to-be-grabbed image
    /** E.g. if the image directory "images/" contains 6 valid ".jpg"-files,
        the following code grabs all images with even indices (0,2, and 4)
        \code
        FileGrabber g("images/ *.jpg");
        for(int i=0;i<g.getFileCount()/2;++i){
          g.grab();
          g.next();
        }
        \endcode
    */
    void next();
    
    /// internally sets the next-image pointer back
    /** \code 
        FileGrabber g(...);
        g.grab(); // grabs image A
        g.grab(); // grabs image B
        g.prev();
        g.grab(); // again grabs image B
        \endcode
    */
    void prev();
    
    private:
    /// internal file list
    FileList m_oFileList;
    
    /// current file list index
    int m_iCurrIdx;
    
    /// buffer for buffered mode
    std::vector<ImgBase*> m_vecImageBuffer;
    
    /// flag whether to pre-buffer images or not
    bool m_bBufferImages;
    
    /// A special buffer image
    ImgBase *m_poBufferImage;
    
    /// map of plugins
    static std::map<std::string,FileGrabberPlugin*> s_mapPlugins;
  };
  
}

#endif
