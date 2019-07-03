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

    static boost::shared_ptr<std::string> pack(const std::string &s){
      return boost::shared_ptr<std::string>(new std::string(s));
    }

    struct ConfigurableRemoteServer::Client : public Configurable{
      mutable RemoteServerPtr remoteServer;

      Client(const std::string &serverScope){
        Factory& factory = getFactory();
        remoteServer = factory.createRemoteServer(serverScope);
      }

      virtual void setPropertyValue(const std::string &propertyName, const Any &value){
        remoteServer->call<void>("setPropertyValue",pack(propertyName+"="+value));
      }
      virtual std::vector<std::string> getPropertyList() const{
        boost::shared_ptr<std::string> s = remoteServer->call<std::string>("getPropertyList");
        if(!s){
          ERROR_LOG("remote method getPropertyList: returned null");
          return std::vector<std::string>();
        }else{
          return tok(*s,",");
        }
      }

      std::string rmi(const std::string &propertyName, const std::string &method, bool intVersion=false) const{
        boost::shared_ptr<std::string> s = remoteServer->call<std::string>("getProperty"+method,
                                                                           pack(propertyName));
        if(!s){
          if(intVersion){
            return "0";
          }else{
            ERROR_LOG("remote method getProperty" << method << ": returned null");
            return "";
          }
        }else{
          return *s;
        }
      }

      virtual std::string getPropertyType(const std::string &propertyName) const{
        return rmi(propertyName,"Type");
      }
      virtual std::string getPropertyInfo(const std::string &propertyName) const{
        return rmi(propertyName,"Info");
      }
      virtual Any getPropertyValue(const std::string &propertyName) const{
        return rmi(propertyName,"Value");
      }
      virtual std::string getPropertyToolTip(const std::string &propertyName) const{
        return rmi(propertyName,"ToolTip");
      }
      virtual int getPropertyVolatileness(const std::string &propertyName) const{
        return parse<int>(rmi(propertyName,"Volatileness",true));
      }

    };


    struct ConfigurableRemoteServer::Data{
      Configurable *c;
      LocalServerPtr server;

      struct GenericCallback{
        Configurable *c;
        GenericCallback(Configurable *c):c(c){}


      };

      struct ListCallback : public LocalServer::Callback<void,std::string>, public GenericCallback{
        ListCallback(Configurable *c):GenericCallback(c){}

        boost::shared_ptr<std::string> call(const std::string&  /*methodName*/){
          boost::shared_ptr<std::string> s = pack(cat(c->getPropertyList(),","));
          //DEBUG_LOG("list callback called, returning '" << *s << "'");
          return s;
        }
      };

      struct InfoCallback : public LocalServer::Callback<std::string,std::string>, public GenericCallback{
        std::string method;
        InfoCallback(Configurable *c, const std::string &method):GenericCallback(c),method(method){}
        boost::shared_ptr<std::string> call(const std::string& methodName,
                                            boost::shared_ptr<std::string> arg){
          if(method == "Type"){
            return pack(c->getPropertyType(*arg));
          }else if(method == "Info"){
            return pack(c->getPropertyInfo(*arg));
          }else if(method == "Value"){
            return pack(c->getPropertyValue(*arg));
          }else if(method == "Volatileness"){
            return pack(str(c->getPropertyVolatileness(*arg)));
          }else if(method == "ToolTip"){
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
          std::vector<std::string> ts = tok(*arg,"=");
          if(ts.size() == 1) ts.push_back("");
          c->setPropertyValue(ts[0],ts[1]);
        }
      };

      Data(Configurable *c, const std::string &scope) : c(c){
        Factory &factory = getFactory();
        server = factory.createLocalServer(scope);
        server->registerMethod("getPropertyList", LocalServer::CallbackPtr(new ListCallback(c)));

        std::string ips[5] = { "Info", "Type", "Value", "Volatileness", "ToolTip" };
        for(int i=0;i<5;++i){
          LocalServer::CallbackPtr ptr(new InfoCallback(c,ips[i]));
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
      return new Client(remoteServerScope);
    }
  }
}
