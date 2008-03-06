#include "iclInplaceArithmeticalOp.h"
#include <iclImg.h>
#include <cmath>

namespace icl{
  
  namespace{
    
    // general implementation for transforming a single value 
    // implemented as class to provide part-specialization
    template<class T,InplaceArithmeticalOp::optype O> struct Apply{
      static inline T f(T &t, icl64f v){ return t; }
    };

    
    // define how to specialize a single optype O##Op with given func F
#define SPECIALIZE(O,F)                                                     \
    template<class T> struct Apply<T,InplaceArithmeticalOp::O##Op>{         \
      static inline T f(T &t, icl64f v){ return Cast<icl64f,T>::cast(F); }  \
    };
    
    // define specializations for all optypes
    SPECIALIZE(add,t+v);
    SPECIALIZE(sub,t-v);
    SPECIALIZE(mul,t*v);
    SPECIALIZE(div,t/v);
    SPECIALIZE(sqr,t*t);
    SPECIALIZE(ln,::log(t));
    SPECIALIZE(exp,::exp(t));
    SPECIALIZE(abs,::fabs(t));
#undef SPECIALIZE    
    
    // how to iterate an operation Oover an array of type T
    template<class T,InplaceArithmeticalOp::optype O>
    inline void transform_array(T *src, int dim ,icl64f v){
      for(T* end= src+dim-1; end >= src; end--){    
        Apply<T,O>::f(*end,v);
      }
    }
    
    // check whether to apply on whole image or line by line on the images roi
    // and iterate over all channels
    template<class T,InplaceArithmeticalOp::optype O>
    Img<T> *apply_inplace_arithmetical_op_2(Img<T> *src, bool roiOnly, icl64f val){
      if(roiOnly && !src->hasFullROI()){
        for(int i=0;i<src->getChannels();i++){
          for(ImgIterator<T> it=src->getROIIterator(i); it.inRegion(); it.incRow()){
            transform_array<T,O>(&(*it),it.getROIWidth(),val);
          }
        }
      }else{
        for(int i=0;i<src->getChannels();i++){
          transform_array<T,O>(src->getData(i),src->getDim(),val);
        }
      }
      return src;
    }  

    
    // expand the optype from runtime parameter to template parameter
    // by this means, switching over optype is done at compile type
    template<class T>
    Img<T> *apply_inplace_arithmetical_op(Img<T> *src, bool roiOnly, icl64f val, InplaceArithmeticalOp::optype t){
      switch(t){
#define CASE(X)                                                                             \
        case InplaceArithmeticalOp::X##Op:                                                  \
          apply_inplace_arithmetical_op_2<T,InplaceArithmeticalOp::X##Op>(src,roiOnly,val); \
          break
        CASE(add);CASE(sub);CASE(mul);CASE(div);CASE(sqr);CASE(sqrt);CASE(ln);CASE(exp);CASE(abs);
#undef CASE
        default:
          break;
      }
      return src;
    }
  }
  
  // underlying apply function: switches over the src images depth
  ImgBase *InplaceArithmeticalOp::apply(ImgBase *src){
    ICLASSERT_RETURN_VAL(src,0);
    ICLASSERT_RETURN_VAL(src->getChannels(),src);
    
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                         \
      case depth##D:                                                                                     \
        return apply_inplace_arithmetical_op(src->asImg<icl##D>(), getROIOnly(),getValue(),getOpType()); \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH  
    }
    return src;
  }
  
}
