/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WienerOp.h                     **
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

#pragma once

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace filter{
    /// Class for Wiener Filter (IPP only) \ingroup UNARY \ingroup NBH
    /** @see WienerOp.h for full Wiener filter documentation */
    class WienerOp : public NeighborhoodOp, public core::ImageBackendDispatching {
     public:

      /// Backend selector keys
      enum class Op : int { apply };

      /// Dispatch signature: src, dst, maskSize, anchor, roiOffset, noise
      using WienerSig = void(const core::Image&, core::Image&,
                              const utils::Size&, const utils::Point&,
                              const utils::Point&, icl32f);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      /// Constructor
      WienerOp (const utils::Size &maskSize, icl32f noise=0);

      /// Filters an image using the Wiener algorithm.
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import unaryOps apply function without destination image
      using NeighborhoodOp::apply;

      icl32f getNoise() const { return m_fNoise; }
      void setNoise(icl32f noise) { m_fNoise = noise; }

      private:
      icl32f m_fNoise;
    };

    /// ADL-visible toString for WienerOp::Op (defined in WienerOp.cpp)
    ICLFilter_API const char* toString(WienerOp::Op op);

  } // namespace filter
} // namespace icl
