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
    Mutex mutex;
    ImgBase *bufferImage;
    ImgBase *outputImage;
    bool hasNewImage;
    ImageCompressor compressor;
    
    struct Handler : public DataHandler<RSBImage>{
      RSBGrabberImpl::Data *data;
      Handler(RSBGrabberImpl::Data *data):data(data){}
      virtual void notify(shared_ptr<RSBImage> image){
        data->update(*image);
      }
    };
    shared_ptr<rsb::Handler> handler;
    
    void update(RSBImage &image){
      Mutex::Locker lock(mutex); // "reentrant-ness" and external access
      const std::string &data = image.data();
      const std::string &mode = image.compressionmode();
      
      if(mode == "off"){
        // quality is not used here!
        ImageSerializer::deserialize((const icl8u*)&data[0],&bufferImage);
      }else if(mode == "rle" || mode == "jpeg"){
        compressor.decode((const icl8u*)&data[0], data.length()).deepCopy(&bufferImage);
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
    m_data->handler = shared_ptr<Handler>(new Data::Handler(m_data));
    m_data->listener->addHandler(m_data->handler);
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

}
