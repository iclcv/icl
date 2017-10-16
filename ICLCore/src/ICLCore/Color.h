/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Color.h                            **
** Module : ICLCore                                                **
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
#include <ICLUtils/BasicTypes.h>
#include <ICLMath/FixedVector.h>
#include <string>

namespace icl{
  namespace core{

    /// Default color type of the ICL
    typedef math::FixedColVector<icl8u,3> Color;

    /// RGB Color
    typedef math::FixedColVector<icl8u,3> RGB;

    /// RGBA Color
    typedef math::FixedColVector<icl8u,4> RGBA;

    /// Special color type for float valued colors
    typedef math::FixedColVector<icl32f,3> Color32f;


    /// Special color type for e.g. rgba color information
    typedef math::FixedColVector<icl8u,4> Color4D;

    /// Special color type for e.g. rgba color information (float)
    typedef math::FixedColVector<icl32f,4> Color4D32f;

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
