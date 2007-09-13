#ifndef ICL_FILE_GRABBER_H
#define ICL_FILE_GRABBER_H

#include <string>
#include <map>
#include <iclUncopyable.h>
#include <iclFileGrabberPlugin.h>
#include <iclFileList.h>
#include <iclGrabber.h>

namespace icl{
  
  /// Grabber implementation to grab from files \ingroup FILEIO_G \ingroup GRABBER_G
  /** This implementation of a file grabber class provides an internally used
      and extendible plugin-based interface for reading files of different types.
      
      \section EX Example
      \code
      FileGrabber g("image/image*.jpg"); // 
      while(true){
        const ImgBase *image = g.grab();
        ...
      }
      \endcode
  **/
  class FileGrabber : public Grabber, public Uncopyable{
    public:
    
    /// for the internal plugin concept
    friend class FileGrabberPluginMapInitializer;

    /// Create a NULL FileGrabber
    FileGrabber();
    
    /// Create a file grabber with given pattern and buffer flag
    FileGrabber(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false);

    /// Destructor
    virtual ~FileGrabber();
    
    /// grab implementation
    virtual const ImgBase *grab(ImgBase **ppoDst=0);
    
    /// sets whether to ignore desired params or not (TODO as "setProperty(...)")
    void setIgnoreDesiredParams(bool flag);

    private:
    /// internal file list
    FileList m_oFileList;
    
    /// current file list index
    int m_iCurrIdx;
    
    /// buffer for buffered mode
    std::vector<ImgBase*> m_vecImageBuffer;
    
    /// flag whether to pre-buffer images or not
    bool m_bBufferImages;
    
    /// flag whether to ignore the parent Grabber's desired params
    bool m_bIgnoreDesiredParams;
    
    /// A special buffer image
    ImgBase *m_poBufferImage;
    
    /// map of plugins
    static std::map<std::string,FileGrabberPlugin*> s_mapPlugins;
  };
  
}

#endif
