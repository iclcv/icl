/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileGrabberPluginImageMagick.h           **
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
