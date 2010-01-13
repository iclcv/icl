#ifndef ICL_FILE_WRITER_PLUGIN_H
#define ICL_FILE_WRITER_PLUGIN_H

#include <ICLCore/Img.h>
#include <ICLIO/File.h>


namespace icl{
  /// Interface class for writer plugins writing images in different file formats \ingroup FILEIO_G
  class FileWriterPlugin{
    public:
     virtual ~FileWriterPlugin() {}
    /// pure virtual writing function
    virtual void write(File &file, const ImgBase *image)=0;
  };
}
#endif
