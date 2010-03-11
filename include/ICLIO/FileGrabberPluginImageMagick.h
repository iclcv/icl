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

#ifndef ICL_FILE_GRABBER_PLUGIN_IMAGE_MAGIC_H
#define ICL_FILE_GRABBER_PLUGIN_IMAGE_MAGIC_H

#include <ICLIO/FileGrabberPlugin.h>

namespace icl{

  /// Interface class for reading images using an ImageMagick++ wrapper  \ingroup FILEIO_G
  /** @copydoc icl::FileWriterPluginImageMagick
  */
  class FileGrabberPluginImageMagick : public FileGrabberPlugin{
    public:
    /// Create a new Plugin
    FileGrabberPluginImageMagick();
    
    /// Destructor
    ~FileGrabberPluginImageMagick();
    
    /// grab implementation
    virtual void grab(File &file, ImgBase **dest);
    
    /// Internal data storage class
    struct InternalData;
    
    private:
    /// Internal data storage
    InternalData *m_data;
  };  
}

#endif
