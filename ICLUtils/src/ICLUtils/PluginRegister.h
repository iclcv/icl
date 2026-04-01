// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/TextTable.h>

#include <functional>
#include <string>
#include <map>
#include <sstream>

namespace icl{
  namespace utils{

      /// Utility class for plugin registration
    template<class T>
    class PluginRegister{
      public:

      /// data used for instance creation
      using Data = std::map<std::string,std::string>;

      /// creator function for instances
      using CreateFunction = std::function<T*(const Data&)>;

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

      /// creates an instance (or throws)
      inline T *createInstance(const std::string &name, const Data &data){
        typename std::map<std::string,Plugin>::iterator it = plugins.find(name);
        if(it == plugins.end()){
          std::ostringstream all;
          for(typename std::map<std::string,Plugin>::iterator jt = plugins.begin(); jt != plugins.end();){
            all << jt->first;
            if(++jt != plugins.end()) all << ",";
          }
          throw utils::ICLException("PluginRegister<T>: unknow type: '"+ name + "' (registered are: " +all.str() + ")");
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
            it != plugins.end(); ++it, ++i){
          t(0,i) = it->second.name;
          t(1,i) = it->second.description;
          t(2,i) = it->second.creationSyntax;
        }
        return t.toString();
      }

      /// returns a all registered plugins
      const std::map<std::string,Plugin> &getRegisteredPlugins() const{
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
