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

namespace icl{
  using namespace core;
  using namespace utils;

  namespace io{
    struct UdpImageOutput::Data{
      QUdpSocket *socket;
      QHostAddress addr;
      quint16 port;
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
    }
    
    void UdpImageOutput::send(const core::ImgBase *image){
      ICLASSERT_THROW(!isNull(), ICLException("UdpImageOutput::send: instance was null!"));
      const CompressedData data = compress(image);
      
      m_data->socket->writeDatagram((char*)data.bytes, data.len, m_data->addr, m_data->port);
    }

  }
}

