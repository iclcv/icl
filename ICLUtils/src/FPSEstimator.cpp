/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/FPSEstimator.cpp                          **
** Module : ICLUtils                                               **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
