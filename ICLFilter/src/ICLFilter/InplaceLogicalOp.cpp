/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/InplaceLogicalOp.cpp           **
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

#include <ICLFilter/InplaceLogicalOp.h>
#include <ICLCore/Img.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

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
      SPECIALIZE(xor,(!!val ^ !!t)*TRUE_VAL); // using !! to convert v into boolean
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
            src->forEach(Apply<T,InplaceLogicalOp::X##Op>(clipped_cast<icl64f,T>(val))); \
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
          src->forEach(Apply<T,InplaceLogicalOp::X##Op>(clipped_cast<icl64f,T>(val))); \
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

  } // namespace filter
}
