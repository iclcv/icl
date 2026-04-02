// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

namespace icl::geom {
  enum PrimitiveType{
    vertexPrimitive   = 1<<0, //<! vertex
    linePrimitive     = 1<<1, //<! line primitive (adressing two vertices -> start and end position of the line)
    trianglePrimitive = 1<<2, //<! triange primitive (adressing three vertices)
    quadPrimitive     = 1<<3, //<! quad primitve (adressing four vertices)
    polygonPrimitive  = 1<<4, //<! polygon primitive (adressing at least 3 vertices)
    texturePrimitive  = 1<<5, //<! texture primitive (using 4 vertices like a quad as textured rectangle)
    textPrimitive     = 1<<6, //<! text primitive (internally implmented as texture or as billboard)
    noPrimitive       = 1<<7, //<! internally used type
    PRIMITIVE_TYPE_COUNT = 8  //<! also for internal use only
  };
  } // namespace icl::geom