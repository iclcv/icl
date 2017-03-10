/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OptrisGrabber.cpp                      **
** Module : ICLIO                                                  **
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


#include <ICLIO/OptrisGrabber.h>
#include <libirimager/IRImager.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/File.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/Img.h>
#include <ICLUtils/XML.h>
#include <ICLIO/FileList.h>
#include <ICLCore/PseudoColorConverter.h>
#include <ICLFilter/LocalThresholdOp.h>

#include <ICLIO/V4L2Grabber.h>
#include <ICLMath/LinearTransform1D.h>
#include <fstream>

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
        std::vector<unsigned char> buf;
        Img32f image;
        Img32f outBuf;
        Mutex mutex;
        Time lastTimeAcquired;

        Img8u visibleFrame;
        Img8u visibleOutBuf;
        OptrisGrabber::Mode mode;

        ImgBase &getImage() { return mode == OptrisGrabber::IR_IMAGE ? (ImgBase&)image :  (ImgBase&)visibleFrame; }
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
          visibleFrame = Img8u(Size(1,1),1);
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
      b.image.setChannels(1);
      b.image.setSize(Size(w,h));
      //      b.image.setTime(Time(timestamp));
      b.image.setTime(Time::now()); // the timestamp has some other meaning!
      
      Channel8u c = b.visibleFrame[0];
      for(size_t i=0;i<w*h;++i){
        c[i] = data[i];
      }
    }

    struct OptrisGrabber::Data : public utils::Thread{
      SmartPtr<IRImager> imager;
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


      
      /* right now, this is not used. If we (later) must be able to specify the cali-path: use this here
      static std::string create_temp_calib_file(int serial, int minTemp, int maxTemp, int framerate, 
                                         int fov, const std::string &calibrationFileDir){
        std::string fn = "/tmp/temp_optris_calib_file_"+str(Time::now().toMilliSeconds())+".xml";
        {
          std::ofstream s(fn.c_str());
          s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<imager xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n"
            << "  <serial>" << serial << "</serial>\n"
            << "  <videoformatindex>0</videoformatindex> <!-- index of the used video format (USB enpoint) -->\n"
            << "  <fov>"<< fov <<"</fov>\n"
            << "  <temperature>\n"
            << "    <min>"<< minTemp << "</min>\n"
            << "    <max>"<< maxTemp << "</max>\n"
            << "  </temperature>\n"
          //<< "  <calipath>" << calibrationFileDir << "</calipath>\n"
            << "  <framerate>" << framerate << "</framerate>\n"
            << "  <controller>1</controller>  <!-- 1=HID, 2=UVC -->\n"
            << "  <outputmode>2</outputmode>  <!-- 1=Energy, 2=Temperature -->\n"
            << "  <bispectral>0</bispectral>  <!-- 0=only thermal sensor, 1=bispectral technology (only PI200/PI230) -->\n"
            << "  <average>0</average>        <!-- average callback frames over intermediate frames -->\n"
            << "</imager>" << std::endl;
        }
        return fn;
      }
        
      void del_temp_calib_file(const std::string &filename){
        File f(filename);
        if(f.exists()){
          f.erase();
        }
      }
      
          ...
      this would also be needed if a custom cali-path is neccessary{
          std::string fn = create_temp_calib_file(serial, -20, 100, 120, 72, 
                                                  "/usr/share/libirimager/cali");
          imager = new optris::IRImager(fn.c_str());
          del_temp_calib_file(fn);
       }
          */
      std::string  init(const std::string &serialPattern, OptrisGrabber::Mode mode) throw (utils::ICLException){
        buffer.mode = mode;
        
        FileList cfgs("/usr/share/libirimager/cali/Cali-*.xml");
       
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
          if(match(ds[d], "/dev/video([0-9]+)",2) &&
             V4L2Grabber(ds[d]).getPropertyValue("device name") == "PI-IMAGER"){
            v4lDev = ds[d];
          }else{
            continue;
          }
          
          const int framerate = (mode == VISIBLE_IMAGE) ? 32 : 120;
          static const int videoformatindex = 0;
          
          imager = new IRImager;

          imager->init(v4lDev.c_str(), 0, videoformatindex, HIDController, 
                       fov, TM20_100, framerate, Temperature, mode == VISIBLE_IMAGE ? 1 : 0);
          
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
        }else{
          imager->setFrameCallback(frame_callback);
        }
        imager->startStreaming();
        start();
      }
      
      Size getSize() {
        return Size(imager->getWidth(), imager->getHeight());
      }
        
      
      virtual void run(){
        while(true){
          {
            Mutex::Locker lock(buffer.mutex);
            imager->getFrame(buffer.buf.data());
            imager->process(buffer.buf.data(), &buffer);
            imager->releaseFrame();
          }
          Thread::msleep(5);
        }
      }

    };
    
 
    
    OptrisGrabber::OptrisGrabber(const std::string &serialPattern, bool testOnly,
                                 Mode mode) throw(utils::ICLException) : m_data(new Data){
      std::string v4lDev = m_data->init(serialPattern,mode);
      addProperty("v4l device","info","",v4lDev);
      addProperty("format","menu","Temperature celsius [32f],Pseudo Color [RGB8],Pseudo Color + Temp. [RGBT 32f]","Temperature celsius [32f]");
      addProperty("size","menu",str(m_data->getSize()),m_data->getSize());
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
          MatchResult r = match(cfgs[c], "Cali-([0-9]+).xml",2);
          if(r && r.submatches.size() == 2){
            try{
              std::string s = r.submatches[1];
              OptrisGrabber g(s,true);
              all.push_back(GrabberDeviceDescription("optris",s,
                                                     "IR-IMAGER (serial: " +s+ 
                                                     " @ " + g.getPropertyValue("v4l device") +")"));
              all.push_back(GrabberDeviceDescription("optrisv",s,
                                                     "IR-IMAGER (serial: " +s+ 
                                                     " @ " + g.getPropertyValue("v4l device") +")"));

            }catch(...){ /* Combination did not work*/
            }
          }
        }
      }
      return all;
    }

    const core::ImgBase* OptrisGrabber::acquireImage(){
      bool omitDoubledFrames = getPropertyValue("omit doubled frames");
      
      Mutex::Locker lock(m_data->buffer.mutex);
      if(omitDoubledFrames){
        while(m_data->buffer.getImage().getTime() == m_data->buffer.lastTimeAcquired){
          m_data->buffer.mutex.unlock();
          Thread::msleep(1);
          m_data->buffer.mutex.lock();
        }
      }
      m_data->buffer.lastTimeAcquired = m_data->buffer.getImage().getTime();
      m_data->buffer.deepCopy();

      const ImgBase *cvt = 0;
      if(m_data->buffer.mode == IR_IMAGE){
        cvt = m_data->convert_output(m_data->buffer.outBuf, 
                                     getPropertyValue("format"));
      }else{
        cvt = &m_data->buffer.visibleOutBuf;
      }

      if(getPropertyValue("threshold output")){
        cvt = m_data->lt.apply(cvt);
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
        }
      }
      return devices;
    }

    REGISTER_GRABBER(optris,utils::function(create_optris_grabber<OptrisGrabber::IR_IMAGE>), 
                     utils::function(create_optris_grabber_device_list<OptrisGrabber::IR_IMAGE>), 
                     "optris:camera serial ID or pattern:LibImager-based camera grabber source (ir camera)");

    REGISTER_GRABBER(optrisv,utils::function(create_optris_grabber<OptrisGrabber::VISIBLE_IMAGE>), 
                     utils::function(create_optris_grabber_device_list<OptrisGrabber::VISIBLE_IMAGE>),
                     "optrisv:camera serial ID or pattern:LibImager-based camera grabber source (color camera)");
  
    //REGISTER_GRABBER_BUS_RESET_FUNCTION(xi,reset_xi_bus);
  } // namespace io
}

