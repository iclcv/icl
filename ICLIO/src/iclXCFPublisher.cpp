#ifdef HAVE_XCF

#include "iclXCFPublisher.h"
#include "iclXCFUtils.h"

namespace icl{
  
  XCFPublisher::XCFPublisher(const std::string &streamName, const std::string &imageURI):m_uri(imageURI){
    m_publisher= XCF::Publisher::create(streamName);
    m_ctu = new XCF::CTU();
    m_btu = new XCF::Binary::TransportUnit();
  }
  
  XCFPublisher::~XCFPublisher(){
    m_publisher->destroy();
    
  }
  
  void XCFPublisher::publish(const ImgBase *image){
    xmltio::Location loc = XCFUtils::createXMLDoc(image,m_uri,"");
    std::string xml = loc.getText();
    m_ctu->setXML(xml);
    m_btu = XCFUtils::ImageToCTU(image,m_btu);
    m_ctu->deleteAllBinaries();
    m_ctu->addBinary(m_uri,m_btu);
    m_publisher->send(m_ctu);
  }
  
}

#endif
