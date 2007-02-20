#include <UnaryArithmeticalOp.h>
#include <Img.h>
#include <math.h>

namespace icl {
  namespace{
    ///-----------------------------------------------------------------------------------------------
    ///          no val
    ///-----------------------------------------------------------------------------------------------
    template<class T> struct AbsFunc{ static inline T f(const T t) { return abs(t); }  };
    template<> struct AbsFunc<icl32f>{ static inline icl32f f(const icl32f t) { return fabs(t); }  };
    template<> struct AbsFunc<icl64f>{ static inline icl64f f(const icl64f t) { return fabs(t); }  };


    template<class T, UnaryArithmeticalOp::optype OT> struct PixelFuncNoVal{ 
      static inline T apply(const T t){ return t; } 
    };
    template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::sqrOp>{ 
      static inline T apply(const T t){ return t*t; }
    };
    template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::sqrtOp>{
      static inline T apply(const T t){ return Cast<double,T>::cast(sqrt((double)t)); } 
    };
    template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::lnOp>{
      static inline T apply(const T t){ return Cast<double,T>::cast(log((double)t)); } 
    };
    template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::expOp>{
      static inline T apply(const T t){ return Cast<double,T>::cast(exp((double)t)); } 
    };
    template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::absOp>{
      static inline T apply(const T t){ return AbsFunc<T>::f(t); }
    };

    template<class T, UnaryArithmeticalOp::optype OT> struct LoopFuncNoVal{  
      // {{{ open

      static inline void apply(const  Img<T> *src, Img<T> *dst ){
        for(int c=src->getChannels()-1; c >= 0; --c) {
          ImgIterator<T> itDst = dst->getROIIterator(c);
          for(ConstImgIterator<T> itSrc = src->getROIIterator(c) ; itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = PixelFuncNoVal<T,OT>::apply(*itSrc);
          }
        }
      }
    };

    // }}}
  
    ///-----------------------------------------------------------------------------------------------
    ///           with val
    ///-----------------------------------------------------------------------------------------------
    template<class T, UnaryArithmeticalOp::optype OT> struct PixelFuncWithVal{ 
      static inline T apply(const T t, T val){ return t+val; } 
    };
    template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::addOp>{ 
      static inline T apply(const T t, T val){ return t+val; } 
    };
    template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::subOp>{ 
      static inline T apply(const T t, T val){ return t-val; } 
    };
    template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::divOp>{ 
      static inline T apply(const T t, T val){ return t/val; } 
    };
    template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::mulOp>{ 
      static inline T apply(const T t, T val){ return t*val; } 
    };
    template<class T, UnaryArithmeticalOp::optype OT> struct LoopFuncWithVal{  
      // {{{ open

      static inline void apply(const  Img<T> *src, Img<T> *dst, T val ){
        for(int c=src->getChannels()-1; c >= 0; --c) {
          ImgIterator<T> itDst = dst->getROIIterator(c);
          for(ConstImgIterator<T> itSrc = src->getROIIterator(c) ; itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst = PixelFuncWithVal<T,OT>::apply(*itSrc,val);
          }
        }
      }
    };

    // }}}

    
#ifdef WITH_IPP_OPTIMIZATION
    /*** IPP function specializations for "no val":
        sqrOp=10,
        sqrtOp=11,
        lnOp=12,
        expOp=13,
        absOp=14
    ***/
    template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T*, int, IppiSize)>
    inline void ipp_call_no_val(const Img<T> *src, Img<T> *dst){
      // {{{ open
      for (int c=src->getChannels()-1; c >= 0; --c) {
        func (src->getROIData (c), src->getLineStep(), dst->getROIData (c), dst->getLineStep(), dst->getROISize());
      }
  }
  // }}}
    
    template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T*, int, IppiSize,int)>
    inline void ipp_call_no_val_sfs(const Img<T> *src, Img<T> *dst){
      // {{{ open
      for (int c=src->getChannels()-1; c >= 0; --c) {
        func (src->getROIData (c), src->getLineStep(), dst->getROIData (c), dst->getLineStep(), dst->getROISize(), 0); 
      }
  }
  // }}}

#define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                                                                                   \
    template<> struct LoopFuncNoVal<icl8u, UnaryArithmeticalOp::OP##Op>{                                                                        \
      static inline void apply(const  Img<icl8u> *src, Img<icl8u> *dst ){ ipp_call_no_val_sfs<icl8u,ippi##IPPOP##_8u_C1RSfs>(src, dst); }       \
    };                                                                                                                                          \
    template<> struct LoopFuncNoVal<icl16s, UnaryArithmeticalOp::OP##Op>{                                                                       \
      static inline void apply(const  Img<icl16s> *src, Img<icl16s> *dst ){ ipp_call_no_val_sfs<icl16s,ippi##IPPOP##_16s_C1RSfs>(src, dst); }   \
    };                                                                                                                                          \
    template<> struct LoopFuncNoVal<icl32f, UnaryArithmeticalOp::OP##Op>{                                                                       \
      static inline void apply(const  Img<icl32f> *src, Img<icl32f> *dst ){ ipp_call_no_val<icl32f,ippi##IPPOP##_32f_C1R>(src, dst); }          \
    }
    CREATE_IPP_FUNCTIONS_FOR_OP(sqr,Sqr);
    CREATE_IPP_FUNCTIONS_FOR_OP(sqrt,Sqrt);
    CREATE_IPP_FUNCTIONS_FOR_OP(ln,Ln);
    CREATE_IPP_FUNCTIONS_FOR_OP(exp,Exp);
#undef CREATE_IPP_FUNCTIONS_FOR_OP    
    
    template<> struct LoopFuncNoVal<icl16s, UnaryArithmeticalOp::absOp>{
      static inline void apply(const  Img<icl16s> *src, Img<icl16s> *dst ){ ipp_call_no_val<icl16s,ippiAbs_16s_C1R>(src, dst); }
    }; 
    template<> struct LoopFuncNoVal<icl32f, UnaryArithmeticalOp::absOp>{
      static inline void apply(const  Img<icl32f> *src, Img<icl32f> *dst ){ ipp_call_no_val<icl32f,ippiAbs_32f_C1R>(src, dst); }
    }; 
    
     /*** IPP function specializations for "with val":
        addOp=0,
        subOp=1,
        divOp=2,
        mulOp=3,
     ***/    
    template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T,  T*, int, IppiSize)>
    inline void ipp_call_with_val(const Img<T> *src, Img<T> *dst, T val){
      // {{{ open
      for (int c=src->getChannels()-1; c >= 0; --c) {
        func (src->getROIData (c), src->getLineStep(),val, dst->getROIData (c), dst->getLineStep(), dst->getROISize());
      }
  }
  // }}}
    
    template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T,  T*, int, IppiSize, int)>
    inline void ipp_call_with_val_sfs(const Img<T> *src, Img<T> *dst, T val){
      // {{{ open
      for (int c=src->getChannels()-1; c >= 0; --c) {
        func (src->getROIData (c), src->getLineStep(),val, dst->getROIData (c), dst->getLineStep(), dst->getROISize(),0);
      }
  }
  // }}}

#define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                           \
    template<> struct LoopFuncWithVal<icl8u, UnaryArithmeticalOp::OP##Op>{              \
      static inline void apply(const Img<icl8u> *src, Img<icl8u> *dst, icl8u val ){     \
        ipp_call_with_val_sfs<icl8u,ippi##IPPOP##_8u_C1RSfs>(src, dst,val);             \
      }                                                                                 \
    };                                                                                  \
    template<> struct LoopFuncWithVal<icl16s, UnaryArithmeticalOp::OP##Op>{             \
      static inline void apply(const Img<icl16s> *src, Img<icl16s> *dst, icl16s val ){  \
        ipp_call_with_val_sfs<icl16s,ippi##IPPOP##_16s_C1RSfs>(src, dst,val);           \
      }                                                                                 \
    };                                                                                  \
    template<> struct LoopFuncWithVal<icl32f, UnaryArithmeticalOp::OP##Op>{             \
      static inline void apply(const Img<icl32f> *src, Img<icl32f> *dst, icl32f val ){  \
        ipp_call_with_val<icl32f,ippi##IPPOP##_32f_C1R>(src, dst,val);                  \
      }                                                                                 \
    }

    CREATE_IPP_FUNCTIONS_FOR_OP(add,AddC);
    CREATE_IPP_FUNCTIONS_FOR_OP(sub,SubC);
    CREATE_IPP_FUNCTIONS_FOR_OP(div,DivC);
    CREATE_IPP_FUNCTIONS_FOR_OP(mul,MulC);
#undef CREATE_IPP_FUNCTIONS_FOR_OP  
    
#endif
    
    template<UnaryArithmeticalOp::optype OT>
    void apply_unary_arithmetical_op_no_val(const ImgBase *src, ImgBase *dst){
      // {{{ open

      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: LoopFuncNoVal<icl##D, OT>::apply(src->asImg<icl##D>(),dst->asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_DEPTH;
      }
    } 

    // }}}
    template<UnaryArithmeticalOp::optype OT>
    void apply_unary_arithmetical_op_with_val(const ImgBase *src, ImgBase *dst, icl64f val){
      // {{{ open

      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                 \
        case depth##D: LoopFuncWithVal<icl##D, OT>::apply(src->asImg<icl##D >(), \
        dst->asImg<icl##D >(), Cast<icl64f,icl##D>::cast(val)); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH  
        default: ICL_INVALID_DEPTH;
      }
    } 

    // }}}
  } // end of anonymous namespace 

  void UnaryArithmeticalOp::apply(const ImgBase *poSrc, ImgBase **poDst){
    // {{{ open
    ICLASSERT_RETURN( poSrc );
    if(!UnaryOp::prepare(poDst,poSrc)) return;
    switch(m_eOpType){
      case addOp:  apply_unary_arithmetical_op_with_val<addOp>(poSrc,*poDst,m_dValue); break;
      case mulOp:  apply_unary_arithmetical_op_with_val<mulOp>(poSrc,*poDst,m_dValue); break;
      case divOp:  apply_unary_arithmetical_op_with_val<divOp>(poSrc,*poDst,m_dValue); break;
      case subOp:  apply_unary_arithmetical_op_with_val<subOp>(poSrc,*poDst,m_dValue); break;
        
      case sqrOp:  apply_unary_arithmetical_op_no_val<sqrOp>(poSrc,*poDst); break;
      case sqrtOp: apply_unary_arithmetical_op_no_val<sqrtOp>(poSrc,*poDst); break;
      case lnOp:   apply_unary_arithmetical_op_no_val<lnOp>(poSrc,*poDst); break;
      case expOp:  apply_unary_arithmetical_op_no_val<expOp>(poSrc,*poDst); break;
      case absOp:  apply_unary_arithmetical_op_no_val<absOp>(poSrc,*poDst); break;
    }
  }
  // }}}
}
