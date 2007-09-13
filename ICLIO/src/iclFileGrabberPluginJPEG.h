#ifndef ICL_FILE_READER_PLUGIN_JPEG_H
#define ICL_FILE_READER_PLUGIN_JPEG_H

#include <iclFileGrabberPlugin.h>

namespace icl{
  
  /** \cond */
  class JPEGDataHandle;
  /** \endcond*/

  /// Plugin class to read "jpeg" and "jpg" images \ingroup FILEIO_G
  class FileGrabberPluginJPEG : public FileGrabberPlugin {
    public:
    /// Constructor
    FileGrabberPluginJPEG();
    
    /// Destructor
    ~FileGrabberPluginJPEG();
    
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest); 
    
    private:
    /// Internally used handle for libjpeg specific data
    JPEGDataHandle *m_poJPEGHandle;
  };  
}

#endif
