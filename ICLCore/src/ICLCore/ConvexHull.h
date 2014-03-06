/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ConvexHull.h                       **
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
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <vector>

namespace icl{
  namespace core{

    /// convex hull monotone chain algorithm for int-points
    /** @param P list of utils::Point (input) call-by-value, as we need an inplace-sort
                 internally
        @return list of points of the convex hull first point is identical 
        to the last point in this list!
    */
    ICLCore_API std::vector<utils::Point> convexHull(std::vector<utils::Point> P);
  
    /// convex hull monotone chain algorithm for float-points
    /** @param P list of utils::Point32f (input) call-by-value, as we need an inplace-sort
                 internally
        @return list of points of the convex hull first point is identical 
        to the last point in this list!
    */
    ICLCore_API std::vector<utils::Point32f> convexHull(std::vector<utils::Point32f> P);
  
  } // namespace geom
}
