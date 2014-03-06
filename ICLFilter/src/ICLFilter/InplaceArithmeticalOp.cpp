/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/InplaceArithmeticalOp.cpp      **
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

#include <ICLFilter/InplaceArithmeticalOp.h>
#include <ICLCore/Img.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
    
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
#ifdef ICL_SYSTEM_WINDOWS
      SPECIALIZE(abs, ::fabs((double)t));
#else
      SPECIALIZE(abs, ::fabs(t));
#endif
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
    
  } // namespace filter
}
