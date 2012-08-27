/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileGrabberPluginPNG.h                   **
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

#ifndef ICL_FILE_READER_PLUGIN_PNG_H
#define ICL_FILE_READER_PLUGIN_PNG_H

#include <ICLIO/FileGrabberPlugin.h>
#include <vector>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace io{
    
  
    /// Plugin class to read "png" images \ingroup FILEIO_G
    class FileGrabberPluginPNG : public FileGrabberPlugin {
      std::vector<unsigned char> data;
      std::vector<unsigned char*> rows;
  
      /// ensures, that data and rows is not used from several threads
      Mutex mutex; 
      
      public:
      /// grab implementation
      virtual void grab(File &file, ImgBase **dest); 
    };  
  } // namespace io
}

#endif
