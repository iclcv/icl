/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/StraightLine2D.h                   **
** Module : ICLMath                                                **
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

#include <ICLMath/FixedVector.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace math{
    
    /// A straight line is parameterized in offset/direction form
    /** This formular is used: 
        \f[ L(x) = \vec{o} + x\vec{v} \f]
        
        The template is instantiated for template parameter Pos type
        Point32f and FixedColVector<float,2>
    */
    struct ICL_MATH_API StraightLine2D{
      /// internal typedef 
      typedef FixedColVector<float,2> PointPolar;
  
      /// internal typedef for 2D points
      typedef FixedColVector<float,2> Pos;
      
      /// creates a straight line from given angle and distance to origin
      StraightLine2D(float angle, float distance);
      
      /// creates a straight line from given 2 points
      StraightLine2D(const Pos &o=Pos(0,0), const Pos &v=Pos(0,0));
  
      /// creates a straight line from given point32f
      StraightLine2D(const utils::Point32f &o, const utils::Point32f &v);
      
      /// 2D offset vector
      Pos o;
      
      /// 2D direction vector
      Pos v;
      
      /// computes closest distance to given 2D point
      float distance(const Pos &p) const;
      
      /// computes closest distance to given 2D point
      /* result is positive if p is left of this->v
          and negative otherwise */
      float signedDistance(const Pos &p) const;
  
      /// computes closest distance to given 2D point
      inline float distance(const utils::Point32f &p) const {
        return distance(Pos(p.x,p.y)); 
      }
      
      /// computes closest distance to given 2D point
      /* result is positive if p is left of this->v
          and negative otherwise */
      float signedDistance(const utils::Point32f &p) const { 
        return signedDistance(Pos(p.x,p.y)); 
      }
      
      /// computes intersection with given other straight line
      /** if lines are parallel, an ICLException is thrown */
      Pos intersect(const StraightLine2D &o) const throw(utils::ICLException);
      
      /// returns current angle and distance
      PointPolar getAngleAndDistance() const;
      
      /// retunrs the closest point on the straight line to a given other point
      Pos getClosestPoint(const Pos &p) const;
    };  
  } // namespace math
}

