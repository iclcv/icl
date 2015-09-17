/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryLogicalOp.cpp             **
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

#include <ICLFilter/UnaryLogicalOp.h>
#include <ICLCore/Img.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
    namespace{
  
  
      template<class T, UnaryLogicalOp::optype OT> struct PixelFuncNoVal{ 
        static inline T apply(const T t){ return t; } 
      };
  
      template<class T> struct PixelFuncNoVal<T,UnaryLogicalOp::notOp>{
        static inline T apply(const T t){ return ~t; }
      };
  
      template<class T, UnaryLogicalOp::optype OT> struct LoopFuncNoVal{  
        static inline void apply(const  Img<T> *src, Img<T> *dst ){
          for(int c=src->getChannels()-1; c >= 0; --c) {
            ImgIterator<T> itDst = dst->beginROI(c);
            for(const ImgIterator<T> itSrc = src->beginROI(c),itSrcEnd=src->endROI(c) ; itSrc!=itSrcEnd; ++itSrc, ++itDst){
              *itDst = PixelFuncNoVal<T,OT>::apply(*itSrc);
            }
          }
        }
      };
  
      template<class T, UnaryLogicalOp::optype OT> struct PixelFuncWithVal{ 
        static inline T apply(const T t, T val){ return t &val; } 
      };
      template<class T> struct PixelFuncWithVal<T,UnaryLogicalOp::andOp>{ 
        static inline T apply(const T t, T val){ return t & val; } 
      };
      template<class T> struct PixelFuncWithVal<T,UnaryLogicalOp::orOp>{ 
        static inline T apply(const T t, T val){ return t | val; } 
      };
      template<class T> struct PixelFuncWithVal<T,UnaryLogicalOp::xorOp>{ 
        static inline T apply(const T t, T val){ return t ^ val; } 
      };
      template<class T, UnaryLogicalOp::optype OT> struct LoopFuncWithVal{  
        static inline void apply(const  Img<T> *src, Img<T> *dst, T val ){
          for(int c=src->getChannels()-1; c >= 0; --c) {
            ImgIterator<T> itDst = dst->beginROI(c);
            for(const ImgIterator<T> itSrc = src->beginROI(c),itSrcEnd = src->endROI(c) ; itSrc!=itSrcEnd; ++itSrc, ++itDst){
              *itDst = PixelFuncWithVal<T,OT>::apply(*itSrc,val);
            }
          }
        }
      };
      
  #ifdef ICL_HAVE_IPP
      // IPP function specializations for "no val":
      template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T*, int, IppiSize)>
      inline void ipp_call_no_val(const Img<T> *src, Img<T> *dst){
        // {{{ open
        for (int c=src->getChannels()-1; c >= 0; --c) {
          func (src->getROIData (c), src->getLineStep(), dst->getROIData (c), dst->getLineStep(), dst->getROISize());
        }
      }
      
  #define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                                                                                   \
      template<> struct LoopFuncNoVal<icl8u, UnaryLogicalOp::OP##Op>{                                                                        \
        static inline void apply(const  Img<icl8u> *src, Img<icl8u> *dst ){ ipp_call_no_val<icl8u,ippi##IPPOP##_8u_C1R>(src, dst); }       \
      }                                                                                                                                          
      CREATE_IPP_FUNCTIONS_FOR_OP(not,Not);
      
  #undef CREATE_IPP_FUNCTIONS_FOR_OP    
      
      
       /*** IPP function specializations for "with val":
          addOp=0,
          subOp=1,
          divOp=2,
          mulOp=3,
       ***/    
      template <typename T, IppStatus (IPP_DECL *func) (const T*, int, T,  T*, int, IppiSize)>
      inline void ipp_call_with_val(const Img<T> *src, Img<T> *dst, T val){
        for (int c=src->getChannels()-1; c >= 0; --c) {
          func (src->getROIData (c), src->getLineStep(),val, dst->getROIData (c), dst->getLineStep(), dst->getROISize());
        }
      }
      
  
  #define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                           \
      template<> struct LoopFuncWithVal<icl8u, UnaryLogicalOp::OP##Op>{              \
        static inline void apply(const Img<icl8u> *src, Img<icl8u> *dst, icl8u val ){     \
          ipp_call_with_val<icl8u,ippi##IPPOP##_8u_C1R>(src, dst,val);             \
        }                                                                                 \
      };                                                                                  \
      template<> struct LoopFuncWithVal<icl32s, UnaryLogicalOp::OP##Op>{             \
        static inline void apply(const Img<icl32s> *src, Img<icl32s> *dst, icl32s val ){  \
          ipp_call_with_val<icl32s,ippi##IPPOP##_32s_C1R>(src, dst,val);                  \
        }                                                                                 \
      }
  
      CREATE_IPP_FUNCTIONS_FOR_OP(and,AndC);
      CREATE_IPP_FUNCTIONS_FOR_OP(or,OrC);
      CREATE_IPP_FUNCTIONS_FOR_OP(xor,XorC);
  //fallback f16s TODO
  #undef CREATE_IPP_FUNCTIONS_FOR_OP  
      
  #endif
      
      template<UnaryLogicalOp::optype OT>
      void apply_unary_logical_op_no_val(const ImgBase *src, ImgBase *dst){

  
        switch(src->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: LoopFuncNoVal<icl##D, OT>::apply(src->asImg<icl##D>(),dst->asImg<icl##D>()); break;
          ICL_INSTANTIATE_ALL_INT_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
          default: ICL_INVALID_DEPTH;
        }
      } 
  
      template<UnaryLogicalOp::optype OT>
      void apply_unary_logical_op_with_val(const ImgBase *src, ImgBase *dst, icl32s val){
  
        switch(src->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                                 \
          case depth##D: LoopFuncWithVal<icl##D, OT>::apply(src->asImg<icl##D >(), \
          dst->asImg<icl##D >(), clipped_cast<icl32s,icl##D>(val)); break;
          ICL_INSTANTIATE_ALL_INT_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH  
          default: ICL_INVALID_DEPTH;
        }
      } 
  
    } // end of anonymous namespace 
  
    void UnaryLogicalOp::apply(const ImgBase *poSrc, ImgBase **poDst){
      ICLASSERT_RETURN( poSrc );
      if(!UnaryOp::prepare(poDst,poSrc)) return;
      switch(m_eOpType){
        case andOp:  apply_unary_logical_op_with_val<andOp>(poSrc,*poDst,m_dValue); break;
        case orOp:  apply_unary_logical_op_with_val<orOp>(poSrc,*poDst,m_dValue); break;
        case xorOp:  apply_unary_logical_op_with_val<xorOp>(poSrc,*poDst,m_dValue); break;
                
        case notOp:  apply_unary_logical_op_no_val<notOp>(poSrc,*poDst); break;
      }
    }
  } // namespace filter
}
