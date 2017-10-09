/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileGrabberPlugin.h                    **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/File.h>
#include <ICLCore/Img.h>

namespace icl{
  namespace io{
    /// interface for ImageGrabber Plugins for reading different file types \ingroup FILEIO_G
    class ICLIO_API FileGrabberPlugin{
      public:
  #ifdef ICL_HAVE_LIBJPEG
      friend class JPEGDecoder;
  #endif

      virtual ~FileGrabberPlugin() {}
      /// pure virtual grab function
      virtual void grab(utils::File &file, core::ImgBase **dest)=0;

      protected:
      /// Internally used collection of image parameters
      struct HeaderInfo{
        core::format imageFormat; ///!< format
        core::depth imageDepth;   ///!< depth
        utils::Rect roi;           ///!< roi rect
        utils::Time time;          ///!< time stamp
        utils::Size size;          ///!< image size
        int channelCount;   ///!< image channel count
        int imageCount;     ///!< image count (e.g. when writing color images as .pgm gray-scale images)
      };
    };
  } // namespace io
}

