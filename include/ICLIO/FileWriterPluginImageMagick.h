#ifndef ICL_FILE_WRITER_PLUGIN_IMAGEMAGICK_H
#define ICL_FILE_WRITER_PLUGIN_IMAGEMAGICK_H

#include <ICLIO/FileWriterPlugin.h>

namespace icl{
  /// Interface class for writing images using an ImageMagick++ wrapper  \ingroup FILEIO_G
  /** ImageMagick provides reading and writing routines for many file formats: The following
      list shows all formats currently implemented for ICL's FileWriter.
      Notify ICL's support if a common format, supported by ImageMagick is missing!
      
      <pre>
      "png", "gif", "pdf",  "ps",  "avs", "bmp", "cgm",  "cin",   "cur",  "cut", "dcx",
      "dib", "dng", "dot",  "dpx", "emf", "epdf","epi",  "eps",   "eps2", "eps3",
      "epsf","epsi","ept",  "fax", "gplt","gray","hpgl", "html",  "ico",  "info",
      "jbig","jng", "jp2",  "jpc", "man", "mat", "miff", "mono",  "mng",  "mpeg","m2v",
      "mpc", "msl", "mtv",  "mvg", "palm","pbm", "pcd",  "pcds",  "pcl",  "pcx", "pdb",
      "pfa", "pfb", "picon","pict","pix", "ps",  "ps2",  "ps3",   "psd",  "ptif","pwp",
      "rad", "rgb", "pgba", "rla", "rle", "sct", "sfw",  "sgi",   "shtml","sun", "svg",
      "tga", "tiff","tim",  "ttf", "txt", "uil", "uyuv", "vicar", "viff", "wbmp",
      "wmf", "wpg", "xbm",  "xcf", "xpm", "xwd", "ydbcr","ycbcra","yuv"
      </pre>
      

      Use the following linux shell command for a list of supported formats of your 
      ImageMagick library:
      <pre>
      > identify -list format
      </pre>
      
  */
  class FileWriterPluginImageMagick : public FileWriterPlugin{
    public:
    /// creates a plugin
    FileWriterPluginImageMagick();
    
    /// Destructor
    virtual ~FileWriterPluginImageMagick();
    
    /// pure virtual writing function
    virtual void write(File &file, const ImgBase *image);

    /// InternalData storage class
    class InternalData;

    private:
    /// Pointer to internal data storage
    InternalData *m_data;
  };
}
#endif
