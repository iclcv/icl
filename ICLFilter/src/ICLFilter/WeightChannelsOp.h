/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WeightChannelsOp.h             **
** Module : ICLFilter                                              **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{

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
      void apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst);

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

  } // namespace filter
} // namespace icl

