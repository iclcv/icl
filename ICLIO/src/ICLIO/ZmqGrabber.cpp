/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ZmqGrabber.cpp                         **
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

#include <ICLIO/ZmqGrabber.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/ImageCompressor.h>
#include <zmq.hpp>

namespace icl{
  
  using namespace utils;
  using namespace core;
  
  namespace io{
    
    struct ZmqGrabber::Data : public Thread{
      ImageCompressor cmp;
      std::vector<icl8u> receivedBuffer;
      Mutex mutex;
      
      SmartPtr<zmq::context_t> context;
      SmartPtr<zmq::socket_t> subscriber;
      SmartPtr<zmq::message_t> msg;
      
      virtual void run(){
        subscriber->recv(msg.get());
        mutex.lock();
        receivedBuffer.resize(msg->size());
        std::copy((icl8u*)msg->data(), ((icl8u*)msg->data())+msg->size(), receivedBuffer.begin());
        mutex.unlock();
      }
    };
    
    
    ZmqGrabber::ZmqGrabber(const std::string &host, int port) throw(utils::ICLException):m_data(0){
      m_data = new Data;
      m_data->context = new zmq::context_t(1);
      m_data->subscriber = new zmq::socket_t(*m_data->context, ZMQ_SUB);
      m_data->subscriber->connect(("tcp://"+host+":"+str(port)).c_str());
      m_data->msg = new zmq::message_t;
    }
    
    ZmqGrabber::~ZmqGrabber(){
      if(m_data) {
        delete m_data;
      };
    }
    

    const std::vector<GrabberDeviceDescription> &ZmqGrabber::getDeviceList(bool rescan){
      (void)rescan;
      static std::vector<GrabberDeviceDescription> deviceList;
      return deviceList;
    }
    
    const core::ImgBase* ZmqGrabber::acquireImage(){
      m_data->mutex.lock();
      if(!m_data->receivedBuffer.size()){
        m_data->mutex.unlock();
        return 0;
      }
      const ImgBase *image = m_data->cmp.uncompress(m_data->receivedBuffer.data(), m_data->receivedBuffer.size());
      m_data->mutex.unlock();
      return image;
    }

    REGISTER_CONFIGURABLE(ZmqGrabber, return new ZmqGrabber("localhost",44444));

    Grabber* createZmqGrabber(const std::string &param){
      std::vector<std::string> ts = tok(param,":");
      if(ts.size() != 2) throw ICLException("unable to create Zmq-Grabber backend (expected host:port)");
      return new ZmqGrabber(ts[0],parse<int>(ts[1]));
    }

    const std::vector<GrabberDeviceDescription>& getZmqDeviceList(std::string filter, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(filter.size()){
        GrabberDeviceDescription d("zmq", 
                                   filter, 
                                   "A Zmq-based network grabber");
        deviceList.push_back(d);
      }
      return deviceList;
    }


    REGISTER_GRABBER(zmq,utils::function(createZmqGrabber), utils::function(getZmqDeviceList), 
                     "zmq:host\\:port:Zmq-based network grabber")
    
  } // namespace io
}

