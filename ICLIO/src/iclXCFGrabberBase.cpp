#include <iclXCFGrabberBase.h>
#include <xmltio/xmltio.hpp>

#include "iclXCFUtils.h"

using namespace std;
using namespace xmltio;

namespace icl {


  XCFGrabberBase::XCFGrabberBase(): m_result(0), m_poSource(0), m_poBayerBuffer(0),m_poDesiredParamsBuffer(0){
    
    m_poBayerConverter = new BayerConverter(BayerConverter::simple,
                                            BayerConverter::bayerPattern_RGGB, 
                                            m_oDesiredParams.getSize());
  }
  
  XCFGrabberBase::~XCFGrabberBase(){
    ICL_DELETE(m_poBayerConverter);
    ICL_DELETE(m_poSource);
    ICL_DELETE(m_poBayerBuffer);
    ICL_DELETE(m_poDesiredParamsBuffer);
  }
  
  void XCFGrabberBase::makeOutput (const xmltio::Location& l, ImgBase *poOutput) {
    xmltio::LocationPtr locBayer = xmltio::find (l, "PROPERTIES/@bayerPattern");
    if (locBayer) {
      const std::string& bayerPattern =  xmltio::extract<std::string>(*locBayer);
      ImgParams p = m_poSource->getParams();
      p.setFormat (formatRGB);
      m_poBayerBuffer = icl::ensureCompatible (&m_poBayerBuffer, m_poSource->getDepth(), p);
         
      m_poBayerConverter->setBayerImgSize(m_poSource->getSize());
      //poBC->setConverterMethod(BayerConverter::nearestNeighbor);
      m_poBayerConverter->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
      
      m_poBayerConverter->apply(m_poSource->asImg<icl8u>(), &m_poBayerBuffer);
      m_oConverter.apply (m_poBayerBuffer, poOutput);
    } else {
      m_oConverter.apply (m_poSource, poOutput);
    }
  }

   const ImgBase* XCFGrabberBase::grab (ImgBase **ppoDst) {
     receive (m_result);
     
     //     Location loc (m_result->getXML(), "/IMAGESET/IMAGE"); // why /IMAGESET/IMAGE
     LocationPtr loc = xmltio::find(xmltio::Location(m_result->getXML()), "//IMAGE");
     
     if(loc){
       ImgBase *poOutput = prepareOutput (ppoDst);
       XCFUtils::CTUtoImage(m_result, *loc,&m_poSource);
       makeOutput (*loc, poOutput);
       return poOutput;
     }else{
       ERROR_LOG("unable to find XPath: \"//IMAGE\"");
       return 0;
     }
   }

   void XCFGrabberBase::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);

      vGrabbedImages.resize (m_result->getBinaryMap().size());
      xmltio::LocationPtr locResult  = xmltio::find(xmltio::Location(m_result->getXML()), "//IMAGESET");
      if(locResult){
        XPathIterator locIt = XPath("IMAGE").evaluate(*locResult);
        vector<ImgBase*>::iterator imgIt  = vGrabbedImages.begin();
        vector<ImgBase*>::iterator imgEnd = vGrabbedImages.end();
        for (; (!locIt == false) && (imgIt != imgEnd); ++locIt, ++imgIt) {
          *imgIt = prepareOutput (&(*imgIt));
          XCFUtils::CTUtoImage(m_result, *locIt, &m_poSource);
          makeOutput (*locIt, *imgIt);
        }
      }else{
       ERROR_LOG("unable to find XPath: \"//IMAGESET\"");
      }
   }
   
}
