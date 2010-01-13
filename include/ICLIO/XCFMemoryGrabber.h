#ifdef HAVE_XCF
#ifndef ICL_XCF_MEMORY_GRABBER_H
#define ICL_XCF_MEMORY_GRABBER_H

#include <ICLIO/Grabber.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  
  /** \cond */
  class XCFMemoryGrabberImpl;
  struct XCFMemoryGrabberImplDelOp{ 
    static void delete_func(XCFMemoryGrabberImpl *impl);
  };
  /** \endcond */
  
  
  /// Grabber implementation to acquire images from an ActiveMemory Server application
  class XCFMemoryGrabber : public Grabber{
    public:
    /// Create a new instance
    XCFMemoryGrabber(const std::string &memoryName,const std::string &imageXPath="//IMAGE");

    /// get next image
    virtual const ImgBase *grabUD(ImgBase **ppoDst=0);
   
    // TODO setIgnoreDesiredParams(bool) 
    // TODO const xmltio::TIODocment *getLastDocument() const;
    SmartPtr<XCFMemoryGrabberImpl,XCFMemoryGrabberImplDelOp> impl;
  };
  
}


#endif // ICL_XCF_MEMORY_GRABBER_H

#endif // HAVE_XCF
