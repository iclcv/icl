/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Point.h                               **
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

#ifndef ICL_POINT_H
#define ICL_POINT_H
#ifdef HAVE_IPP
#include <ipp.h>
#endif

#include <ICLUtils/Macros.h>
#include <ostream>

namespace icl{
#ifndef HAVE_IPP
  /// fallback implementation for the IppiPoint struct, defined in the ippi libb \ingroup TYPES
  struct IppiPoint {
    /// xpos
    int x;
    /// ypos
    int y;
  };
#else
#endif

  /** \cond */
  class Point32f;
  /** \endcond */
  
  /// Point class of the ICL used e.g. for the Images ROI offset \ingroup TYPES
  class Point : public IppiPoint{
    public:
    /// null Point is x=0, y=0
    static const Point null;

    /// deep copy of a Point
    Point(const Point& p=null){ this->x = p.x; this->y = p.y; }

    /// Create a point from given float point (values are rounded)
    Point(const Point32f &p);
    
    /// create a special point
    Point(int x,int y){this->x = x;this->y = y;}

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two points are equal
    bool operator==(const Point &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point operator+(const Point &s) const {return  Point(x+s.x,y+s.y);}

    /// substracts two Points as vectors 
    Point operator-(const Point &s) const {return  Point(x-s.x,y-s.y);}

    /// scales a Points variables with a scalar value
    Point operator*(double d) const { return Point((int)(d*x),(int)(d*y));}

    /// Adds another Point inplace
    Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point inplace
    Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}

    /// scales the Point inplace with a scalar
    Point& operator*=(double d) {x=(int)((double)x*d); y=(int)((double)y*d); return *this;};

    /// transforms the point by element-wise scaling
    Point transform(double xfac, double yfac) const{ 
      return Point((int)(xfac*x),(int)(yfac*y));
    }
    
    /// returns the euclidian distance to another point
    float distanceTo(const Point &p) const;
  };
  
  /// ostream operator (x,y)
  std::ostream &operator<<(std::ostream &s, const Point &p);
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Point &p);

  
} // namespace icl

#endif // ICLPOINT_H
