#ifndef ICL_STRAIGHT_LINE_2D_H
#define ICL_STRAIGHT_LINE_2D_H

#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Exception.h>

namespace icl{
  
  /// A straight line is parameterized in offset/direction form
  /** This formular is used: 
      \f[ L(x) = \vec{o} + x\vec{v} \f]
      
      The template is instantiated for template parameter Pos type
      Point32f and FixedColVector<float,2>
  */
  struct StraightLine2D{
    /// internal typedef 
    typedef FixedColVector<float,2> PointPolar;

    /// internal typedef for 2D points
    typedef FixedColVector<float,2> Pos;
    
    /// creates a straight line from given angle and distance to origin
    StraightLine2D(float angle, float distance);
    
    /// creates a straight line from given 2 points
    StraightLine2D(const Pos &o=Pos(0,0), const Pos &v=Pos(0,0));

    /// 2D offset vector
    Pos o;
    
    /// 2D direction vector
    Pos v;
    
    /// computes closest distance to given 2D point
    float distance(const Pos &p) const;
    
    /// computes intersection with given other straight line
    /** if lines are parallel, an ICLException is thrown */
    Pos intersect(const StraightLine2D &o) const throw(ICLException);
    
    /// returns current angle and distance
    PointPolar getAngleAndDistance() const;
    
    /// retunrs the closest point on the straight line to a given other point
    Pos getClosestPoint(const Pos &p) const;
  };  
}

#endif
