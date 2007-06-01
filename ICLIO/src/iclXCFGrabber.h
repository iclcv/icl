#ifdef HAVE_XCF
#ifndef ICLXCFGRABBER_H
#define ICLXCFGRABBER_H

#include <iclGrabber.h>
#include <string>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <iclBayer.h>

namespace icl {
  
   /// Create descriptive XML xmltio::Location for given ICL image
	xmltio::Location createXML(const ImgBase* poImg, 
                              const std::string& uri="image", 
                              const std::string& bayerPattern="");
   /// Pack image into XCF binary transport unit
   XCF::Binary::TransportUnitPtr packImage (const ImgBase* poImg, 
                                            XCF::Binary::TransportUnitPtr btu=0);
   /// Extract image from XCF composite transport unit using given XML description 
   /** ICL image poImg is created from XCF composite transport unit
       @arg ctu   CTU containing various images in its <IMAGESET>
       @arg l     XML Location describing parameters of image
       @arg poImg destination image pointer
   */
   void extractImage (const XCF::CTUPtr ctu, const xmltio::Location& l, 
                      ImgBase*& poImg);


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
   private:   
      /// retrieve most current image set in provided composite transport unit
      void receive (XCF::CTUPtr& result);

      xmltio::Location     m_locRequest;
      XCF::RemoteServerPtr m_remoteServer;
      XCF::CTUPtr          m_result;
      ImgBase*             m_poSource;
	  BayerConverter*	   m_poBayerConverter;

   };
  
}

#endif
#endif // HAVE_XCF
