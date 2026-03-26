/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WienerOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLFilter/WienerOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

#ifdef ICL_HAVE_IPP

    template<class T> struct WienerImpl {
      static void apply(const Img<T> &, Img<T> &,
                        const Size &, const Point &, const Point &,
                        std::vector<icl8u> &, icl32f) {
        throw ICLException("WienerOp: unsupported depth");
      }
    };

    template<>
    struct WienerImpl<icl8u> {
      static void apply(const Img8u &src, Img8u &dst,
                        const Size &maskSize, const Point &anchor,
                        const Point &roiOffset,
                        std::vector<icl8u> &buf, icl32f noise) {
        int bufSize;
        ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
        buf.reserve(bufSize);
        for(int c = src.getChannels()-1; c >= 0; --c) {
          ippiFilterWiener_8u_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                  dst.getROIData(c), dst.getLineStep(),
                                  dst.getROISize(), maskSize, anchor,
                                  &noise, buf.data());
        }
      }
    };

    template<>
    struct WienerImpl<icl16s> {
      static void apply(const Img16s &src, Img16s &dst,
                        const Size &maskSize, const Point &anchor,
                        const Point &roiOffset,
                        std::vector<icl8u> &buf, icl32f noise) {
        int bufSize;
        ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
        buf.reserve(bufSize);
        for(int c = src.getChannels()-1; c >= 0; --c) {
          ippiFilterWiener_16s_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                   dst.getROIData(c), dst.getLineStep(),
                                   dst.getROISize(), maskSize, anchor,
                                   &noise, buf.data());
        }
      }
    };

    template<>
    struct WienerImpl<icl32f> {
      static void apply(const Img32f &src, Img32f &dst,
                        const Size &maskSize, const Point &anchor,
                        const Point &roiOffset,
                        std::vector<icl8u> &buf, icl32f noise) {
        int bufSize;
        ippiFilterWienerGetBufferSize(dst.getROISize(), maskSize, 1, &bufSize);
        buf.reserve(bufSize);
        for(int c = src.getChannels()-1; c >= 0; --c) {
          ippiFilterWiener_32f_C1R(src.getROIData(c, roiOffset), src.getLineStep(),
                                   dst.getROIData(c), dst.getLineStep(),
                                   dst.getROISize(), maskSize, anchor,
                                   &noise, buf.data());
        }
      }
    };

    void WienerOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;
      src.visit([&](const auto &s) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        auto &d = dst.as<T>();
        WienerImpl<T>::apply(s, d, getMaskSize(), getAnchor(), getROIOffset(),
                             m_vecBuffer, m_fNoise);
      });
    }

#else

    void WienerOp::apply(const Image &, Image &) {
      throw utils::ICLException("WienerOp requires Intel IPP");
    }

#endif

  } // namespace filter
}
