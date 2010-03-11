/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifdef HAVE_XCF
#ifndef ICL_XCF_SERVER_GRABBER_H
#define ICL_XCF_SERVER_GRABBER_H

#include <ICLIO/XCFGrabberBase.h>
#include <string>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <ICLCC/Bayer.h>

namespace icl {
  

   /// Grabber to access XCF Image Server \ingroup GRABBER_G
   /** The XCFServerGrabber provides access to an XCF Image Server. */
  
   class XCFServerGrabber : public XCFGrabberBase {
   public:
    
      /// Base constructor
      XCFServerGrabber(const std::string& sServer, 
                       const std::string& sMethodName = "retrieveImage",
                 ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                 ::XCF::Implementation::Properties::singleton()
                 ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    
      /// Destructor
      ~XCFServerGrabber(void);
    
   
      /// set XCF recover level
      void setRecoverLevel (XCF::RecoverLevel l) {
         m_remoteServer->setRecoverLevel (l);
      }

      /// set request string for image server
      void setRequest (const std::string& sRequest);

   protected:   
      /// retrieve most current image set in provided composite transport unit
      virtual void receive (XCF::CTUPtr& result);
      
   private:
      std::string          m_methodName;
      xmltio::Location     m_locRequest;
      XCF::RemoteServerPtr m_remoteServer;
   };
  
}

#endif
#endif // HAVE_XCF
