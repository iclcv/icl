#include <iclXCFGrabber.h>
#include <xmltio/xmltio.hpp>

using namespace std;
using namespace xmltio;

namespace icl {

   void extractImage (XCF::CTUPtr ctu, xmltio::Location l, ImgBase*& poImg) {
      const string& sURI = extract<string>(l["@uri"]);
      xmltio::Location  p(l, "PROPERTIES");
      int iWidth   = extract<int>(p["width"]);
      int iHeight  = extract<int>(p["height"]);
      depth eDepth = depth8u; // extract<int>(p[XPath("@depth")]);
      int iChannels   = extract<int>(p["channels"]);
		icl::format fmt = translateFormat(extract<string>(p["format"]));
      
      Location  r (l, "ROI");
      icl::Rect roi ((int) extract<int>(r["offsetX"]), 
                     (int) extract<int>(r["offsetY"]),
                     (int) extract<int>(r["width"]),
                     (int) extract<int>(r["height"]));

      ensureCompatible (&poImg, eDepth, Size(iWidth, iHeight), iChannels, fmt, roi);

		Time::value_type t 
         = extract<Time::value_type>(l[XPath("TIMESTAMPS/CREATED")]);
      poImg->setTime (Time::microSeconds (t));

      XCF::Binary::TransportUnitPtr btu = ctu->getBinary (sURI);
      XCF::Binary::TransportVecByte *pTypedBTU = dynamic_cast<XCF::Binary::TransportVecByte*>(btu.get());
      const std::vector<Ice::Byte> &vecImage = pTypedBTU->value;
    
      int imgSize = iWidth * iHeight * getSizeOf(eDepth);
      for (int i=0; i < iChannels; i++) {
         memcpy(poImg->getDataPtr(i), &vecImage[i*imgSize], imgSize);
      }
   }

   XCFGrabber::XCFGrabber (const std::string& sServer, XCF::RecoverLevel l) :
      m_locRequest ("<IMAGEREQUEST/>"), m_remoteServer(0)
   {
      // create remote server instance
      m_remoteServer = XCF::RemoteServer::create(sServer, XCF::NONE);
      // and on success, set default recover level
      m_remoteServer->setRecoverLevel (l);
   }

   XCFGrabber::~XCFGrabber () {
      m_remoteServer->destroy ();
   }

   const ImgBase* XCFGrabber::grab (ImgBase **ppoDst) {
      receive (m_result);

      Location loc (m_result->getXML(), "/IMAGESET/IMAGE[@uri]");

      ImgBase *poOutput = prepareOutput (ppoDst);
      extractImage (m_result, loc, m_poSource);
      m_oConverter.apply (m_poSource, poOutput);
      return poOutput;
   }

   void XCFGrabber::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);

      vGrabbedImages.resize (m_result->getBinaryMap().size());
      xmltio::Location locResult (m_result->getXML(), "/IMAGESET");

      XPathIterator locIt = XPath("Image[@uri]").evaluate(locResult);
      vector<ImgBase*>::iterator imgIt = vGrabbedImages.begin();
      for (; locIt; ++locIt, ++imgIt) {
         ImgBase *poOutput = prepareOutput (&(*imgIt));
         extractImage (m_result, *locIt, m_poSource);
         m_oConverter.apply (m_poSource, poOutput);
      }
   }
   
   void XCFGrabber::receive (XCF::CTUPtr& result) {
      m_locRequest["timestamp"] = ""; // most-recent image
      
      string xml ("<IMAGEREQUEST>");
      m_remoteServer->callMethod ("retrieveImages", 
                                  m_locRequest.getDocumentText(), result);
   }
}
