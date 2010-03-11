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

#ifndef ICL_FILE_WRITER_PLUGIN_JPEG_H
#define ICL_FILE_WRITER_PLUGIN_JPEG_H

#include <ICLIO/FileWriterPlugin.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  
  /// A Writer Plugin for writing ".jpeg" and ".jpg" images \ingroup FILEIO_G
  class FileWriterPluginJPEG : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(File &file, const ImgBase *image);
    
    /// sets the currently used jped quality (0-100) (by default 90%)
    static void setQuality(int value);
    
    private:
    
    /// current quality (90%) by default
    static int s_iQuality;
    
    /// (static!) internal buffer for Any-to-icl8u conversion
    static Img8u s_oBufferImage;
    
    /// mutex to protect the static buffer
    static Mutex s_oBufferImageMutex;
  };
}
#endif
