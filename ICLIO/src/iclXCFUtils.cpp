#ifdef HAVE_XCF
#include "iclXCFUtils.h"
#include <iclCore.h>
#include <cstring>

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

  XCFUtils::ImageDescription XCFUtils::getImageDescription(const xmltio::Location &l){
    ImageDescription d;
    d.uri = xmltio::extract<std::string>(l["uri"]);
    
    xmltio::Location  p(l, "PROPERTIES");
    d.size.width = xmltio::extract<int>(p["width"]);
    d.size.height  = xmltio::extract<int>(p["height"]);
    d.imagedepth = translateDepth(xmltio::extract<std::string>(p["depth"]));
    d.channels   = xmltio::extract<int>(p["channels"]);
    d.imageformat = translateFormat(xmltio::extract<std::string>(p["format"]));
    
    xmltio::Location  r (l, "ROI");
    d.roi = Rect((int) xmltio::extract<int>(r["offsetX"]), 
                 (int) xmltio::extract<int>(r["offsetY"]),
                 (int) xmltio::extract<int>(r["width"]),
                 (int) xmltio::extract<int>(r["height"]));
    
    d.time = Time::microSeconds(xmltio::extract<Time::value_type>(l[xmltio::XPath("TIMESTAMPS/CREATED/@timestamp")]));

    return d;
  }

  void XCFUtils::CTUtoImage (const XCF::CTUPtr ctu, const xmltio::Location& l, ImgBase** ppoImg){
    // {{{ open
    ImageDescription d = getImageDescription(l);

    *ppoImg = ensureCompatible (ppoImg, d.imagedepth, d.size,d.channels,d.imageformat,d.roi);
    
    (*ppoImg)->setTime (d.time);

    
    XCF::Binary::TransportUnitPtr btu = ctu->getBinary (d.uri);
    switch ((*ppoImg)->getDepth()) {
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

  void XCFUtils::serialize(const ImgBase *image, std::vector<unsigned char> &dst){
    // {{{ open

    ICLASSERT_RETURN(image);
    unsigned int channeldim = image->getDim()*icl::getSizeOf(image->getDepth());
    dst.resize(channeldim*image->getChannels());
    
    for(int i=0;i<image->getChannels();++i){
      memcpy(dst.data()+i*channeldim,image->getDataPtr(i),channeldim);
    }    
  }

  // }}}

  void XCFUtils::unserialize(const std::vector<unsigned char> &src, const XCFUtils::ImageDescription &d, ImgBase **dst){
    // {{{ open

    unsigned int channeldim = d.size.width*d.size.height*icl::getSizeOf(d.imagedepth);
    ICLASSERT_RETURN( src.size() == channeldim*d.channels );    
    *dst = ensureCompatible (dst, d.imagedepth, d.size,d.channels,d.imageformat,d.roi);
    ImgBase *image = *dst;
    image->setTime(d.time);
    
    for(int i=0;i<d.channels;++i){
      memcpy(image->getDataPtr(0),src.data()+i*channeldim,channeldim);
    }
  }

  // }}}

}


#endif // HAVE_XCF
