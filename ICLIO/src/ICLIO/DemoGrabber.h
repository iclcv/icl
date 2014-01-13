/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DemoGrabber.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>
#include <ICLCore/Color.h>

#include <ICLIO/Grabber.h>

namespace icl{
  namespace io{

    
    /// Demo Grabber class providing am image with a moving rect
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It can be set up to work at a certain fps to avoid
        some real unexpected behaviour */
    class ICL_IO_API DemoGrabber : public Grabber{
      public:
        /// default grab function
        virtual const core::ImgBase* acquireImage();

        /// Create a DemoGrabber with given max. fps count
        DemoGrabber(float maxFPS=30);

        /// Destructor
        ~DemoGrabber();

      private:

        /// Current rel. location of the rect
        utils::Point32f m_x;

        /// Current rel. velocity of the rect
        utils::Point32f m_v;

        /// maximum velocity of the rect
        utils::Point32f m_maxV;

        /// relative size of the rect
        utils::Size32f m_size;

        /// Color of the rect (light red)
        core::Color m_color;

        /// max. fpsCount for this grabber instance
        float m_maxFPS;

        /// time variable to ensure max. fpsCount
        utils::Time m_lastTime;

        /// extra buffer for the output image
        core::ImgBase *m_drawBuffer;

        /// current output format
        core::format m_drawFormat;

        /// current output depth
        core::depth m_drawDepth;

        /// current output size
        utils::Size m_drawSize;

        /// mutex for locking properties and grabbing
        utils::Mutex m_mutex;

      public:
        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };

  } // namespace io
}

