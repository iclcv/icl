/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/IntegralImgOp.h                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <vector>

namespace icl{
  namespace filter{
    /// class for creating integral images \ingroup UNARY
    /**
    The integral image A of an image a (introduced by Viola & Jones (2001) in the
    paper : "Rapid Object Detection using a Boosted Cascade of Simple Features" )
    defined as follows:
    \f[
    A(i,j) = \sum\limits_{x=0}^i \sum\limits_{y=0}^j  a(i,j)
    \f]
    It can be calculated incrementally using the following equation:
    \f[
    A(i,j) = a(i,j)+A(i-1,j)+A(i,j-1)-A(i-1,j-1)
    \f]
    <h3>illustration:</h3>
    <pre>
      +++++..
      +++CA..
      +++BX..
      .......
      X = src(x) + A + B - C
    </pre>

    <h1>Implementation</h1>
    <h3>Other definitions</h3>
    There are alternative definitions (like the ipp):
     \f[
    A(i,j) = \sum\limits_{x=0}^{i-1} \sum\limits_{y=0}^{j-1}  a(i,j)
    \f]
    From this it follows, that the first integral image row and colum is filled with
    zeros and the internal image grows by one pixel in each dimension, with the benefit
    of some better usability of the result image. We use the above definition, which
    is a bit more convenient for the most common applications in our sight.

    <h1>IPP Optimization</h1>
    IPP optimization is not used, as IPP uses a different formulation of the integral image
    which leads to an integral image that is one row and one column larger as the source image.
    IPPs integral image formulation is
    \f[
    A(i,j) = \sum\limits_{x=0}^{i-1} \sum\limits_{y=0}^{j-1}  a(i,j)
    \f].
    As our implementation comes really close to the IPP-version in terms of performance,
    we dont use the IPP for this UnaryOp.

    <h1>Supported Type-Combinations</h1>
    We support all source image depth, the integral image always needs a large value domain,
    so here, only icl32s, icl32f and icl64f are supported.

    <h1>Performance</h1>
    The calculation is very fast and we come close to the IPP performace.
    - Benchmark plattworm: Intel Core2Duo with 2GHz, 2GB Ram (single core-performance)
    - Test Image: 1000x1000, 1-channel, icl8u (uchar)
    - Destination depth 32s (int)
    - Compiler: g++ 4.3
    - Compilation flags: -O4 -march=native
    - Times
        - our implementation: 2.3ms
        - Intel IPP 1.8ms (but different integral image, and not supported)

    */
    class ICLFilter_API IntegralImgOp : public UnaryOp{
      public:

      /// Constructor
      /** @param integralImageDepth the depth of the integralImage (depth8u etc)
      */
      IntegralImgOp(core::depth integralImageDepth=core::depth32s);

      /// Destructor
      ~IntegralImgOp();

      /// sets the depth of the integralImage (depth8u etc)
      /** @param integralImageDepth the depth of the integralImage (depth8u etc)
      */
      void setIntegralImageDepth(core::depth integralImageDepth);

      /// returns the depth of the integralImage
      /** @return the depth of the integralImage
      */
      core::depth getIntegralImageDepth() const;

      /// applies the integralimage Operaor
      /** @param src The source image
        @param dst Pointer to the destination image
      */
      void apply(const core::ImgBase *src, core::ImgBase **dst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      private:
      core::depth m_integralImageDepth; //!< destination depth
      core::ImgBase *m_buf; //!< used only if IPP is available
    };
  } // namespace filter
}

