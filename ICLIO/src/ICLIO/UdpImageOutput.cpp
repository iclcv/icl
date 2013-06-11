/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/UdpImageOutput.cpp                     **
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

#include <ICLIO/UdpImageOutput.h>

#include <QtNetwork/QUdpSocket>
#include <ICLUtils/Thread.h>

#include <QtCore/QCoreApplication>

namespace icl{
  using namespace core;
  using namespace utils;

  namespace io{

    struct SendEvent : public QEvent{
      SendEvent(UdpImageOutput::Data *data, const ImgBase *image, bool *done):
        QEvent((QEvent::Type)QEvent::registerEventType()),data(data),image(image),done(done){}
      UdpImageOutput::Data *data;
      const ImgBase *image;
      bool *done;
    };

    struct UdpImageOutput::Data : public QObject{
      QUdpSocket *socket;
      QHostAddress addr;
      quint16 port;
      ImageCompressor *cmp;
      
      bool event(QEvent *e){
        SendEvent *s = dynamic_cast<SendEvent*>(e);
        if(s){
          DEBUG_LOG("sending data");
          const CompressedData data = s->data->cmp->compress(s->image);
          s->data->socket->writeDatagram((char*)data.bytes, data.len, addr, port);
          *s->done = true;
          return true;
        }
        return false;
      }
    };

      

    
    UdpImageOutput::~UdpImageOutput(){
      if(isNull()) return;
      delete m_data->socket;
      delete m_data;
    }
    
    UdpImageOutput::UdpImageOutput(const std::string &targetPC, int port):m_data(0){
      if(!targetPC.length()){
        return;
      }
      init(targetPC,port);
    }
    
    void UdpImageOutput::init(const std::string &targetPC, int port){
      if(isNull()){
        m_data = new Data;
        m_data->socket = new QUdpSocket(0);
      }
      m_data->port = (quint16)port;
      m_data->addr = QHostAddress(QString(targetPC.c_str()));
      m_data->cmp = this;
      DEBUG_LOG("init(" << targetPC << ", " << port << ")");
    }
    
    void UdpImageOutput::send(const core::ImgBase *image){
      ICLASSERT_THROW(!isNull(), ICLException("UdpImageOutput::send: instance was null!"));
      bool done = false;
      QCoreApplication::postEvent(m_data,new SendEvent(m_data,image,&done));
      while(!done) Thread::usleep(100);
    }

  }
}

