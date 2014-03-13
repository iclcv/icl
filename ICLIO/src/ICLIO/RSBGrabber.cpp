/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/RSBGrabber.cpp                         **
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

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/ImageSerializer.h>

#include <ICLIO/RSBGrabber.h>
#include <ICLIO/RSBImage.pb.h>
#include <ICLIO/ImageCompressor.h>

using namespace boost;
using namespace rsb;
using namespace rsb::converter;

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    struct RSBGrabber::Data {

        Data() : mutex(Mutex::mutexTypeRecursive){}

        ListenerPtr listener;
        Informer<std::string>::Ptr propertyInformer;
        std::string propertyScopeName;

        Mutex mutex;
        ImgBase *bufferImage;
        ImgBase *outputImage;
        bool hasNewImage;
        ImageCompressor compressor;

        int setJPEGQuality;
        int setRLEQuality;
        std::string setCompressionMode;

        int receivedJPEGQuality;
        int receivedRLEQuality;
        std::string receivedCompressionMode;

        int lastImageDataSize;
        float lastCompressionRatio;

        void sendUpdateToSource(){
          shared_ptr<std::string> cmd;
          if(setCompressionMode == "none"){
            cmd = shared_ptr<std::string>(new std::string("none:none"));
          }else if(setCompressionMode == "rlen"){
            cmd = shared_ptr<std::string>(new std::string("rlen:" + str(setRLEQuality)));
          }else if(setCompressionMode == "jpeg"){
            cmd = shared_ptr<std::string>(new std::string("jpeg:" + str(setJPEGQuality)));
          }else{
            ERROR_LOG("cannot update the image source (error that should not occur)");
          }
          if(cmd){
            propertyInformer->publish(cmd);
          }
        }

        struct Handler : public DataHandler<RSBImage>{
            RSBGrabber::Data *data;
            RSBGrabber *impl;
            Handler(RSBGrabber::Data *data, RSBGrabber *impl):data(data),impl(impl){}
            virtual void notify(shared_ptr<RSBImage> image){
              data->update(*image,impl);
              data->lastImageDataSize = image->data().length();

              float realImageSize = image->width() * image->height() * image->channels() * getSizeOf((depth)image->depth());
              float r = float(data->lastImageDataSize) / realImageSize;
              data->lastCompressionRatio = float((int)(r*10000))/100;
            }
        };
        shared_ptr<rsb::Handler> handler;

        void update(RSBImage &image, RSBGrabber *impl){
          Mutex::Locker lock(mutex); // "reentrant-ness" and external access
          const std::string &data = image.data();
          const std::string &mode = image.compressionmode();
          DEBUG_LOG2(mode);

          receivedCompressionMode = mode;

          compressor.uncompress((const icl8u*)&data[0], data.length(), &bufferImage); //)->deepCopy(&bufferImage);

          if(bufferImage){
            bufferImage->setTime(image.time());
            bufferImage->setROI(Rect(image.roix(),image.roiy(), image.roiw(), image.roih()));
            bufferImage->setMetaData(image.metadata());
            hasNewImage = true;
          }
          impl->notifyNewImageAvailable(bufferImage);
        }
    };
    
    RSBGrabber::RSBGrabber(){
      m_data = 0;
    }
    
    RSBGrabber::RSBGrabber(const std::string &scope, const std::string &transportList):m_data(0){
      init(scope,transportList);
    }
    
    void RSBGrabber::init(const std::string &scope, const std::string &transportList){
      ICL_DELETE(m_data);
      m_data = new Data;
      m_data->outputImage = 0;
      m_data->bufferImage = 0;
      m_data->hasNewImage = false;
      
      Scope rsbScope(scope);
#if 1
      Factory &factory = rsc::patterns::Singleton<Factory>::getInstance();
#else 
      Factory &factory = Factory::getInstance()
#endif
      ParticipantConfig rsbCfg = factory.getDefaultParticipantConfig();
      typedef std::set<ParticipantConfig::Transport> TSet;
      typedef std::vector<ParticipantConfig::Transport> TVec;
      
      TSet ts2 = rsbCfg.getTransports(true);
      TVec ts(ts2.begin(),ts2.end());
      std::vector<std::string> transports = tok(transportList,",");

      for(TVec::iterator it = ts.begin(); it != ts.end(); ++it){
        ParticipantConfig::Transport &t = *it;
        if( find(transports.begin(), transports.end(), it->getName()) == transports.end() ){
          t.setEnabled(false);
        }else{
          it->setEnabled(true);
        }
      }
      rsbCfg.setTransports(TSet(ts.begin(),ts.end()));
      m_data->listener = factory
.createListener(rsbScope,rsbCfg);
      m_data->handler = shared_ptr<Handler>(new Data::Handler(m_data,this));
      m_data->listener->addHandler(m_data->handler);

      m_data->propertyScopeName = "/icl/RSBImageOutput/configuration"+scope;
      m_data->propertyInformer = factory.createInformer<std::string>(Scope(m_data->propertyScopeName),rsbCfg);

      m_data->receivedJPEGQuality = 90;
      m_data->receivedRLEQuality = 1;
      m_data->receivedCompressionMode = "off";

      m_data->lastImageDataSize = 0;
      m_data->lastCompressionRatio = 0;

      // Configurable
      addProperty("format", "info", "", "-", 0, "");
      addProperty("size", "info", "", "", 0, "");
      addProperty("compression-type", "menu", "none,rlen,jpeg", m_data->receivedCompressionMode, 0, "");
      addProperty("RLE-quality", "menu", "1 Bit,4 Bit,6 Bit,8 Bit", m_data->receivedRLEQuality, 0, "");
      addProperty("JPEG-quality", "range", "[1,100]:1", m_data->receivedJPEGQuality, 0, "");
      addProperty("image data size", "info", "", m_data->lastImageDataSize, 0, "");
      addProperty("compression ratio", "info", "", str(m_data->lastCompressionRatio) + "%", 0, "");
      Configurable::registerCallback(utils::function(this,&RSBGrabber::processPropertyChange));
    }
    
    RSBGrabber::~RSBGrabber(){
      if(m_data){
        ICL_DELETE(m_data->outputImage);
        ICL_DELETE(m_data->bufferImage);
        m_data->listener->removeHandler(m_data->handler);
        delete m_data;
      }
    }

     
    const ImgBase *RSBGrabber::acquireImage(){
      ICLASSERT_RETURN_VAL(!isNull(),0);
      Mutex::Locker lock(m_data->mutex);
      while(!m_data->bufferImage || !m_data->hasNewImage){
        m_data->mutex.unlock();
        Thread::msleep(0);
        m_data->mutex.lock();
      }
      m_data->bufferImage->deepCopy(&m_data->outputImage);
      m_data->hasNewImage = false;
      setPropertyValue("image data size", m_data->lastImageDataSize);
      setPropertyValue("compression ratio", str(m_data->lastCompressionRatio) + "%");
      setPropertyValue("size", m_data->outputImage->getSize());
      return m_data->outputImage;
    }

    const std::vector<GrabberDeviceDescription> &RSBGrabber::getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> all;
      if(!rescan) return all;
      
      /// TODO: list segments

      return all;
    }

    // callback for changed configurable properties
    void RSBGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker lock(m_data->mutex);
      if(prop.name == "compression-type"){
        m_data->setRLEQuality = m_data->receivedRLEQuality;
        m_data->setJPEGQuality = m_data->receivedJPEGQuality;
        m_data->setCompressionMode = prop.value;
        m_data->sendUpdateToSource();
      }else if(prop.name == "RLE-quality"){
        m_data->setCompressionMode = m_data->receivedCompressionMode;
        m_data->setRLEQuality = parse<int>(prop.value);
        m_data->sendUpdateToSource();
      }else if(prop.name == "JPEG-quality"){
        m_data->setCompressionMode = m_data->receivedCompressionMode;
        m_data->setRLEQuality = m_data->receivedRLEQuality;
        m_data->setJPEGQuality = parse<int>(prop.value);
        m_data->sendUpdateToSource();
      }
    }

    REGISTER_CONFIGURABLE(RSBGrabber, return new RSBGrabber("/icl/foo", "spread"));

    Grabber* createRSBGrabber(const std::string &param){
      std::vector<std::string> ts = tok(param,":");
      if(!ts.size()){
        throw ICLException("invalid argument count (expected 1 or 2)");
      } else if(ts.size() == 1){
        return new RSBGrabber(ts[0]);
      } else if(ts.size() == 2){
        return new RSBGrabber(ts[1],ts[0]);
      } else {
        throw ICLException("invalid definition string (exptected: [transport-list]:scope");
      }
    }

    const std::vector<GrabberDeviceDescription>& getRSBDeviceList(std::string filter, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(filter.size()) deviceList.push_back(
        GrabberDeviceDescription("rsb", filter, "A grabber for images published via rsb.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(rsb,utils::function(createRSBGrabber), utils::function(getRSBDeviceList), "rsb:[comma sep. transport list=spread]\\:scope:Robotics Service Bus based image source");

  } // namespace io
}
