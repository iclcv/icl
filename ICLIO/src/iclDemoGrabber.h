#ifndef ICL_DEMO_GRABBER_H
#define ICL_DEMO_GRABBER_H

#include <iclGrabber.h>
#include <iclColor.h>
#include <iclTime.h>
#include <iclSize32f.h>

namespace icl{
  /// Demo Grabber implementation providing am image with a moving rect
  /** This grabber can be used as placeholder whenever no senseful Grabber
      is available. It can be set up to work at a certain fps to avoid
      some real unexpected behaviour*/
  class DemoGrabber : public Grabber{
    public:
    /// Create a DemoGrabber with given max. fps count
    DemoGrabber(float maxFPS=30);
    
    /// default grab function
    virtual const ImgBase* grab(ImgBase **ppoDst=0);
    
    private:
    /// Current rel. location of the rect
    Point32f m_x; 
    
    /// Current rel. velocity of the rect
    Point32f m_v;

    /// relative size of the rect
    Size32f m_size;

    /// Color of the rect (light red)
    Color m_color;
    
    /// max. fpsCount for this grabber instance
    float m_maxFPS;
    
    /// time variable to ensure max. fpsCount
    Time m_lastTime;
    
  };  
}

#endif
