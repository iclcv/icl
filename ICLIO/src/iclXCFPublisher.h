#ifdef HAVE_XCF

#ifndef ICL_XCF_PUBLISHER_H
#define ICL_XCF_PUBLISHER_H

#include <iclImgBase.h>
#include <xcf/Publisher.hpp>
#include <xcf/CTU.hpp>
#include <xcf/TransportObject.hpp>

namespace icl{
  class XCFPublisher{
    public:
    XCFPublisher();
    XCFPublisher(const std::string &streamName, const std::string &imageURI);
    
    ~XCFPublisher();
    
    void createPublisher(const std::string &streamName, 
                         const std::string &imageURI);
    
    void publish(const ImgBase *image);

    const std::string &getImageURI() const { return m_uri; }
    const std::string &getStreamName() const { return m_streamName; }
    
    private:
    XCF::PublisherPtr m_publisher;
    XCF::Binary::TransportUnitPtr m_btu;
    XCF::CTUPtr m_ctu;
    std::string m_uri;
    std::string m_streamName;
  };
}

#endif

#endif
