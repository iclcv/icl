#include "iclInplaceLogicalOp.h"
#include <iclImg.h>
#include <cmath>

namespace icl{
  
  namespace{
    
#define TRUE_VAL 255
    
    // general implementation for transforming a single value 
    // implemented as class to provide part-specialization
    template<class T,InplaceLogicalOp::optype O> struct Apply{
      static inline T f(T &t, T &v){ return t; }
    };
    
    
    // define how to specialize a single optype O##Op with given func F
#define SPECIALIZE(O,F)                                                \
    template<class T> struct Apply<T,InplaceLogicalOp::O##Op>{         \
      static inline T f(T &t, T &v){ return Cast<icl64f,T>::cast(F); } \
    };
    
    // define specializations for all optypes
    SPECIALIZE(and,(t&&v)*TRUE_VAL);
    SPECIALIZE(or,(t||v)*TRUE_VAL);
    SPECIALIZE(xor,(!!v xor !!t)*TRUE_VAL); // using !! to convert v into boolean
    SPECIALIZE(not,(!t)*TRUE_VAL);
    SPECIALIZE(binAnd,t&v);
    SPECIALIZE(binOr,t|v);
    SPECIALIZE(binXor,t^v);
    SPECIALIZE(binNot,~t);
    
#undef SPECIALIZE    
    
    // how to iterate an operation Oover an array of type T
    template<class T,InplaceLogicalOp::optype O>
    inline void transform_array(T *src, int dim ,T v){
      for(T* end= src+dim-1; end >= src; end--){    
        Apply<T,O>::f(*end,v);
      }
    }
    
    // check whether to apply on whole image or line by line on the images roi
    // and iterate over all channels
    template<class T,InplaceLogicalOp::optype O>
    Img<T> *apply_inplace_arithmetical_op_2(Img<T> *src, bool roiOnly, icl64f val64){
      T val = Cast<icl64f,T>::cast(val64);
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
    Img<T> *apply_inplace_arithmetical_op_all(Img<T> *src, 
                                              bool roiOnly, 
                                              icl64f val, 
                                              InplaceLogicalOp::optype t){
      switch(t){
#define CASE(X)                                                                        \
        case InplaceLogicalOp::X##Op:                                                  \
          apply_inplace_arithmetical_op_2<T,InplaceLogicalOp::X##Op>(src,roiOnly,val); \
          break
        CASE(and);
        CASE(or); 
        CASE(xor);
        CASE(not); 
        CASE(binAnd);
        CASE(binOr);
        CASE(binXor);
        CASE(binNot); 
#undef CASE
        default:
          break;
      }
      return src;
    }
    
    // expand the optype from runtime parameter to template parameter
    // by this means, switching over optype is done at compile type
    template<class T>
    Img<T> *apply_inplace_arithmetical_op_float(Img<T> *src, 
                                                bool roiOnly, 
                                                icl64f val, 
                                                InplaceLogicalOp::optype t){
      switch(t){
#define CASE(X)                                                                        \
        case InplaceLogicalOp::X##Op:                                                  \
          apply_inplace_arithmetical_op_2<T,InplaceLogicalOp::X##Op>(src,roiOnly,val); \
        break
        CASE(and);
        CASE(or); 
        CASE(xor);
        CASE(not); 
#undef CASE
        default:
          break;
      }
      return src;
    }
  }


  
  // underlying apply function: switches over the src images depth
  // binary operations are only instantiated for int depths pure 
  // logical operations are instantiated for all depths
  ImgBase *InplaceLogicalOp::apply(ImgBase *src){
    ICLASSERT_RETURN_VAL(src,0);
    ICLASSERT_RETURN_VAL(src->getChannels(),src);
    
    depth d = src->getDepth();
    if(d==depth32f || d ==depth64f){
      if(m_eOpType > 3){
        ERROR_LOG("Bitwise operations are not implemented for floating point types");
        return src;
      }
    }
    
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D)                                            \
      case depth##D:                                                        \
        return apply_inplace_arithmetical_op_all(src->asImg<icl##D>(),      \
                                             getROIOnly(),                  \
                                             getValue(),                    \
                                             getOpType());                  \
        break;
      ICL_INSTANTIATE_ALL_INT_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH  

#define ICL_INSTANTIATE_DEPTH(D)                                          \
      case depth##D:                                                      \
        return apply_inplace_arithmetical_op_float(src->asImg<icl##D>(),  \
                                                   getROIOnly(),          \
                                                   getValue(),            \
                                                   getOpType());          \
        break;
      ICL_INSTANTIATE_ALL_FLOAT_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH  
      
      

    }
    return src;
  }
  
}
