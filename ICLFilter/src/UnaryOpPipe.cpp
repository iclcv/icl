/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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

#include <ICLFilter/UnaryOpPipe.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/ImgBase.h>

namespace icl{
  
  UnaryOpPipe::UnaryOpPipe(){}
  
  UnaryOpPipe::~UnaryOpPipe(){
    for(int i=0;i<getLength();i++){
      delete ops[i];
      delete ims[i];
    }
  }
  
  void UnaryOpPipe::add(UnaryOp *op, ImgBase*im){
    ops.push_back(op);
    ims.push_back(im);
  }
  
  void UnaryOpPipe::apply(const ImgBase *src, ImgBase **dst){
    int length = getLength();
    switch(length){
      case 0: ERROR_LOG("length must be > 0"); break;
      case 1: getOp(0)->apply(src,dst); break;
      default:
        getOp(0)->apply(src,&getImage(0));
        for(int i=1;i<length-1;i++){
          getOp(i)->apply(getImage(i-1),&getImage(i));
        }
        getOp(length-1)->apply(getImage(length-2),dst);
        break;
    }
  }

  const ImgBase *UnaryOpPipe::apply(const ImgBase *src){
    apply(src,&getLastImage());
    return getLastImage();
  }
  
  int UnaryOpPipe::getLength() const {
    return (int)ops.size();
  }
  ImgBase *&UnaryOpPipe::getLastImage() {
    return ims[getLength()-1];
  }
  UnaryOp *&UnaryOpPipe::getOp(int i) {
    return ops[i];
  }
  ImgBase *&UnaryOpPipe::getImage(int i) {
    return ims[i];
  }
}
