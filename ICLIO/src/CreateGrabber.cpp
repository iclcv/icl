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
    
