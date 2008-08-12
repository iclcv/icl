#ifndef ICL_FILE_GRABBER_PLUGIN_IMAGE_MAGIC_H
#define ICL_FILE_GRABBER_PLUGIN_IMAGE_MAGIC_H

#include "iclFileGrabberPlugin.h"

namespace icl{

  /// Interface class for reading images using an ImageMagick++ wrapper  \ingroup FILEIO_G
  /** @copydoc icl::FileWriterPluginImageMagick
  */
  class FileGrabberPluginImageMagick : public FileGrabberPlugin{
    public:
    /// Create a new Plugin
    FileGrabberPluginImageMagick();
    
    /// Destructor
    ~FileGrabberPluginImageMagick();
    
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest);
    
    /// Internal data storage class
    struct InternalData;
    
    private:
    /// Internal data storage
    InternalData *m_data;
  };  
}

#endif
