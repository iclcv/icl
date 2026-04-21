// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/ZmqImageOutput.h>
#include <zmq.hpp>
#include <icl/utils/StringUtils.h>

namespace icl{
  using namespace core;
  using namespace utils;

  namespace io{


    struct ZmqImageOutput::Data{
      std::shared_ptr<zmq::context_t> context;
      std::shared_ptr<zmq::socket_t> publisher;
      std::shared_ptr<zmq::message_t> message;
    };


    ZmqImageOutput::~ZmqImageOutput(){
      if(isNull()) return;
      delete m_data;
    }

    ZmqImageOutput::ZmqImageOutput(int port):m_data(0){
      init(port);
    }

    void ZmqImageOutput::init(int port){
      if(isNull()){
        m_data = new Data;
        m_data->context = new zmq::context_t(1);
        m_data->message = new zmq::message_t;
      }
      m_data->publisher = new zmq::socket_t(*m_data->context, ZMQ_PUB);
      m_data->publisher->bind(("tcp://*:"+str(port)).c_str());
      //DEBUG_LOG("publishing to |" << ("tcp://*:"+str(port)).c_str() << "|");
    }

    void send(const core::Image &image){
      const CompressedData d = ImageCompressor::compress(image);
      zmq::message_t m(d.bytes,d.len,0);
      m_data->publisher->send(m);


    }

  }
}
