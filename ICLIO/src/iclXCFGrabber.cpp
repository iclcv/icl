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
		  "<PROPERTIES width=\"\" height=\"\" depth=\"\" channels=\"\" format=\"\" bayerPattern=\"\" />"
		  "<ROI offsetX=\"\" offsetY=\"\" width=\"\" height=\"\" />"
		"</IMAGE>";

	xmltio::Location createXML(const ImgBase* poImg, const string& uri, 
                              const string& bayerPattern)
   {
      xmltio::Location l(IMG_XML_STRING, "/IMAGE");
      l[XPath("@uri")]					   = uri;
      l[XPath("PROPERTIES/@width")]		= poImg->getWidth();
      l[XPath("PROPERTIES/@height")]	= poImg->getHeight();
      l[XPath("PROPERTIES/@depth")]		= translateDepth(poImg->getDepth());
      l[XPath("PROPERTIES/@channels")]	= poImg->getChannels();
      l[XPath("PROPERTIES/@format")]	= translateFormat(poImg->getFormat());
	  if (bayerPattern != "") {
         l[XPath("PROPERTIES/@bayerPattern")] = bayerPattern;
		 l[XPath("PROPERTIES/@format")]	= translateFormat(formatRGB);
	  }
      l[XPath("ROI/@offsetX")]			= poImg->getROIXOffset();
      l[XPath("ROI/@offsetY")]			= poImg->getROIYOffset();
      l[XPath("ROI/@width")]			   = poImg->getROIWidth();
      l[XPath("ROI/@height")]			   = poImg->getROIHeight();
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
      icl::format fmt = translateFormat(extract<string>(p["format"]));

      Location  r (l, "ROI");
      icl::Rect roi ((int) extract<int>(r["offsetX"]), 
                     (int) extract<int>(r["offsetY"]),
                     (int) extract<int>(r["width"]),
                     (int) extract<int>(r["height"]));

	  string bayerPattern =  extract<string>(p["bayerPattern"]);
	  if (bayerPattern != "") {
		  poImg = ensureCompatible (&poImg, eDepth, Size(iWidth, iHeight), 
										iChannels, formatGray, roi);
	  } else {
		  poImg = ensureCompatible (&poImg, eDepth, Size(iWidth, iHeight), 
										iChannels, fmt, roi);
	  }

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
      m_remoteServer(0)
   {
      // create remote server instance
      m_remoteServer = XCF::RemoteServer::create(sServer, XCF::NONE);
      // and on success, set default recover level
      m_remoteServer->setRecoverLevel (l);

	  m_poBayerConverter = new BayerConverter(BayerConverter::nearestNeighbor,
		  BayerConverter::bayerPattern_RGGB, Size(320, 240));
   }

   XCFGrabber::~XCFGrabber () {
      m_remoteServer->destroy ();
	  delete m_poBayerConverter;
   }

   const ImgBase* XCFGrabber::grab (ImgBase **ppoDst) {
      receive (m_result);

      Location loc (m_result->getXML(), "/IMAGESET/IMAGE");

      ImgBase *poOutput = prepareOutput (ppoDst);
      extractImage (m_result, loc, m_poSource);

	  string bayerPattern =  extract<string>(loc["PROPERTIES/@bayerPattern"]);
	  if (bayerPattern != "") {
		  m_poBayerConverter->setBayerImgSize(m_poSource->getSize());
		  //m_poBayerConverter->setConverterMethod(BayerConverter::nearestNeighbor);
		  m_poBayerConverter->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
		  
		  //poOutput->setChannels(3);
		  //poOutput->setFormat(formatRGB);
		  m_poBayerConverter->apply(m_poSource->asImg<icl8u>(), poOutput);
		  poOutput->deepCopy(&m_poSource);
	  }

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

		 string bayerPattern =  extract<string>((*locIt)["PROPERTIES/@bayerPattern"]);
		 if (bayerPattern != "") {
			m_poBayerConverter->setBayerImgSize(m_poSource->getSize());
			//m_poBayerConverter->setConverterMethod(BayerConverter::nearestNeighbor);
			m_poBayerConverter->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
		  
			//poOutput->setChannels(3);
			//poOutput->setFormat(formatRGB);
			m_poBayerConverter->apply(m_poSource->asImg<icl8u>(), poOutput);
			poOutput->deepCopy(&m_poSource);
		 }

         m_oConverter.apply (m_poSource, poOutput);
      }
   }
   
   void XCFGrabber::receive (XCF::CTUPtr& result) {
      m_locRequest["timestamp"] = ""; // most-recent image
      m_remoteServer->callMethod ("retrieveImage", 
                                  m_locRequest.getDocumentText(), result);
   }
}
