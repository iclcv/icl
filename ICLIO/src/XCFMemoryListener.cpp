/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLIO/XCFMemoryListener.h>

#include <log4cxx/propertyconfigurator.h>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <Memory/SynchronizedQueue.hpp>

#include <ICLUtils/Time.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>

using namespace memory::interface;
using namespace xmltio;
using namespace icl;


namespace icl{

    
  XCFMemoryListener::XCFMemoryListener(memory::interface::MemoryPtr mem, 
                                       const std::string &xpath, 
                                       const std::string &eventTypeList){
    this->mem = mem;
    memoryName = "";
    init(xpath,eventTypeList);
  }
  
  
  XCFMemoryListener::XCFMemoryListener(const std::string &memoryName, 
                                 const std::string &xpath,
                                 const std::string &eventTypeList){
    mem = MemoryInterface::getInstance(memoryName); 
    if(!mem) throw ICLException("unable to find connection to memory interface with memory name " + memoryName);
    
    this->memoryName = memoryName;

    init(xpath,eventTypeList);

  }
  
  void XCFMemoryListener::init(const std::string &xpath, const std::string &eventTypeList){
    printTimeStamps = true;
    printPretty = true;
    printSubLocationsOnly = true;

    std::vector<std::string> es = tok(eventTypeList,"|");
    
    flags = 0;
    for(unsigned int i=0;i<es.size();++i){
      if(es[i]=="REMOVE") flags |= Event::REMOVE;
      else if(es[i]=="REPLACE") flags |= Event::REPLACE;
      else if(es[i]=="INSERT") flags |= Event::INSERT;
      else {
        ERROR_LOG("noting known about event type \"" << es[i] << "\" (skipping)");
      }
    }
    if(flags == 0){
      ERROR_LOG("no valid eventtype was found: using all (REPLACE|INSERT|REMOVE)");
      flags = Event::REPLACE | 
              Event::INSERT  |
              Event::REMOVE;
    }
    this->xpath = xpath;
  
  }
  
  void XCFMemoryListener::handle(Location rootLoc){
    if(printTimeStamps){
      std::cout << Time::now().toString() << std::endl;
      std::cout << "------------------------------------------------------------" << std::endl;
    }
    if(printSubLocationsOnly){
      for(XPathIterator it = XPath(xpath).evaluate(rootLoc);it;++it){
        std::cout << (*it).getText(printPretty) << std::endl << std::endl;
      }
    }else{
      std::cout << rootLoc.getText(printPretty) << std::endl;
    }
    std::cout << "------------------------------------------------------------" << std::endl;
  }
    
  void XCFMemoryListener::run(){
    if(!mem){
      ERROR_LOG("unable to start listener thread: memory interface is null");
    }
    boost::shared_ptr<EventSource> evtsrc =  boost::shared_ptr<EventSource>(new EventSource());  
    TriggeredAction action(boost::bind(&EventSource::push, evtsrc, _1));
    Condition cond(flags, (xpath== "") ? str("/") : xpath);
    Subscription sub(mem->add(cond,action));
    
    Event e;
    
    while (evtsrc->next(e)) { 
      TIODocument doc(e.getMemoryElement().asString());
      Location loc(doc.getRootLocation());
      handle(doc.getRootLocation());
    }
  }
  
}
