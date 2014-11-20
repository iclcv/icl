/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/GeometricTools.h             **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace physics{

    bool line_segment_intersect(const utils::Point32f &a, const utils::Point32f &b,
                                const utils::Point32f &c, const utils::Point32f &d,
                                utils::Point32f *dst=0,
                                float *dstr=0, float *dsts=0);

    /// Checks whether the given point p lies within the triangle defined by v1,v2 and v3
    bool point_in_triangle(const utils::Point32f &p, const utils::Point32f &v1,
                           const utils::Point32f &v2, const utils::Point32f &v3);
  }
}
