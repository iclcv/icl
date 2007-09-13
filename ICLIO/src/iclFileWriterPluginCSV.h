#ifndef ICL_FILE_WRITER_PLUGIN_CSV_H
#define ICL_FILE_WRITER_PLUGIN_CSV_H

#include <iclFileWriterPlugin.h>

namespace icl{

  /// Writer plugins for ".csv"-files (<b>Comma</b>-<b>Separated</b> <b>Values</b>) \ingroup FILEIO_G
  class FileWriterPluginCSV : public FileWriterPlugin{
    public:
    
    /// write implementation
    virtual void write(File &file, const ImgBase *image);

    /// static feature adaption function
    /** if the flag is set to true, the writer will encode image
        properties by extending the given filename 
        @see FileGrabberPluginCSV 
    **/
    static void setExtendFileName(bool value);

    private:
    /// static flag
    static bool s_bExtendFileName;
  };
}
#endif
