#ifdef HAVE_XCF
#ifndef ICL_XCF_SERVER_GRABBER_H
#define ICL_XCF_SERVER_GRABBER_H

#include <iclGrabber.h>
#include <string>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <iclBayer.h>

namespace icl {
  

   /// Grabber to access XCF Image Server \ingroup GRABBER_G
   /** The XCFServerGrabber provides access to an XCF Image Server. */
  
   class XCFServerGrabber : public Grabber {
   public:
    
      /// Base constructor
      XCFServerGrabber(const std::string& sServer, 
                 ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                 ::XCF::Implementation::Properties::singleton()
                 ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    
      /// Destructor
      ~XCFServerGrabber(void);
    
      /// grabbing function  
      /** \copydoc icl::Grabber::grab(ImgBase**)*/
      virtual const ImgBase* grab(ImgBase **ppoDst=0);
      
      /// grabbing a whole image set, e.g. from a stereo camera
      /** The vector of images is resized to match the number of grabbed
          images. Existing images in the vector are adapted to the desired
          output depth and parameters. Ownership for all grabbed images goes
          to the caller! 
      */
      void grab (std::vector<ImgBase*>& vGrabbedImages);

      /// set XCF recover level
      void setRecoverLevel (XCF::RecoverLevel l) {
         m_remoteServer->setRecoverLevel (l);
      }

      /// set request string for image server
      void setRequest (const std::string& sRequest);

   private:   
      /// retrieve most current image set in provided composite transport unit
      void receive (XCF::CTUPtr& result);
      void makeOutput (const xmltio::Location& l, ImgBase* poOutput);
      
      xmltio::Location     m_locRequest;
      XCF::RemoteServerPtr m_remoteServer;
      XCF::CTUPtr          m_result;
      ImgBase*             m_poSource;
      BayerConverter*	   m_poBayerConverter;
      ImgBase*             m_poBayer;
   };
  
}

#endif
#endif // HAVE_XCF
