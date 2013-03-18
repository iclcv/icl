/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriterPluginJPEG.h                 **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#pragma once

#include <ICLIO/FileWriterPlugin.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace io{
    
    /// A Writer Plugin for writing ".jpeg" and ".jpg" images \ingroup FILEIO_G
    class FileWriterPluginJPEG : public FileWriterPlugin{
      public:
      /// write implementation
      virtual void write(utils::File &file, const core::ImgBase *image);
      
      /// sets the currently used jped quality (0-100) (by default 90%)
      static void setQuality(int value);
      
      private:
      
      /// current quality (90%) by default
      static int s_iQuality;
      
      /// (static!) internal buffer for Any-to-icl8u conversion
      static core::Img8u s_oBufferImage;
      
      /// mutex to protect the static buffer
      static utils::Mutex s_oBufferImageMutex;
    };
  } // namespace io
}
