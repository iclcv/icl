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
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Random.h>

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
    m_size = Size32f(0.05,0.05);
    m_maxFPS = maxFPS;
    m_maxV = Point32f(0.2,0.2);
    m_lastTime = Time::now();
    
    m_drawBuffer = 0;
    m_drawFormat = formatRGB;
    m_drawSize = Size::VGA;
    m_drawDepth = depth8u;
  }

  DemoGrabberImpl::~DemoGrabberImpl(){
    ICL_DELETE(m_drawBuffer);
  }
  
  
  template<class T>
  void erode_buffer(Img<T> &t){
    t.transform(std::bind2nd(std::multiplies<float>(),0.99),t);
  }
  
  const ImgBase* DemoGrabberImpl::grabUD(ImgBase **ppoDst){
    Mutex::Locker __lock(m_mutex);
    ensureCompatible(&m_drawBuffer,m_drawDepth,m_drawSize,m_drawFormat);

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
    Size s = m_drawBuffer->getSize();
    Rect r((int)((m_x.x-m_size.width)*s.width),
           (int)((m_x.y-m_size.height)*s.height),
           (int)(m_size.width*s.width),
           (int)(m_size.height*s.height));
    r &= m_drawBuffer->getImageRect();
    
    if(m_drawBuffer->getDepth() == depth8u){
      rect(*m_drawBuffer->asImg<icl8u>(),m_color,r); 
    }else{
      rect(*m_drawBuffer->asImg<icl32f>(),m_color,r); 
    }
    
    if(m_drawBuffer->getDepth() == depth8u){
      erode_buffer(*m_drawBuffer->asImg<icl8u>());;
    }else{
      erode_buffer(*m_drawBuffer->asImg<icl32f>());;
    }

    Time now = Time::now();
    Time neededInterval = Time(1000000)/m_maxFPS;
    if((now-m_lastTime) < neededInterval){
      Time restSleepTime = neededInterval-(now-m_lastTime);
      Thread::msleep(restSleepTime.toMilliSeconds());
    }
    
    m_drawBuffer->setTime(now);
    m_lastTime = now;

    if(!getIgnoreDesiredParams()){
      ImgBase *image = prepareOutput(ppoDst);
      m_oConverter.apply(m_drawBuffer,image);
      return image;
    }else{
      if(!ppoDst) return m_drawBuffer;
      m_drawBuffer->deepCopy(ppoDst);
      return *ppoDst;
    }   
  }


  std::vector<std::string> DemoGrabberImpl::getPropertyList(){
    std::vector<std::string> ps;
    ps.push_back("blob-size");
    ps.push_back("blob-red");
    ps.push_back("blob-green");
    ps.push_back("blob-blue");
    ps.push_back("max-speed");
    ps.push_back("set-to-center");
    ps.push_back("current-pos");
    ps.push_back("format");
    ps.push_back("size");
    return ps;
  }

  std::string DemoGrabberImpl::getInfo(const std::string &name){
    if(name == "blob-size"){
      return "{\"5% of image size\",\"10% of image size\",\"20% of image size\"}";
    }else if(name == "format"){
      return "{\"formatRGB-depth8u\",\"formatRGB-depth32f\",\"formatGray-depth8u\",\"formatGray-depth32f\",\"formatYUV-depth8u\"}";
    }else if(name == "size"){
      return "{\"VGA\",\"SVGA\",\"QVGA\"}";
    }else if(name == "blob-red"){
      return "[0,255]:1";
    }else if(name == "blob-green"){
      return "[0,255]:1";
    }else if(name == "blob-blue"){
      return "[0,255]:1";
    }else if(name == "max-speed"){
      return "{0.1,0.2,0.3,0.4}";
    }else if(name == "set-to-center"){
      return "command";
    }else if(name == "current-pos"){
      return "undefined";
    }else{
      return "undefined";
    }
    
  }

  int DemoGrabberImpl::isVolatile(const std::string &propertyName){
    if(propertyName == "current-pos"){
      return 100;
    }else{
      return 0;
    }
  }

  
  void DemoGrabberImpl::setProperty(const std::string &property, const std::string &value){
    Mutex::Locker __lock(m_mutex);
    if(property == "blob-size"){
      int percent = parse<int>(value);
      m_size = Size32f(percent/100.,percent/100.);
    }else if(property == "format"){
      std::vector<std::string> x = tok(value,"-");
      if(x.size() != 2){
        ERROR_LOG("invalid value for prorerty \"format\"" << value);
      }else{
        m_drawFormat = parse<format>(x[0]);
        m_drawDepth = parse<depth>(x[1]);
      }
    }else if(property == "size"){
      m_drawSize = parse<Size>(value);
    }else if(property == "blob-red"){
      m_color[0] = parse<int>(value);
    }else if(property == "blob-green"){
      m_color[1] = parse<int>(value);
    }else if(property == "blob-blue"){
      m_color[2] = parse<int>(value);
    }else if(property == "max-speed"){
      float m = parse<float>(value);
      m_maxV.x = m_maxV.y = m;
    }else if(property == "set-to-center"){
      m_x.x = 0.5;
      m_x.y = 0.5;
    }else if(property == "current-pos"){
      ERROR_LOG("property \"current-pos\" cannot be set!");
    }
  }
  
  std::string DemoGrabberImpl::getType(const std::string &name){
    if(name == "blob-size" || name == "format" || name == "size") return "menu";
    if(name == "blob-red" || name == "blob-green" || name == "blob-blue") return "range";
    if(name == "max-speed") return  "value-list";
    if(name == "set-to-center") return "command";
    if(name == "current-pos") return "info";
    DEBUG_LOG("nothing known about property \"" << name << "\"");
    return "undefined";
  }
  

  std::string DemoGrabberImpl::getValue(const std::string &name){
    Mutex::Locker __lock(m_mutex);
    if(name == "blob-size"){
      if( fabs(m_size.width-0.05< 1E-5) ) return "5% of image size";
      if( fabs(m_size.width-0.10< 1E-5) ) return "10% of image size";
      if( fabs(m_size.width-0.20< 1E-5) ) return "20% of image size";
      else {
        ERROR_LOG("invalid value for property \"blob-size\" detected [this should not happen]");
        return "undefined";
      }
    }else if(name == "format"){
      return str(m_drawFormat) + "-" + str(m_drawDepth);
    }else if(name == "size"){
      if(m_drawSize == Size::QVGA) return "QVGA";
      if(m_drawSize == Size::VGA) return "VGA";
      if(m_drawSize == Size::SVGA) return "SVGA";
      else{
        ERROR_LOG("invalid value for property \"size\" detected [this should not happen]");
        return "undefined";
      }
    }else if(name == "blob-red"){
      return str(m_color[0]);
    }else if(name == "blob-green"){
      return str(m_color[1]);
    }else if(name == "blob-blue"){
      return str(m_color[2]);
    }else if(name == "max-speed"){
      return str(m_maxV.x);
    }else if(name == "current-pos"){
      return "x:" + str(m_x.x*m_drawSize.width) + " y:" + str(m_x.y*m_drawSize.height);
    }
    return "undefined";
  }

  

}

