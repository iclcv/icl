/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLUtils/FPSEstimator.h>
#include <ICLUtils/Macros.h>
#include <cstdio>
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
