/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/CreateGrabber.cpp                            **
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

#include <ICLIO/CreateGrabber.h>
#include <ICLIO/TestImages.h>

namespace icl{

  const ImgBase* CreateGrabberImpl::grabUD(ImgBase **ppoDst){
    if(getIgnoreDesiredParams()){
      if(!ppoDst) return m_image;
      if(!*ppoDst) *ppoDst = m_image->deepCopy();
      else m_image->deepCopy(ppoDst);
    }else{
      ImgBase *image = prepareOutput(ppoDst);
      m_oConverter.apply(m_image,image);
      return image;
    }
    return 0;
  }

  CreateGrabberImpl::CreateGrabberImpl(const std::string &what){
    m_image = TestImages::create(what);
    if(!m_image) throw ICLException("unable to create a 'CreateGrabber' from given description '"+what+"'");
  }
  CreateGrabberImpl::~CreateGrabberImpl(){
    ICL_DELETE(m_image);
  }

}
    
