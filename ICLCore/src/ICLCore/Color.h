// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLMath/FixedVector.h>
#include <string>

namespace icl{
  namespace core{

    /// Default color type of the ICL
    using Color = math::FixedColVector<icl8u,3>;

    /// RGB Color
    using RGB = math::FixedColVector<icl8u,3>;

    /// RGBA Color
    using RGBA = math::FixedColVector<icl8u,4>;

    /// Special color type for float valued colors
    using Color32f = math::FixedColVector<icl32f,3>;


    /// Special color type for e.g. rgba color information
    using Color4D = math::FixedColVector<icl8u,4>;

    /// Special color type for e.g. rgba color information (float)
    using Color4D32f = math::FixedColVector<icl32f,4>;

    // Create a color by given name (see GeneralColor Constructor)
    const Color &iclCreateColor(std::string name);

    /// Creates a (by default 20 percent) darker color
    inline Color darker(const Color &c, double factor=0.8){
      return c*factor;
    }

    /// Creates a (by default 20 percent) lighter color
    inline Color lighter(const Color &c,double factor=0.8){
      return c/factor;
    }

    /// Parses a color string representation into a color structur
    /** If an error occurs, a warning is shown and black color is returned
        first checks for some default color names:
        - black
        - white
        - red
        - green
        - blue
        - cyan
        - magenta
        - yellow
        - gray50
        - gray100
        - gray150
        - gray200
    */
    Color ICLCore_API color_from_string(const std::string &name);

  } // namespace core
}
