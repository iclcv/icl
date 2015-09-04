/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/RSBIOUtil.h                            **
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

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Mutex.h>
#include <string>
#include <vector>
#include <map>

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <rst/generic/Value.pb.h>
#include <rst/generic/Dictionary.pb.h>
#include <rst/generic/KeyValuePair.pb.h>

namespace icl{
  namespace io{

    
    
    // Tier 2: Basic data containment slice
    template<class T>
    struct RSBIOUtilDataBase{
      virtual ~RSBIOUtilDataBase(){}
      
      typedef rsb::Informer<T> Informer;
      typedef typename Informer::Ptr InformerPtr;
      typedef typename Informer::DataPtr DataPtr;
      typedef typename rsb::Scope Scope;
      typedef typename rsb::ListenerPtr ListenerPtr;
      
      /// Callback type that is used for listener_callbacks
      typedef typename utils::Function<void,const T&> Callback;
      
      InformerPtr m_informer;
      DataPtr m_data;
      Scope m_scope;
      ListenerPtr m_listener;
      
      utils::Mutex m_mutex;
      std::map<std::string,Callback> m_callbacks;
    };

    /// Tier 3: branding for using protocol-buffer types by  default!
    template<class T>
    struct RSBIOUtilDataExtra : public RSBIOUtilDataBase<T>{
      typedef typename rsb::converter::ProtocolBufferConverter<T> Converter;
      typedef typename boost::shared_ptr<Converter> ConverterPtr;
      static void register_type(){
        ConverterPtr c(new Converter);
        rsb::converter::converterRepository<std::string>()->registerConverter(c);
      }
    };

    /// specialization non-protocol-buffer types
#define REGISTER_RSBIOUtil_COMMON_TYPE(T,NAME,FULL_NAME)                \
    template<>                                                          \
    struct RSBIOUtilDataExtra<T> : public RSBIOUtilDataBase<T>{         \
      static void register_type(){}                                     \
    };
     
    
    REGISTER_RSBIOUtil_COMMON_TYPE(std::string,string,primitive.string);
    REGISTER_RSBIOUtil_COMMON_TYPE(int32_t,int,primitive.int);
    REGISTER_RSBIOUtil_COMMON_TYPE(uint32_t,int,primitive.int);
    REGISTER_RSBIOUtil_COMMON_TYPE(int64_t,int,primitive.int);
    REGISTER_RSBIOUtil_COMMON_TYPE(uint64_t,int,primitive.int);
    REGISTER_RSBIOUtil_COMMON_TYPE(float,float,primitive.float);
    REGISTER_RSBIOUtil_COMMON_TYPE(double,double,primitive.double);
    REGISTER_RSBIOUtil_COMMON_TYPE(bool,bool,primitive.bool);

    /*   
        REGISTER_RSBIOUtil_COMMON_TYPE( ::rst::generic::Value,Value,rst.generic.Value);
        REGISTER_RSBIOUtil_COMMON_TYPE( ::rst::generic::Dictionary,Dictionary,rst.generic.Dictionary);
        REGISTER_RSBIOUtil_COMMON_TYPE( ::rst::generic::KeyValuePair,KeyValuePair,rst.generic.KeyValuePair);
    */

    /// Simple and ready to use RSB-Informer and RSB-Listener Interface
    /** This utility class will help to significantly reduce the amount
        of boiler-plate code in 95% of all cases where RSB-communication 
        is needed. 
        In the current implementation, only protocol buffer types are 
        supported.
    */
    template<class T>
    class RSBIOUtil : public RSBIOUtilDataExtra<T>, public utils::Uncopyable{
      public:
        
      typedef  RSBIOUtilDataExtra<T> Super;
      /// creates an instance with given mode and scope
      /** mode must be either send or receive*/
      inline RSBIOUtil(const std::string &mode, const std::string &scope, bool autoRegisterType=true){
        if(autoRegisterType){
          static struct StaticTypeRegistration{
            StaticTypeRegistration(){
              Super::register_type();
            }
          } static_type_regitration;
        }
        
        Super::m_scope = rsb::Scope(scope);
        
        rsb::Factory &factory = rsb::getFactory();        
        if(mode == "send"){
          rsb::ParticipantConfig cfg = factory.getDefaultParticipantConfig();
          Super::m_informer = factory.createInformer<T>(Super::m_scope,cfg);
          Super::m_data = typename Super::DataPtr(new T); 
        }else  if(mode == "receive"){
          Super::m_listener = factory.createListener(Super::m_scope);
          typedef typename rsb::DataFunctionHandler<T> FHandler;
          typename FHandler::DataFunction f = boost::bind(&RSBIOUtil<T>::handle,this,_1);
          Super::m_listener->addHandler(rsb::HandlerPtr(new FHandler(f)));
        }else{
          throw utils::ICLException("RSBIOUtil: invalid mode " + mode);
        }      
      }

      void send(const T &t){
        *Super::m_data = t;
        Super::m_informer->publish(Super::m_data);
      }

      void send(typename Super::DataPtr data){
        Super::m_informer->publish(data);
      }
      
      void handle(typename Super::DataPtr data){
        utils::Mutex::Locker lock(Super::m_mutex);
        for(typename std::map<std::string,typename Super::Callback>::iterator it = Super::m_callbacks.begin();
            it != Super::m_callbacks.end();++it){
          it->second(*data);
        }
      }
      
      void registerListenerCallback(typename Super::Callback cb, const std::string &id="default"){
        utils::Mutex::Locker lock(Super::m_mutex);
        Super::m_callbacks[id] = cb;
      }
      
      void unregisterListenerCallback(const std::string &id="default"){
        utils::Mutex::Locker lock(Super::m_mutex);
        typename std::map<std::string,typename Super::Callback>::iterator it = Super::m_callbacks.find(id);
        if(it != Super::m_callbacks.end()){
          Super::m_callbacks.erase(it);
        }else{
          WARNING_LOG("could not remove callback " << id << " callback was not registered");
        }
      }
    };

    template<class T>
    class RSBSender {
      utils::SmartPtr<RSBIOUtil<T> > impl;
      public:

      typedef typename RSBIOUtilDataBase<T>::DataPtr DataPtr;
      
      inline RSBSender(const std::string &scope=""){
        if(scope.length()) init(scope);
      }
      inline bool isNull() const { return !impl; }
      
      inline operator bool() const { return !isNull(); }

      inline void init(const std::string &scope){
        impl = new RSBIOUtil<T>("send",scope);
      }

      inline void send(const T &t){
        null_check(__FUNCTION__);
        impl->send(t);
      }

      void send(DataPtr data){
        impl->send(data);
      }


      protected:
      inline void null_check(const std::string &fn) const throw (utils::ICLException) {
        if (isNull()) throw utils::ICLException(fn + ": RSBListener is null");
      }
    };
    
    template<class T>
    class RSBListener {
      utils::SmartPtr<RSBIOUtil<T> > impl;
      public:

      typedef typename RSBIOUtilDataBase<T>::Callback Callback;
       typedef typename RSBIOUtilDataBase<T>::DataPtr DataPtr;
       
      inline RSBListener(const std::string &scope=""){
        if(scope.length()) init(scope);
      }
      
      inline bool isNull() const { return !impl; }
      
      inline operator bool() const { return !isNull(); }
      
      inline void init(const std::string &scope){
        impl = new RSBIOUtil<T>("receive",scope);
      }

      void registerCallback(Callback cb, const std::string &id="default"){
        null_check(__FUNCTION__);
        impl->registerListenerCallback(cb,id);
      }

      void unregisterCallback(const std::string &id="default"){
        null_check(__FUNCTION__);
        impl->unregisterListenerCallback(id);
      }

      protected:
      inline void null_check(const std::string &fn) const throw (utils::ICLException) {
        if (isNull()) throw utils::ICLException(fn + ": RSBSender is null");
      }
      
    };
    
  }
}
