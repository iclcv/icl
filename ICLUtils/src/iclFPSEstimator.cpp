#include <iclFPSEstimator.h>
#include <iclMacros.h>

namespace icl{
  FPSEstimator::FPSEstimator(int n){
    reset(n);
  }
  
  void FPSEstimator::reset(int n){
    if(n<2){
      n = 2;
    }
    m_iN = n;
    m_qTimes.clear();
    for(int i=0;i<n;i++){
      m_qTimes.push_back(Time());
    }
  }
  
  void FPSEstimator::tic() const{
    m_qTimes.push_back(Time::now());
    m_qTimes.pop_front();
  }
    
  float FPSEstimator::getFPSVal() const{
    tic();
    if(m_qTimes.front()==Time::null){
      return -1;
    }else{
      double avgDt = 0;
      
      Time t = m_qTimes.front();
      for(std::deque<Time>::iterator it=++m_qTimes.begin();it!=m_qTimes.end();++it){
        avgDt += ((*it)-t).toMicroSecondsDouble();
        t = *it;
      }
      return 1.0/(avgDt/(1000000.0*(m_iN-1)));
    }
  }
    
  std::string FPSEstimator::getFPSString(const std::string &fmt, int bufferSize) const{
    char *buf = new char[bufferSize];
    sprintf(buf,fmt.c_str(),getFPSVal());
    std::string s(buf);
    delete [] buf;
    return s;

  }
    
  void FPSEstimator::showFPS(const std::string &text) const{
    printf("%s:%s\n",text.c_str(),getFPSString().c_str());
  }
  
}
