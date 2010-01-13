#ifndef ICL_DEMO_GRABBER_H
#define ICL_DEMO_GRABBER_H

#include <ICLCC/Color.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/GrabberHandle.h>

namespace icl{

  
  /// Implementation class for the DemoGrabber
  class DemoGrabberImpl : public Grabber{
    public:
    friend class DemoGrabber;

    /// default grab function
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);
    
    private:
    /// Create a DemoGrabber with given max. fps count
    DemoGrabberImpl(float maxFPS=30);

    /// Current rel. location of the rect
    Point32f m_x; 
    
    /// Current rel. velocity of the rect
    Point32f m_v;

    /// maximum velocity of the rect
    Point32f m_maxV;

    /// relative size of the rect
    Size32f m_size;

    /// Color of the rect (light red)
    Color m_color;
    
    /// max. fpsCount for this grabber instance
    float m_maxFPS;
    
    /// time variable to ensure max. fpsCount
    Time m_lastTime;
    
  };  
  /** \endcond */

  /// Demo Grabber class providing am image with a moving rect
  /** This grabber can be used as placeholder whenever no senseful Grabber
      is available. It can be set up to work at a certain fps to avoid
      some real unexpected behaviour */
  class DemoGrabber : public GrabberHandle<DemoGrabberImpl>{
    public:
    inline DemoGrabber(float maxFPS=30){
      std::string id = str(maxFPS);
      if(isNew(id)){
        initialize(new DemoGrabberImpl(maxFPS),id);
      }else{
        initialize(id);
      }
    }
  };
}

#endif
