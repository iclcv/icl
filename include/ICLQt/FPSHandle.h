/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/FPSHandle.h                              **
** Module : ICLQt                                                  **
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
*********************************************************************/

#ifndef ICL_GUI_FPS_H
#define ICL_GUI_FPS_H

#include <string>
#include <QString>
#include <QWidget>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>
#include <ICLUtils/FPSEstimator.h>

namespace icl{


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
    void update(){
      lab()->setText(m_oFpsEstimator.getFPSString().c_str());
      lab()->updateFromOtherThread();
    }
    
    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }
    
    FPSEstimator m_oFpsEstimator;
  };

 
  
 
}                               

#endif
