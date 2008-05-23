#ifdef HAVE_XCF
#ifndef ICL_XCF_UTILS_H
#define ICL_XCF_UTILS_H

#include <string>
#include <xmltio/xmltio.hpp>
#include <iclImgBase.h>
#include <iclBayer.h>
#include <iclConverter.h>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <vector>


namespace icl{
  class XCFUtils{
    public:
    static const std::string &createEmptyXMLDoc();
    static xmltio::Location createXMLDoc(const ImgBase* image, const std::string& uri, const std::string& bayerPattern = "");
    /*
        static xmltio::Location createXMLDoc(const std::vector<ImgBase*> images,
        const std::vector<std::string> &uris,
        const std::vector<std::string> &bayerPatterns);
    */
    
    static void CTUtoImage(const XCF::CTUPtr ctu, const xmltio::Location& l, ImgBase** ppoImage);

    static  XCF::Binary::TransportUnitPtr ImageToCTU(const ImgBase* image, XCF::Binary::TransportUnitPtr btu);

    static void createOutputImage(const xmltio::Location& l, 
                                  ImgBase *poSrc, 
                                  ImgBase *poOutput, 
                                  ImgBase **poBayerBuffer, 
                                  BayerConverter *poBC, 
                                  Converter *poConv);
    struct ImageDescription{
      std::string uri;
      Size size;
      depth imagedepth;
      int channels;
      format imageformat;
      Rect roi;
      Time time;
      void show();
    };
    static ImageDescription getImageDescription(const xmltio::Location &l);
    
    static void serialize(const ImgBase *image, std::vector<unsigned char> &dst);
    static void unserialize(const std::vector<unsigned char> &src, const ImageDescription &descr, ImgBase **dst);
  };  
}


#endif // ICL_XCF_UTILS_H

#endif // HAVE_XCF
