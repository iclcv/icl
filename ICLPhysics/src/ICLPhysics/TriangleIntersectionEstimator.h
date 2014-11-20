/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/TriangleIntersectionEstimator.h**
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

#include <ICLGeom/ViewRay.h>
#include <ICLUtils/Point32f.h>

namespace icl{
namespace physics{

  class ICLPhysics_API TriangleIntersectionEstimator{
    public:
    enum IntersectionType{
      noIntersection,
      foundIntersection,
      wrongDirection,
      degenerateTriangle,
      rayIsCollinearWithTriangle
    };
    
    struct ICLPhysics_API Triangle{
      Triangle(){}
      Triangle(const geom::Vec &a, const geom::Vec &b, const geom::Vec &c):a(a),b(b),c(c){}
      geom::Vec a,b,c;
    };
    
    
    struct ICLPhysics_API Intersection{
        Intersection(IntersectionType type=noIntersection, 
                     const geom::Vec &position=geom::Vec(),
                     const utils::Point32f &trianglePosition=utils::Point32f::null):
        type(type),position(position),trianglePosition(trianglePosition){}
      IntersectionType type;
      geom::Vec position;
      utils::Point32f trianglePosition;
      operator bool() const { return type == foundIntersection; }
    };
    
    static Intersection find(const Triangle &t, const geom::ViewRay &r);
  };

}
}
