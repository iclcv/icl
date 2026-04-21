// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/OptrisGrabber.h>
#include <libirimager/IRImager.h>
#include <icl/utils/File.h>
#include <icl/utils/Thread.h>
#include <icl/core/Img.h>
#include <icl/utils/XML.h>
#include <icl/io/FileList.h>
#include <icl/core/PseudoColorConverter.h>
#include <icl/filter/LocalThresholdOp.h>

#include <icl/io/V4L2Grabber.h>
#include <icl/io/ColorFormatDecoder.h>
#include <icl/math/LinearTransform1D.h>
#include <fstream>
#include <mutex>

namespace optris {}
namespace evo {}

namespace icl{
  using namespace utils;
  using namespace math;
  using namespace core;

  namespace io{
    using namespace optris;
    using namespace evo;

#ifdef ICL_HAVE_LIBIRIMAGER_EVO
#endif



    namespace{
      struct Buffer{
        ColorFormatDecoder decoder;
        std::vector<unsigned char> buf;
        Img32f image;
        Img32f outBuf;
        std::recursive_mutex mutex;
        Time lastTimeAcquired;

        Img8u visibleFrame;
        Img8u visibleOutBuf;
        OptrisGrabber::Mode mode;

        ImgBase &getDisplay() { return mode == OptrisGrabber::IR_IMAGE ? (ImgBase&)image :  (ImgBase&)visibleFrame; }
        ImgBase &getOutBuf() { return mode == OptrisGrabber::IR_IMAGE ?  (ImgBase&)outBuf : (ImgBase&)visibleOutBuf; }
        void deepCopy(){
          if(mode == OptrisGrabber::IR_IMAGE){
            image.deepCopy(&outBuf);
          }else{
            visibleFrame.deepCopy(&visibleOutBuf);
          }
        }
        Buffer(){
          image = Img32f(Size(1,1),1);
          visibleFrame = Img8u(Size::VGA,formatRGB);
        }
      };
    }


    void frame_callback(unsigned short* data, unsigned int w, unsigned int h, long long timestamp, void *arg){
      Buffer &b = *reinterpret_cast<Buffer*>(arg);
      b.image.setChannels(1);
      b.image.setSize(Size(w,h));
      //      b.image.setTime(Time(timestamp));
      b.image.setTime(Time::now()); // the timestamp has some other meaning!
      Channel32f c = b.image[0];
      for(size_t i=0;i<w*h;++i){
        c[i] = 0.1*(float(data[i])-1000.f);
      }
    }

    void visible_frame_callback(unsigned char* data, unsigned int w, unsigned int h, long long timestamp, void *arg){
      Buffer &b = *reinterpret_cast<Buffer*>(arg);
      b.decoder.decode("YUYV", data, Size(w,h), bpp(b.visibleFrame));
      b.visibleFrame.setTime(Time::now());
    }

    struct OptrisGrabber::Data : public utils::Thread{
      std::shared_ptr<IRImager> imager;
      Buffer buffer;
      PseudoColorConverter pcc;
      Img8u pccSrc;
      Img8u pccOutNull;
      Img32f pccOutNull32f;
      Img32f combinedImage;
      filter::LocalThresholdOp lt;

      const ImgBase *convert_output(const Img32f &s, const std::string &fmt){
        if(fmt == "Temperature celsius [32f]"){
          return &s;
        }
        // find min and max
        Range32f r = s.getMinMax();

        const Img8u *pcImage = 0;

        pccSrc.setChannels(1);
        pccSrc.setSize(s.getSize());
        pccOutNull.setFormat(formatRGB);
        pccOutNull.setSize(s.getSize());
        pccOutNull32f.setChannels(4);
        pccOutNull32f.setSize(s.getSize());

        if(!r.getLength()){
          if(fmt == "Pseudo Color + Temp. [RGBT 32f]"){
            pccOutNull32f.fill(0);
            pccOutNull32f.setTime(s.getTime());
            return &pccOutNull32f;
          }else{
            pccOutNull.fill(0);
            pccOutNull.setTime(s.getTime());
            return &pccOutNull;
          }
        }
        LinearTransform1D t(r, Range32f(0,255));
        const Channel32f cs = s[0];
        Channel8u d = pccSrc[0];
        for(int i=0;i<d.getDim();++i){
          d[i] = (icl8u)t(cs[i]);
        }

        pcImage =  &pcc.apply(pccSrc);

        if(fmt == "Pseudo Color + Temp. [RGBT 32f]"){
          combinedImage.setChannels(4);
          combinedImage.setSize(s.getSize());
          for(int c=0;c<3;++c){
            convertChannel(pcImage, c, &combinedImage, c);
          }
          deepCopyChannel(&s,0,&combinedImage,3);

          return &combinedImage;
        }else{
          return pcImage;
        }
      }


      std::string  init(const std::string &serialPattern, OptrisGrabber::Mode mode){
        buffer.mode = mode;

        FileList cfgs("/usr/share/libirimager/cali/Cali-*.xml");
        if(cfgs.size() > 2){
          WARNING_LOG("note: if you face problem instantiating your camera, please consider moving unused "
                      "calibration files /usr/share/libirimager/cali/Cali-*.xml to somewhere else");
        }
        int64_t serial = parse<int64_t>(serialPattern);

        std::string fn = "/usr/share/libirimager/cali/Cali-"+str(serial)+".xml";
        File f(fn);
        if(!f.exists()){
          throw ICLException("missing calibration file:" + fn);
        }
        XMLDocument doc;
        doc.load_file(fn.c_str());
        XPathNode xn = doc.select_single_node("/CaliData/Temperature/Optics/OpticsDef/FOV");
        int fov = 72;

        if(xn && xn.node() && xn.node().first_child()){
          fov = parse<int>(xn.node().first_child().value());
        }else{
          throw ICLException("could not parse calibration file " + fn +
                             " missing entry /CaliData/Temperature/Optics/OpticsDef/FOV");
        }

        std::string v4lDev;
        FileList ds("/dev/video*");
        for(int d=0;d<ds.size();++d){
          std::string deviceName = V4L2Grabber(ds[d]).getPropertyValue("device name");
          DEBUG_LOG("Devicename for " + ds[d] + ": " + deviceName);
          if(match(ds[d], "/dev/video([0-9]+)",2) && match(deviceName, ".*PI-IMAGER.*",2)){
            v4lDev = ds[d];
          }else{
            continue;
          }

          const int framerate = (mode == VISIBLE_IMAGE) ? 32 : 120;
          static const int videoformatindex = 0;

          bool verbose = false;
          imager = new IRImager(verbose);

          std::cout << "initializing IRImager device " << v4lDev << std::endl;
          imager->init(v4lDev.c_str(), 0, videoformatindex, HIDController,
                       fov, TM20_100, framerate, Temperature, mode == VISIBLE_IMAGE ? 1 : 0);

          if(!imager->hasBispectralTechnology() && mode == VISIBLE_IMAGE){
            throw ICLException("the device does not supoort bispectral technology, so color images cannot be aquired!");
          }

          if((int64_t)imager->getSerial() != serial){
            DEBUG_LOG("serials do not match! trying next v4l device (if there is any)");
            v4lDev = "";
            continue;
          }
        }
        if(!v4lDev.length()){
          throw ICLException("could not find any v4l device for serial " + str(serial));
        }
        if(!imager->isOpen()){
          throw ICLException("could not open device /dev/video" + str(v4lDev) +
                             " with serial " + str(serial));
        }
        return v4lDev;
      }

      void start_capturing(){
        buffer.buf.resize(imager->getRawBufferSize());

        if(buffer.mode == VISIBLE_IMAGE){
          imager->setVisibleFrameCallback(visible_frame_callback);
          //imager->setFrameCallback(frame_callback);
        }else{
          imager->setFrameCallback(frame_callback);
        }
        if(imager->startStreaming() == IRIMAGER_DISCONNECTED){
          throw ICLException("could not connect to camera: please re-connect device");
        }else{
          //DEBUG_LOG("IRImages started steaming");
        }
        start();
      }

      Size getSize() {
        return Size(imager->getWidth(), imager->getHeight());
      }


      virtual void run(){
        while(true){
          {
            std::scoped_lock<std::recursive_mutex> lock(buffer.mutex);
            if(imager->getFrame(buffer.buf.data()) == IRIMAGER_SUCCESS){
              imager->process(buffer.buf.data(), &buffer);
              imager->releaseFrame();
            }
          }
          Thread::msleep(5);
        }
      }

    };



    OptrisGrabber::OptrisGrabber(const std::string &serialPattern, bool testOnly,
                                 Mode mode) : m_data(new Data){
      std::string v4lDev = m_data->init(serialPattern,mode);
      addProperty("v4l device","info","",v4lDev);
      if(mode == IR_IMAGE){
        addProperty("format","menu","Temperature celsius [32f],Pseudo Color [RGB8],Pseudo Color + Temp. [RGBT 32f]","Temperature celsius [32f]");
        addProperty("size","menu",str(m_data->getSize()),m_data->getSize());
      }else{
        addProperty("format","menu","RGB 8");
        addProperty("size","menu","640x480",Size::VGA);
      }
      addProperty("omit doubled frames","flag","",true);
      addProperty("threshold output","flag","",false);
      addChildConfigurable(&m_data->lt,"thresh");
      if(!testOnly){
        m_data->start_capturing();
      }
    }

    OptrisGrabber::~OptrisGrabber() {
      m_data->stop();
      delete m_data;
    }


    const std::vector<GrabberDeviceDescription> &OptrisGrabber::getDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> all;
      if(rescan){
        all.clear();
        FileList cfgs("/usr/share/libirimager/cali/Cali-*.xml");

        for(int c=0;c<cfgs.size();++c){
          //std::cout << "processing cali file " << cfgs[c] << std::endl;
          MatchResult r = match(cfgs[c], "Cali-([0-9]+).xml",2);
          if(r && r.submatches.size() == 2){
            try{
              std::string s = r.submatches[1];
              //              std::cout << "trying to create grabber " << s << std::endl;
              OptrisGrabber g(s,true);
              //std::cout << "--> creation successful" << std::endl;
              all.push_back(GrabberDeviceDescription("optris",s,
                                                     "IR-IMAGER (serial: " +s+
                                                     " @ " + g.getPropertyValue("v4l device") +")"));
              all.push_back(GrabberDeviceDescription("optrisv",s,
                                                     "IR-IMAGER (serial: " +s+
                                                     " @ " + g.getPropertyValue("v4l device") +")"));

            }catch(std::exception &ex){ /* Combination did not work*/
              //std::cout << "--> creation threw exception ex:-" << ex.what() << "-" << std::endl;
            }
          }
        }
      }
      return all;
    }

    const core::ImgBase* OptrisGrabber::acquireDisplay(){
      bool omitDoubledFrames = getPropertyValue("omit doubled frames");

      std::scoped_lock<std::recursive_mutex> lock(m_data->buffer.mutex);
      if(omitDoubledFrames){
        while(m_data->buffer.getDisplay().getTime() == m_data->buffer.lastTimeAcquired){
          m_data->buffer.mutex.unlock();
          Thread::msleep(1);
          m_data->buffer.mutex.lock();
        }
      }
      m_data->buffer.lastTimeAcquired = m_data->buffer.getDisplay().getTime();
      m_data->buffer.deepCopy();

      const ImgBase *cvt = 0;
      if(m_data->buffer.mode == IR_IMAGE){
        cvt = m_data->convert_output(m_data->buffer.outBuf,
                                     getPropertyValue("format"));
      }else{
        cvt = &m_data->buffer.visibleOutBuf;
      }

      if(getPropertyValue("threshold output")){
        static ImgBase *ltBuf = 0;
        m_data->lt.apply(cvt, &ltBuf);
        cvt = ltBuf;
      }

      return cvt;
    }

    void OptrisGrabber::processPropertyChange(const utils::Configurable::Property &prop){

    }

    template<OptrisGrabber::Mode M>
    static Grabber *create_optris_grabber(const std::string &param){
      return new OptrisGrabber(param,false,M);
    }

    template<OptrisGrabber::Mode M>
    const std::vector<GrabberDeviceDescription> &create_optris_grabber_device_list(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(!devices.size() || rescan){
        const std::vector<GrabberDeviceDescription> &get = OptrisGrabber::getDeviceList(hint,rescan);
        std::string s = str("optris") + (M == OptrisGrabber::IR_IMAGE ? "" : "v");
        for(size_t i=0;i<get.size();++i){
          if(get[i].type == s) devices.push_back(get[i]);
          //std::cout << "XXX device list[" << i << "] := " << get[i].type << " id: " << get[i].id << std::endl;
        }
      }
      return devices;
    }

    REGISTER_GRABBER(optris,create_optris_grabber<OptrisGrabber::IR_IMAGE>,
                     create_optris_grabber_device_list<OptrisGrabber::IR_IMAGE>,
                     "optris:camera serial ID or pattern:LibImager-based camera grabber source (ir camera)");

    REGISTER_GRABBER(optrisv,create_optris_grabber<OptrisGrabber::VISIBLE_IMAGE>,
                     create_optris_grabber_device_list<OptrisGrabber::VISIBLE_IMAGE>,
                     "optrisv:camera serial ID or pattern:LibImager-based camera grabber source (color camera)");

    //REGISTER_GRABBER_BUS_RESET_FUNCTION(xi,reset_xi_bus);
  } // namespace io
}
