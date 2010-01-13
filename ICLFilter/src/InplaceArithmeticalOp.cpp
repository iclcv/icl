#include <ICLFilter/InplaceArithmeticalOp.h>
#include <ICLCore/Img.h>
#include <cmath>

namespace icl{
  
  namespace{
    
    // general implementation for transforming a single value 
    // implemented as class to provide part-specialization
    template<class T,InplaceArithmeticalOp::optype O> struct Apply{
      Apply(icl64f val):val(val){}
      inline void operator()(T &t){}
      icl64f val;      
    };

    
    // define how to specialize a single optype O##Op with given func F
#define SPECIALIZE(O,F)                                              \
    template<class T> struct Apply<T,InplaceArithmeticalOp::O##Op>{  \
      Apply(icl64f val):val(val){}                                   \
      inline void operator()(T &t){                                  \
        t= clipped_cast<icl64f,T>(F);                                \
      }                                                              \
      icl64f val;                                                    \
    };
    
    // define specializations for all optypes
    SPECIALIZE(add,t+val);
    SPECIALIZE(sub,t-val);
    SPECIALIZE(mul,t*val);
    SPECIALIZE(div,t/val);
    SPECIALIZE(sqr,t*t);
    SPECIALIZE(ln,::log(t));
    SPECIALIZE(exp,::exp(t));
    SPECIALIZE(abs,::fabs(t));
#undef SPECIALIZE    
    
    
    // expand the optype from runtime parameter to template parameter
    // by this means, switching over optype is done at compile type
    template<class T>
    Img<T> *apply_inplace_arithmetical_op(Img<T> *src, bool roiOnly, icl64f val, InplaceArithmeticalOp::optype t){
      switch(t){
#define CASE(X) case InplaceArithmeticalOp::X##Op: src->forEach(Apply<T,InplaceArithmeticalOp::X##Op>(val)); break
        CASE(add);
        CASE(sub);
        CASE(mul);
        CASE(div);
        CASE(sqr);
        CASE(sqrt);
        CASE(ln);
        CASE(exp);
        CASE(abs);
#undef CASE
        default:
          break;
      }
      return src;
    }
  }
  
  // underlying apply function: switches over the src images depth
  ImgBase *InplaceArithmeticalOp::apply(ImgBase *srcIn){
    ICLASSERT_RETURN_VAL(srcIn,0);
    ICLASSERT_RETURN_VAL(srcIn->getChannels(),srcIn);
   
    ImgBase *src = 0;
    if(!getROIOnly() && !srcIn->hasFullROI()){
      src = srcIn->shallowCopy(srcIn->getImageRect());
    }else{
      src = srcIn;
    }
    
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                         \
      case depth##D:                                                                                     \
        return apply_inplace_arithmetical_op(src->asImg<icl##D>(), getROIOnly(),getValue(),getOpType()); \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH  
    }
    if (!getROIOnly() && !srcIn->hasFullROI()){
      delete src;
    }
    return srcIn;
  }
  
}
