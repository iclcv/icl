#include "iclXCFMemoryGrabber.h"

#include <log4cxx/propertyconfigurator.h>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "Memory/Interface.hpp"
#include "Memory/SynchronizedQueue.hpp"

#include <iclProgArg.h>
#include <string>
#include <cstdlib>

#include <iclStringUtils.h>
#include <cstring>

using memory::interface::MemoryPtr;
using memory::interface::MemoryInterface;
using memory::interface::EventSource;
using memory::interface::TriggeredAction;
using memory::interface::Condition;
using memory::interface::Subscription;
using memory::interface::Event;
using memory::interface::MemoryInterfaceException;
using memory::interface::Attachments;

namespace icl{

  struct XCFMemoryGrabberImpl{
    std::string xpath;
    MemoryPtr memInterface;
    boost::shared_ptr<EventSource> evtSrc;
    TriggeredAction *action;
    Condition condition;
    Subscription *subscription;
    
    XCFMemoryGrabberImpl(const std::string &memoryName,const std::string &xpath):
      xpath(xpath),action(0),subscription(0){
      try{
        memInterface = MemoryPtr(MemoryInterface::getInstance(memoryName));

        evtSrc = boost::shared_ptr<EventSource>(new EventSource());
        
        action = new TriggeredAction(boost::bind(&EventSource::push, evtSrc, _1));
        
        condition = Condition(Event::INSERT,xpath);
        
        subscription = new Subscription(memInterface->add(condition,*action));
        
      }catch(const MemoryInterfaceException& ex){
        std::cerr << "MemoryInterfaceException: " << ex.what() << std::endl;
      }catch(const std::exception &ex){
        std::cerr << "std::exception: " << ex.what() << std::endl; 
      }catch(...){
        std::cerr << "An Unknown Error Occured!" << std::endl;
      }
    }
    
    const ImgBase *grab(const ImgBase **ppoDst){
      Event e;
      if(evtSrc->next(e)){ // this call locks!
        
        Attachments a;
        memInterface->getAttachments(e.getDocument().getRootLocation().getDocumentText(),a);
        
        // now extract the image from e.getDocument() and the binary attachment!
      }else{
        // exception or something
      }
      return 0;
      
    }
  };


  /// this can be implemented only if XCFMemoryGrabberImpl is defined properly (here)
  void XCFMemoryGrabberImplDelOp::delete_func(XCFMemoryGrabberImpl *impl){
      ICL_DELETE(impl);
  }
  


  XCFMemoryGrabber::XCFMemoryGrabber(const std::string &memoryName, const std::string &imageXPath):
    ParentSC(new XCFMemoryGrabberImpl(memoryName,imageXPath)){
    
  }

  const ImgBase *XCFMemoryGrabber::grab(const ImgBase **ppoDst){
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return impl->grab(ppoDst);
  }

}
