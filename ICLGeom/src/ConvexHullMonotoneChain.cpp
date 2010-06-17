/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ConvexHullMonotoneChain.cpp                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLGeom/ConvexHullMonotoneChain.h>
#include <algorithm>
#include <vector>
using namespace std;


namespace icl{
  
  bool operator <(const Point &a, const Point &p){
    return a.x < p.x || (a.x == p.x && a.y < p.y);
  }
  // Implementation was found here: http://www.algorithmist.com/index.php/Monotone_Chain_Convex_Hull.cpp
  // 2D cross product.
  // Return a positive value, if OAB makes a counter-clockwise turn,
  // negative for clockwise turn, and zero if the points are collinear.
  int cross(const Point &O, const Point &A, const Point &B){
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
  }
  
  // Returns a list of points on the convex hull in counter-clockwise order.
  // Note: the last point in the returned list is the same as the first one.
  vector<Point> convexHull(vector<Point> P){
    int n = P.size(), k = 0;
    vector<Point> H(2*n);
    
    // Sort points lexicographically
    sort(P.begin(), P.end());
    
    // Build lower hull
    for (int i = 0; i < n; i++) {
      while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
      H[k++] = P[i];
    }
    
    // Build upper hull
    for (int i = n-2, t = k+1; i >= 0; i--) {
      while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
      H[k++] = P[i];
    }
    
    H.resize(k);
    return H;
  }
  
  int chainHull_2D(CHPoint* P, int n, CHPoint* H){
    ERROR_LOG("do not use the implementation of monotone chain algorithm!");
    // the output array H[] will be used as the stack
    int    bot=0, top=(-1);  // indices for bottom and top of the stack
    int    i;                // array scan index
    
    // Get the indices of points with min x-coord and min|max y-coord
    int minmin = 0, minmax;
    float xmin = P[0].x;
    for (i=1; i<n; i++)
      if (P[i].x != xmin) break;
    minmax = i-1;
    if (minmax == n-1) {                  // degenerate case: all x-coords == xmin
      H[++top] = P[minmin];
      if (P[minmax].y != P[minmin].y)     // a nontrivial segment
        H[++top] = P[minmax];
      H[++top] = P[minmin];               // add polygon endpoint
      return top+1;
    }
    
    // Get the indices of points with max x-coord and min|max y-coord
    int maxmin, maxmax = n-1;
    float xmax = P[n-1].x;
    for (i=n-2; i>=0; i--)
      if (P[i].x != xmax) break;
    maxmin = i+1;
    
    // Compute the lower hull on the stack H
    H[++top] = P[minmin];                      // push minmin point onto stack
    i = minmax;
    while (++i <= maxmin){
        // the lower line joins P[minmin] with P[maxmin]
      if (isLeft( P[minmin], P[maxmin], P[i]) >= 0 && i < maxmin)
        continue;                              // ignore P[i] above or on the lower line
      
      while (top > 0){                         // there are at least 2 points on the stack
        // test if P[i] is left of the line at the stack top
        if (isLeft( H[top-1], H[top], P[i]) > 0)
          break;                               // P[i] is a new hull vertex
        else
          top--;                               // pop top point off stack
      }
      H[++top] = P[i];                         // push P[i] onto stack
    }
    
    // Next, compute the upper hull on the stack H above the bottom hull
    if (maxmax != maxmin)                      // if distinct xmax points
      H[++top] = P[maxmax];                    // push maxmax point onto stack
    bot = top;                                 // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax){
      // the upper line joins P[maxmax] with P[minmax]
      if (isLeft( P[maxmax], P[minmax], P[i]) >= 0 && i > minmax)
        continue;                              // ignore P[i] below or on the upper line
      
      while (top > bot){                       // at least 2 points on the upper stack
        // test if P[i] is left of the line at the stack top
        if (isLeft( H[top-1], H[top], P[i]) > 0)
          break;                               // P[i] is a new hull vertex
        else
          top--;                               // pop top point off stack
      }
      H[++top] = P[i];                         // push P[i] onto stack
    }
    if (minmax != minmin)
      H[++top] = P[minmin];                    // push joining endpoint onto stack
    
    return top+1;
  }
}
