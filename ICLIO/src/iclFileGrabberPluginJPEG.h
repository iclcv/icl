#ifndef ICL_FILE_READER_PLUGIN_JPEG_H
#define ICL_FILE_READER_PLUGIN_JPEG_H

#include <iclFileGrabberPlugin.h>

namespace icl{
  

  /// Plugin class to read "jpeg" and "jpg" images \ingroup FILEIO_G
  class FileGrabberPluginJPEG : public FileGrabberPlugin {
    public:
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest); 
  };  
}

#endif
