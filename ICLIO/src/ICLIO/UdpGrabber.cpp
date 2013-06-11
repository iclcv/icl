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
    
    struct UdpGrabber::Data : public Thread{
      QUdpSocket *socket;
      std::vector<icl8u> receivedBuffer;
      int receivedSize;
      Mutex mutex;
      bool running;
      ImageCompressor cmp;
      
      virtual void run(){
        while(running){
          while(!socket->hasPendingDatagrams()){
            Thread::usleep(100);
          }
          while(socket->hasPendingDatagrams()){
            mutex.lock();
            size_t s = socket->pendingDatagramSize();
            if(receivedBuffer.size() < s) {
              receivedBuffer.resize(s);
            }
            receivedSize = s;
            socket->readDatagram((char*)receivedBuffer.data(),s);
            mutex.unlock();
          }
        }
      }
    };
    
    void UdpGrabber::init(int port) throw (utils::ICLException){
      if(m_data) throw ICLException("UdpGrabber was tried to initialize twice!");
      m_data = new Data;
      m_data->socket = new QUdpSocket(0);
      m_data->socket->bind((quint16)port, QUdpSocket::ShareAddress);
      m_data->running = true;
      m_data->start();
    }
    
    UdpGrabber::UdpGrabber(int port) throw(utils::ICLException):m_data(0){
      if(port < 0) return;
      init(port);
    }
    
    UdpGrabber::~UdpGrabber(){
      if(m_data) {
        m_data->running=false;
        m_data->stop();
        delete m_data->socket;
        delete m_data;
      };
    }
    
    const std::vector<GrabberDeviceDescription> &UdpGrabber::getDeviceList(bool rescan){
      (void)rescan;
      static std::vector<GrabberDeviceDescription> deviceList;
      return deviceList;
    }
    
    const core::ImgBase* UdpGrabber::acquireImage(){
      while(true){
        m_data->mutex.lock();
        if(m_data->receivedBuffer.size()) break;
        m_data->mutex.unlock();
        Thread::usleep(100);
      }
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
        GrabberDeviceDescription d("upd", 
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

