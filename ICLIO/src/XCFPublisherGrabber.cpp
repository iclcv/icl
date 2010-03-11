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

#include <ICLIO/XCFPublisherGrabber.h>
#include <ICLIO/XCFUtils.h>
#include <ICLUtils/Thread.h>

namespace icl{

  XCFPublisherGrabber::XCFPublisherGrabber(const std::string &streamName, 
                                           XCF::RecoverLevel l){
    m_subscriber = XCF::Subscriber::create(streamName, XCF::NONE);
    m_subscriber->setOnlyReceiveLast (true);
    // and on success, set desired recover level
    m_subscriber->setRecoverLevel (l);
  }

  XCFPublisherGrabber::~XCFPublisherGrabber(){
    m_subscriber->destroy();
  }
  
  
  void XCFPublisherGrabber::receive (XCF::CTUPtr& result){
    ICLASSERT_RETURN(m_subscriber->isAlive());
    while(true){
      try{
        result = m_subscriber->receiveBinary(true);
        return;
      }catch(XCF::PublisherEmptyException &ex){
        Thread::msleep(100);
      }
    }
  } 
  
}

