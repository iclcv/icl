/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/PluginRegister.h                 **
** Module : ICLUtils                                               **
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

#include <ICLUtils/Function.h>
#include <ICLUtils/TextTable.h>

#include <string>
#include <map>

namespace icl{
  namespace utils{

      /// Utility class for plugin registration
    template<class T>
    class PluginRegister{
      public:
      
      /// data used for instance creation
      typedef std::map<std::string,std::string> Data;
      
      /// creator function for instances
      typedef utils::Function<T*,const Data&> CreateFunction;

      /// internally used instance type:
      struct Plugin{
        std::string name;            //!< instance ID
        std::string description;     //!< description 
        std::string creationSyntax;  //!< syntax used for creation
        CreateFunction create;       //!< factory function
      };
      
      /// returns the static instance for the specific type
      static PluginRegister<T> &instance(){
        static PluginRegister<T> inst;
        return inst;
      }
        
      /// adds a new plugin
      inline void add(const std::string &name, CreateFunction create, 
               const std::string &description, const std::string &creationSyntax){
        Plugin p = { name, description, creationSyntax, create };
        plugins[name] = p;
      }
        
      /// creates an instance (or throws
      inline T *createInstance(const std::string &name, const Data &data) 
        throw (utils::ICLException){
        typename std::map<std::string,Plugin>::iterator it = plugins.find(name);
        if(it == plugins.end()){
          throw utils::ICLException("PluginRegister<T>: unknow type: "+ name);
          return 0;
        }
        return it->second.create(data);
      }
        
      /// returns a string representation of all registered types
      inline std::string getRegisteredInstanceDescription(){
        utils::TextTable t(3,plugins.size()+1,40);
        t(0,0) = "ID";
        t(1,0) = "Description";
        t(2,0) = "Creation Syntax";
        int i=1;
        for(typename std::map<std::string,Plugin>::iterator it = plugins.begin(); 
            it != plugins.end(); ++it){
          t(0,i) = it->second.name;
          t(1,i) = it->second.description;
          t(2,i) = it->second.creationSyntax;
        }
        return t.toString();
      }
      
      /// returns a all registered plugins
      const std::map<std::string,Plugin> getRegisteredPlugins() const{
        return plugins;
      }

      private:
      /// private constructor (use singelton creator function instance())
      PluginRegister(){}
      
      /// internal plugin list
      std::map<std::string,Plugin> plugins;
      
    };
  }
}

#define REGISTER_PLUGIN(TYPE,NAME,CREATE_FUNCTION,DESCRIPTION,SYNTAX)   \
  struct Static_##TYPE##_PluginRegistration__##NAME{                    \
    Static_##TYPE##_PluginRegistration__##NAME(){                       \
      PluginRegister<TYPE> &r = PluginRegister<TYPE>::instance();       \
      r.add(#NAME,CREATE_FUNCTION,DESCRIPTION,SYNTAX);                  \
    }                                                                   \
  } static_##TYPE##_PluginRegistration__##NAME;

