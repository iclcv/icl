#ifndef ICL_FILE_WRITER_PLUGIN_PNM_H
#define ICL_FILE_WRITER_PLUGIN_PNM_H

#include <iclFileWriterPlugin.h>
#include <iclMutex.h>
#include <vector>
#include <iclTypes.h>

namespace icl{
  
  /// Writer plugin to write images as ".ppm", ".pgm", ".pnm" and ".icl" \ingroup FILEIO_G
  class FileWriterPluginPNM : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(File &file, const ImgBase *image);
    
    private:
    /// internal mutex to protect the buffer
    Mutex m_oBufferMutex;
    
    /// internal data conversion buffer
    std::vector<icl8u> m_vecBuffer;
  };
}
#endif
