/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/src/XCFGrabberBase.cpp                           **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifdef HAVE_XCF
#include <ICLIO/XCFGrabberBase.h>
#include <xmltio/xmltio.hpp>

#include <ICLIO/XCFUtils.h>

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

   const ImgBase* XCFGrabberBase::grabUD (ImgBase **ppoDst) {
     receive (m_result);
     
     LocationPtr loc = xmltio::find(xmltio::Location(m_result->getXML()), 
                                    "//IMAGE[@uri]");
     
     if(loc){
       if(!getIgnoreDesiredParams()){
         ImgBase *poOutput = prepareOutput (ppoDst);
         XCFUtils::CTUtoImage(m_result, *loc, &m_poSource);
         makeOutput (*loc, poOutput);
         return poOutput;
       }else if(!ppoDst){
         XCFUtils::CTUtoImage(m_result, *loc, &m_poSource);
         //makeOutput (*loc, poOutput); bayer pattern is not yet supported here
         return m_poSource;
       }else{
         XCFUtils::CTUtoImage(m_result, *loc, ppoDst);
         return *ppoDst;
       }
     }else{
       ERROR_LOG("unable to find XPath: \"//IMAGE[@uri]\"");
       return 0;
     }
   }

   void XCFGrabberBase::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);
      static bool first = true;
      if(first){
        first = false;
        ERROR_LOG("please note that mulitiple image grabbing does not support to ignore grabbers desired parameters!");
      }
      xmltio::Location doc (m_result->getXML());
      vector<ImgBase*>::iterator imgIt  = vGrabbedImages.begin();
      vector<ImgBase*>::iterator imgEnd = vGrabbedImages.end();
      unsigned int nCount = 0;
      for (XPathIterator imLoc = XPath("//IMAGE[@uri]").evaluate(doc);
           imLoc; ++imLoc) {
         ImgBase *poOutput=0;
         if (imgIt != imgEnd) {
            // use existing images
            poOutput = prepareOutput (&(*imgIt));
            ++imgIt;
         } else {
            // create new image
            poOutput = 0;
            poOutput = prepareOutput (&poOutput);
            vGrabbedImages.push_back (poOutput);
         }
         XCFUtils::CTUtoImage(m_result, *imLoc, &m_poSource);
         makeOutput (*imLoc, poOutput);
         nCount++;
      }
      // shrink vGrabbedImages to number of actually grabbed images
      vGrabbedImages.resize (nCount);
   }
   
}
#endif
