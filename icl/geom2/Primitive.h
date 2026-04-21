// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Color.h>
#include <vector>

namespace icl::geom2 {

  using GeomColor = core::Color4D32f;

  /// Primitive type flags (bitmask)
  enum PrimitiveType {
    PrimVertex   = 1 << 0,
    PrimLine     = 1 << 1,
    PrimTriangle = 1 << 2,
    PrimQuad     = 1 << 3,
    PrimAll      = 0xFF
  };

  /// A line primitive referencing 2 vertex indices
  struct LinePrimitive {
    int a, b;
    GeomColor color{0.4f, 0.4f, 0.4f, 1.0f};
  };

  /// A triangle primitive referencing 3 vertex + optional normal + optional texcoord indices
  struct TrianglePrimitive {
    int v[3];
    int n[3] = {-1, -1, -1};
    int t[3] = {-1, -1, -1};
  };

  /// A quad primitive referencing 4 vertex + optional normal + optional texcoord indices
  struct QuadPrimitive {
    int v[4];
    int n[4] = {-1, -1, -1, -1};
    int t[4] = {-1, -1, -1, -1};
  };

} // namespace icl::geom2
