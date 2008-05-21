#include <iclXCFServerGrabber.h>
#include <xmltio/xmltio.hpp>

#include "iclXCFUtils.h"

using namespace std;
using namespace xmltio;

namespace icl {


   XCFServerGrabber::XCFServerGrabber (const std::string& sServer, XCF::RecoverLevel l) :
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

   XCFServerGrabber::~XCFServerGrabber () {
      m_remoteServer->destroy ();
      delete m_poBayerConverter;
      ICL_DELETE(m_poSource);
      ICL_DELETE(m_poBayer)
   }

  void XCFServerGrabber::makeOutput (const xmltio::Location& l, ImgBase *poOutput) {
    XCFUtils::createOutputImage(l,m_poSource,poOutput,&m_poBayer,m_poBayerConverter,&m_oConverter);
  }

   const ImgBase* XCFServerGrabber::grab (ImgBase **ppoDst) {
      receive (m_result);

      Location loc (m_result->getXML(), "/IMAGESET/IMAGE");

      ImgBase *poOutput = prepareOutput (ppoDst);
      XCFUtils::CTUtoImage(m_result, loc,&m_poSource);
      makeOutput (loc, poOutput);
      return poOutput;
   }

   void XCFServerGrabber::grab (std::vector<ImgBase*>& vGrabbedImages) {
      receive (m_result);

      vGrabbedImages.resize (m_result->getBinaryMap().size());
      xmltio::Location locResult (m_result->getXML(), "/IMAGESET");

      XPathIterator locIt = XPath("IMAGE").evaluate(locResult);
      vector<ImgBase*>::iterator imgIt  = vGrabbedImages.begin();
      vector<ImgBase*>::iterator imgEnd = vGrabbedImages.end();
      for (; (!locIt == false) && (imgIt != imgEnd); ++locIt, ++imgIt) {
         *imgIt = prepareOutput (&(*imgIt));
         XCFUtils::CTUtoImage(m_result, *locIt, &m_poSource);
         makeOutput (*locIt, *imgIt);
      }
   }
   
   void XCFServerGrabber::setRequest (const string& sRequest) {
      m_locRequest = Location (sRequest, "/IMAGEREQUEST/GRAB");
   }

   void XCFServerGrabber::receive (XCF::CTUPtr& result) {
      m_locRequest["timestamp"] = ""; // most-recent image
      m_remoteServer->callMethod ("retrieveImage", 
                                  m_locRequest.getDocumentText(), result);
   }
}
