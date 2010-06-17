/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/UnaryOpPipe.cpp                          **
** Module : ICLFilter                                              **
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
