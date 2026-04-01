// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/FPSEstimator.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>
#include <QtCore/QString>
#include <QWidget>
#include <string>

namespace icl{
  namespace qt{


    /// Class for Frames-per-second GUI-Label \ingroup HANDLES
    /** FPSHandles are created by the GUI class. FPSHandles must be notified
        each step using its
        @see GUI */
    class FPSHandle : public GUIHandle<CompabilityLabel>{
      public:

      // create an empty handle
      FPSHandle():GUIHandle<CompabilityLabel>(),m_oFpsEstimator(1){}

      /// Create a new LabelHandle
      FPSHandle(int fpsEstimatorFrames,CompabilityLabel *l, GUIWidget *w):
      GUIHandle<CompabilityLabel>(l,w),m_oFpsEstimator(fpsEstimatorFrames){}

      // notifies and updates the internal fps estimator and the shown fps-string
      void render(){
        lab()->setText(m_oFpsEstimator.getFPSString().c_str());
        lab()->updateFromOtherThread();
      }

      private:
      /// utitlity function
      CompabilityLabel *lab() { return **this; }

      utils::FPSEstimator m_oFpsEstimator;
    };




  } // namespace qt
}
