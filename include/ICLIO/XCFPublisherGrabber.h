/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/XCFPublisherGrabber.h                    **
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

#ifdef HAVE_XCF
#ifndef ICL_XCF_PUBLISHER_GRABBER_H
#define ICL_XCF_PUBLISHER_GRABBER_H

#include <ICLIO/XCFGrabberBase.h>
#include <xcf/Subscriber.hpp>
#include <xmltio/Location.hpp>

namespace icl{

  class XCFPublisherGrabber : public XCFGrabberBase{
    public:
    XCFPublisherGrabber(const std::string &streamName, 
                        ::XCF::RecoverLevel l = (::XCF::RecoverLevel)
                        ::XCF::Implementation::Properties::singleton()
                        ->getPropertyAsInt("XCF.Global.RecoverLevel"));
    virtual ~XCFPublisherGrabber();
    
    /// set XCF recover level
    void setRecoverLevel (XCF::RecoverLevel l) {
       m_subscriber->setRecoverLevel (l);
    }

    protected:
    virtual void receive (XCF::CTUPtr& result);
    
    private:
    XCF::SubscriberPtr m_subscriber;
  };
}

#endif // ICL_XCF_PUBLISHER_GRABBER_H

#endif // HAVE_XCF
