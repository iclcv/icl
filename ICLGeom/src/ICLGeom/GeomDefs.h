/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GeomDefs.h                         **
** Module : ICLGeom                                                **
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
#include <ICLCore/Types.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>
#include <ICLMath/HomogeneousMath.h>
#include <vector>
#include <ICLCore/Color.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace geom{
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

  } // namespace geom
}
