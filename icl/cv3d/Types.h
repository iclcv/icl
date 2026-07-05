// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Types.h>
#include <icl/math/la/FixedMatrix.h>
#include <icl/math/la/FixedVector.h>
#include <icl/math/transform/HomogeneousMath.h>
#include <vector>
#include <icl/core/cc/Color.h>
#include <icl/utils/Point.h>

namespace icl::cv3d {
  /// color for geometry primitives
  using GeomColor = core::Color4D32f;

  /// inline utililty function to create a white color instance
  inline GeomColor geom_white(float alpha=255) { return GeomColor(255,255,255,alpha); }

  /// inline utililty function to create a red color instance
  inline GeomColor geom_red(float alpha=255) { return GeomColor(255,0,0,alpha); }

  /// inline utililty function to create a blue color instance
  inline GeomColor geom_blue(float alpha=255) { return GeomColor(0,100,255,alpha); }

  /// inline utililty function to create a green color instance
  inline GeomColor geom_green(float alpha=255) { return GeomColor(0,255,0,alpha); }

  /// inline utililty function to create a yellow color instance
  inline GeomColor geom_yellow(float alpha=255) { return GeomColor(255,255,0,alpha); }

  /// inline utililty function to create a magenta color instance
  inline GeomColor geom_magenta(float alpha=255) { return GeomColor(255,0,255,alpha); }

  /// inline utililty function to create a cyan color instance
  inline GeomColor geom_cyan(float alpha=255) { return GeomColor(0,255,255,alpha); }

  /// inline utililty function to create a cyan color instance
  inline GeomColor geom_black(float alpha=255) { return GeomColor(0,0,0,alpha); }

  /// inline utililty function to create an invisible color instance (alpha is 0.0f)
  inline GeomColor geom_invisible() { return GeomColor(0,0,0,0); }

  // Homogeneous vector/matrix types — single-sourced from icl::math (see
  // icl/math/Types.h). These names are kept for backward compatibility.
  using Mat4D32f = math::Mat4;   //!< 4x4 float matrix
  using Mat4D64f = math::Mat4d;  //!< 4x4 double matrix
  using Vec4D32f = math::Vec4;   //!< 4D float vector
  using Vec4D64f = math::Vec4d;  //!< 4D double vector

  /// Short typedef for 4D float vectors
  using Vec = math::Vec4;

  /// Short typedef for 4D float matrices
  using Mat = math::Mat4;

  /// typedef for vector of Vec instances
  using VecArray = std::vector<Vec>;

  } // namespace icl::cv3d