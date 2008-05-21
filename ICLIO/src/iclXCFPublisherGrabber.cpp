#include "iclXCFPublisherGrabber.h"
#include "iclXCFUtils.h"

namespace icl{

  XCFPublisherGrabber::XCFPublisherGrabber(const std::string &streamName){
    m_subscriber = XCF::Subscriber::create(streamName, XCF::NONE);
    m_subscriber->setOnlyReceiveLast (true);
    
  }

  XCFPublisherGrabber::~XCFPublisherGrabber(){
    m_subscriber->destroy();
  }
  
  
  void XCFPublisherGrabber::receive (XCF::CTUPtr& result){
    result = m_subscriber->receiveBinary(true);
  }
  
  
}

