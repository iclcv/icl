/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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
