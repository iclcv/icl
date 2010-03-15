/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/src/XCFServerGrabber.cpp                         **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Robert Haschke, Felix Reinhard     **
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

#include <ICLIO/XCFServerGrabber.h>
#include <xmltio/xmltio.hpp>

#include <ICLIO/XCFUtils.h>

using namespace std;
using namespace xmltio;

namespace icl {


   XCFServerGrabber::XCFServerGrabber (const std::string& sServer, 
                                       const std::string& sMethodName,
                                       XCF::RecoverLevel l) :
      m_methodName (sMethodName),
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
    try {
       m_remoteServer->callMethod (m_methodName, 
                                   m_locRequest.getDocumentText(), result);
    } catch (XCF::GenericException &e) {
       ERROR_LOG("xcf exception: " << e.reason);
    }
  }
}
