#include "iclInplaceLogicalOp.h"
#include <iclImg.h>
#include <cmath>

namespace icl{
  
  namespace{
    
#define TRUE_VAL 255
    
    // general implementation for transforming a single value 
    // implemented as class to provide part-specialization
    template<class T,InplaceLogicalOp::optype O> struct Apply{
      Apply(T val):val(val){}
      inline void operator()(T &t){}
      T val;
    };
    
    
    // define how to specialize a single optype O##Op with given func F
#define SPECIALIZE(O,F)                                                \
    template<class T> struct Apply<T,InplaceLogicalOp::O##Op>{         \
      Apply(icl64f val):val(val){}                                     \
      inline void operator()(T &t){                                    \
        t = (F);                                                       \
      }                                                                \
      T val;                                                           \
    };
    
    // define specializations for all optypes
    SPECIALIZE(and,(t&&val)*TRUE_VAL);
    SPECIALIZE(or,(t||val)*TRUE_VAL);
    SPECIALIZE(xor,(!!val xor !!t)*TRUE_VAL); // using !! to convert v into boolean
    SPECIALIZE(not,(!t)*TRUE_VAL);
    SPECIALIZE(binAnd,t&val);
    SPECIALIZE(binOr,t|val);
    SPECIALIZE(binXor,t^val);
    SPECIALIZE(binNot,~t);
    
#undef SPECIALIZE    
    
    
    
    // expand the optype from runtime parameter to template parameter
    // by this means, switching over optype is done at compile type
    template<class T>
    Img<T> *apply_inplace_arithmetical_op_all(Img<T> *src, 
                                              bool roiOnly, 
                                              icl64f val, 
                                              InplaceLogicalOp::optype t){
      switch(t){
#define CASE(X)                                                                                 \
        case InplaceLogicalOp::X##Op:                                                           \
          src->applyPixelFunction(Apply<T,InplaceLogicalOp::X##Op>(Cast<icl64f,T>::cast(val))); \
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
#define CASE(X)                                                                               \
        case InplaceLogicalOp::X##Op:                                                         \
        src->applyPixelFunction(Apply<T,InplaceLogicalOp::X##Op>(Cast<icl64f,T>::cast(val))); \
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
  ImgBase *InplaceLogicalOp::apply(ImgBase *srcIn){
    ICLASSERT_RETURN_VAL(srcIn,0);
    ICLASSERT_RETURN_VAL(srcIn->getChannels(),srcIn);
   
    ImgBase *src = 0;
    if(!getROIOnly() && !srcIn->hasFullROI()){
      src = srcIn->shallowCopy(srcIn->getImageRect());
    }else{
      src = srcIn;
    }
    
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
    if (!getROIOnly() && !srcIn->hasFullROI()){
      delete src;
    }
    return srcIn;
  }
  
}
