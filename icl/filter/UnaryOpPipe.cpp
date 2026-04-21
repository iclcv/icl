// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/UnaryOpPipe.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>
#include <icl/core/Img.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
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

  void UnaryOpPipe::applyImgBase(const ImgBase *src, ImgBase **dst) {
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

  int UnaryOpPipe::getLength() const {
    return static_cast<int>(ops.size());
  }
  ImgBase *&UnaryOpPipe::getLastDisplay() {
    return ims[getLength()-1];
  }
  UnaryOp *&UnaryOpPipe::getOp(int i) {
    return ops[i];
  }
  ImgBase *&UnaryOpPipe::getImage(int i) {
    return ims[i];
  }

  void UnaryOpPipe::apply(const core::Image &src, core::Image &dst) {
    // TODO: use Image natively!
    ImgBase *dstPtr = dst.isNull() ? nullptr : dst.ptr();
    applyImgBase(src.ptr(), &dstPtr);
    if(dstPtr) dst = core::Image(*dstPtr);
  }

  } // namespace icl::filter