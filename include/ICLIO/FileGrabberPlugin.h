#ifndef ICL_FILE_GRABBER_PLUGIN_H
#define ICL_FILE_GRABBER_PLUGIN_H

#include <ICLIO/File.h>
#include <ICLCore/Img.h>

namespace icl{
  /// interface for ImageGrabber Plugins for reading different file types \ingroup FILEIO_G
  class FileGrabberPlugin{
    public:
#ifdef HAVE_LIBJPEG
    friend class JPEGDecoder;
#endif

    virtual ~FileGrabberPlugin() {}
    /// pure virtual grab function
    virtual void grab(File &file, ImgBase **dest)=0;

    protected:
    /// Internally used collection of image parameters
    struct HeaderInfo{
      format imageFormat; ///!< format
      depth imageDepth;   ///!< depth
      Rect roi;           ///!< roi rect
      Time time;          ///!< time stamp
      Size size;          ///!< image size
      int channelCount;   ///!< image channel count
      int imageCount;     ///!< image count (e.g. when writing color images as .pgm gray-scale images)
    };
  };  
}

#endif
