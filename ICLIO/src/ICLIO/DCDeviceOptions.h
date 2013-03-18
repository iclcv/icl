/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCDeviceOptions.h                      **
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

#include <ICLIO/DC.h>

namespace icl{
  namespace io{
    
    /// Utility struct for DC Camera device options \ingroup DC_G
    class DCDeviceOptions{
      public:
      
      /// bayer method
      dc1394bayer_method_t bayermethod;
  
      /// framerate
      dc1394framerate_t framerate;
  
      /// video mode
      dc1394video_mode_t videomode;
  
      /// flag whether images should be labeled or not
      bool enable_image_labeling;
  
      /// iso MBits
      int isoMBits;
      
      /// if set, each frame can be grabbed only once
      bool suppressDoubledImages;
    };
  } // namespace io
}

