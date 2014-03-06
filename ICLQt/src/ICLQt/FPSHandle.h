/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/FPSHandle.h                            **
** Module : ICLQt                                                  **
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
#include <ICLUtils/FPSEstimator.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>
#include <QtCore/QString>
#include <QtGui/QWidget>
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

