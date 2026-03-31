/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgOps.cpp                         **
** Module : ICLCore                                                **
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
    }

  } // namespace core
} // namespace icl
