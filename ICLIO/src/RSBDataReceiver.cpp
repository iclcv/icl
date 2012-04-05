/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/RSBDataReceiver.cpp                          **
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

#include <ICLIO/RSBDataReceiver.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  struct RSBDataReceiver::Data{
    Mutex mutex;
    bool storeLast;
    std::string lastMetaData;
    std::vector<RSBDataReceiver::callback> callbacks;
  };

  void RSBDataReceiver::newDataAvailable(const ImgBase *image, const std::string &metadata){
    Mutex::Locker l(m_data->mutex);
    for(size_t i=0;i<m_data->callbacks.size();++i){
      m_data->callbacks[i](image,metadata);
    }
    if(m_data->storeLast){
      m_data->lastMetaData = metadata;
    }else{
      m_data->lastMetaData = std::string();
    }
  }

  RSBDataReceiver::RSBDataReceiver():m_data(0){}
  
  RSBDataReceiver::RSBDataReceiver(const std::string &scope, const std::string &transportList, bool storeLast):
    RSBGrabberImpl(scope,transportList),m_data(new Data){
    m_data->storeLast = storeLast;
  }

  RSBDataReceiver::~RSBDataReceiver(){
    ICL_DELETE(m_data);
  }
  
  void RSBDataReceiver::init(const std::string &scope, const std::string &transportList, bool storeLast){
    if(!m_data){
      m_data = new Data;
    }
    Mutex::Locker l(m_data->mutex);
    RSBGrabberImpl::init(scope,transportList);
    m_data->storeLast = storeLast;
  }                                             
    
  const ImgBase *RSBDataReceiver::receive(std::string *meta){
    const ImgBase *image = RSBGrabberImpl::grab();
    Mutex::Locker l(m_data->mutex);
    if(meta && !m_data->storeLast){
      ERROR_LOG("cannot adapt given metadata string (please enalble the storeLast)");
    }
    if(meta) *meta = m_data->lastMetaData;
    return image;
  }
    
  void RSBDataReceiver::registerCallback(callback cb){
    Mutex::Locker l(m_data->mutex);
    m_data->callbacks.push_back(cb);
  }
    
  void RSBDataReceiver::removeCallbacks(){
    Mutex::Locker l(m_data->mutex);
    m_data->callbacks.clear();  
  }
    
}
