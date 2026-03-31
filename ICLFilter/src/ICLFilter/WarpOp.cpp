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

    const char* toString(WarpOp::Op op) {
      switch(op) {
        case WarpOp::Op::warp: return "warp";
      }
      return "?";
    }

    core::ImageBackendDispatching& WarpOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<WarpSig>(Op::warp);
        return true;
      }();
      (void)init;
      return proto;
    }

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
      ImageBackendDispatching(prototype()),
      m_allowWarpMapScaling(allowWarpMapScaling), m_scaleMode(mode) {
      warpMap.deepCopy(&m_warpMap);
      prepare_warp_table_inplace(m_warpMap);
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

      auto* impl = getSelector<WarpSig>(Op::warp).resolve(src);
      if(!impl) {
        ERROR_LOG("no applicable backend for WarpOp");
        return;
      }
      impl->apply(src, dst, cwm, warpOffset, m_scaleMode);
    }

  } // namespace filter
}
