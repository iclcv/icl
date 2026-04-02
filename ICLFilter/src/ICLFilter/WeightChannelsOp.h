// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>

namespace icl::filter {
    /// Weight pixel values of all image channels \ingroup UNARY
    /** Pixels of all channels in source image are weighted
        by a channel-wise weight:
        \f[
        D(x,y,c) = S(x,y,c)*w(c)
        \f]
        where \f$D\f$ is the destination image, \f$S\f$ is the source
        image, \f$w\f$ is the weight vector and \f$C\f$ is the source
        images channel count.

        Performance: 1000x1000 image with 10 channels averaged over 100 runs (1,83 GHz Core Duo)
        - icl8u: 34ms
        - icl32f: 37ms
        - icl64f: 69ms

        Performance: 1000x1000 image with 3 channels averaged over 100 runs (1,83 GHz Core Duo)
        - icl8u: 10.9ms
        - icl32f: 10.9ms
        - icl64f: 21ms
        **/
    class ICLFilter_API WeightChannelsOp : public UnaryOp {
      public:
      /// creates a new WeightChannelsOp object
      WeightChannelsOp(){}

      /// creates an new WeightChannelsOp object  with a given weights vector
      /** @param weights channel weights vector
          **/
      WeightChannelsOp(const std::vector<icl64f> &weights):
      m_vecWeights(weights){}

      /// applies this operation on the source image
      /** @param poSrc source image
          @param ppoDst destination image (adapted to icl32f by default,
          if the source image has depth64f, ppoDst is adapted to
          icl64f too.

      **/
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;


      /// returns the current weight vector
      /** @return reference to the current weight vector **/
      const std::vector<icl64f> &getWeights() const { return m_vecWeights; }

      /// sets up the current weights vector
      /** @param weights new weight vector **/
      void setWeights(const std::vector<icl64f> &weights) {
        m_vecWeights = weights; }

      private:
      /// internal storage for the channel weights
      std::vector<icl64f> m_vecWeights;
    };

  } // namespace icl::filter