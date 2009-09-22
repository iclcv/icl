#ifndef ICL_XCF_MEMORY_LISTENER_H
#define ICL_XCF_MEMORY_LISTENER_H

#include <memory>
#include <stdint.h>
#include <Memory/Interface.hpp>
#include <iclThread.h>

namespace icl{
  
  class XCFMemoryListener : public Thread{
    std::string memoryName;
    memory::interface::MemoryPtr mem;
    std::string xpath;
    int flags;
    bool printTimeStamps;
    bool printPretty;
    bool printSubLocationsOnly;
    
    void init(const std::string &xpath, const std::string &eventTypeList);
    public:
    
    /// empty constructor
    XCFMemoryListener(){}
    XCFMemoryListener(memory::interface::MemoryPtr mem,
                      const std::string &xpath="/",
                      const std::string &eventTypeList="REPLACE|INSERT|REMOVE");
    
    XCFMemoryListener(const std::string &memoryName, 
                   const std::string &xpath="/",
                   const std::string &eventTypeList="REPLACE|INSERT|REMOVE");
    
    memory::interface::MemoryPtr getMemory(){ return mem; }
    
    /// default implementation just prints received docs to std::out
    virtual void handle(xmltio::Location rootLoc);
    virtual void run();
    
    void setPrintTimeStamps(bool enabled){
      printTimeStamps = enabled;
    }
    void setPrintPretty(bool enabled){
      printPretty = enabled;
    }
    void setPrintSubLocationsOnly(bool enabled){
      printSubLocationsOnly = enabled;
    }
    

  };
  
}

#endif
