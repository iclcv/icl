/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileGrabber.h                            **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FILE_GRABBER_H
#define ICL_FILE_GRABBER_H

#include <string>
#include <map>
#include <ICLIO/FileGrabberPlugin.h>
#include <ICLIO/FileList.h>
#include <ICLIO/GrabberHandle.h>

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
  class FileGrabberImpl : public Grabber{
    public:
    
    /// for the internal plugin concept
    friend class FileGrabberPluginMapInitializer;

    /// Create a NULL FileGrabber
    FileGrabberImpl();
    
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
    FileGrabberImpl(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false) 
      throw(FileNotFoundException);

    /// Destructor
    virtual ~FileGrabberImpl();
    
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
    
    
    /// forces the filegrabber to use a plugin for the given suffix
    /** suffix must be something like png or csv (without a trailing .)
        By default, the forced plugin type string is "". In this case,
        the filename suffix is evaluated to determine the appropriate 
        grabber plugin.
    */
    void forcePluginType(const std::string &suffix);
    
    
    /// interface for the setter function for video device properties 
    /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set usingsetProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();
    
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);

    /// get information of a property valid values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);

    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);

    /// returns whether property is volatile
    virtual int isVolatile(const std::string &propertyName);
    
    private:
    /// internal file list
    FileList m_oFileList;
    
    /// current file list index
    int m_iCurrIdx;
    
    /// buffer for buffered mode
    std::vector<ImgBase*> m_vecImageBuffer;
    
    /// flag whether to pre-buffer images or not
    bool m_bBufferImages;

    /// indicates whether to jump to next frame automatically
    bool m_bAutoNext;
    
    /// A special buffer image
    ImgBase *m_poBufferImage;
    
    /// map of plugins
    static std::map<std::string,FileGrabberPlugin*> s_mapPlugins;
    
    /// forced plugin name
    std::string m_forcedPluginType;
  };
  

  /// FileGrabber class (see FileGrabberImpl)
  class FileGrabber : public GrabberHandle<FileGrabberImpl>{
    public:
    /// Empty constructor
    /** @see FileGrabberImpl::FileGrabberImpl() */
    inline FileGrabber(){}

    /// Empty constructor
    /** @see FileGrabberImpl::FileGrabberImpl(const std::string&,bool,bool); */
    inline FileGrabber(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false){
      if(isNew(pattern)){
        initialize(new FileGrabberImpl(pattern,buffer,ignoreDesiredParams),pattern);
      }else{
        initialize(pattern);
      }
    }
    
    /// returns number of image files available
    /** @see FileGrabberImpl::getFileCount */
    inline unsigned int getFileCount() const{
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->getFileCount();
    }

    /// returns nextFileName that will be grabbed
    /** @see FileGrabberImpl::getNextFileName */
    inline const std::string &getNextFileName() const{
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->getNextFileName();
    }

    /// makes the grabber buffer all images internally
    /** @see FileGrabberImpl::bufferImages(bool) */
    inline const FileList &bufferImages(bool omitExceptions=true){
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->bufferImages(omitExceptions);
    }

    /// skips the next to-be-grabbed image
    /** @see FileGrabberImpl::next() */
    inline void next(){
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->next();
    }

    /// makes the grabber grab the last image again
    /** @see FileGrabberImpl::prev() */
    inline void prev(){
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->prev();  
    }

    /// forces a certain plugin type
    /** @see FileGrabberImpl::forcePluginType(const std::string&)() */
    inline void forcePluginType(const std::string &suffix){
      Mutex::Locker l(m_instance->mutex);
      return m_instance.get()->ptr->forcePluginType(suffix); 
    }

  };
  
}

#endif
