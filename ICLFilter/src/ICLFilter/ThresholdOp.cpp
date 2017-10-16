/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ThresholdOp.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
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

#include <ICLFilter/ThresholdOp.h>
#include <ICLCore/Img.h>

#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    ThresholdOp::ThresholdOp(optype ttype,float lowThreshold, float highThreshold,float lowVal, float highVal){
      m_eType=ttype;
      m_fLowThreshold=lowThreshold;
      m_fHighThreshold=highThreshold;
      m_fLowVal=lowVal;
      m_fHighVal=highVal;
    }
    ThresholdOp::~ThresholdOp(){
      }
      void ThresholdOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
        switch (m_eType){
          case lt:tlt(poSrc,ppoDst,m_fLowThreshold);break;
          case gt:tgt(poSrc,ppoDst,m_fHighThreshold);break;
          case ltgt:tltgt(poSrc,ppoDst,m_fLowThreshold,m_fHighThreshold);break;
          case ltVal:tltVal(poSrc,ppoDst,m_fLowThreshold,m_fLowVal);break;
          case gtVal:tgtVal(poSrc,ppoDst,m_fHighThreshold,m_fHighVal);break;
          case ltgtVal:tltgtVal(poSrc,ppoDst,m_fLowThreshold,m_fLowVal,m_fHighThreshold,m_fHighVal);break;
        }
      }

     // {{{ C++ fallback ThreshOp classes

     template <typename T> class ThreshOpLTVal {
        // {{{ open

     public:
        ThreshOpLTVal (T t, T v) : threshold(t), value(v) {}
        inline T operator()(T val) const {
           if (val < threshold) return value;
           return val;
        }
     private:
        T threshold;
        T value;
     };

     // }}}
     template <typename T> class ThreshOpGTVal {
        // {{{ open

     public:
        ThreshOpGTVal (T t, T v) : threshold(t), value(v) {}
        inline T operator()(T val) const {
           if (val > threshold) return value;
           return val;
        }
     private:
        T threshold;
        T value;
     };

     // }}}
     template <typename T> class ThreshOpLTGTVal {
        // {{{ open
     public:
        ThreshOpLTGTVal(T tLow, T vLow, T tUp, T vUp) :
           tLow(tLow), tUp(tUp), vLow(vLow), vUp(vUp) {}
        inline T operator()(T val) const {
           if (val < tLow) return vLow;
           if (val > tUp)  return vUp;
           return val;
        }
     private:
        T tLow, tUp;
        T vLow, vUp;
     };

     // }}}

     // }}}

     // {{{ C++ fallback threshold function for all threshold operations

     template <typename T, class ThresholdOp>
     inline void fallbackThreshold(const Img<T> *src, Img<T> *dst,
                            const ThresholdOp &threshold) {
        ICLASSERT_RETURN( src && dst );
        ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
        ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

        for(int c=src->getChannels()-1; c >= 0; --c) {
           const ImgIterator<T> itSrc = src->beginROI(c);
           const ImgIterator<T> itSrcEnd = src->endROI(c);
           ImgIterator<T> itDst = dst->beginROI(c);
           for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
              *itDst = threshold(*itSrc);
           }
        }
     }

     // }}}

  #define ICL_INSTANTIATE_ALL_IPP_DEPTHS \
    ICL_INSTANTIATE_DEPTH(8u)  \
    ICL_INSTANTIATE_DEPTH(16s) \
    ICL_INSTANTIATE_DEPTH(32f)

  #define ICL_INSTANTIATE_ALL_FB_DEPTHS \
    ICL_INSTANTIATE_DEPTH(32s)  \
    ICL_INSTANTIATE_DEPTH(64f)



  #ifdef ICL_HAVE_IPP
     // {{{ ippi-function call templates

     template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, T)>
     inline void ippiThresholdCall_1T(const Img<T> *src, Img<T> *dst, T t){
        // {{{ open

        ICLASSERT_RETURN( src && dst );
        ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
        ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

        for (int c=src->getChannels()-1; c >= 0; --c) {
           ippiFunc (src->getROIData (c), src->getLineStep(),
                     dst->getROIData (c), dst->getLineStep(),
                     dst->getROISize(), t);
        }
     }

     // }}}

     template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, T, T)>
     inline void ippiThresholdCall_2T(const Img<T> *src, Img<T> *dst, T t1, T t2){
        // {{{ open

        ICLASSERT_RETURN( src && dst );
        ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
        ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

        for (int c=src->getChannels()-1; c >= 0; --c) {
           ippiFunc (src->getROIData (c), src->getLineStep(),
                     dst->getROIData (c), dst->getLineStep(),
                     dst->getROISize(), t1, t2);
        }
     }

     // }}}

     template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
     inline void ippiThresholdCall_4T(const Img<T> *src, Img<T> *dst, T t1,T t2, T t3, T t4){
        // {{{ open

        ICLASSERT_RETURN( src && dst );
        ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
        ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );

        for (int c=src->getChannels()-1; c >= 0; --c) {
           ippiFunc (src->getROIData (c), src->getLineStep(),
                     dst->getROIData (c), dst->getLineStep(),
                     dst->getROISize(), t1,t2,t3,t4);
        }
     }

     // }}}

     // }}}

     // {{{ function specializations without Val postfix

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tlt(const Img ## T *src,Img ## T *dst, icl ## T t){\
      ippiThresholdCall_1T<icl ## T, ippiThreshold_LT_ ## T ## _C1R> (src, dst, t);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tgt(const Img ## T *src,Img ## T *dst, icl ## T t){\
      ippiThresholdCall_1T<icl ## T, ippiThreshold_GT_ ## T ## _C1R> (src, dst, t);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltgt(const Img ## T *src,Img ## T *dst, icl ## T tMin, icl ## T tMax){\
        ippiThresholdCall_4T<icl ## T, ippiThreshold_LTValGTVal_ ## T ## _C1R> (src, dst, tMin,tMin, tMax,tMax);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

     // }}}

     // {{{ function specializations with Val postfix

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltVal(const Img ## T *src,Img ## T *dst, icl ## T t, icl ## T val){\
      ippiThresholdCall_2T<icl ## T , ippiThreshold_LTVal_ ## T ## _C1R> (src, dst, t, val);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tgtVal(const Img ## T *src,Img ## T *dst, icl ## T t, icl ## T val){\
      ippiThresholdCall_2T<icl ## T , ippiThreshold_GTVal_ ## T ## _C1R> (src, dst, t, val);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltgtVal(const Img ## T *src,Img ## T *dst, icl ## T tMin,icl ## T minVal, icl ## T tMax,icl ## T maxVal){\
        ippiThresholdCall_4T<icl ## T, ippiThreshold_LTValGTVal_ ## T ##_C1R> (src, dst, tMin, minVal, tMax, maxVal);}

    ICL_INSTANTIATE_ALL_IPP_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

     // }}}
  #endif

     // {{{ function specializations without Val postfix (fallback)
     /* We just use the appropriate *Val functions, because there is no performance
        gain implementing a specialised variant */


  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tlt(const Img ## T *src, Img ## T *dst, icl ## T t){\
      ThresholdOp::tltVal(src, dst, t, t);}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tgt(const Img ## T *src, Img ## T *dst, icl ## T t){\
      ThresholdOp::tgtVal(src, dst, t, t);}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltgt(const Img ## T *src, Img ## T *dst, icl ## T tMin, icl ## T tMax){\
      ThresholdOp::tltgtVal(src, dst, tMin,tMin, tMax,tMax);}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

     // }}}

     // {{{ function specializations with Val postfix (fallback)

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltVal(const Img ##T *src, Img ## T *dst, icl ## T t, icl ## T val){\
      fallbackThreshold (src, dst, ThreshOpLTVal<icl ## T>(t,val));}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tgtVal(const Img ##T *src, Img ## T *dst, icl ## T t, icl ## T val){\
      fallbackThreshold (src, dst, ThreshOpGTVal<icl ## T>(t,val));}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

  #define ICL_INSTANTIATE_DEPTH(T) \
    void ThresholdOp::tltgtVal(const Img ## T *src, Img ## T *dst, icl ## T tMin, icl ## T minVal, icl ##T tMax, icl ## T maxVal){\
        fallbackThreshold (src, dst, ThreshOpLTGTVal<icl ## T>(tMin,minVal,tMax,maxVal));}
  #ifdef ICL_HAVE_IPP
    ICL_INSTANTIATE_ALL_FB_DEPTHS
  #else
    ICL_INSTANTIATE_ALL_DEPTHS
  #endif
  #undef ICL_INSTANTIATE_DEPTH

     // }}}

  #undef ICL_INSTANTIATE_ALL_IPP_DEPTHS

  #undef ICL_INSTANTIATE_ALL_FB_DEPTHS


     // {{{ ImgBase* versions

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tlt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(t)); break;
    void ThresholdOp::tlt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
    // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tgt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(t)); break;
    void ThresholdOp::tgt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
    // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tltgt(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(tMin), clipped_cast<icl32f,icl ## T>(tMax)); break;
    void ThresholdOp::tltgt(const ImgBase *poSrc, ImgBase **ppoDst, icl32f tMin, icl32f tMax)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
    // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tltVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(t), clipped_cast<icl32f,icl ## T>(val)); break;
    void ThresholdOp::tltVal(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t, icl32f val)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
    // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tgtVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(t), clipped_cast<icl32f,icl ## T>(val)); break;
    void ThresholdOp::tgtVal(const ImgBase *poSrc, ImgBase **ppoDst, icl32f t, icl32f val)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
    // }}}

  #define ICL_INSTANTIATE_DEPTH(T) \
      case depth ## T: tltgtVal(poSrc->asImg<icl ## T>(), (*ppoDst)->asImg<icl ## T>(), clipped_cast<icl32f,icl ## T>(tMin),\
                  clipped_cast<icl32f,icl ##T>(minVal), clipped_cast<icl32f,icl ## T>(tMax), clipped_cast<icl32f,icl ## T>(maxVal));break;
    void ThresholdOp::tltgtVal(const ImgBase *poSrc, ImgBase **ppoDst,
                             icl32f tMin, icl32f minVal, icl32f tMax, icl32f maxVal)
      // {{{ open
    {
      if (!UnaryOp::prepare (ppoDst, poSrc)) return;
      switch (poSrc->getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS
        default: ICL_INVALID_DEPTH; break;
      }
    }
  #undef ICL_INSTANTIATE_DEPTH
  // }}}

  } // namespace filter
} // namespace icl
