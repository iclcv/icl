#ifndef ICL_GUI_FPS_H
#define ICL_GUI_FPS_H

#include <string>
#include <QString>
#include <QWidget>
#include <iclGUIHandle.h>
#include <iclCompabilityLabel.h>
#include <iclFPSEstimator.h>

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
      lab()->setText(m_oFpsEstimator.getFpsString().c_str());
      lab()->updateFromOtherThread();
    }
    
    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }
    
    FPSEstimator m_oFpsEstimator;
  };

 
  
 
}                               

#endif
