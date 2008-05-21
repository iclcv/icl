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
      m_remoteServer(0)
   {
     
     // create remote server instance
     m_remoteServer = XCF::RemoteServer::create(sServer, XCF::NONE);
     // and on success, set default recover level
     m_remoteServer->setRecoverLevel (l);
   }

  XCFServerGrabber::~XCFServerGrabber () {
    m_remoteServer->destroy ();
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
