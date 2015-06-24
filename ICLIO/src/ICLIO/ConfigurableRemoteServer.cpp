/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ConfigurableRemoteServer.h             **
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

#include <ICLIO/ConfigurableRemoteServer.h>

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <rsb/Factory.h>

/*#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <rst/generic/Value.pb.h>
#include <rst/generic/Dictionary.pb.h>
#include <rst/generic/KeyValuePair.pb.h>
    */
namespace icl{
  namespace io{
    using namespace utils;
    using namespace rsb;
    using namespace rsb::patterns;
    
    struct ConfigurableRemoteServer::Data{
      Configurable *c;
      LocalServerPtr server;
      
      struct GenericCallback{
        Configurable *c;
        GenericCallback(Configurable *c):c(c){}
        
        static boost::shared_ptr<std::string> pack(const std::string &s){
          return boost::shared_ptr<std::string>(new std::string(s));
        }
      };
      
      struct ListCallback : public LocalServer::Callback<void,std::string>, public GenericCallback{
        ListCallback(Configurable *c):GenericCallback(c){}

        boost::shared_ptr<std::string> call(const std::string&  /*methodName*/){
          return pack(cat(c->getPropertyList(),","));
        }
      };

      struct InfoCallback : public LocalServer::Callback<std::string,std::string>, public GenericCallback{
        InfoCallback(Configurable *c):GenericCallback(c){}
        boost::shared_ptr<std::string> call(const std::string& methodName,
                                            boost::shared_ptr<std::string> arg){
          if(methodName == "getPropertyType"){
            return pack(c->getPropertyType(*arg));
          }else if(methodName == "getPropertyInfo"){
            return pack(c->getPropertyInfo(*arg));
          }else if(methodName == "getPropertyValue"){
            return pack(c->getPropertyValue(*arg));
          }else if(methodName == "getPropertyVolatileness"){
            return pack(str(c->getPropertyVolatileness(*arg)));
          }else if(methodName == "getPropertyToolTip"){
            return pack(c->getPropertyToolTip(*arg));
          }else{
            ERROR_LOG("invalid method name: " << methodName);
          }
          return pack("");
        }
      };
      struct SetCallback : public LocalServer::Callback<std::string,void>, public GenericCallback{
        SetCallback(Configurable *c):GenericCallback(c){}
        void call(const std::string&, /* method name */
                  boost::shared_ptr<std::string> arg){
          // arg is arg=value
          std::vector<std::string> ts = tok("arg","=");
          if(ts.size() == 1) ts.push_back("");
          c->setPropertyValue(ts[0],ts[1]);
        }
      };

      Data(Configurable *c, const std::string &scope) : c(c){
        Factory &factory = getFactory();
        server = factory.createLocalServer(scope);
        server->registerMethod("getPropertyList", LocalServer::CallbackPtr(new ListCallback(c)));
        LocalServer::CallbackPtr ptr(new InfoCallback(c));
        std::string ips[5] = { "Info", "List", "Value", "Volatileness", "ToolTip" };
        for(int i=0;i<5;++i){
          server->registerMethod("getProperty"+ips[i],ptr);
        }
        server->registerMethod("setPropertyValue", LocalServer::CallbackPtr(new SetCallback(c)));
      }
    };

    ConfigurableRemoteServer::ConfigurableRemoteServer():m_data(0){}
    ConfigurableRemoteServer::ConfigurableRemoteServer(utils::Configurable *configurable, 
                                                       const std::string &scope):m_data(0){
      init(configurable,scope);
    }
    ConfigurableRemoteServer::ConfigurableRemoteServer(const std::string &configurableID,
                                                       const std::string &scope):m_data(0){
      init(configurableID,scope);
    }
    
    void ConfigurableRemoteServer::init(utils::Configurable *configurable, 
                                        const std::string &scope){
      if(!configurable){
        ERROR_LOG("Could not initialize ConfigurableRemoteServer:"
                  " given input Configurable instance is null");
        return;
      }
      m_data = new Data(configurable,scope);
      /// here we go!
    }

    void ConfigurableRemoteServer::init(const std::string &configurableID,
                                        const std::string &scope){
      Configurable *c = Configurable::get(configurableID);
      if(c){
        init(c,scope);
      }else{
        ERROR_LOG("Could not initialize ConfigurableRemoteServer: "
                  "invalid configurableID:" << configurableID);
      }
    }
      
    ConfigurableRemoteServer::~ConfigurableRemoteServer(){
      ICL_DELETE(m_data);
    }
    
    utils::Configurable *ConfigurableRemoteServer::create_client(const std::string &remoteServerScope){
      /// well that is actually some work here!
      return 0;
    }
  }
}
