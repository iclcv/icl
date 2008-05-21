#ifdef HAVE_XCF
#ifndef ICL_XCF_SERVER_GRABBER_H
#define ICL_XCF_SERVER_GRABBER_H

#include <iclXCFGrabberBase.h>
#include <string>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <iclBayer.h>

namespace icl {
  

   /// Grabber to access XCF Image Server \ingroup GRABBER_G
   /** The XCFServerGrabber provides access to an XCF Image Server. */
  
   class XCFServerGrabber : public XCFGrabberBase {
   public:
    
      /// Base constructor
      XCFServerGrabber(const std::string& sServer, 
                 ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                 ::XCF::Implementation::Properties::singleton()
                 ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    
      /// Destructor
      ~XCFServerGrabber(void);
    
   
      /// set XCF recover level
      void setRecoverLevel (XCF::RecoverLevel l) {
         m_remoteServer->setRecoverLevel (l);
      }

      /// set request string for image server
      void setRequest (const std::string& sRequest);

   protected:   
      /// retrieve most current image set in provided composite transport unit
      virtual void receive (XCF::CTUPtr& result);
      
    private:
      xmltio::Location     m_locRequest;
      XCF::RemoteServerPtr m_remoteServer;
   };
  
}

#endif
#endif // HAVE_XCF
