#include "iclXCFPublisherGrabber.h"
#include "iclXCFUtils.h"
#include <iclThread.h>

namespace icl{

  XCFPublisherGrabber::XCFPublisherGrabber(const std::string &streamName, 
                                           XCF::RecoverLevel l){
    m_subscriber = XCF::Subscriber::create(streamName, XCF::NONE);
    m_subscriber->setOnlyReceiveLast (true);
    // and on success, set desired recover level
    m_subscriber->setRecoverLevel (l);
  }

  XCFPublisherGrabber::~XCFPublisherGrabber(){
    m_subscriber->destroy();
  }
  
  
  void XCFPublisherGrabber::receive (XCF::CTUPtr& result){
    ICLASSERT_RETURN(m_subscriber->isAlive());
    while(true){
      try{
        result = m_subscriber->receiveBinary(true);
        return;
      }catch(XCF::PublisherEmptyException &ex){
        Thread::msleep(100);
      }
    }
  } 
  
}

