/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/RSBGrabber.cpp                               **
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

namespace icl{

  struct RSBGrabberImpl::Data {
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
    
    void sendUpdateToSource(){
      shared_ptr<std::string> cmd;
      if(setCompressionMode == "off"){
        cmd = shared_ptr<std::string>(new std::string("off:off"));
      }else if(setCompressionMode == "rle"){
        cmd = shared_ptr<std::string>(new std::string("rle:" + str(setRLEQuality)));
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
      RSBGrabberImpl::Data *data;
      RSBGrabberImpl *impl;
      Handler(RSBGrabberImpl::Data *data, RSBGrabberImpl *impl):data(data),impl(impl){}
      virtual void notify(shared_ptr<RSBImage> image){
        data->update(*image);
        data->lastImageDataSize = image->data().length();
      }
    };
    shared_ptr<rsb::Handler> handler;
    
    void update(RSBImage &image){
      Mutex::Locker lock(mutex); // "reentrant-ness" and external access
      const std::string &data = image.data();
      const std::string &mode = image.compressionmode();
      
      receivedCompressionMode = mode;
      if(mode == "off"){
        // quality is not used here!
        ImageSerializer::deserialize((const icl8u*)&data[0],&bufferImage);
      }else if(mode == "rle" || mode == "jpeg"){
        compressor.decode((const icl8u*)&data[0], data.length()).deepCopy(&bufferImage);
        
        if(mode == "jpeg"){
          receivedJPEGQuality = parse<int>(image.compressionquality());
        }else{
          receivedRLEQuality = parse<int>(image.compressionquality());
        }
      }
      if(bufferImage){
        bufferImage->setTime(image.time());
        bufferImage->setROI(Rect(image.roix(),image.roiy(), image.roiw(), image.roih()));
        hasNewImage = true;
      }
    }
  };
  
  RSBGrabberImpl::RSBGrabberImpl(){
    m_data = 0;
  }
  
  RSBGrabberImpl::RSBGrabberImpl(const std::string &scope, const std::string &transportList):m_data(0){
    init(scope,transportList);
  }
  
  void RSBGrabberImpl::init(const std::string &scope, const std::string &transportList){
    ICL_DELETE(m_data);
    m_data = new Data;
    m_data->outputImage = 0;
    m_data->bufferImage = 0;
    m_data->hasNewImage = false;
    
    Scope rsbScope(scope);
    ParticipantConfig rsbCfg;
    std::vector<std::string> transports = tok(transportList,",");
    for(size_t i=0;i<transports.size();++i){
      rsbCfg.addTransport(ParticipantConfig::Transport(transports[i]));
    }
    m_data->listener = Factory::getInstance().createListener(rsbScope,rsbCfg);
    m_data->handler = shared_ptr<Handler>(new Data::Handler(m_data,this));
    m_data->listener->addHandler(m_data->handler);

    m_data->propertyScopeName = "/icl/RSBImageOutput/configuration"+scope;
    m_data->propertyInformer = Factory::getInstance().createInformer<std::string>(Scope(m_data->propertyScopeName),rsbCfg);

    m_data->receivedJPEGQuality = 90;
    m_data->receivedRLEQuality = 1;
    m_data->receivedCompressionMode = "off";

    m_data->lastImageDataSize = 0;
  }
  
  RSBGrabberImpl::~RSBGrabberImpl(){
    if(m_data){
      ICL_DELETE(m_data->outputImage);
      ICL_DELETE(m_data->bufferImage);
      m_data->listener->removeHandler(m_data->handler);
      delete m_data;
    }
  }
  
  const ImgBase *RSBGrabberImpl::acquireImage(){
    ICLASSERT_RETURN_VAL(!isNull(),0);
    Mutex::Locker lock(m_data->mutex);
    while(!m_data->bufferImage || !m_data->hasNewImage){
      m_data->mutex.unlock();
      Thread::msleep(0);
      m_data->mutex.lock();
    }
    m_data->bufferImage->deepCopy(&m_data->outputImage);
    m_data->hasNewImage = false;
    return m_data->outputImage;
  }

  const std::vector<GrabberDeviceDescription> &RSBGrabberImpl::getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> all;
    if(!rescan) return all;
    
    /// TODO: list segments

    return all;
  }

  void RSBGrabberImpl::setProperty(const std::string &property, const std::string &value){
    Mutex::Locker lock(m_data->mutex);
    if(property == "compression-type"){
      m_data->setRLEQuality = m_data->receivedRLEQuality;
      m_data->setJPEGQuality = m_data->receivedJPEGQuality;
      m_data->setCompressionMode = value;
      m_data->sendUpdateToSource();
    }else if(property == "RLE-quality"){
      m_data->setCompressionMode = m_data->receivedCompressionMode;
      m_data->setRLEQuality = parse<int>(value);
      m_data->sendUpdateToSource();
    }else if(property == "JPEG-quality"){
      m_data->setCompressionMode = m_data->receivedCompressionMode;
      m_data->setRLEQuality = m_data->receivedRLEQuality;
      m_data->setJPEGQuality = parse<int>(value);
      m_data->sendUpdateToSource();
    }else{
      ERROR_LOG("invalid property: " << property);
    }
  }

  int RSBGrabberImpl::isVolatile(const std::string &name){
    return name == "image data size" ? 100 : 0;
  }

  
  std::vector<std::string> RSBGrabberImpl::getPropertyList(){
    static std::vector<std::string> ps = tok("compression-type,RLE-quality,JPEG-quality,image data size",",");
    return ps;
  }
    
  std::string RSBGrabberImpl::getType(const std::string &name){
    Mutex::Locker lock(m_data->mutex);
    if(name == "compression-type"){
      return "menu";
    }else if(name == "RLE-quality"){
      return "menu";
    }else if(name == "JPEG-quality"){
      return "range";
    }else if(name == "image data size"){
      return "info";
    }else{
      ERROR_LOG("invalid property: " << name);
    }
    return "unknown";
    
  }

  std::string RSBGrabberImpl::getInfo(const std::string &property){
    Mutex::Locker lock(m_data->mutex);
    if(property == "compression-type"){
      return "{\"off\",\"rle\",\"jpeg\"}";
    }else if(property == "RLE-quality"){
      return "{\"1 Bit\",\"4 Bit\",\"6 Bit\"}";
    }else if(property == "JPEG-quality"){
      return "[1,100]";
    }else if(property == "image data size"){
      return "";
    }else{
      ERROR_LOG("invalid property: " << property);
    }
    return "unknown";
  }

  std::string RSBGrabberImpl::getValue(const std::string &property){
    Mutex::Locker lock(m_data->mutex);
    if(property == "compression-type"){
      return m_data->receivedCompressionMode;
    }else if(property == "RLE-quality"){
      return str(m_data->receivedRLEQuality);
    }else if(property == "JPEG-quality"){
      return str(m_data->receivedJPEGQuality);
    }else if(property == "image data size"){
      return str(m_data->lastImageDataSize);
    }else{
      ERROR_LOG("invalid property: " << property);
    }
    return "unknown";
  }


}
