/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileWriterPluginCSV.h                    **
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

#ifndef ICL_FILE_WRITER_PLUGIN_CSV_H
#define ICL_FILE_WRITER_PLUGIN_CSV_H

#include <ICLIO/FileWriterPlugin.h>

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
