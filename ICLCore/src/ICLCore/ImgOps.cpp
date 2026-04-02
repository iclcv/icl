// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImgOps.h>

namespace icl {
  namespace core {

    ImgOps& ImgOps::instance() {
      static ImgOps ops;
      return ops;
    }

    const char* toString(ImgOps::Op op) {
      switch(op) {
        case ImgOps::Op::mirror:           return "mirror";
        case ImgOps::Op::clearChannelROI:  return "clearChannelROI";
        case ImgOps::Op::lut:              return "lut";
        case ImgOps::Op::getMax:           return "getMax";
        case ImgOps::Op::getMin:           return "getMin";
        case ImgOps::Op::getMinMax:        return "getMinMax";
        case ImgOps::Op::normalize:        return "normalize";
        case ImgOps::Op::flippedCopy:      return "flippedCopy";
        case ImgOps::Op::channelMean:      return "channelMean";
        case ImgOps::Op::replicateBorder:  return "replicateBorder";
        case ImgOps::Op::planarToInterleaved: return "planarToInterleaved";
        case ImgOps::Op::interleavedToPlanar: return "interleavedToPlanar";
        case ImgOps::Op::scaledCopy:          return "scaledCopy";
      }
      return "?";
    }

    ImgOps::ImgOps() {

      // Enum values must match insertion order (asserted at runtime).
      // Registry names derived from toString(Op) via ADL.
      addSelector<MirrorSig>(Op::mirror);
      addSelector<ClearChannelROISig>(Op::clearChannelROI);
      addSelector<LutSig>(Op::lut);
      addSelector<GetMaxSig>(Op::getMax);
      addSelector<GetMinSig>(Op::getMin);
      addSelector<GetMinMaxSig>(Op::getMinMax);
      addSelector<NormalizeSig>(Op::normalize);
      addSelector<FlippedCopySig>(Op::flippedCopy);
      addSelector<ChannelMeanSig>(Op::channelMean);
      addSelector<ReplicateBorderSig>(Op::replicateBorder);
      addSelector<PlanarToInterleavedSig>(Op::planarToInterleaved);
      addSelector<InterleavedToPlanarSig>(Op::interleavedToPlanar);
      addSelector<ScaledCopySig>(Op::scaledCopy);
    }

  } // namespace core
} // namespace icl
