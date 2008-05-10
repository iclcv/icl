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
		  "<PROPERTIES width=\"\" height=\"\" depth=\"\" channels=\"\" format=\"\"/>"
		  "<ROI offsetX=\"\" offsetY=\"\" width=\"\" height=\"\" />"
		"</IMAGE>";

	xmltio::Location createXML(const ImgBase* poImg, const string& uri, 
                              const string& bayerPattern)
  {
      xmltio::Location l(IMG_XML_STRING, "/IMAGE");
      l["uri"]					   = uri;

      xmltio::Location p(l, "PROPERTIES");
      p["width"]		= poImg->getWidth();
      p["height"]	  = poImg->getHeight();
      p["depth"]		= translateDepth(poImg->getDepth());
      p["channels"]	= poImg->getChannels();
      p["format"]	  = translateFormat(poImg->getFormat());
      if (bayerPattern != "") p["bayerPattern"] = bayerPattern;

      xmltio::Location r(l, "ROI");
      r["offsetX"]			= poImg->getROIXOffset();
      r["offsetY"]			= poImg->getROIYOffset();
      r["width"]			  = poImg->getROIWidth();
      r["height"]			  = poImg->getROIHeight();

      l[XPath("TIMESTAMPS/CREATED/@timestamp")]	= poImg->getTime().toMicroSeconds();
      return l;
  }

// {{{ packImage

   template <class XCF_T, typename ICE_T>
   XCF::Binary::TransportUnitPtr packImage (const ImgBase* poImg,
                                            XCF::Binary::TransportUnitPtr btu) {
      XCF_T *pTypedBTU = dynamic_cast<XCF_T*>(btu.get());
      // on type mismatch, create new instance
      if (!pTypedBTU) pTypedBTU = new XCF_T;
      // get reference to data content
      std::vector<ICE_T> &vecImage = pTypedBTU->value;
      vecImage.resize (poImg->getChannels() * poImg->getDim());
      // copy data
      int imgSize = poImg->getDim();
      int imgByteSize = imgSize * getSizeOf(poImg->getDepth());
      for (int i=0, nChannels=poImg->getChannels(); i < nChannels; i++) {
         memcpy(&vecImage[i*imgSize], poImg->getDataPtr(i), imgByteSize);
      }
      return pTypedBTU;
   }

	XCF::Binary::TransportUnitPtr packImage (const ImgBase* poImg, 
                                            XCF::Binary::TransportUnitPtr btu) {
      switch (poImg->getDepth()) {
        case depth8u:
           return packImage<XCF::Binary::TransportVecByte, Ice::Byte> (poImg, btu);
           break;
        case depth32s:
           return packImage<XCF::Binary::TransportVecInt, int> (poImg, btu);
           break;
        case depth32f:
           return packImage<XCF::Binary::TransportVecFloat, float> (poImg, btu);
           break;
        case depth64f:
           return packImage<XCF::Binary::TransportVecDouble, double> (poImg, btu);
           break;
        default:
           return 0;
           break;
      }
   }

// }}}
      
// {{{ extract image

   template <class XCF_T, typename ICE_T>
   static void extractImage (ImgBase* poImg, XCF::Binary::TransportUnitPtr btu) {
      XCF_T *pTypedBTU = dynamic_cast<XCF_T*>(btu.get());
      const std::vector<ICE_T> &vecImage = pTypedBTU->value;
    
      int imgSize = poImg->getDim() * getSizeOf(poImg->getDepth());
      for (int i=0, nChannels=poImg->getChannels(); i < nChannels; i++) {
         memcpy(poImg->getDataPtr(i), &vecImage[i*imgSize], imgSize);
      }
   }

   void extractImage (const XCF::CTUPtr ctu, const xmltio::Location& l, 
                      ImgBase*& poImg) {
      const string& sURI = extract<string>(l["uri"]);
      xmltio::Location  p(l, "PROPERTIES");
      int iWidth   = extract<int>(p["width"]);
      int iHeight  = extract<int>(p["height"]);
      depth eDepth = translateDepth(extract<string>(p["depth"]));
      int iChannels   = extract<int>(p["channels"]);

      std::cout << "jfdjfldsjfld: "<< (string)extract<string>(p["format"]) << std::endl;
      icl::format fmt = translateFormat(extract<string>(p["format"]));

      LocationPtr  r = find(l, "ROI");
      icl::Rect roi(0, 0, iWidth, iHeight);
      if(r) {
        roi = icl::Rect((int) extract<int>((*r)["offsetX"]), 
                        (int) extract<int>((*r)["offsetY"]),
                        (int) extract<int>((*r)["width"]),
                        (int) extract<int>((*r)["height"]));
      }

      poImg = ensureCompatible (&poImg, eDepth, Size(iWidth, iHeight), 
                                iChannels, fmt, roi);

      Time::value_type t 
         = extract<Time::value_type>(l[XPath("TIMESTAMPS/CREATED/@timestamp")]);
      poImg->setTime (Time::microSeconds (t));

      XCF::Binary::TransportUnitPtr btu = ctu->getBinary (sURI);
      switch (eDepth) {
        case depth8u:
           extractImage<XCF::Binary::TransportVecByte, Ice::Byte> (poImg, btu);
           break;
        case depth32s:
           extractImage<XCF::Binary::TransportVecInt, int> (poImg, btu);
           break;
        case depth32f:
           extractImage<XCF::Binary::TransportVecFloat, float> (poImg, btu);
           break;
        case depth64f:
           extractImage<XCF::Binary::TransportVecDouble, double> (poImg, btu);
           break;
        default:
           return;
           break;
      }
   }

// }}}

   XCFGrabber::XCFGrabber (const std::string& sServer, XCF::RecoverLevel l) :
      m_locRequest ("<IMAGEREQUEST>"
                    "<GRAB stereo=\"false\" timestamp=\"\"/>"
                    "</IMAGEREQUEST>", "/IMAGEREQUEST/GRAB"), 
      m_remoteServer(0), m_result(0), m_poSource(0), m_poBayer(0)
   {
      // create remote server instance
      m_remoteServer = XCF::RemoteServer::create(sServer, XCF::NONE);
      // and on success, set default recover level
      m_remoteServer->setRecoverLevel (l);

	  m_poBayerConverter = new BayerConverter(BayerConverter::simple,
                                             BayerConverter::bayerPattern_RGGB, 
                                             m_oDesiredParams.getSize());
   }

   XCFGrabber::~XCFGrabber () {
      m_remoteServer->destroy ();
      delete m_poBayerConverter;
      delete m_poSource;
   }

   void XCFGrabber::makeOutput (const xmltio::Location& l, ImgBase *poOutput) {
      LocationPtr locBayer = xmltio::find (l, "PROPERTIES/@bayerPattern");
      if (locBayer) {
         const string& bayerPattern =  extract<string>(*locBayer);
         ImgParams p = m_poSource->getParams();
         p.setFormat (formatRGB);
         m_poBayer = ensureCompatible (&m_poBayer, m_poSource->getDepth(), p);
         
         m_poBayerConverter->setBayerImgSize(m_poSource->getSize());
         //m_poBayerConverter->setConverterMethod(BayerConverter::nearestNeighbor);
         m_poBayerConverter->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
         
         m_poBayerConverter->apply(m_poSource->asImg<icl8u>(), &m_poBayer);
         m_oConverter.apply (m_poBayer, poOutput);
      } else {
         m_oConverter.apply (m_poSource, poOutput);
      }
   }

   const ImgBase* XCFGrabber::grab (ImgBase **ppoDst) {
      receive (m_result);

      Location loc (m_result->getXML(), "/IMAGESET/IMAGE");

      ImgBase *poOutput = prepareOutput (ppoDst);
      extractImage (m_result, loc, m_poSource);
      makeOutput (loc, poOutput);
      return poOutput;
   }

   void XCFGrabber::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);

      vGrabbedImages.resize (m_result->getBinaryMap().size());
      xmltio::Location locResult (m_result->getXML(), "/IMAGESET");

      XPathIterator locIt = XPath("IMAGE").evaluate(locResult);
      vector<ImgBase*>::iterator 
         imgIt  = vGrabbedImages.begin(),
         imgEnd = vGrabbedImages.end();
      for (; (!locIt == false) && (imgIt != imgEnd); ++locIt, ++imgIt) {
         ImgBase *poOutput = prepareOutput (&(*imgIt));
         *imgIt = poOutput;
         extractImage (m_result, *locIt, m_poSource);
         makeOutput (*locIt, poOutput);
      }
   }
   
   void XCFGrabber::setRequest (const string& sRequest) {
      m_locRequest = Location (sRequest, "/IMAGEREQUEST/GRAB");
   }

   void XCFGrabber::receive (XCF::CTUPtr& result) {
      m_locRequest["timestamp"] = ""; // most-recent image
      m_remoteServer->callMethod ("retrieveImage", 
                                  m_locRequest.getDocumentText(), result);
   }
}
