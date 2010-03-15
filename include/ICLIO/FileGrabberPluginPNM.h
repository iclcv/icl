/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/FileGrabberPluginPNM.h                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting                   **
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

#ifndef ICL_FILE_GRABBER_PLUGIN_PNM_H
#define ICL_FILE_GRABBER_PLUGIN_PNM_H

#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  
  /// Plugin to grab ".ppm", ".pgm", ".pgm" and ".icl" images \ingroup FILEIO_G
  class FileGrabberPluginPNM : public FileGrabberPlugin{
    public:
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest);
  };  
}

#endif
