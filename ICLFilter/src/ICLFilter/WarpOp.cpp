// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Sergius Gaulik

#include <ICLFilter/WarpOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(WarpOp::Op op) {
    switch(op) {
      case WarpOp::Op::warp: return "warp";
    }
    return "?";
  }

  core::ImageBackendDispatching& WarpOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<WarpSig>(Op::warp);
      return true;
    }();
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

  } // namespace icl::filter