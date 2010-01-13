#ifndef ICL_FILE_GRABBER_PLUGIN_CSV_H
#define ICL_FILE_GRABBER_PLUGIN_CSV_H

#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  
  /** \cond */
  class Mutex;
  /** \endcond */
  
  /// FileGrabber plugins for writing ".csv" files (<b>C</b>omma-<b>S</b>eparated <b>V</b>alues) \ingroup FILEIO_G
  /** image parameters can be found by three different means:
      -# <b>Encoded into the file name.</b> When using the ICLFileWriter to write
         csv files, it can be set up to encode the images parameter into the
         filename using a string extension of the underlying filename. The 
         following pattern is used for this: 
         "DIR/ORIG_FILE_BASE_NANE-ICL:WxHxC:DEPTH:format.csv"
      -# <b>As comment block</b> Although the CSV-file convention does not
         include comments, a common standard is to add a "#"-character to signal
         comment lines. By this means, a default ".icl"-header can be used to 
         define an csv-images shape and parameters.
      -# <b>Interpret a csv file as matrix data</b> If no of the other two
         possibilities were successful to determine a csv-image's size, the
         file is interpreted as double-matrix. The line count of the matrix defines
         the image height; its horizontal comma-separated token count defines its
         width.
      */
  class FileGrabberPluginCSV : public FileGrabberPlugin{
    public:
    /// Create a new Plugin
    FileGrabberPluginCSV();
    
    /// Destructor
    ~FileGrabberPluginCSV();
    
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest);
    
    private:

    /// internally used reading buffer
    Img64f *m_poReadBuffer;
    
    /// internally used mutex to protect the reading buffer
    Mutex *m_poReadBufferMutex;
  };  
}

#endif
