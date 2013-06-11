/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/UdpGrabber.cpp                         **
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

#include <ICLIO/UdpGrabber.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Mutex.h>
#include <ICLIO/ImageCompressor.h>
#include <QtNetwork/QUdpSocket>

namespace icl{
  
  using namespace utils;
  using namespace core;
  
  namespace io{
    
    struct UdpGrabber::Data{
      QUdpSocket *socket;
      std::vector<icl8u> receivedBuffer;
      int receivedSize;
      Mutex mutex;
      ImageCompressor cmp;
    };
    
    void UdpGrabber::init(int port) throw (utils::ICLException){
      if(m_data) throw ICLException("UdpGrabber was tried to initialize twice!");
      m_data = new Data;
      m_data->socket = new QUdpSocket(this);
      m_data->socket->bind((quint16)port, QUdpSocket::ShareAddress);
      //m_data->socket->bind(12345, QUdpSocket::ShareAddress);
      connect(m_data->socket, SIGNAL(readyRead()), this, SLOT(processData()));
    }
    
    UdpGrabber::UdpGrabber(int port) throw(utils::ICLException):m_data(0){
      if(port < 0) return;
      init(port);
    }
    
    UdpGrabber::~UdpGrabber(){
      if(m_data) {
        delete m_data->socket;
        delete m_data;
      };
    }
    
    void UdpGrabber::processData(){
      while(m_data->socket->hasPendingDatagrams()){
        DEBUG_LOG("UdpGrabber::grabber-thread: found pending event");
        m_data->mutex.lock();
        size_t s = m_data->socket->pendingDatagramSize();
        if(m_data->receivedBuffer.size() < s) {
          m_data->receivedBuffer.resize(s);
        }
        m_data->receivedSize = s;
        m_data->socket->readDatagram((char*)m_data->receivedBuffer.data(),s);
        m_data->mutex.unlock();
      }
    }

    const std::vector<GrabberDeviceDescription> &UdpGrabber::getDeviceList(bool rescan){
      (void)rescan;
      static std::vector<GrabberDeviceDescription> deviceList;
      return deviceList;
    }
    
    const core::ImgBase* UdpGrabber::acquireImage(){
      static Img8u bla(Size(100,100),1);
      return &bla;

      DEBUG_LOG("acquireImage called");
      while(true){
        m_data->mutex.lock();
        if(m_data->receivedBuffer.size()) break;
        m_data->mutex.unlock();
        Thread::msleep(10);
      }
      DEBUG_LOG("data available: uncopressing");
      const ImgBase *image = m_data->cmp.uncompress(m_data->receivedBuffer.data(), m_data->receivedSize);
      m_data->mutex.unlock();
      return image;
    }

    REGISTER_CONFIGURABLE(UdpGrabber, return new UdpGrabber(44444));

    Grabber* createUdpGrabber(const std::string &param){
      return new UdpGrabber(parse<int>(param));
    }

    const std::vector<GrabberDeviceDescription>& getUdpDeviceList(std::string filter, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(filter.size()){
        GrabberDeviceDescription d("udp", 
                                   filter, 
                                   "A grabber for images "
                                   "published via udp");
        deviceList.push_back(d);
      }
      return deviceList;
    }


    REGISTER_GRABBER(udp,utils::function(createUdpGrabber), utils::function(getUdpDeviceList), "udp:port:QUdpSocket-based udp network transfer")
    
  } // namespace io
}

