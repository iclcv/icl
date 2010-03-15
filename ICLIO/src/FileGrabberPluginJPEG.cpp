/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/src/FileGrabberPluginJPEG.cpp                    **
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
*********************************************************************/

#include <ICLIO/FileGrabberPluginJPEG.h>

#include <ICLUtils/StrTok.h>
#include <ICLIO/JPEGDecoder.h>

using namespace std;

namespace icl{
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

}// end of the namespace icl
