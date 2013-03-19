/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileGrabberPluginJPEG.cpp              **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/FileGrabberPluginJPEG.h>

#include <ICLUtils/StrTok.h>
#include <ICLIO/JPEGDecoder.h>

using namespace std;
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
  #ifdef HAVE_LIBJPEG
    void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
      // {{{ open 
      JPEGDecoder::decode(file,dest);
    }
    // }}}
  #else
    void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
      ERROR_LOG("JPEG support currently not available! \n" << 
                "To enabled JPEG support: you have to compile the ICLIO package\n" <<
                "with -DHAVE_LIBJPEG compiler flag AND with a valid\n" << 
                "LIBJPEG_ROOT set.");    
      ERROR_LOG("Destination image is set to NULL, which may cause further errors!");
      (void) file;
      ICL_DELETE( *dest );
    }
  #endif
  
  } // namespace io
}// end of the namespace icl
