/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/BinaryCompareOp.cpp                      **
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
*********************************************************************/

#include <ICLFilter/BinaryCompareOp.h>
#include <ICLCore/Img.h>


namespace icl {

#define ICL_COMP_ZERO 0
#define ICL_COMP_NONZERO 255

  namespace{
  
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
  
  template <class T, template<typename T> class C>
  inline void fallbackCompare(const Img<T> *src1,const Img<T> *src2,Img<icl8u> *dst){
    // {{{ open
    for(int c=src1->getChannels()-1; c >= 0; --c) {
      const ImgIterator<T> itSrc1 = src1->beginROI(c);
       const ImgIterator<T> itEnd = src1->endROI(c);
      const ImgIterator<T> itSrc2 = src2->beginROI(c);
      ImgIterator<icl8u>  itDst = dst->beginROI(c);
      for(;itSrc1 != itEnd ; ++itSrc1,++itSrc2, ++itDst){
        *itDst = C<T>::cmp(*itSrc1,*itSrc2);
      }
    }
  }

  // }}}
  
  template <typename T>
  inline void fallbackCompareWithTolerance(const Img<T> *src1,const Img<T> *src2, Img8u *dst,T tolerance) {
    // {{{ open
     for(int c=src1->getChannels()-1; c >= 0; --c) {
       const ImgIterator<T> itSrc1 = src1->beginROI(c);
       const ImgIterator<T> itEnd = src1->endROI(c);
       const ImgIterator<T> itSrc2 = src2->beginROI(c);
       ImgIterator<icl8u>  itDst = dst->beginROI(c);
       for(;itSrc1 != itEnd; ++itSrc1,++itSrc2, ++itDst){
         *itDst = CompareOp_eqt<T>::cmp(*itSrc1,*itSrc2,tolerance);
       }
     }
   }
   // }}}
  
  template<class T>
  void cmp(const Img<T> *src1,const  Img<T> *src2, Img8u *dst, T tolerance, BinaryCompareOp::optype ot){
     // {{{ open

     switch(ot){                
       case BinaryCompareOp::lt: fallbackCompare<T,CompareOp_lt>(src1,src2,dst); break;
       case BinaryCompareOp::gt: fallbackCompare<T,CompareOp_gt>(src1,src2,dst); break;
       case BinaryCompareOp::lteq: fallbackCompare<T, CompareOp_lteq>(src1,src2,dst); break;
       case BinaryCompareOp::gteq: fallbackCompare<T, CompareOp_gteq>(src1,src2,dst); break;
       case BinaryCompareOp::eq: fallbackCompare<T, CompareOp_eq>(src1,src2,dst); break;
       case BinaryCompareOp::eqt: fallbackCompareWithTolerance<T>(src1,src2,dst,tolerance); break; 
     }
   }

  // }}}
   

#ifdef HAVE_IPP 
  template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*,int,const T*,int, icl8u*,int,IppiSize,IppCmpOp)>
  inline void ippCall(const Img<T> *src1,const Img<T>*src2, Img8u *dst, BinaryCompareOp::optype cmpOp){
    // {{{ open
    for (int c=src1->getChannels()-1; c >= 0; --c) {
      ippiFunc (src1->getROIData (c), src1->getLineStep(), 
                src2->getROIData(c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(),
                dst->getROISize(),(IppCmpOp) cmpOp);
    }
  }
  // }}}

  template<> void cmp<icl8u>(const Img8u *src1, const Img8u *src2, Img8u *dst, icl8u tolerance, BinaryCompareOp::optype ot){
    // {{{ open
    if(ot == BinaryCompareOp::eqt){
      fallbackCompareWithTolerance<icl8u>(src1,src2,dst,tolerance);
    }else{
      ippCall<icl8u,ippiCompare_8u_C1R>(src1,src2,dst,ot);
    }
  }  
  // }}}
  template<> void cmp<icl16s>(const Img16s *src1,const Img16s *src2, Img8u *dst, icl16s tolerance, BinaryCompareOp::optype ot){
    // {{{ open
    if(ot == BinaryCompareOp::eqt){
      fallbackCompareWithTolerance<icl16s>(src1,src2,dst,tolerance);
    }else{
      ippCall<icl16s,ippiCompare_16s_C1R>(src1,src2,dst,ot);
    }
  }
  // }}}
  template<> void cmp<icl32f>(const Img32f *src1,const Img32f *src2, Img8u *dst, icl32f tolerance, BinaryCompareOp::optype ot){
    // {{{ open
    if(ot == BinaryCompareOp::eqt){
      for (int c=src1->getChannels()-1; c >= 0; --c) {
        ippiCompareEqualEps_32f_C1R (src1->getROIData (c), src1->getLineStep(),
                                     src2->getROIData (c), src2->getLineStep(),
                                     dst->getROIData (c), dst->getLineStep(),
                                     dst->getROISize(), tolerance);
      }
    }else{
      ippCall<icl32f,ippiCompare_32f_C1R>(src1,src2,dst,ot); 
    }
  }
  // }}}
#endif
  } // end of anonymous namespace
  
  void BinaryCompareOp::apply(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst){
     // {{{ open
    ICLASSERT_RETURN( poSrc1 );
    ICLASSERT_RETURN( poSrc2 );
    ICLASSERT_RETURN( ppoDst );

    if(!BinaryOp::check(poSrc1,poSrc2)){
      ERROR_LOG("source images are not compatible: src 1:" << *poSrc1 << " src 2:" << *poSrc2);
    } 
    if(!BinaryOp::prepare(ppoDst,poSrc1,depth8u)){
      ERROR_LOG("unable to prepare the destintaion imaage to source image params and depth8u, src 1/2: " << *poSrc1);
    }

    switch (poSrc1->getDepth()){
#define ICL_INSTANTIATE_DEPTH(T) case depth##T:                             \
                                 cmp(poSrc1->asImg<icl##T>(),               \
                                 poSrc2->asImg<icl##T>(),                   \
                                  (*ppoDst)->asImg<icl8u>(),                \
                                 clipped_cast<icl64f,icl##T>(m_dTolerance), \
                                 m_eOpType); break;
       ICL_INSTANTIATE_ALL_DEPTHS;
       default: ICL_INVALID_FORMAT; break;
#undef ICL_INSTANTIATE_DEPTH
     }
   }

  // }}}

}// end of namespace icl
