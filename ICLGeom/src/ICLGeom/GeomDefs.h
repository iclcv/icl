// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>
#include <ICLMath/HomogeneousMath.h>
#include <vector>
#include <ICLCore/Color.h>
#include <ICLUtils/Point32f.h>

namespace icl::geom {
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

  /// Matrix Typedef of float matrices
  using Mat4D32f = math::FixedMatrix<icl32f,4,4>;

  /// Matrix Typedef of double matrices
  using Mat4D64f = math::FixedMatrix<icl64f,4,4>;

  /// Vector typedef of float vectors
  using Vec4D32f = math::FixedColVector<icl32f,4>;

  /// Vector typedef of double vectors
  using Vec4D64f = math::FixedColVector<icl64f,4>;

  /// Short typedef for 4D float vectors
  using Vec = Vec4D32f;

  /// Short typedef for 4D float matrices
  using Mat = Mat4D32f;

  /// typedef for vector of Vec instances
  using VecArray = std::vector<Vec>;

  } // namespace icl::geom