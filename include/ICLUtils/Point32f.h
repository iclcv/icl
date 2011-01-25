/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Point32f.h                            **
** Module : ICLUtils                                               **
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

#ifndef ICL_POINT_32F_H
#define ICL_POINT_32F_H

#include <ICLUtils/Point.h>
#include <ICLUtils/ClippedCast.h>

namespace icl{
  
  /// Single precission 3D Vectors Point class of the ICL \ingroup TYPES
  class Point32f{
    public:
    
    /// x position of this point
    float x;

    /// y position of this point
    float y;
    
    /// null Point is x=0, y=0
    static const Point32f null;

    /// deep copy of a Point
    Point32f(const Point32f& p=null):x(p.x),y(p.y){}

    /// create a special point
    Point32f(float x,float y):x(x),y(y){}

    /// craete a point by a given interger point
    Point32f(const Point &p):x(p.x),y(p.y){}
    
    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two points are equal
    bool operator==(const Point32f &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point32f &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point32f operator+(const Point32f &s) const {return  Point32f(x+s.x,y+s.y);}

    /// substracts two Point32fs as vectors 
    Point32f operator-(const Point32f &s) const {return  Point32f(x-s.x,y-s.y);}

    /// scales a Point32fs variables with a scalar value
    Point32f operator*(double d) const { return Point32f(d*x,d*y);}

    /// Adds another Point32f inplace
    Point32f& operator+=(const Point32f &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point32f inplace
    Point32f& operator-=(const Point32f &s){x-=s.x; y-=s.y; return *this;}

    /// scales the Point32f inplace with a scalar
    Point32f& operator*=(double d) {x*=d; y*=d; return *this; }

    /// transforms the point by element-wise scaling
    Point32f transform(double xfac, double yfac) const{ 
      return Point32f(xfac*x,yfac*y);
    }
    
    /// returns the euclidian distance to another point
    float distanceTo(const Point32f &p) const;
    
    /// returns the p-norm of the 2D Vector
    /** - p = 0 -> 2 
        - p = 1 -> city block norm x+y 
        - p = 2 -> euclidian norm 
        - p->inf -> infinity norm 
        @param p chooses the norm 
        @return norm value 
    **/
    float norm(float p=2);
    
    /// normalized this 2D vector to length=1;
    /** uses the euclidian norm!
        @ return a reference to (this) normalized
    **/
    Point32f &normalize(){
      float l = norm(); x/=l; y/=l; return *this; 
    }
    
    /// returns a normalized version of this Point
    /** @return normalized vec*/
    Point32f normalized() const{
      return Point32f(*this).normalize();
    }

    /// index based interface (returns i?y:x)
    float &operator[](int i) { return i?y:x; }

    /// index based interface, const (returns i?y:x)
    float operator[](int i) const { return i?y:x; }
  };

  /// ostream operator (x,y)
  std::ostream &operator<<(std::ostream &s, const Point32f &p);
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Point32f &p);

  
} // namespace icl

#endif // ICLPOINT_32F_H
