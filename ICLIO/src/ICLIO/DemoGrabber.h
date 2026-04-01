// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>
#include <ICLCore/Color.h>

#include <ICLIO/Grabber.h>
#include <mutex>

namespace icl{
  namespace io{


    /// Demo Grabber class providing am image with a moving rect
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It can be set up to work at a certain fps to avoid
        some real unexpected behaviour */
    class ICLIO_API DemoGrabber : public Grabber{
      public:
        /// default grab function
        virtual const core::ImgBase* acquireDisplay();

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
        std::recursive_mutex m_mutex;

      public:
        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };

  } // namespace io
}
