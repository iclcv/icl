#pragma once

#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Point.h>
#include <string>

namespace icl{
  namespace physics{
    
    class PhysicsPaper3ContextMenu : public utils::Uncopyable{
      struct Data;
      Data *m_data;

      public: 
      typedef utils::Function<void,const std::string&> callback;
      
      PhysicsPaper3ContextMenu();
      
      PhysicsPaper3ContextMenu(const std::string &commaSepEntries);
      
      ~PhysicsPaper3ContextMenu();
      
      void addEntry(const std::string &entry);
      
      void addEntries(const std::string &commaSepEntryList);
      
      void setCallback(callback cb);
      
      void show(const utils::Point &screenPos);
    };
  }
}
