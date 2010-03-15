/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/FileWriterPlugin.h                       **
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
