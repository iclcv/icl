/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RSBPointCloudGrabber.cpp           **
** Module : ICLGeom                                                **
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

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <ICLGeom/RSBPointCloudGrabber.h>
#include <../ICLIO/RSBPointCloud.pb.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/StringUtils.h>

using namespace boost;
using namespace rsb;
using namespace rsb::converter;

namespace icl{
  using namespace io;
  using namespace utils;
  using namespace core;

  namespace geom{
  
    struct RSBPointCloudGrabber::Data{
      Mutex mutex;
      bool initialized;

      struct Handler : public DataHandler<RSBPointCloud>{
        RSBPointCloudGrabber::Data *data;
        Handler(RSBPointCloudGrabber::Data *data):data(data){}
        virtual void notify(shared_ptr<RSBPointCloud> image){
          data->receive(*image);
        }
      };
      
      void receive(const RSBPointCloud &pc){
        Mutex::Locker lock(mutex);
        
      } 
      
      shared_ptr<rsb::Handler> handler;
      ListenerPtr listener;
    };
    
    
    RSBPointCloudGrabber::RSBPointCloudGrabber(const std::string &scope, 
                                               const std::string &trasportList){
      m_data = new Data;
      m_data->initialized = false;
      if(scope.length()) init(scope,trasportList);
    }
    
    RSBPointCloudGrabber::~RSBPointCloudGrabber(){
      delete m_data;
    }
    
    void RSBPointCloudGrabber::init(const std::string &scope, const std::string &transportList){
      if(m_data->initialized){
        m_data->listener->removeHandler(m_data->handler);
      }
      Scope rsbScope(scope);
#if 1
      Factory &factory = rsc::patterns::Singleton<Factory>::getInstance();
#else 
      Factory &factory = Factory::getInstance()
#endif
      ParticipantConfig rsbCfg = factory.getDefaultParticipantConfig();
      typedef std::set<ParticipantConfig::Transport> TSet;
      typedef std::vector<ParticipantConfig::Transport> TVec;
      
      TSet ts2 = rsbCfg.getTransports(true);
      TVec ts(ts2.begin(),ts2.end());
      std::vector<std::string> transports = tok(transportList,",");

      for(TVec::iterator it = ts.begin(); it != ts.end(); ++it){
        ParticipantConfig::Transport &t = *it;
        if( find(transports.begin(), transports.end(), it->getName()) == transports.end() ){
          t.setEnabled(false);
        }else{
          it->setEnabled(true);
        }
      }
      rsbCfg.setTransports(TSet(ts.begin(),ts.end()));
      m_data->listener = factory.createListener(rsbScope,rsbCfg);
      m_data->handler = shared_ptr<Handler>(new Data::Handler(m_data));
      m_data->listener->addHandler(m_data->handler);
    }
      
    void RSBPointCloudGrabber::grab(PointCloudObjectBase &dst){
      
    }
  } // namespace geom
}

