/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ZmqImageOutput.cpp                     **
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

#include <ICLIO/ZmqImageOutput.h>
#include <zmq.hpp>
#include <ICLUtils/StringUtils.h>

namespace icl{
  using namespace core;
  using namespace utils;

  namespace io{


    struct ZmqImageOutput::Data{
      SmartPtr<zmq::context_t> context;
      SmartPtr<zmq::socket_t> publisher;
      SmartPtr<zmq::message_t> message;
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

    void ZmqImageOutput::send(const core::ImgBase *image){
      const CompressedData d = ImageCompressor::compress(image);
      zmq::message_t m(d.bytes,d.len,0);
      m_data->publisher->send(m);


    }

  }
}

