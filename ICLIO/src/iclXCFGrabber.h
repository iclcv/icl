#ifdef HAVE_XCF
#ifndef ICLXCFGRABBER_H
#define ICLXCFGRABBER_H

#include <iclGrabber.h>
#include <string>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>

namespace icl {
  
   /// Grabber to access XCF Image Server
   /** The XCFGrabber provides access to an XCF Image Server. */
  
   class XCFGrabber : public Grabber {
   public:
    
      /// Base constructor
      XCFGrabber(const std::string& sServer, 
                 ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                 ::XCF::Implementation::Properties::singleton()
                 ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    
      /// Destructor
      ~XCFGrabber(void);
    
      /// grabbing function  
      /** \copydoc icl::Grabber::grab(icl::ImgBase**)  **/    
      virtual const ImgBase* grab(ImgBase **poDst=0);
      
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
   private:   
      /// retrieve most current image set in provided composite transport unit
      void receive (XCF::CTUPtr& result);

      xmltio::Location     m_locRequest;
      XCF::RemoteServerPtr m_remoteServer;
      XCF::CTUPtr          m_result;
      ImgBase*             m_poSource;
   };
  
}

#endif
#endif // HAVE_XCF
