#ifndef ICL_FILE_GRABBER_PLUGIN_PNM_H
#define ICL_FILE_GRABBER_PLUGIN_PNM_H

#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  
  /// Plugin to grab ".ppm", ".pgm", ".pgm" and ".icl" images \ingroup FILEIO_G
  class FileGrabberPluginPNM : public FileGrabberPlugin{
    public:
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest);
  };  
}

#endif
