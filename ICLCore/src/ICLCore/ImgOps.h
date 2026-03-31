/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgOps.h                           **
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

#pragma once

#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace core {

    /// Singleton that owns BackendSelectors for Img utility operations
    /// (mirror, min, max, lut, etc.). Uses ImgBase* as dispatch context
    /// so Img<T> methods can dispatch directly via `this`.
    ///
    /// Backends self-register from _Cpp.cpp / _Ipp.cpp / _Mkl.cpp files.
    class ICLCore_API ImgOps : public ImgBaseBackendDispatching {
    public:
      /// Operation keys — values must match addSelector() insertion order.
      enum class Op : int {
        mirror, clearChannelROI, lut, getMax, getMin, getMinMax, normalize, flippedCopy,
        channelMean, replicateBorder, planarToInterleaved, interleavedToPlanar
      };

      // ---- Dispatch signatures (ImgBase& + operation args) ----
      using MirrorSig = void(ImgBase&, axis, bool roiOnly);
      using ClearChannelROISig = void(ImgBase&, int ch, icl64f val,
                                      const utils::Point& offs, const utils::Size& size);
      using LutSig = void(ImgBase& src, const void* lut, ImgBase& dst, int bits);
      using GetMaxSig = icl64f(ImgBase&, int ch, utils::Point* coords);
      using GetMinSig = icl64f(ImgBase&, int ch, utils::Point* coords);
      using GetMinMaxSig = void(ImgBase&, int ch,
                                icl64f* minVal, icl64f* maxVal,
                                utils::Point* minCoords, utils::Point* maxCoords);
      using NormalizeSig = void(ImgBase&, int ch,
                                icl64f srcMin, icl64f srcMax,
                                icl64f dstMin, icl64f dstMax);
      using FlippedCopySig = void(axis, ImgBase& src, int srcC,
                                  const utils::Point& srcOffs, const utils::Size& srcSize,
                                  ImgBase& dst, int dstC,
                                  const utils::Point& dstOffs, const utils::Size& dstSize);
      using ChannelMeanSig = icl64f(ImgBase&, int channel, bool roiOnly);
      using ReplicateBorderSig = void(ImgBase&);
      using PlanarToInterleavedSig = void(ImgBase& src, void* dst, int dstLineStep);
      using InterleavedToPlanarSig = void(const void* src, ImgBase& dst, int srcLineStep);

      /// Access the singleton instance (lazy-init, thread-safe)
      static ImgOps& instance();

    private:
      ImgOps();
    };

    /// ADL-visible toString for ImgOps::Op → registry name (defined in ImgOps.cpp)
    ICLCore_API const char* toString(ImgOps::Op op);

  } // namespace core
} // namespace icl
