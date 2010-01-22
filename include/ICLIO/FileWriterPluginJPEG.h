#ifndef ICL_FILE_WRITER_PLUGIN_JPEG_H
#define ICL_FILE_WRITER_PLUGIN_JPEG_H

#include <ICLIO/FileWriterPlugin.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  
  /// A Writer Plugin for writing ".jpeg" and ".jpg" images \ingroup FILEIO_G
  class FileWriterPluginJPEG : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(File &file, const ImgBase *image);
    
    /// sets the currently used jped quality (0-100) (by default 90%)
    static void setQuality(int value);
    
    private:
    
    /// current quality (90%) by default
    static int s_iQuality;
    
    /// (static!) internal buffer for Any-to-icl8u conversion
    static Img8u s_oBufferImage;
    
    /// mutex to protect the static buffer
    static Mutex s_oBufferImageMutex;
  };
}
#endif
