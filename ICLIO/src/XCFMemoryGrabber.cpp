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


#include <ICLIO/XCFMemoryGrabber.h>

#include <log4cxx/propertyconfigurator.h>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "Memory/Interface.hpp"
#include "Memory/SynchronizedQueue.hpp"

#include <ICLUtils/ProgArg.h>
#include <string>
#include <cstdlib>

#include <ICLUtils/StringUtils.h>
#include <cstring>
#include <ICLIO/XCFUtils.h>
#include <ICLCC/Converter.h>
#include <ICLIO/Grabber.h>

using memory::interface::MemoryPtr;
using memory::interface::MemoryInterface;
using memory::interface::EventSource;
using memory::interface::TriggeredAction;
using memory::interface::Condition;
using memory::interface::Subscription;
using memory::interface::Event;
using memory::interface::MemoryInterfaceException;
using memory::interface::Attachments;

namespace icl{

  struct XCFMemoryGrabberImpl : public Grabber{
    std::string m_xpath;
    MemoryPtr m_memInterface;
    boost::shared_ptr<EventSource> m_evtSrc;
    TriggeredAction *m_action;
    Condition m_condition;
    Subscription *m_subscription;
    bool m_ignoreDesired;
    ImgBase *m_buffer;
    Attachments m_attachments;
    Converter m_converter;
    
    XCFMemoryGrabberImpl(const std::string &memoryName,const std::string &xpath):
      m_xpath(xpath),m_action(0),m_subscription(0),m_ignoreDesired(false),m_buffer(0){
      try{
        m_memInterface = MemoryPtr(MemoryInterface::getInstance(memoryName));

        m_evtSrc = boost::shared_ptr<EventSource>(new EventSource());
        
        m_action = new TriggeredAction(boost::bind(&EventSource::push, m_evtSrc, _1));
        
        m_condition = Condition(Event::INSERT,m_xpath);
        
        m_subscription = new Subscription(m_memInterface->add(m_condition,*m_action));
        
      }catch(const MemoryInterfaceException& ex){
        std::cerr << "MemoryInterfaceException: " << ex.what() << std::endl;
      }catch(const std::exception &ex){
        std::cerr << "std::exception: " << ex.what() << std::endl; 
      }catch(...){
        std::cerr << "An Unknown Error Occured!" << std::endl;
      }
    }
    ~XCFMemoryGrabberImpl(){
      m_memInterface->unsubscribe (*m_subscription);
      ICL_DELETE(m_action);
      ICL_DELETE(m_subscription);
      ICL_DELETE(m_buffer);
    }
    
    const ImgBase *grabUD(ImgBase **ppoDst){
      Event e;
      while (m_evtSrc->next(e)) { // this call locks!
        xmltio::LocationPtr loc = xmltio::find (e.getDocument(),m_xpath);
        if (!loc) break;

        ImgBase *poOutput = 0;
        XCFUtils::ImageDescription d = XCFUtils::getImageDescription(*loc);
          
        ImgBase **usedDstImage = m_ignoreDesired ? ppoDst : &m_buffer;
        poOutput->setTime (d.time);
          
        m_memInterface->getAttachments(e.getDocument().getRootLocation().getDocumentText(),
                                       m_attachments);
        
        XCFUtils::unserialize(m_attachments[d.uri],d,usedDstImage);
        
        if(!m_ignoreDesired){
          *ppoDst = prepareOutput (ppoDst); 
          m_converter.apply(*usedDstImage,*ppoDst);
        }
        return *ppoDst;
      }

      // if we arrive here, something went wrong
      ERROR_LOG("Unable to grab next image from memory");
      return 0;
    }
  };


  /// this can be implemented only if XCFMemoryGrabberImpl is defined properly (here)
  void XCFMemoryGrabberImplDelOp::delete_func(XCFMemoryGrabberImpl *impl){
    ERROR_LOG("this class should not be used yet, because it was not tested at all!");
    ICL_DELETE(impl);
  }
  


  XCFMemoryGrabber::XCFMemoryGrabber(const std::string &memoryName, const std::string &imageXPath):
    impl(new XCFMemoryGrabberImpl(memoryName,imageXPath)){
    
  }

  const ImgBase *XCFMemoryGrabber::grabUD(ImgBase **ppoDst){
    return impl->grab(ppoDst);
  }

}
