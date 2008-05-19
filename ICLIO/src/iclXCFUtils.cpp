#ifdef HAVE_XCF
#include "iclXCFUtils.h"

namespace icl{

  const std::string &XCFUtils::createEmptyXMLDoc(){
    // {{{ open

    static const std::string s("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                               "<IMAGE uri=\"\">"
                               "<TIMESTAMPS>"
                               "<CREATED timestamp=\"\"/>"
                               "</TIMESTAMPS>"
                               "<PROPERTIES width=\"\" height=\"\" depth=\"\" channels=\"\" format=\"\"/>"
                               "<ROI offsetX=\"\" offsetY=\"\" width=\"\" height=\"\" />"
                               "</IMAGE>");
    return s;
  }

  // }}}
  
  xmltio::Location XCFUtils::createXMLDoc(const ImgBase* poImg, const std::string& uri, const std::string& bayerPattern){
    // {{{ open

    xmltio::Location l(createEmptyXMLDoc(), "/IMAGE");
    l["uri"] = uri;
    
    xmltio::Location p(l, "PROPERTIES");
    p["width"]	  = poImg->getWidth();
    p["height"]	  = poImg->getHeight();
    p["depth"]	  = translateDepth(poImg->getDepth());
    p["channels"] = poImg->getChannels();
    p["format"]	  = translateFormat(poImg->getFormat());
    if (bayerPattern != "") p["bayerPattern"] = bayerPattern;
    
    xmltio::Location r(l, "ROI");
    r["offsetX"] = poImg->getROIXOffset();
    r["offsetY"] = poImg->getROIYOffset();
    r["width"]	 = poImg->getROIWidth();
    r["height"]	 = poImg->getROIHeight();
    
    l[xmltio::XPath("TIMESTAMPS/CREATED/@timestamp")] = poImg->getTime().toMicroSeconds();
    return l;
  }

  // }}}


  namespace{
    template <class XCF_T, typename ICE_T>
    static void CTUtoImage_Template (ImgBase* poImg, XCF::Binary::TransportUnitPtr btu) {
      // {{{ open

      XCF_T *pTypedBTU = dynamic_cast<XCF_T*>(btu.get());
      const std::vector<ICE_T> &vecImage = pTypedBTU->value;
    
      int imgSize = poImg->getDim() * getSizeOf(poImg->getDepth());
      for (int i=0, nChannels=poImg->getChannels(); i < nChannels; i++) {
         memcpy(poImg->getDataPtr(i), &vecImage[i*imgSize], imgSize);
      }
   }

    // }}}

    template <class XCF_T, typename ICE_T>
    XCF::Binary::TransportUnitPtr ImageToCTU_Template(const ImgBase* poImg, XCF::Binary::TransportUnitPtr btu) {
      // {{{ open

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

    // }}}
  }

  void XCFUtils::CTUtoImage (const XCF::CTUPtr ctu, const xmltio::Location& l, ImgBase** ppoImg){
    // {{{ open

    const std::string& sURI = xmltio::extract<std::string>(l["uri"]);
    xmltio::Location  p(l, "PROPERTIES");
    int iWidth   = xmltio::extract<int>(p["width"]);
    int iHeight  = xmltio::extract<int>(p["height"]);
    depth eDepth = translateDepth(xmltio::extract<std::string>(p["depth"]));
    int iChannels   = xmltio::extract<int>(p["channels"]);
    icl::format fmt = translateFormat(xmltio::extract<std::string>(p["format"]));
    
    xmltio::Location  r (l, "ROI");
    icl::Rect roi ((int) xmltio::extract<int>(r["offsetX"]), 
                   (int) xmltio::extract<int>(r["offsetY"]),
                   (int) xmltio::extract<int>(r["width"]),
                   (int) xmltio::extract<int>(r["height"]));


    *ppoImg = ensureCompatible (ppoImg, eDepth, Size(iWidth, iHeight), 
                                iChannels, fmt, roi);
    
    Time::value_type t = xmltio::extract<Time::value_type>(l[xmltio::XPath("TIMESTAMPS/CREATED/@timestamp")]);
    (*ppoImg)->setTime (Time::microSeconds (t));
    
    XCF::Binary::TransportUnitPtr btu = ctu->getBinary (sURI);
    switch (eDepth) {
      case depth8u:
        CTUtoImage_Template<XCF::Binary::TransportVecByte, Ice::Byte> (*ppoImg, btu);
        break;
      case depth32s:
        CTUtoImage_Template<XCF::Binary::TransportVecInt, int> (*ppoImg, btu);
        break;
      case depth32f:
        CTUtoImage_Template<XCF::Binary::TransportVecFloat, float> (*ppoImg, btu);
        break;
      case depth64f:
        CTUtoImage_Template<XCF::Binary::TransportVecDouble, double> (*ppoImg, btu);
        break;
      default:
        return;
        break;
    }
  }

  // }}}
  

  XCF::Binary::TransportUnitPtr XCFUtils::ImageToCTU(const ImgBase* poImg, XCF::Binary::TransportUnitPtr btu){
    // {{{ open

    switch (poImg->getDepth()){
      case depth8u:
        return ImageToCTU_Template<XCF::Binary::TransportVecByte, Ice::Byte> (poImg, btu);
      case depth32s:
        return ImageToCTU_Template<XCF::Binary::TransportVecInt, int> (poImg, btu);
      case depth32f:
        return ImageToCTU_Template<XCF::Binary::TransportVecFloat, float> (poImg, btu);
      case depth64f:
        return ImageToCTU_Template<XCF::Binary::TransportVecDouble, double> (poImg, btu);
      default:
        return 0;
        break;
    }
  }

  // }}}
}


#endif // HAVE_XCF
