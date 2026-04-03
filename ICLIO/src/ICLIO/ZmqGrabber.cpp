// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/ZmqGrabber.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/ImageCompressor.h>
#include <zmq.hpp>
#include <mutex>

namespace icl{

  using namespace utils;
  using namespace core;

  namespace io{

    struct ZmqGrabber::Data : public Thread{
      ImageCompressor cmp;

      std::shared_ptr<zmq::context_t> context;
      std::shared_ptr<zmq::socket_t> subscriber;
      std::shared_ptr<zmq::message_t> msg;
      bool running;
      std::string host;
      int port;
      std::vector<icl8u> rbuf;
      std::recursive_mutex mutex;

      Data(const std::string &host, int port):host(host),port(port){
        context = new zmq::context_t(1);
        subscriber = new zmq::socket_t(*context, ZMQ_SUB);
        subscriber->connect(("tcp://"+host+":"+str(port)).c_str());
        subscriber->setsockopt(ZMQ_SUBSCRIBE, 0,0); // subscribe to all messages (pass-all filter)
        msg = new zmq::message_t;
        running = true;
        start();
      }
      void run(){
        while(running){
          subscriber->recv(msg.get());
          mutex.lock();
          rbuf.resize(msg->size());
          memcpy(&rbuf[0],msg->data(), msg->size());
          mutex.unlock();
        }
      }

      ~Data(){
        running = false;
        stop();
      }
    };


    ZmqGrabber::ZmqGrabber(const std::string &host, int port):m_data(0){
      m_data = new Data(host,port);
    }

    ZmqGrabber::~ZmqGrabber(){
      if(m_data) {
        m_data->running = false;
        delete m_data;
      };
    }


    const std::vector<GrabberDeviceDescription> &ZmqGrabber::getDeviceList([[maybe_unused]] bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      return deviceList;
    }

    const core::ImgBase* ZmqGrabber::acquireDisplay(){
      m_data->mutex.lock();
      while(!m_data->rbuf.size()){
        m_data->mutex.unlock();
        Thread::msleep(10);
        m_data->mutex.lock();
      }
      const ImgBase *image = m_data->cmp.uncompress(m_data->rbuf.data(), m_data->rbuf.size());
      m_data->mutex.unlock();
      return image;
      //      m_data->subscriber->recv(m_data->msg.get());
      //return m_data->cmp.uncompress((const icl8u*)m_data->msg->data(), m_data->msg->size());
    }

    REGISTER_CONFIGURABLE(ZmqGrabber, return new ZmqGrabber("localhost",18243));

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


    REGISTER_GRABBER(zmq,createZmqGrabber, getZmqDeviceList,
                     "zmq:host\\:port (host where data is published) :Zmq-based network grabber")

  } // namespace io
}
