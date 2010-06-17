/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/XCFPublisherGrabber.cpp                      **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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

