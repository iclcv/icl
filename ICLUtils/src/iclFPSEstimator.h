#ifndef ICL_FPS_ESTIMATOR_H
#define ICL_FPS_ESTIMATOR_H

#include <iclTime.h>
#include <deque>
#include <string>

namespace icl{
  ///Utility clas for online FPS estimation \ingroup TIME
  class FPSEstimator{
    public:

    ///Constructor
    FPSEstimator(int n=1){
      reset(n);
    }
    void reset(int n){
      m_iN = n;
      for(int i=0;i<n;i++){
        m_qTimes.push_back(Time());
      }
    }
    
    std::string getFpsString(){
      m_qTimes.push_back(Time::now());
      m_qTimes.pop_front();
      

      if(m_qTimes.front()==Time::null){
        return "---.---- fps";
      }else{
        char acBuf[30];
        sprintf(acBuf,"%3.4f fps",(1000.0*(m_iN-1))/((m_qTimes.back()-m_qTimes.front()).toMilliSecondsDouble()));
        return acBuf;
      }
    }
    void showFps(const std::string &text=""){
      printf("%s:%s\n",text.c_str(),getFpsString().c_str());
    }
    
    private:
    std::deque<Time> m_qTimes;
    int m_iN;
  };

#define FPS_LOG_THIS_FUNCTION(N) static FPSEstimator __FPSEstimator__((N)); \
                                 __FPSEstimator__.showFps(__FUNCTION__);
  
}


#endif
