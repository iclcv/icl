#ifdef HAVE_XCF
#ifndef ICL_XCF_PUBLISHER_GRABBER_H
#define ICL_XCF_PUBLISHER_GRABBER_H

#include "iclXCFGrabberBase.h"
#include <xcf/Subscriber.hpp>
#include <xmltio/Location.hpp>

namespace icl{

  class XCFPublisherGrabber : public XCFGrabberBase{
    public:
    XCFPublisherGrabber(const std::string &streamName, XCF::RecoverLevel l);
    virtual ~XCFPublisherGrabber();
    
    /// set XCF recover level
    void setRecoverLevel (XCF::RecoverLevel l) {
       m_subscriber->setRecoverLevel (l);
    }

    protected:
    virtual void receive (XCF::CTUPtr& result);
    
    private:
    XCF::SubscriberPtr m_subscriber;
  };
}

#endif // ICL_XCF_PUBLISHER_GRABBER_H

#endif // HAVE_XCF
