/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/V4L2Grabber.cpp                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <fcntl.h>
#include <errno.h>
#include <fstream>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>

#ifdef USE_VIDEODEV2_HEADER
#include <linux/videodev2.h>
#else
#include <libv4l1-videodev.h>
#include <libv4l1.h>
#include <libv4l2.h>
#include <libv4lconvert.h>
#endif

#include <ICLUtils/Thread.h>
#include <ICLIO/FileList.h>
#include <ICLIO/ColorFormatDecoder.h>
#include <ICLIO/V4L2Grabber.h>
#include <ICLUtils/File.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    std::string fourcc_to_string(int fourcc){
      int tmp[2] = {fourcc,0};
      return std::string((char*)tmp);
    }

    class V4L2Grabber::Impl : public Thread {
      public:
        struct V4L2Buffer{
            void *data;
            size_t length;
        };

        struct SupportedFormat{
            SupportedFormat(){}
            SupportedFormat(int index, int fourcc, const std::string &description):
              index(index),fourcc(fourcc),description(description),
              continuousSize(false){}
            int index;
            int fourcc;
            std::string description;
            std::vector<Size> sizes;
            bool continuousSize; // actually this is not supported
        };

        typedef SmartPtr<SupportedFormat> SupportedFormatPtr;

        std::string deviceName;
        int file;
        std::vector<V4L2Buffer> buffers;
        Mutex mutex;

        /// the ID is the format description
        typedef std::map<std::string,SupportedFormatPtr> FMap;
        FMap supportedFormats;
        SupportedFormatPtr currentFormat;
        Size currentSize;
        std::string deviceNameInfo;
        bool isGrabbing;
        bool avoidDoubleFrames;
        Time lastTime;

        ImgBase *image;
        ImgBase *imageOut;
        std::vector<icl8u> convertBuffer;
        ColorFormatDecoder decoder;
        bool stoppedAlready;

        Impl(const std::string &deviceName, const std::string &initialFormat="", bool startGrabbing=true):
          deviceName(deviceName),isGrabbing(startGrabbing),avoidDoubleFrames(true),lastTime(Time::now()),
          image(0),imageOut(0),stoppedAlready(false){

          // note, \b is the word boundary special character (while $ is a line end which does not work so well here)
          if(deviceName.length() == 1 && match(deviceName,"^[0-9]\\b")){
            if(File("/dev/video/"+deviceName).exists()){
              this->deviceName = "/dev/video/"+deviceName;
              std::cout << "V4L2Grabber added device prefix '/dev/video/' automatically" << std::endl;
            }else if(File("/dev/video"+deviceName).exists()){
              this->deviceName = "/dev/video"+deviceName;
              std::cout << "V4L2Grabber added device prefix '/dev/video' automatically" << std::endl;
            }else{
              throw ICLException("V4L2Grabber initialization unable to find video device (given device ID: "
                                 +deviceName + ", tried /dev/video and /dev/video/ prefix, but both files"
                                 +" were not found");
            }
          }
          open_device();
          init_device(initialFormat);
          find_supported_properties();
          if(startGrabbing){
            init_mmap();
            start_capturing();
          }

        }

        ~Impl(){
          stoppedAlready = true;
          if(isGrabbing){
            stop();
            stop_capturing();
            release_device();
          }
          close_device();

          ICL_DELETE(image);
          ICL_DELETE(imageOut);
        }

        std::string getSupportedFormats() const {
          std::ostringstream stream;
          stream << "{";
          for(FMap::const_iterator it = supportedFormats.begin(); it != supportedFormats.end(); ++it){
            const SupportedFormatPtr &f = it->second;
            for(unsigned int i=0;i<f->sizes.size();++i){
              stream << "\"" << f->description << "~" << f->sizes[i] << "\"";
              stream << ',';
            }
          }
          stream << '}';
          return stream.str();
        }

        void errno_exception(const std::string &text) throw (ICLException){
          throw ICLException(text +" (deviceName: " + deviceName + " errno: "+ str(errno) +", " + strerror(errno) + ")");
        }
        void normal_exception(const std::string &text) throw (ICLException){
          throw ICLException(text +" (deviceName: " + deviceName + ")");
        }

        void open_device(){
          struct stat st;
          if(stat(deviceName.c_str(),&st)==-1){
            errno_exception("cannot identify device");
          }
          if(!S_ISCHR(st.st_mode)){
            errno_exception("no device:");
          }
          //      file = open(deviceName.c_str(), O_RDWR | O_NONBLOCK, 0);
          file = open(deviceName.c_str(), O_RDWR, 0);

          if(file==-1){
            errno_exception("cannot open device in RDWR mode");
          }
        }

        int xioctl(int request, void *arg){
          int r;
          do r = ioctl (file, request, arg);
          while (-1 == r && EINTR == errno);
          return r;
        }

        std::string get_current_format(){
          v4l2_format fmt;
          memset(&fmt,0,sizeof(fmt));

          fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          if (-1 == xioctl (VIDIOC_G_FMT, &fmt)){
            errno_exception("VIDIOC_G_FMT failed");
          }

          int fourcc = fmt.fmt.pix.pixelformat;
          int w = fmt.fmt.pix.width;
          int h = fmt.fmt.pix.height;

          for(FMap::const_iterator it = supportedFormats.begin(); it != supportedFormats.end(); ++it){
            if(it->second->fourcc == fourcc){
              return it->second->description + "~" + str(Size(w,h));
            }
          }
          normal_exception("current camera format does not seem to be supported [?]");
          return "";
        }

        void find_device_formats(){
          for(int i=0;true;++i){
            struct v4l2_fmtdesc format;
            memset(&format,0,sizeof(format));
            format.index = i;
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if(xioctl(VIDIOC_ENUM_FMT,&format) != 0) break;

            SupportedFormatPtr &f = supportedFormats[(const char*)format.description];
            f = new SupportedFormat(format.index,format.pixelformat, (const char*)format.description);

            for(int j=0;true;++j){
              v4l2_frmsizeenum sizes;
              memset(&sizes,0,sizeof(sizes));
              sizes.index = j;
              sizes.pixel_format = format.pixelformat;

              if(xioctl(VIDIOC_ENUM_FRAMESIZES,&sizes) != 0) break;

              switch(sizes.type){
                case V4L2_FRMSIZE_TYPE_DISCRETE:
                  f->sizes.push_back(Size(sizes.discrete.width,sizes.discrete.height));

                  break;
                case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                  f->continuousSize = true;
                  break;
                case V4L2_FRMSIZE_TYPE_STEPWISE:
                  for(size_t w = sizes.stepwise.min_width; w<= sizes.stepwise.max_width; w+=sizes.stepwise.step_width){
                    for(size_t h = sizes.stepwise.min_height; h<= sizes.stepwise.max_height; h+=sizes.stepwise.step_height){
                      f->sizes.push_back(Size(w,h));
                    }
                  }
                  break;
                default:
                  WARNING_LOG("found invalid format type");
                  break;
              }
            }
          }
        }

        /// id is v4l2_format.description~size
        void change_format(const std::string &id, bool apply_ioctl=true){
          std::string description, size;
          std::vector<std::string> ts = tok(id,"~");
          if(ts.size() == 2){
            description = ts[0];
            size = ts[1];
          }else if(ts.size() > 2){
            size = ts.back();
            ts.pop_back();
            description = cat(ts,"~");
          }else{
            normal_exception("unable to parse combined 'format~size' description " + id);
          }
          Size s = parse<Size>(size);

          FMap::iterator it = supportedFormats.find(description);
          if(it == supportedFormats.end()){
            normal_exception("cannot change to unsupported format '" + description + "'");
          }

          SupportedFormatPtr &f = it->second;
          if(std::find(f->sizes.begin(),f->sizes.end(),s) == f->sizes.end()){
            normal_exception("given format '" + description + "' does not support associated size " + size);
          }

          if(apply_ioctl){
            v4l2_format fmt;
            memset(&fmt,0,sizeof(fmt));

            fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width       = s.width;
            fmt.fmt.pix.height      = s.height;
            fmt.fmt.pix.pixelformat = f->fourcc;
            fmt.fmt.pix.field       = V4L2_FIELD_NONE; // preferred non-interlaced ...//V4L2_FIELD_INTERLACED;

            if (-1 == xioctl (VIDIOC_S_FMT, &fmt)){
              errno_exception("VIDIOC_S_FMT failed");
            }
            this->currentFormat = f;
            this->currentSize = s;
          }else{
            this->currentFormat = f;
            this->currentSize = s;
          }

          //show_field(fmt.fmt.pix.field);
          //show_pixelformat(fmt.fmt.pix.pixelformat);
          //std::cout << "using width: " << fmt.fmt.pix.width << std::endl;
          //std::cout << "using height: " << fmt.fmt.pix.height << std::endl;
          /* Note VIDIOC_S_FMT may change width and height. */

          /* Buggy driver paranoia.
            min = fmt.fmt.pix.width * 2;
            if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
            min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
            if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;
        */
        }

        void init_device(const std::string &initialFormat=""){
          v4l2_capability cap;
          memset(&cap,0,sizeof(cap));

          if (-1 == xioctl (VIDIOC_QUERYCAP, &cap)) {
            errno_exception(errno==EINVAL ? "no V4L2 device" : "VIDIOC_QUERYCAP failed");
          }

          if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            normal_exception("no capture device");
          }

          if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            normal_exception("device does not support streaming i/o (mmap)");
          }

          this->deviceNameInfo = (const char*)cap.card;

          find_device_formats();

          if(initialFormat == ""){
            std::string f = get_current_format();
            change_format(f,true); // this is just used as a check
          }else{
            change_format(initialFormat,true);
          }
        }

        void init_mmap(){
          v4l2_requestbuffers req;

          memset(&req,0,sizeof(req));

          req.count               = 4; //buffers ??
          req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          req.memory              = V4L2_MEMORY_MMAP;

          if (-1 == xioctl (VIDIOC_REQBUFS, &req)) {
            WARNING_LOG("if VIDIOC_REQBUFS fails, it usually helps to try to access the device using unicap or cvcam once");
            errno_exception(EINVAL==errno ? "device does not support memory mapping (mmap)" : "VIDIOC_REQBUFS failed" );
          }

          if (req.count < 2) {
            normal_exception("not enough buffer memory for two mmap buffers on device ");
          }

          buffers.resize(req.count);
          for(unsigned int i=0;i<buffers.size();++i){
            struct v4l2_buffer buf;
            memset(&buf,0,sizeof(buf));

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;

            if (-1 == xioctl (VIDIOC_QUERYBUF, &buf)){
              errno_exception("VIDIOC_QUERYBUF failed");
            }

            buffers[i].length = buf.length;
            buffers[i].data = mmap (NULL /* start anywhere */,
                                    buf.length,
                                    PROT_READ | PROT_WRITE /* required */,
                                    MAP_SHARED /* recommended */,
                                    file, buf.m.offset);

            if (MAP_FAILED == buffers[i].data){
              errno_exception ("mmap call failed");
            }
          }
        }
        void start_capturing(){
          for (unsigned i=0; i<buffers.size();++i){
            struct v4l2_buffer buf;
            memset(&buf,0,sizeof(buf));

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;

            if (-1 == xioctl(VIDIOC_QBUF, &buf)){
              errno_exception("VIDIOC_QBUF failed");
            }
          }

          v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

          if (-1 == xioctl(VIDIOC_STREAMON, &type)){
            errno_exception("VIDIOC_STREAMON failed");
          }

          isGrabbing = true;
          start(); // starts the thread as well
        }

        virtual void run(){
          while(!stoppedAlready){
            fd_set fds;
            struct timeval tv;

            FD_ZERO (&fds);
            FD_SET (file, &fds);

            /* Timeout. */
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            int r = select(file + 1, &fds, NULL, NULL, &tv);
            if(-1 == r && errno != EINTR){
              errno_exception("select failed");
            }
            if(0 == r){
              //          normal_exception("select timeout"); we suppress this
            }
            bool result = read_frame();
            if(!result){
              ERROR_LOG("unable to read frame!");
            }
          }
        }

        bool read_frame(){
          //          DEBUG_LOG("<read_frame>");
          v4l2_buffer buf;
          memset(&buf,0,sizeof(buf));

          buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          buf.memory = V4L2_MEMORY_MMAP;

          if (-1 == xioctl (VIDIOC_DQBUF, &buf)){
            if(errno == EAGAIN) return false;
            else if(errno == EIO) {} // nothing here!
            else errno_exception("dequeue video buffer (VIDIOC_DQBUF) failed");
          }

          if(buf.index >= buffers.size()) normal_exception("got an invalid buffer index! ");

          /*      {
            DEBUG_LOG("warning mjpeg file is written to /tmp/data.jpg");
            std::ofstream file("/tmp/data.jpg");
            file << std::string((const char*)buffers[buf.index].data, buffers[buf.index].length) << std::flush;
          }
          SHOW(fourcc_to_string(this->currentFormat->fourcc));
              */
          process_image ((const icl8u*)buffers[buf.index].data, this->currentFormat->fourcc);

          if (-1 == xioctl (VIDIOC_QBUF, &buf)){
            errno_exception("enqueue video buffer VIDIOC_QBUF failed");
          }
          //DEBUG_LOG("</read_frame>");
          return true;
        }



        void process_image(const icl8u *p, int fourcc){
          Mutex::Locker lock(mutex);
          Time t = Time::now();
          if(deviceNameInfo == "Myrmex"){ // spezialization for the myrmex tactile device
            fourcc = FourCC("MYRM");
          }
          decoder.decode(fourcc,p, currentSize, &image);
          if(image) image->setTime(t);
        }

        const ImgBase *acquireImage(){
          Mutex::Locker lock(mutex);
          while(!image || (avoidDoubleFrames && lastTime == image->getTime())){
            mutex.unlock();
            Thread::msleep(0);
            mutex.lock();
          }
          image->deepCopy(&imageOut);
          lastTime = image->getTime();
          return imageOut;
        }

        void release_device(){
          for (unsigned int i = 0; i < buffers.size() ; ++i){
            if (-1 == munmap (buffers[i].data, buffers[i].length)){
              errno_exception("munmap failed");
            }
          }
          buffers.clear();
        }
        void stop_capturing(){
          v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          if (-1 == xioctl (VIDIOC_STREAMOFF, &type)){
            errno_exception("VIDIOC_STREAMOFF failed");
          }
        }

        void close_device(){
          if (-1 == close (file)){
            errno_exception ("unable to close device");
          }
          file = -1;
        }


        struct SupportedProperty{
            SupportedProperty(){}
            SupportedProperty(Impl *impl, const std::string &name ,const std::string &type, const std::string &info,
                              int value, int internal_type):
              impl(impl),name(name),type(type),info(info),value(value),internal_type(internal_type){}
            Impl *impl;
            std::string name;
            std::string type;
            std::string info;
            int value;
            std::map<std::string,icl32s> menu;
            int internal_type;

            std::string toString() const {
              std::ostringstream stream;
              stream << "SupportedProperty: " << name << "\ntype: " << type << "\nvalue:" << value
                     << "\ninfo:" << const_cast<SupportedProperty*>(this)->getInfo()
                     << "\ninternal type: " << internal_type << "\n";

              return stream.str();
            }

            void setValue(const std::string &value){
              if(type == "menu"){
                std::map<std::string,int>::const_iterator it = menu.find(value);
                if(it == menu.end()) impl->normal_exception("unable to set menu property " + name + " to invalid value " + value);
                impl->set_property(internal_type,it->second);
              }else{
                impl->set_property(internal_type,parse<icl32s>(value));
              }
            }
            std::string getType(){
              return type;
            }
            std::string getValue(){
              if(type == "menu"){
                int idx = impl->get_property(internal_type);
                for(std::map<std::string,int>::const_iterator it = menu.begin();
                    it != menu.end(); ++it){
                  if(it->second == idx) return it->first;
                }
                impl->normal_exception("unable to determine the current value of menu property " + name);
                return "";
              }else{
                return str(impl->get_property(internal_type));
              }

            }
            std::string getInfo(){
              if(type == "menu"){
                std::ostringstream stream;
                stream << "{";
                for(std::map<std::string,icl32s>::const_iterator it = menu.begin(); it != menu.end();){
                  stream << it->first;
                  if(++it != menu.end()) stream << ",";
                }
                stream << "}";
                return stream.str();
              }else return info;
            }
        };
        typedef SmartPtr<SupportedProperty> SupportedPropertyPtr;
        typedef std::map<std::string,SupportedPropertyPtr> PMap;
        PMap supportedProperties;

        SupportedPropertyPtr findProperty(const std::string &name)  throw (ICLException){
          PMap::const_iterator it = supportedProperties.find(name);
          //if(it == supportedProperties.end()) throw ICLException("V4L2Grabber: unknown property '" + name + "'");
          if(it == supportedProperties.end()) return SupportedPropertyPtr();
          return it->second;
        }

        void find_supported_properties(){
          v4l2_queryctrl queryctrl;
          memset (&queryctrl, 0, sizeof (queryctrl));

          for(queryctrl.id = V4L2_CID_BASE;true;queryctrl.id++) {
            if(queryctrl.id == V4L2_CID_LASTP1) {
              queryctrl.id = V4L2_CID_PRIVATE_BASE;
            }
            if(xioctl(VIDIOC_QUERYCTRL,&queryctrl)){
              break;
            }
            if(!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)){
              std::string controlName = (const char*)queryctrl.name;
              switch(queryctrl.type){
                case V4L2_CTRL_TYPE_MENU:{
                  v4l2_querymenu querymenu;
                  memset(&querymenu,0,sizeof(querymenu));
                  querymenu.id = queryctrl.id;

                  int value = get_property(queryctrl.id);
                  SupportedPropertyPtr &p = supportedProperties[controlName];
                  p = new SupportedProperty(this,controlName,"menu","",value, queryctrl.id);

                  for(querymenu.index = queryctrl.minimum; (int)querymenu.index <= queryctrl.maximum; querymenu.index++) {
                    if(xioctl(VIDIOC_QUERYMENU,&querymenu)){
                      errno_exception("VIDIOC_QUERYMENU failed");
                    }
                    std::string menuName = (const char*)querymenu.name;
                    p->menu[menuName] = querymenu.index;
                  }
                }
                  break;
                case V4L2_CTRL_TYPE_INTEGER	:
                  supportedProperties[controlName] = new SupportedProperty(this,controlName,"range",
                                                                           str(SteppingRange32f(queryctrl.minimum,
                                                                                                queryctrl.maximum,
                                                                                                queryctrl.step)),
                                                                           get_property(queryctrl.id),
                                                                           queryctrl.id);
                  break;
                case V4L2_CTRL_TYPE_BOOLEAN:
                  supportedProperties[controlName] = new SupportedProperty(this,controlName,"flag","",
                                                                           get_property(queryctrl.id),
                                                                           queryctrl.id);
                  break;
                case V4L2_CTRL_TYPE_BUTTON :
                  supportedProperties[controlName] = new SupportedProperty(this,controlName,"command","",
                                                                           get_property(queryctrl.id),
                                                                           queryctrl.id);
                  break;
                case V4L2_CTRL_TYPE_INTEGER64:
                case V4L2_CTRL_TYPE_CTRL_CLASS:
                default:
                  WARNING_LOG("unsupported control type: " << (const char*)queryctrl.name);
              }
            }
          }

          //      for(unsigned int i=0;i<supportedProperties.size();++i){
          //  std::cout << i << ": "<< supportedProperties[i].toString() << std::endl << std::endl;
          //}
        }

        void set_property(int type, int value){
          v4l2_control control;
          memset (&control, 0, sizeof (control));

          control.id = type;
          control.value = value;

          if(-1==xioctl(VIDIOC_S_CTRL, &control)) {
            errno_exception("unable to set property");
          }
        }

        int get_property(int type){
          v4l2_control control;
          memset (&control, 0, sizeof (control));
          control.id = type;

          if(-1==xioctl(VIDIOC_G_CTRL, &control)) {
            errno_exception("unable to get property");
          }

          return control.value;
        }

        static std::vector<GrabberDeviceDescription> getDeviceList() {
          FileList l("/dev/video*");
          std::vector<GrabberDeviceDescription> all;
          for(int i=0;i<l.size();++i){
            const std::string &deviceName = l[i];
            SmartPtr<Impl> test;
            try{
              test = new Impl(deviceName,"",false);
              all.push_back(GrabberDeviceDescription("v4l",deviceName,test->deviceNameInfo + " (" + deviceName + ")"));
            }catch(...){}
          }
          return all;
        }
    }; // end of Impl class



    V4L2Grabber::V4L2Grabber(const std::string &device)
      : implMutex(utils::Mutex::mutexTypeRecursive)
    {
      impl = new Impl(device);
      addProperties();
    }

    V4L2Grabber::~V4L2Grabber(){
      Mutex::Locker lock(implMutex);
      delete impl;
    }

    const ImgBase *V4L2Grabber::acquireImage(){
      Mutex::Locker lock(implMutex);
      const ImgBase *image = 0;
      do{ image = impl->acquireImage(); } while(!image || !image->getDim() );
      return image;
    }

    // removes braces and quotation marks from format string
    // replaces simple braces by -
    std::string clearFormatString(std::string format) {
      std::ostringstream ret;
      for (unsigned int j = 0; j < format.length(); ++j){
        switch (format[j]){
          case '"':
          case '{':
          case '}':
            break;
          case '(':
          case ')':
            ret << '-';
            break;
          default:
            ret << format[j];
        }
      }
      return ret.str();
    }

    // replaces - by braces.
    std::string addBraces(const std::string source){
      std::string ret(source);
      bool first = true;
      for (unsigned int j = 0; j < ret.length(); ++j){
        if(ret[j] == '-'){
          ret[j] = first ? '(' : ')';
          first = false;
        }
      }
      return ret;
    }

    // adds properties to Configurable
    void V4L2Grabber::addProperties(){
      addProperty("device name","info","",impl->deviceNameInfo);
      addProperty("avoid doubled frames", "flag", "", impl->avoidDoubleFrames, 0, "");
      addProperty("format", "menu", clearFormatString(impl->getSupportedFormats()), impl->get_current_format(), 0, "The image format.");
      addProperty("size", "menu", "ajusted by format", Any(), 0, "This is set by the format-property.");
      for(Impl::PMap::const_iterator it=impl->supportedProperties.begin();
          it != impl->supportedProperties.end();++it){
        Impl::SupportedPropertyPtr p = it -> second;
        addProperty(it->first,p -> getType(), p -> getInfo(), p -> getValue(), 0,"");
      }
      Configurable::registerCallback(utils::function(this,&V4L2Grabber::processPropertyChange));
    }

    // callback for changed configurable properties
    void V4L2Grabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker lock(implMutex);
      if(prop.name == "format"){
        std::string oldDeviceName = impl->deviceName;
        impl->stop();
        delete impl;
        impl = new Impl(oldDeviceName,addBraces(prop.value));
        setPropertyValue("avoid doubled frames",impl->avoidDoubleFrames);
        for(Impl::PMap::const_iterator it=impl->supportedProperties.begin();
            it != impl->supportedProperties.end();++it){
          Impl::SupportedPropertyPtr p = it -> second;
          setPropertyValue(it->first, p->getValue());
        }
      }else if(prop.name == "size"){
        // this is adjusted by the format
      }else if(prop.name == "avoid doubled frames"){
        impl->avoidDoubleFrames = parse<bool>(prop.value);
      }else{
        Impl::SupportedPropertyPtr p = impl->findProperty(prop.name);
        if (p.get()) p -> setValue(prop.value);
      }
    }

    const std::vector<GrabberDeviceDescription> &V4L2Grabber::getDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> last;
      if(!last.size() || rescan){
        last = Impl::getDeviceList();
      }
      return last;
    }

    REGISTER_CONFIGURABLE(V4L2Grabber, return new V4L2Grabber("/dev/video0"));

    Grabber* createVlGrabber(const std::string &param){
      return new V4L2Grabber(param);
    }

    REGISTER_GRABBER(v4l,utils::function(createVlGrabber), utils::function(V4L2Grabber::getDeviceList), "v4l:/dev/videoX index or device-file:V4l2 based camera source");

  } // namespace io
}


