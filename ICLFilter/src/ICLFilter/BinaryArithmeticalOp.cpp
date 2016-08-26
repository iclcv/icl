/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryArithmeticalOp.cpp       **
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

#include <ICLCore/Img.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
    namespace{
     
      template<class T, BinaryArithmeticalOp::optype OT> struct PixelFunc{ 
        static inline T apply(const T t1, const T t2){ return t1+t2; } 
      };
      template<class T> struct PixelFunc<T,BinaryArithmeticalOp::addOp>{ 
        static inline T apply(const T t1, const T t2){ return t1+t2; }
      };
      template<class T> struct PixelFunc<T,BinaryArithmeticalOp::subOp>{ 
        static inline T apply(const T t1, const T t2){ return t1-t2; }
      };
      template<class T> struct PixelFunc<T,BinaryArithmeticalOp::mulOp>{ 
        static inline T apply(const T t1, const T t2){ return t1*t2; }
      };
      template<class T> struct PixelFunc<T,BinaryArithmeticalOp::divOp>{ 
        static inline T apply(const T t1, const T t2){ return t1/t2; }
      };
      template<class T> struct PixelFunc<T,BinaryArithmeticalOp::absSubOp>{ 
        static inline T apply(const T t1, const T t2){ return std::abs(t1-t2); }
      };
      template<class T, BinaryArithmeticalOp::optype OT> struct LoopFunc{  
        // {{{ open
        static inline void apply(const  Img<T> *src1, const Img<T> *src2, Img<T> *dst ){
          for(int c=src1->getChannels()-1; c >= 0; --c) {
            ImgIterator<T> itDst = dst->beginROI(c);
            for(const ImgIterator<T> itSrc1 = src1->beginROI(c),itSrc2 = src2->beginROI(c),
                itSrc1End = src1->endROI(c); itSrc1 != itSrc1End; ++itSrc1, ++itSrc2, ++itDst){
              *itDst = PixelFunc<T,OT>::apply(*itSrc1,*itSrc2);
            }
          }
        }
      };
  
      // }}}
    
     
  #ifdef ICL_HAVE_IPP
      template <typename T, IppStatus (IPP_DECL *func) (const T*, int,const T*, int, T*, int, IppiSize)>
      inline void ipp_call(const Img<T> *src1,const Img<T> *src2, Img<T> *dst){
        // {{{ open
        for (int c=src1->getChannels()-1; c >= 0; --c) {
          func (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(), 
                dst->getROISize());
        }
    }
    // }}}
      
      template <typename T, IppStatus (IPP_DECL *func) (const T*, int, const T*, int, T*, int, IppiSize, int)>
      inline void ipp_call_sfs(const Img<T> *src1,const Img<T> *src2, Img<T> *dst){
        // {{{ open
        for (int c=src1->getChannels()-1; c >= 0; --c) {
          func (src1->getROIData (c), src1->getLineStep(),
                src2->getROIData (c), src2->getLineStep(),
                dst->getROIData (c), dst->getLineStep(), 
                dst->getROISize(), 0);
        }
    }
    // }}}
     
  #define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                                           \
      template<> struct LoopFunc<icl8u, BinaryArithmeticalOp::OP##Op>{                                    \
        static inline void apply(const  Img<icl8u> *src1,const  Img<icl8u> *src2, Img<icl8u> *dst ){      \
          ipp_call_sfs<icl8u,ippi##IPPOP##_8u_C1RSfs>(src2,src1,dst);                                     \
        }                                                                                                 \
      };                                                                                                  \
      template<> struct LoopFunc<icl16s, BinaryArithmeticalOp::OP##Op>{                                   \
        static inline void apply(const  Img<icl16s> *src1,const  Img<icl16s> *src2, Img<icl16s> *dst ){   \
          ipp_call_sfs<icl16s,ippi##IPPOP##_16s_C1RSfs>(src2,src1, dst);                                  \
        }                                                                                                 \
      };                                                                                                  \
      template<> struct LoopFunc<icl32f, BinaryArithmeticalOp::OP##Op>{                                   \
        static inline void apply(const  Img<icl32f> *src1,const  Img<icl32f> *src2, Img<icl32f> *dst ){   \
          ipp_call<icl32f,ippi##IPPOP##_32f_C1R>(src2,src1, dst);                                         \
        }                                                                                                 \
      }
      CREATE_IPP_FUNCTIONS_FOR_OP(add,Add);
      CREATE_IPP_FUNCTIONS_FOR_OP(sub,Sub);
      CREATE_IPP_FUNCTIONS_FOR_OP(mul,Mul);
      CREATE_IPP_FUNCTIONS_FOR_OP(div,Div);
      
      /// for absDiff only 8u and 32f all supported by the IPP
      template<> struct LoopFunc<icl8u, BinaryArithmeticalOp::absSubOp>{
        static inline void apply(const  Img<icl8u> *src1,const  Img<icl8u> *src2, Img<icl8u> *dst ){
          ipp_call<icl8u,ippiAbsDiff_8u_C1R>(src2,src1,dst);
        } 
      };  
      template<> struct LoopFunc<icl32f, BinaryArithmeticalOp::absSubOp>{
        static inline void apply(const  Img<icl32f> *src1,const  Img<icl32f> *src2, Img<icl32f> *dst ){
          ipp_call<icl32f,ippiAbsDiff_32f_C1R>(src2,src1,dst);
        } 
      };  
  
  #undef CREATE_IPP_FUNCTIONS_FOR_OP    
      
      
  #endif
      
     
      template<BinaryArithmeticalOp::optype OT>
      void apply_op(const ImgBase *src1,const ImgBase *src2, ImgBase *dst){
        // {{{ open
        switch(src1->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: LoopFunc<icl##D,OT>::apply(src1->asImg<icl##D>(),src2->asImg<icl##D>(), dst->asImg<icl##D>()); break;
          ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
        }
      } 
      
      // }}}
    
    } // end of anonymous namespace 
  
    void BinaryArithmeticalOp::apply(const ImgBase *poSrc1,const ImgBase *poSrc2, ImgBase **poDst){
      // {{{ open
      ICLASSERT_RETURN( poSrc1 );
      ICLASSERT_RETURN( poSrc2 );
      ICLASSERT_RETURN( poDst );
      
      if(!BinaryOp::check(poSrc1,poSrc2) || !BinaryOp::prepare(poDst,poSrc1)){
        ERROR_LOG("source and destination images are not compatible [operation skipped!]");
        return;
      }
      switch(m_eOpType){
        case addOp:  apply_op<addOp>(poSrc1,poSrc2,*poDst); break;
        case mulOp:  apply_op<mulOp>(poSrc1,poSrc2,*poDst); break;
        case divOp:  apply_op<divOp>(poSrc1,poSrc2,*poDst); break;
        case subOp:  apply_op<subOp>(poSrc1,poSrc2,*poDst); break;
        case absSubOp:  apply_op<absSubOp>(poSrc1,poSrc2,*poDst); break;
      }
    }
    // }}}
  
  } // namespace filter
}
