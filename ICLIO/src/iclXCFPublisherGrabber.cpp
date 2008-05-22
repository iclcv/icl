#include "iclXCFPublisherGrabber.h"
#include "iclXCFUtils.h"
#include <iclThread.h>

namespace icl{

  XCFPublisherGrabber::XCFPublisherGrabber(const std::string &streamName){
    m_subscriber = XCF::Subscriber::create(streamName, XCF::NONE);
    m_subscriber->setOnlyReceiveLast (true);
  }

  XCFPublisherGrabber::~XCFPublisherGrabber(){
    m_subscriber->destroy();
  }
  
  
  void XCFPublisherGrabber::receive (XCF::CTUPtr& result){
    ICLASSERT_RETURN(m_subscriber->isAlive());
    bool ok = false;
    while(!ok){
      ok = true;
      try{
        result = m_subscriber->receiveBinary(true);
      }catch(XCF::PublisherEmptyException &ex){
        ok = false;
        Thread::msleep(100);
      }
    }
  }
  
  
}

