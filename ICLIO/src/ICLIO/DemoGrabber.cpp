// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <ICLUtils/Thread.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Point32f.h>
#include <ICLIO/DemoGrabber.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

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
    }


    DemoGrabber::DemoGrabber(float maxFPS)
      : m_mutex()
    {
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

      // Configurable
      std::string blobvalue;
      if(fabs(m_size.width-0.05) < 1E-5){
        blobvalue = "5% of image size";
      } else if (fabs(m_size.width-0.10) < 1E-5){
        blobvalue = "10% of image size";
      } else if (fabs(m_size.width-0.20) < 1E-5){
        blobvalue = "20% of image size";
      }
      std::string sizevalue;
      if(m_drawSize == Size::QVGA) sizevalue = "QVGA";
      if(m_drawSize == Size::VGA) sizevalue = "VGA";
      if(m_drawSize == Size::SVGA) sizevalue = "SVGA";
      addProperty("blob-size", "menu",
                  "5% of image size,10% of image size,20% of image size",
                  blobvalue, 0, "The size of the blob.");
      addProperty("blob-red", "range", "[0,255]:1", m_color[0], 0,
                  "The amount of red color in the blob.");
      addProperty("blob-green", "range", "[0,255]:1", m_color[1], 0,
                  "The amount of green color in the blob.");
      addProperty("blob-blue", "range", "[0,255]:1", m_color[2], 0,
                  "The amount of blue color in the blob.");
      addProperty("max-speed", "value-list", "0.1,0.2,0.3,0.4", m_maxV.x, 0,
                  "The blobs maximum speed.");
      addProperty("set-to-center", "command", "", "", 0,
                  "Resets the blob to the image center.");
      addProperty("current-pos", "info", "",
                  "x:" + str(m_x.x*m_drawSize.width) + " y:"
                  + str(m_x.y*m_drawSize.height),
                  10, "The current position of the blob.");
      addProperty("format", "menu",
                  "formatRGB-depth8u,formatRGB-depth32f,formatGray-depth8u,"
                  "formatGray-depth32f,formatYUV-depth8u",
                  str(m_drawFormat) + "-" + str(m_drawDepth), 0,
                  "The image format.");
      addProperty("size", "menu", "VGA,SVGA,QVGA", sizevalue, 0,
                  "The image size.");
      Configurable::registerCallback(
            [this](const utils::Configurable::Property &p){ processPropertyChange(p); });
    }

    DemoGrabber::~DemoGrabber(){
      ICL_DELETE(m_drawBuffer);
    }


    template<class T>
    void erode_buffer(Img<T> &t){
      t.transform([](float v){ return v * 0.99f; },t);
    }

    const ImgBase* DemoGrabber::acquireDisplay(){
      std::lock_guard<std::recursive_mutex> __lock(m_mutex);
      ensureCompatible(&m_drawBuffer,m_drawDepth,m_drawSize,m_drawFormat);

      m_v += Point32f(utils::random(-0.001, 0.001),utils::random(-0.001, 0.001));

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
      Rect r(static_cast<int>((m_x.x-m_size.width)*s.width),
             static_cast<int>((m_x.y-m_size.height)*s.height),
             static_cast<int>(m_size.width*s.width),
             static_cast<int>(m_size.height*s.height));
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

      prop("current-pos").value = "x:" + str(m_x.x*m_drawSize.width) + " y:"
                       + str(m_x.y*m_drawSize.height);

      return m_drawBuffer;
    }

    // callback for changed configurable properties
    void DemoGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "current-pos"){
        // do nothing. info field
        return;
      }
      if(prop.name == "blob-red"){
        m_color[0] = parse<int>(prop.value);
      }else if(prop.name == "blob-green"){
        m_color[1] = parse<int>(prop.value);
      }else if(prop.name == "blob-blue"){
        m_color[2] = parse<int>(prop.value);
      }else if(prop.name == "blob-size"){
        int percent = parse<int>(prop.value);
        m_size = Size32f(percent/100.,percent/100.);
      }else if(prop.name == "format"){
        std::vector<std::string> x = tok(prop.value,"-");
        if(x.size() != 2){
          ERROR_LOG("invalid value for prorerty \"format\"" << prop.value);
        }else{
          m_drawFormat = parse<format>(x[0]);
          m_drawDepth = parse<depth>(x[1]);
        }
      }else if(prop.name == "size"){
        m_drawSize = parse<Size>(prop.value);
      }else if(prop.name == "max-speed"){
        float m = parse<float>(prop.value);
        m_maxV.x = m_maxV.y = m;
      }else if(prop.name == "set-to-center"){
        m_x.x = 0.5;
        m_x.y = 0.5;
      }
    }

    REGISTER_CONFIGURABLE(DemoGrabber, return new DemoGrabber(30));

    Grabber* createDemoGrabber(const std::string &param){
      float fps = to32f(param);
      return new DemoGrabber(fps > 0 ? fps : 30);
    }

    const std::vector<GrabberDeviceDescription>& getDemoDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
      deviceList.clear();
        if(hint.size()){
          deviceList.push_back(GrabberDeviceDescription("demo",
              hint,
              "a demo image source"));
        } else {
          deviceList.push_back(GrabberDeviceDescription("demo",
              "0",
              "a demo image source"));
        }
      }
      return deviceList;
    }

    REGISTER_GRABBER(demo,createDemoGrabber, getDemoDeviceList,"demo:0:demo image source");

  } // namespace io
}
