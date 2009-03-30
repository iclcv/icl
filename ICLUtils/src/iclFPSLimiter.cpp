#include <iclFPSLimiter.h>
#include <iclThread.h>

namespace icl{
  
  namespace{
    struct StackedBoolTrueSetter{
      bool &m_b;
      bool m_needSetToFalse;
      StackedBoolTrueSetter(bool &b):m_b(b){
        if(!b){
          m_b=true;
          m_needSetToFalse = true;
        }else{
          m_needSetToFalse = false;
        }
      }
      ~StackedBoolTrueSetter(){
        if(m_needSetToFalse){
          m_b=false;
        }
      }
    };
  }
  
  FPSLimiter::FPSLimiter(float maxFPS, int fpsEstimationInterval):
    FPSEstimator(fpsEstimationInterval),m_maxFPS(maxFPS),m_waitOff(false){}
  
  float FPSLimiter::wait() const{
    if(m_waitOff) return 0;
    float desiredInterval = (1000000.0/m_maxFPS);
    Time now = Time::now();
    float interval =  now.toMicroSecondsDouble() - m_lastTime.toMicroSecondsDouble();

    float timeToWait = desiredInterval - interval;
    if(timeToWait > 0){
      Thread::usleep((unsigned int)timeToWait);
    }
    m_lastTime = Time::now();
    return timeToWait;
  }
  
  void FPSLimiter::tic() const{
    wait();
    StackedBoolTrueSetter x(m_waitOff);
    FPSEstimator::tic();
  }
  
  float FPSLimiter::getFPSVal() const{
    wait();
    StackedBoolTrueSetter x(m_waitOff);
    return FPSEstimator::getFPSVal();
  }
     
  std::string FPSLimiter::getFPSString(const std::string &fmt, int bufferSize) const{
    wait();
    StackedBoolTrueSetter x(m_waitOff);
    return FPSEstimator::getFPSString(fmt,bufferSize);
  }
    
  void FPSLimiter::showFPS(const std::string &text) const{
    wait();
    StackedBoolTrueSetter x(m_waitOff);
    FPSEstimator::showFPS(text);
  }

}
