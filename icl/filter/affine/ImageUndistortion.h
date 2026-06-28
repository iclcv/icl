// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Img.h>

namespace icl::filter {
  class ICLFilter_API ImageUndistortion{
    public:
    struct Impl; //!< internal impl

    private:
    Impl *impl;  //!< internal impl pointer


    public:
    /// creates a null instance
    ImageUndistortion();

    /// creates an Undistortion instance given parameters
    /** @param model distortion mode possible values are MatlabModel5Params and SimpleARTBased
        @param params parameters for the given model
          (MatlabModel5Params needs 10 parameters: fx, fy, ix, iy, skew, k1, k2, k3, k4, k5;
          SimpleARTBased needs 4 parameters: x, y, f, scale)
        @param imageSize underlying image size */
    ImageUndistortion(const std::string &model, const std::vector<double> &params,
                      const utils::Size &imageSize);

    /// copy constructor
    ImageUndistortion(const ImageUndistortion &other);

    /// assignment operator
    ImageUndistortion &operator=(const ImageUndistortion &other);

    /// loads ImageUndistortion from file using the istream operator
    ImageUndistortion(const std::string &filename);

    /// returns current image size
    const utils::Size &getImageSize() const;
    const std::vector<double> &getParams() const;
    const std::string &getModel() const;
    const utils::Point32f operator()(const utils::Point32f &distortedPos) const;
    void setParams(const std::vector<double> &params);

    /// Warp map that RECTIFIES (undistorts) a distorted image when used with a
    /// filter::WarpOp: dst(p) = src(model(p)). Cached; rebuilt on a parameter
    /// change.
    /// @param autoScale when true, the sampled source coordinates are uniformly
    ///   scaled about the distortion centre by the largest factor that keeps the
    ///   WHOLE output frame within source bounds — i.e. no out-of-bounds samples
    ///   (no black border) while filling the frame maximally (alpha=0 cropping).
    const core::Img32f &createWarpMap(bool autoScale = false) const;

    /// Inverse of createWarpMap(): warp map that DISTORTS an ideal image into a
    /// lens-distorted one (dst(p) = src(model^{-1}(p))) — the "distortion part".
    /// Built by fixed-point inversion of the model (assumes a near-identity /
    /// small-distortion mapping). Cached separately; rebuilt on a parameter change.
    /// @param autoScale see createWarpMap().
    const core::Img32f &createInverseWarpMap(bool autoScale = false) const;

    inline bool isNull() const { return !impl; }
  };

  /// overloaded ostream operator for ImageUndistortion instances
  ICLFilter_API std::istream &operator>>(std::istream &is, ImageUndistortion &udist);

  /// overloaded istream operator for ImageUndistortion instances
  ICLFilter_API std::ostream &operator<<(std::ostream &s, const ImageUndistortion &udist);
  } // namespace icl::filter