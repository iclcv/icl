/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryCompareOp.cpp             **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLFilter/UnaryCompareOp.h>
#include <ICLCore/Img.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

  #define ICL_COMP_ZERO 0
  #define ICL_COMP_NONZERO 255


  #define CREATE_COMPARE_OP(NAME,OPERATOR)                              \
    template <typename T> struct CompareOp_##NAME {                     \
      static inline icl8u cmp(T val1, T val2){                          \
        return val1 OPERATOR val2 ? ICL_COMP_NONZERO : ICL_COMP_ZERO;   \
      }                                                                 \
    }

    CREATE_COMPARE_OP(eq,==);
    CREATE_COMPARE_OP(lt,<);
    CREATE_COMPARE_OP(lteq,<=);
    CREATE_COMPARE_OP(gteq,>=);
    CREATE_COMPARE_OP(gt,>);

  #undef CREATE_COMPARE_OP

    template <typename T> struct CompareOp_eqt {
      // {{{ open

      static inline icl8u cmp(T val1, T val2, T tolerance){
        return std::abs(val1-val2)<=tolerance ? ICL_COMP_NONZERO : ICL_COMP_ZERO;
      }
    };

    // }}}

    template <class T, template<typename X> class C>
    inline void fallbackCompare(const Img<T> *src, T value,Img<icl8u> *dst){
      // {{{ open
      for(int c=src->getChannels()-1; c >= 0; --c) {
        const ImgIterator<T> itSrc = src->beginROI(c);
        const ImgIterator<T> itSrcEnd = src->endROI(c);
        ImgIterator<icl8u>  itDst = dst->beginROI(c);
        for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
          *itDst = C<T>::cmp(*itSrc,value);
        }
      }
    }

    // }}}

    template <typename T>
    inline void fallbackCompareWithTolerance(const Img<T> *src, T value, Img8u *dst,T tolerance) {
      // {{{ open
       for(int c=src->getChannels()-1; c >= 0; --c) {
         const ImgIterator<T> itSrc = src->beginROI(c);
         const ImgIterator<T> itSrcEnd = src->endROI(c);
         ImgIterator<icl8u>  itDst = dst->beginROI(c);
         for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
           *itDst = CompareOp_eqt<T>::cmp(*itSrc,value,tolerance);
         }
       }
     }
     // }}}

    template<class T>
    void cmp(const Img<T> *src, Img8u *dst, T value, T tolerance, UnaryCompareOp::optype ot){
       // {{{ open

       switch(ot){
         case UnaryCompareOp::lt: fallbackCompare<T,CompareOp_lt>(src,value,dst); break;
         case UnaryCompareOp::gt: fallbackCompare<T,CompareOp_gt>(src,value,dst); break;
         case UnaryCompareOp::lteq: fallbackCompare<T, CompareOp_lteq>(src,value,dst); break;
         case UnaryCompareOp::gteq: fallbackCompare<T, CompareOp_gteq>(src,value,dst); break;
         case UnaryCompareOp::eq: fallbackCompare<T, CompareOp_eq>(src,value,dst); break;
         case UnaryCompareOp::eqt: fallbackCompareWithTolerance<T>(src,value,dst,tolerance); break;
       }
     }

    // }}}


  #ifdef ICL_HAVE_IPP
    template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*,int,T,icl8u*,int,IppiSize,IppCmpOp)>
    inline void ippCall(const Img<T> *src, T value, Img8u *dst, UnaryCompareOp::optype cmpOp){
      // {{{ open
      for (int c=src->getChannels()-1; c >= 0; --c) {
        ippiFunc (src->getROIData (c), src->getLineStep(), value,
                  dst->getROIData (c), dst->getLineStep(),
                  dst->getROISize(),(IppCmpOp) cmpOp);
      }
    }
    // }}}

    template<> void cmp<icl8u>(const Img8u *src, Img8u *dst, icl8u value, icl8u tolerance, UnaryCompareOp::optype ot){
      // {{{ open
      if(ot == UnaryCompareOp::eqt){
        fallbackCompareWithTolerance<icl8u>(src,value,dst,tolerance);
      }else{
        ippCall<icl8u,ippiCompareC_8u_C1R>(src,value,dst,ot);
      }
    }
    // }}}
    template<> void cmp<icl16s>(const Img16s *src, Img8u *dst, icl16s value, icl16s tolerance, UnaryCompareOp::optype ot){
      // {{{ open
      if(ot == UnaryCompareOp::eqt){
        fallbackCompareWithTolerance<icl16s>(src,value,dst,tolerance);
      }else{
        ippCall<icl16s,ippiCompareC_16s_C1R>(src,value,dst,ot);
      }
    }
    // }}}
    template<> void cmp<icl32f>(const Img32f *src, Img8u *dst, icl32f value, icl32f tolerance, UnaryCompareOp::optype ot){
      // {{{ open
      if(ot == UnaryCompareOp::eqt){
        for (int c=src->getChannels()-1; c >= 0; --c) {
          ippiCompareEqualEpsC_32f_C1R (src->getROIData (c), src->getLineStep(), value,
                                        dst->getROIData (c), dst->getLineStep(),
                                        dst->getROISize(), tolerance);
        }
      }else{
        ippCall<icl32f,ippiCompareC_32f_C1R>(src,value,dst,ot);
      }
    }
    // }}}
  #endif

    void UnaryCompareOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
       // {{{ open
      ICLASSERT_RETURN(poSrc);
      ICLASSERT_RETURN(ppoDst);
      if( *ppoDst ){
        ICLASSERT_RETURN( (*ppoDst)->getDepth()==depth8u || (*ppoDst) != poSrc );
      }

       if (!UnaryOp::prepare (ppoDst, poSrc, depth8u)) return;
       switch (poSrc->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(T) case depth##T:                                              \
         icl::filter::cmp(poSrc->asImg<icl##T>(),(*ppoDst)->asImg<icl8u>(), \
                          clipped_cast<icl64f,icl##T>(m_dValue),        \
                                   clipped_cast<icl64f,icl##T>(m_dTolerance),                   \
                          m_eOpType); break;
         ICL_INSTANTIATE_ALL_DEPTHS;
         default: ICL_INVALID_FORMAT; break;
  #undef ICL_INSTANTIATE_DEPTH
       }
     }

    // }}}





  } // namespace filter
}
