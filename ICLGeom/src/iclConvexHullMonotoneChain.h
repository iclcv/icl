#ifndef ICL_CONVEX_HULL_MONOTONE_CHAIN_H
#define ICL_CONVEX_HULL_MONOTONE_CHAIN_H

#include <iclGeom.h>
namespace icl{
  
  /// Utility Structure used for the Convex-Hull Algorithm
  struct CHPoint{
    CHPoint(float x=0, float y=0, Vec *v=0):x(x),y(y),v(v){}
    float x,y;
    Vec *v;
    inline bool operator<(const CHPoint &p) const{
      if(y==p.y) return x<p.x;
      return y<p.y;
    }
  };
  
  // Copyright 2001, softSurfer (www.softsurfer.com)
  // This code may be freely used and modified for any purpose
  // providing that this copyright notice is included with it.
  // SoftSurfer makes no warranty for this code, and cannot be held
  // liable for any real or imagined damage resulting from its use.
  // Users of this code must verify correctness for their application.
  
  ///  tests if a point is Left|On|Right of an infinite line.
  /** Input:  three points P0, P1, and P2
      Return: >0 for P2 left of the line through P0 and P1
              =0 for P2 on the line
              <0 for P2 right of the line
      See: the January 2001 Algorithm on Area of Triangles
  **/
  inline float isLeft(const CHPoint P0, const CHPoint P1, const CHPoint P2){
    return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
  }
  
  ///chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
  /** Input:  P[] = an array of 2D points
      presorted by increasing x- and y-coordinates \n
      To be used: this ordering:
      \code
      inline bool operator<(const CHPoint &p) const{
        if(y==p.y) return x<p.x;
        return y<p.y;
      }
      \endcode
      n = the number of points in P[]
      Output: H[] = an array of the convex hull vertices (max is n)
      Return: the number of points in H[]
  **/
  int chainHull_2D(CHPoint* P, int n, CHPoint* H);
}
#endif
