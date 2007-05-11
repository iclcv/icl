#include <iclXCFGrabber.h>
#include <xmltio/xmltio.hpp>

using namespace std;
using namespace xmltio;

namespace icl {

	const string IMG_XML_STRING = 
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<IMAGE uri=\"\">"
		"<TIMESTAMPS>"
		"<CREATED timestamp=\"\"/>"
		"</TIMESTAMPS>"
		"<PROPERTIES width=\"\" height=\"\" depth=\"\" channels=\"\" format=\"\" />"
		"<ROI offsetX=\"\" offsetY=\"\" width=\"\" height=\"\" />"
		"<BAYERFORMAT/>"
		"</IMAGE>";

	xmltio::Location createXML(ImgBase* poImg, string uri="image", int bayerFormat=0) {
			xmltio::Location l(IMG_XML_STRING);
			l[XPath("IMAGE/@uri")]					= uri;
			l[XPath("IMAGE/PROPERTIES/@width")]		= poImg->getWidth();
			l[XPath("IMAGE/PROPERTIES/@height")]	= poImg->getHeight();
			l[XPath("IMAGE/PROPERTIES/@depth")]		= translateDepth(poImg->getDepth());
			l[XPath("IMAGE/PROPERTIES/@channels")]	= poImg->getChannels();
			l[XPath("IMAGE/PROPERTIES/@format")]	= translateFormat(poImg->getFormat());
			l[XPath("IMAGE/ROI/@offsetX")]			= poImg->getROIXOffset();
			l[XPath("IMAGE/ROI/@offsetY")]			= poImg->getROIYOffset();
			l[XPath("IMAGE/ROI/@width")]			= poImg->getROIWidth();
			l[XPath("IMAGE/ROI/@height")]			= poImg->getROIHeight();
			l[XPath("IMAGE/TIMESTAMPS/CREATED")]	= poImg->getTime().toMicroSeconds();
			l[XPath("IMAGE/BAYERFORMAT")]			= bayerFormat;
			return l;
	}

	XCF::Binary::TransportUnitPtr createBTU (ImgBase* poImg, string uri="image") {
		XCF::Binary::TransportUnitPtr tup = new XCF::Binary::TransportVecByte;
		int imgSize = poImg->getWidth() * poImg->getHeight() * getSizeOf(poImg->getDepth());
		//TODO      !!!!!
		std::vector<icl8u> vecImage (imgSize * poImg->getChannels());
		for (int i=0; i<poImg->getChannels(); i++) {
			memcpy(&vecImage[i*imgSize], poImg->getDataPtr(i), imgSize);
		}
		tup->setUri(uri);
		tup->set(vecImage);
		return tup;
	}

   void extractImage (XCF::CTUPtr ctu, xmltio::Location l, ImgBase*& poImg) {
      const string& sURI = extract<string>(l["uri"]);
      xmltio::Location  p(l, "PROPERTIES");
      int iWidth   = extract<int>(p["width"]);
      int iHeight  = extract<int>(p["height"]);
      depth eDepth = translateDepth(extract<string>(p["depth"]));
      int iChannels   = extract<int>(p["channels"]);
	  icl::format fmt = translateFormat(extract<string>(p["format"]));

	  int bayerFormat = extract<int>(l["BAYERFORMAT"]);
	  //TODO return bayerFormat
      
      Location  r (l, "ROI");
      icl::Rect roi ((int) extract<int>(r["offsetX"]), 
                     (int) extract<int>(r["offsetY"]),
                     (int) extract<int>(r["width"]),
                     (int) extract<int>(r["height"]));

      ensureCompatible (&poImg, eDepth, Size(iWidth, iHeight), 
                        iChannels, fmt, roi);

		Time::value_type t 
         = extract<Time::value_type>(l[XPath("TIMESTAMPS/CREATED")]);
      poImg->setTime (Time::microSeconds (t));

      XCF::Binary::TransportUnitPtr btu = ctu->getBinary (sURI);
      XCF::Binary::TransportVecByte *pTypedBTU 
         = dynamic_cast<XCF::Binary::TransportVecByte*>(btu.get());
      const std::vector<Ice::Byte> &vecImage = pTypedBTU->value;
    
      int imgSize = iWidth * iHeight * getSizeOf(eDepth);
      for (int i=0; i < iChannels; i++) {
         memcpy(poImg->getDataPtr(i), &vecImage[i*imgSize], imgSize);
      }
   }

   XCFGrabber::XCFGrabber (const std::string& sServer, XCF::RecoverLevel l) :
      m_locRequest ("<IMAGEREQUEST>"
                    "<GRAB stereo=\"false\" timestamp=\"\"/>"
                    "</IMAGEREQUEST>", "/IMAGEREQUEST/GRAB"), 
      m_remoteServer(0)
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

      Location loc (m_result->getXML(), "/IMAGESET/IMAGE");

      ImgBase *poOutput = prepareOutput (ppoDst);
      extractImage (m_result, loc, m_poSource);
      m_oConverter.apply (m_poSource, poOutput);
      return poOutput;
   }

   void XCFGrabber::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);

      vGrabbedImages.resize (m_result->getBinaryMap().size());
      xmltio::Location locResult (m_result->getXML(), "/IMAGESET");

      XPathIterator locIt = XPath("IMAGE[uri]").evaluate(locResult);
      vector<ImgBase*>::iterator imgIt = vGrabbedImages.begin();
      for (; locIt; ++locIt, ++imgIt) {
         ImgBase *poOutput = prepareOutput (&(*imgIt));
         extractImage (m_result, *locIt, m_poSource);
         m_oConverter.apply (m_poSource, poOutput);
      }
   }
   
   void XCFGrabber::receive (XCF::CTUPtr& result) {
      m_locRequest["timestamp"] = ""; // most-recent image
      m_remoteServer->callMethod ("retrieveImage", 
                                  m_locRequest.getDocumentText(), result);
   }
}
