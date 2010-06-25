/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileGrabberPluginCSV.h                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
