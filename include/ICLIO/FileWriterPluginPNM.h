/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileWriterPluginPNM.h                    **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FILE_WRITER_PLUGIN_PNM_H
#define ICL_FILE_WRITER_PLUGIN_PNM_H

#include <ICLIO/FileWriterPlugin.h>
#include <ICLUtils/Mutex.h>
#include <vector>
#include <ICLCore/Types.h>

namespace icl{
  
  /// Writer plugin to write images as ".ppm", ".pgm", ".pnm" and ".icl" \ingroup FILEIO_G
  class FileWriterPluginPNM : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(File &file, const ImgBase *image);
    
    private:
    /// internal mutex to protect the buffer
    Mutex m_oBufferMutex;
    
    /// internal data conversion buffer
    std::vector<icl8u> m_vecBuffer;
  };
}
#endif
