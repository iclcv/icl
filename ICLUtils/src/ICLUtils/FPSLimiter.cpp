/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/FPSLimiter.cpp                   **
** Module : ICLUtils                                               **
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

#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Thread.h>

namespace icl{
  namespace utils{
    
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
  
  } // namespace utils
}
