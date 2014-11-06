#pragma once

#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Point.h>
#include <string>

namespace icl{
  class Menu : public utils::Uncopyable{
    struct Data;
    Data *m_data;
    public: 
    typedef utils::Function<void,const std::string&> callback;
    
    Menu();
    
    Menu(const std::string &commaSepEntries);

    ~Menu();
    
    void addEntry(const std::string &entry);

    void addEntries(const std::string &commaSepEntryList);
    
    void setCallback(callback cb);
    
    void show(const utils::Point &screenPos);
  };
}
