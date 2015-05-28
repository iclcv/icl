/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RSBPointCloudSender.cpp            **
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

#include <ICLGeom/RSBPointCloudSender.h>
#include <ICLGeom/PointCloudSerializer.h>
#include <ICLGeom/RSBPointCloud.pb.h>
#include <ICLGeom/ProtoBufSerializationDevice.h>

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/PluginRegister.h>

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>


using namespace boost;
using namespace rsb;
using namespace rsb::converter;


namespace icl{

  using namespace utils;
  using namespace core;

  namespace geom{

    struct StaticRSBPointCloudTypeRegistration{
      StaticRSBPointCloudTypeRegistration(){
        shared_ptr<ProtocolBufferConverter<RSBPointCloud> > p(new ProtocolBufferConverter<RSBPointCloud>());
        //#if RSB_VERSION_MINOR < 8 && RSB_VERSION_MAJOR < 1
        //        stringConverterRepository()->registerConverter(p);
        //#else
        converterRepository<std::string>()->registerConverter(p);
        //#endif
  
  
      }
    } static_RSBImage_type_registration;

    struct RSBPointCloudSender::Data{
      //Mutex mutex;
      Informer<RSBPointCloud>::Ptr informer;
      Informer<RSBPointCloud>::DataPtr out;
      SmartPtr<ProtoBufSerializationDevice> sdev;
    };
    
    RSBPointCloudSender::RSBPointCloudSender(const std::string &scope, 
                                             const std::string &transportList):m_data(new Data){
      if(scope.length()) init(scope,transportList);
    }
    
    RSBPointCloudSender::~RSBPointCloudSender(){
      delete m_data;
    }

    void RSBPointCloudSender::init(const std::string &scope, 
                                   const std::string &transportList){
      if(!scope.length()) throw ICLException("RSBPointCloudSender::init: empty scope string");
      
      Scope rsbScope(scope);
      Factory &factory = rsb::getFactory();
      ParticipantConfig rsbCfg = factory.getDefaultParticipantConfig();
      typedef std::set<ParticipantConfig::Transport> TSet;
      typedef std::vector<ParticipantConfig::Transport> TVec;
      
      TSet ts2 = rsbCfg.getTransports(true);
      TVec ts(ts2.begin(),ts2.end());
       
      std::vector<std::string> transports = tok(transportList,",");
      int port = 0;
      for(size_t i=0;i<transports.size();++i){
        std::string &t = transports[i];
        if(t.length() > 7 && t.substr(0,6) == "socket" && t[6] == '@'){
          port = parse<int>(t.substr(7));
          t = "socket";
        }
      }
      
      for(TVec::iterator it = ts.begin(); it != ts.end(); ++it){
        ParticipantConfig::Transport &t = *it;
        if( find(transports.begin(), transports.end(), it->getName()) == transports.end() ){
          t.setEnabled(false);
        }else{
          it->setEnabled(true);
          if(it->getName() == "socket" && port > 0){
            rsc::runtime::Properties &ps = it->mutableOptions();
            ps.set<std::string,std::string>("port",str(port));
          }
        }
      }
      rsbCfg.setTransports(TSet(ts.begin(),ts.end()));

      m_data->informer = factory.createInformer<RSBPointCloud>(rsbScope,rsbCfg);
      m_data->out = Informer<RSBPointCloud>::DataPtr(new RSBPointCloud);
      
      m_data->sdev = new ProtoBufSerializationDevice(&*m_data->out);
    }
      
    bool RSBPointCloudSender::isNull() const{
      return !m_data->informer;
    }
      
    void RSBPointCloudSender::send(const PointCloudObjectBase &dst){
      PointCloudSerializer::serialize(dst,*m_data->sdev);
      m_data->informer->publish(m_data->out);
    }


    static PointCloudOutput *create_rsb_point_cloud_sender(const std::map<std::string,std::string> &d){
      std::map<std::string,std::string>::const_iterator it = d.find("creation-string");
      if(it == d.end()) return 0;
      const std::string &params = it->second;
      std::vector<std::string> ts = tok(params,":");
      if(ts.size() ==  1){
        return new RSBPointCloudSender(ts[0]);
      }else if(ts.size() == 2){
        return new RSBPointCloudSender(ts[1],ts[0]);
      }else{
        throw ICLException("unable to create RSBPointCloudSender from given parameter string '"+params+"'");
      }
      return 0;
    }
    
    REGISTER_PLUGIN(PointCloudOutput,rsb,create_rsb_point_cloud_sender,
                    "RSB based point cloud output",
                    "creation-string: [comma-sep-transport-list:]rsb-scope");

  } 
}

