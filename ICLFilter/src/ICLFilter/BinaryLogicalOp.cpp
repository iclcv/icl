/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryLogicalOp.cpp            **
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

#include <ICLFilter/BinaryLogicalOp.h>
#include <ICLCore/Img.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
    namespace{

      template<class T, BinaryLogicalOp::optype OT> struct PixelFunc{
        static inline T apply(const T t1, const T t2){ return t1 & t2; }
      };
      template<class T> struct PixelFunc<T,BinaryLogicalOp::andOp>{
        static inline T apply(const T t1, const T t2){ return t1 & t2; }
      };
      template<class T> struct PixelFunc<T,BinaryLogicalOp::orOp>{
        static inline T apply(const T t1, const T t2){ return t1 | t2; }
      };
      template<class T> struct PixelFunc<T,BinaryLogicalOp::xorOp>{
        static inline T apply(const T t1, const T t2){ return t1 ^ t2; }
      };

      template<class T, BinaryLogicalOp::optype OT> struct LoopFunc{
        // {{{ open
        static inline void apply(const  Img<T> *src1, const Img<T> *src2, Img<T> *dst ){
          for(int c=src1->getChannels()-1; c >= 0; --c) {
            ImgIterator<T> itDst = dst->beginROI(c);

            for(const ImgIterator<T> itSrc1 = src1->beginROI(c),itSrc2 = src2->beginROI(c),
                itEnd = src1->endROI(c); itSrc1 != itEnd; ++itSrc1, ++itSrc2, ++itDst){
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

  #define CREATE_IPP_FUNCTIONS_FOR_OP(OP,IPPOP)                                                           \
      template<> struct LoopFunc<icl8u, BinaryLogicalOp::OP##Op>{                                    \
        static inline void apply(const  Img<icl8u> *src1,const  Img<icl8u> *src2, Img<icl8u> *dst ){      \
          ipp_call<icl8u,ippi##IPPOP##_8u_C1R>(src2,src1,dst);                                     \
        }                                                                                                 \
      };                                                                                                  \
      template<> struct LoopFunc<icl32s, BinaryLogicalOp::OP##Op>{                                   \
        static inline void apply(const  Img<icl32s> *src1,const  Img<icl32s> *src2, Img<icl32s> *dst ){   \
          ipp_call<icl32s,ippi##IPPOP##_32s_C1R>(src2,src1, dst);                                         \
        }                                                                                                 \
      }
      CREATE_IPP_FUNCTIONS_FOR_OP(and,And);
      CREATE_IPP_FUNCTIONS_FOR_OP(or,Or);
      CREATE_IPP_FUNCTIONS_FOR_OP(xor,Xor);

  #undef CREATE_IPP_FUNCTIONS_FOR_OP


  #endif


      template<BinaryLogicalOp::optype OT>
      void apply_op(const ImgBase *src1,const ImgBase *src2, ImgBase *dst){
        // {{{ open
        switch(src1->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: LoopFunc<icl##D,OT>::apply(src1->asImg<icl##D>(),src2->asImg<icl##D>(), dst->asImg<icl##D>()); break;
          ICL_INSTANTIATE_ALL_INT_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
          default: ICL_INVALID_DEPTH;
        }
      }

      // }}}

    } // end of anonymous namespace

    void BinaryLogicalOp::apply(const ImgBase *poSrc1,const ImgBase *poSrc2, ImgBase **ppoDst){
      // {{{ open
      ICLASSERT_RETURN( poSrc1 );
      ICLASSERT_RETURN( poSrc2 );
      ICLASSERT_RETURN( ppoDst );
      ICLASSERT_RETURN( poSrc1 != *ppoDst && poSrc2 != *ppoDst );
      if(!BinaryOp::check(poSrc1,poSrc2)){
        ERROR_LOG("souce images are incompatible (aborting)");
        return;
      }
      if(!BinaryOp::prepare(ppoDst,poSrc1)){
        ERROR_LOG("unable to prepare destination image (aborting)");
        return;
      }

      switch(m_eOpType){
        case andOp:  apply_op<andOp>(poSrc1,poSrc2,*ppoDst); break;
        case orOp:  apply_op<orOp>(poSrc1,poSrc2,*ppoDst); break;
        case xorOp:  apply_op<xorOp>(poSrc1,poSrc2,*ppoDst); break;
      }
    }
    // }}}

  } // namespace filter
}
