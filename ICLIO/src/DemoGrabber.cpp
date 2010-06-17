/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/DemoGrabber.cpp                              **
** Module : ICLIO                                                  **
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

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/Mathematics.h>

namespace icl{

  namespace{
    template<class T>
    void rect(Img<T> &image, const Color &color, const Rect &r){
      for(int i=0;i<image.getChannels() && i < 3;++i){
        T val = clipped_cast<icl8u,T>(color[i]);
        for(int y=r.y; y<r.y+r.height; ++y){
          T *data = image.getROIData(i,Point(r.x,y));
          std::fill(data,data+r.width,val);
        }
      }
    }

    float signum(float n){
      if (n < 0) return -1;
      if (n > 0) return 1;
      return 0;
    }
  }
  

  DemoGrabberImpl::DemoGrabberImpl(float maxFPS){
    m_x = Point32f(0.5,0.5);
    m_v = Point32f(0.01, 0.01);
    m_color = Color(255,50,10);
    m_size = Size32f(0.06,0.09);
    m_maxFPS = maxFPS;
    m_maxV = Point32f(0.2,0.2);
    m_lastTime = Time::now();
  }
  const ImgBase* DemoGrabberImpl::grabUD(ImgBase **ppoDst){
    ImgBase *image = prepareOutput(ppoDst);
    image->clear();

    m_v += Point32f(icl::random(-0.001, 0.001),icl::random(-0.001, 0.001));

    m_v.x = clip(m_v.x,-m_maxV.x,m_maxV.x); 
    m_v.y = clip(m_v.y,-m_maxV.y,m_maxV.y); 

    m_x += m_v;
    if(m_x.x>1 || m_x.x<0){
      m_v.x *= -1;
      if(m_x.x>1){
        m_x.x = 1;
      }else{
        m_x.x = 0;
      }
    }
    if(m_x.y>1 || m_x.y<0){
      m_v.y *= -1;
      if(m_x.y>1){
        m_x.y = 1;
      }else{
        m_x.y = 0;
      }

    }
    Size s = image->getSize();
    Rect r((int)((m_x.x-m_size.width)*s.width),
           (int)((m_x.y-m_size.height)*s.height),
           (int)(m_size.width*s.width),
           (int)(m_size.height*s.height));
    r &= image->getImageRect();
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: rect(*image->asImg<icl##D>(),m_color,r); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
    
    Time now = Time::now();
    Time neededInterval = Time(1000000)/m_maxFPS;
    if((now-m_lastTime) < neededInterval){
      Time restSleepTime = neededInterval-(now-m_lastTime);
      Thread::msleep(restSleepTime.toMilliSeconds());
    }
    
    image->setTime(now);
    m_lastTime = now;
    
    return image;
    
  }

  

}

