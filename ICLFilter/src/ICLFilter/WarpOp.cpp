/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WarpOp.cpp                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

#include <ICLFilter/WarpOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    namespace {
      template<class T>
      inline T interpolate_pixel_nn(float x, float y, const Channel<T> &src){
        if(x < 0) return T(0);
        return src(round(x),round(y));
      }

      template<class T>
      inline T interpolate_pixel_lin(float x, float y, const Channel<T> &src){
        if(x < 0) return T(0);
        float fX0 = x - floor(x), fX1 = 1.0f - fX0;
        float fY0 = y - floor(y), fY1 = 1.0f - fY0;
        int xll = static_cast<int>(x);
        int yll = static_cast<int>(y);

        const T* pLL = &src(xll,yll);
        float a = *pLL;        //  a b
        float b = *(++pLL);    //  c d
        pLL += src.getWidth();
        float d = *pLL;
        float c = *(--pLL);

        return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
      }

      // C++ fallback: iterates dst ROI, indexes warp map via warpOffset
      template<class T>
      struct WarpImpl {
        static void apply(const Img<T> &src, Img<T> &dst,
                          const Channel32f warpMap[2],
                          const Point &warpOffset,
                          scalemode mode) {
          const int wmW = warpMap[0].getWidth();
          const Rect dstROI = dst.getROI();
          auto interpolator = (mode == interpolateNN)
            ? interpolate_pixel_nn<T>
            : interpolate_pixel_lin<T>;

          for(int c = 0; c < src.getChannels(); ++c) {
            const Channel<T> s = src[c];
            Channel<T> d = dst[c];

            for(int y = dstROI.y; y < dstROI.y + dstROI.height; ++y) {
              const int wmy = y + warpOffset.y;
              const icl32f *wmXRow = warpMap[0].begin() + wmy * wmW + dstROI.x + warpOffset.x;
              const icl32f *wmYRow = warpMap[1].begin() + wmy * wmW + dstROI.x + warpOffset.x;
              T *dstRow = &d(dstROI.x, y);

              for(int i = 0; i < dstROI.width; ++i) {
                dstRow[i] = interpolator(wmXRow[i], wmYRow[i], s);
              }
            }
          }
        }
      };

#ifdef ICL_HAVE_IPP
      template<>
      struct WarpImpl<icl8u> {
        static void apply(const Img8u &src, Img8u &dst,
                          const Channel32f warpMap[2],
                          const Point &warpOffset,
                          scalemode mode) {
          const int wmW = warpMap[0].getWidth();
          const Rect dstROI = dst.getROI();
          const Size roiSize = dst.getROISize();

          for(int c = 0; c < src.getChannels(); ++c) {
            const icl32f *wmX = warpMap[0].begin()
              + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);
            const icl32f *wmY = warpMap[1].begin()
              + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);

            IppStatus s = ippiRemap_8u_C1R(
              src.begin(c), src.getSize(), src.getLineStep(), src.getImageRect(),
              wmX, sizeof(icl32f) * wmW,
              wmY, sizeof(icl32f) * wmW,
              dst.getROIData(c), dst.getLineStep(),
              roiSize, static_cast<int>(mode));

            if(s != ippStsNoErr) {
              ERROR_LOG("IPP-Error:" << ippGetStatusString(s));
              return;
            }
          }
        }
      };

      template<>
      struct WarpImpl<icl32f> {
        static void apply(const Img32f &src, Img32f &dst,
                          const Channel32f warpMap[2],
                          const Point &warpOffset,
                          scalemode mode) {
          const int wmW = warpMap[0].getWidth();
          const Rect dstROI = dst.getROI();
          const Size roiSize = dst.getROISize();

          for(int c = 0; c < src.getChannels(); ++c) {
            const icl32f *wmX = warpMap[0].begin()
              + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);
            const icl32f *wmY = warpMap[1].begin()
              + (dstROI.y + warpOffset.y) * wmW + (dstROI.x + warpOffset.x);

            IppStatus s = ippiRemap_32f_C1R(
              src.begin(c), src.getSize(), src.getLineStep(), src.getImageRect(),
              wmX, sizeof(icl32f) * wmW,
              wmY, sizeof(icl32f) * wmW,
              dst.getROIData(c), dst.getLineStep(),
              roiSize, static_cast<int>(mode));

            if(s != ippStsNoErr) {
              ERROR_LOG("IPP-Error:" << ippGetStatusString(s));
              return;
            }
          }
        }
      };
#endif

      void cpp_warp(const Image& src, Image& dst, const Channel32f* cwm,
                    Point warpOffset, scalemode mode) {
        src.visitWith(dst, [&](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          WarpImpl<T>::apply(s, d, cwm, warpOffset, mode);
        });
      }

    } // anon namespace

    void prepare_warp_table_inplace(Img32f &warpMap){
      const Rect r = warpMap.getImageRect();

      Channel32f cs[2];
      warpMap.extractChannels(cs);
      const Size size = warpMap.getSize();

      for(int x=0;x<size.width;++x){
        for(int y=0;y<size.height;++y){
          if(!r.contains(round(cs[0](x,y)),round(cs[1](x,y)))){
            cs[0](x,y) = cs[1](x,y) = -1;
          }
        }
      }
    }

    WarpOp::WarpOp(const Img32f &warpMap, scalemode mode, bool allowWarpMapScaling):
      m_allowWarpMapScaling(allowWarpMapScaling), m_scaleMode(mode) {
      warpMap.deepCopy(&m_warpMap);
      prepare_warp_table_inplace(m_warpMap);

      initDispatching("WarpOp");
      auto& sel = addSelector<WarpSig>("warp");
      sel.add(Backend::Cpp, cpp_warp, "C++ warp (IPP for 8u/32f where available)");
    }

    WarpOp::~WarpOp() {
    }

    void WarpOp::setScaleMode(scalemode scaleMode){
      m_scaleMode = scaleMode;
    }

    void WarpOp::setWarpMap(const Img32f &warpMap){
      warpMap.deepCopy(&m_warpMap);
      prepare_warp_table_inplace(m_warpMap);
      m_scaledWarpMap = Img32f();
    }

    void WarpOp::setAllowWarpMapScaling(bool allow){
      m_allowWarpMapScaling = allow;
    }

    void WarpOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(!src.isNull());
      ICLASSERT_RETURN(m_warpMap.getSize() != Size::null);

      // Remember src ROI before prepare (prepare may create a clipped dst)
      const Rect srcROI = src.getROI();

      if(!prepare(dst, src)) {
        ERROR_LOG("unable to prepare destination image");
        return;
      }

      // Select and prepare warp map (scale if sizes differ)
      Channel32f cwm[2];
      if(src.getSize() != m_warpMap.getSize()) {
        if(m_allowWarpMapScaling) {
          if(m_scaledWarpMap.getSize() != src.getSize()) {
            m_scaledWarpMap.setSize(src.getSize());
            m_warpMap.scaledCopy(&m_scaledWarpMap);
            prepare_warp_table_inplace(m_scaledWarpMap);
          }
          m_scaledWarpMap.extractChannels(cwm);
        } else {
          ERROR_LOG("warp map size and image size are not equal\n"
                    "warp map can be scaled using setAllowWarpMapScaling(true)");
          return;
        }
      } else {
        m_warpMap.extractChannels(cwm);
      }

      // Warp map offset: maps dst coordinates to warp map coordinates.
      // clip mode:     dst(0,0) → warp map at srcROI.origin → offset = srcROI.origin
      // non-clip mode: dst(roi.x,roi.y) → warp map at (roi.x,roi.y) → offset = (0,0)
      const Rect dstROI = dst.getROI();
      const Point warpOffset(srcROI.x - dstROI.x, srcROI.y - dstROI.y);

      auto* impl = getSelector<WarpSig>("warp").resolve(src);
      if(!impl) {
        ERROR_LOG("no applicable backend for WarpOp");
        return;
      }
      impl->apply(src, dst, cwm, warpOffset, m_scaleMode);
    }

  } // namespace filter
}
