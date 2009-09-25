#include <iclUnaryOpPipe.h>
#include <iclUnaryOp.h>
#include <iclImgBase.h>

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
