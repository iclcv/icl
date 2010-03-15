/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/XCFMemoryListener.h                      **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_XCF_MEMORY_LISTENER_H
#define ICL_XCF_MEMORY_LISTENER_H

#include <memory>
#include <stdint.h>
#include <Memory/Interface.hpp>
#include <ICLUtils/Thread.h>

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
