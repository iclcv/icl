// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Image.h>

namespace icl::filter {
  /// Class for creating a color distance map
  class ICLFilter_API ColorDistanceOp : public UnaryOp{
    public:

    /// Dummy constructor (creates an uninitialized instance)
    ColorDistanceOp(){}

    /// create a new instance with given reference color and optional threshold
    /** If the given threshold is not -1, the resulting distance values are
        thresholded (so that pixels, that are closer than threshold to the
        referenceColor are set to 255. The reference color needs at least 3 entries.*/
    template<class Container>
    ColorDistanceOp(const Container &vec, double threshold=-1){
      init(vec,threshold);
    }

    /// create with given iterator range for the reference color initialization
    template<class ForewardIterator>
    ColorDistanceOp(ForewardIterator begin, ForewardIterator end, double threshold=-1){
      init(std::vector<double>(begin,end), threshold);
    }

    /// creates a color distance map
    /** If the current threshold is not -1, then the result image becomes
        a binary 8-image. Otherwise, the resulting image is adapted to an Img32f.
        Only if the source depth is depth64f, and Img64f is also used for the result.

        The source image is assumed to have 3 channels
    */
    void apply(const core::Image &src, core::Image &dst) override;

    /// Import unaryOps apply function without destination image
    using UnaryOp::apply;

    /// sets the threshold value
    inline void setThreshold(double threshold){
      m_threshold = threshold;
    }

    /// sets the current reference color
    inline void setReferenceColor(const std::vector<double> &refColor){
      init(refColor,m_threshold);
    }

    /// sets both reference color and threshold at once
    template<class Container>
    inline void init(const Container &refColor, double threshold=-1){
      m_refColor.assign(refColor.begin(),refColor.end());
      m_threshold = threshold;
      ICLASSERT_THROW(m_refColor.size() >= 3,
                      utils::ICLException("ColorDistanceOp::setReferenceColor: ref color needs at least 3 entries"));
    }

    private:
    /// internal reference color
    std::vector<double> m_refColor;

    /// internal threshold
    double m_threshold;
  };

  } // namespace icl::filter