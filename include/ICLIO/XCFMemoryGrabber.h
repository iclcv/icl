/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/XCFMemoryGrabber.h                       **
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
#ifndef ICL_XCF_MEMORY_GRABBER_H
#define ICL_XCF_MEMORY_GRABBER_H

#include <ICLIO/Grabber.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  
  /** \cond */
  class XCFMemoryGrabberImpl;
  struct XCFMemoryGrabberImplDelOp{ 
    static void delete_func(XCFMemoryGrabberImpl *impl);
  };
  /** \endcond */
  
  
  /// Grabber implementation to acquire images from an ActiveMemory Server application
  class XCFMemoryGrabber : public Grabber{
    public:
    /// Create a new instance
    XCFMemoryGrabber(const std::string &memoryName,const std::string &imageXPath="//IMAGE");

    /// get next image
    virtual const ImgBase *grabUD(ImgBase **ppoDst=0);
   
    // TODO setIgnoreDesiredParams(bool) 
    // TODO const xmltio::TIODocment *getLastDocument() const;
    SmartPtrBase<XCFMemoryGrabberImpl,XCFMemoryGrabberImplDelOp> impl;
  };
  
}


#endif // ICL_XCF_MEMORY_GRABBER_H

#endif // HAVE_XCF
