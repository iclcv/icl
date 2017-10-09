/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Rect32f.h                        **
** Module : ICLUtils                                               **
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
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Size32f.h>
#include <ICLUtils/Rect.h>
#include <stdio.h>
#include <algorithm>

namespace icl {
  namespace utils{


    /// Floating point precision implementation of the Rect class \ingroup TYPES
    class ICLUtils_API Rect32f{
      public:

      float x;      ///!< x pos (upper left)
      float y;      ///!< y pos (upper left)
      float width;  ///!< width of the rect
      float height; ///!< height of the rect

      /// static null instance (0,0,0,0)
      static const Rect32f null;

	    /// default constructor
	    Rect32f(){
		    this->x = 0.0f;
		    this->y = 0.0f;
		    this->width = 0.0f;
		    this->height = 0.0f;
	    }

      /// creates a defined Rect32f
      Rect32f(float x, float y, float width, float height):
      x(x),y(y),width(width),height(height){
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
      }

      /// creates a new Rect32f with specified offset and size
      Rect32f(const Point32f &p, const Size32f &s){
        this->x = p.x;
        this->y = p.y;
        this->width = s.width;
        this->height = s.height;
      }

      /// create a deep copy of a rect
      Rect32f(const Rect32f &r){
        this->x = r.x;
        this->y = r.y;
        this->width = r.width;
        this->height = r.height;
      }

      /// create a floating point rect from given int-valued rect
      Rect32f(const Rect &rect):
        x((float)rect.x),
        y((float)rect.y),
        width((float)rect.width),
        height((float)rect.height){
      }

      /// checks wether the object instance is null, i.e. all elements are zero
      bool isNull() const { return (*this)==null; }

      /// checks if two rects are equal
      bool operator==(const Rect32f &s) const {
        return x==s.x && y==s.y && width==s.width && height==s.height;
      }

      /// checks if two rects are not equal
      bool operator!=(const Rect32f &s) const {
        return x!=s.x || y!= s.y || width!=s.width || height!=s.height;
      }

      /// scales all parameters of the rect by a double value
      Rect32f operator*(double d) const {
        return Rect32f(d*x,d*y,d*width,d*height);
      }

      /// scales all parameters of the rect by a double value
      Rect32f operator/(double d) const {
        return Rect32f(d/x,d/y,d/width,d/height);
      }

      /// adds a size to the rects size
      Rect32f& operator+=(const Size32f &s){
        width+=s.width; height+=s.height; return *this;
      }

      /// substracs a size to the rects size
      Rect32f& operator-=(const Size32f &s){
        width-=s.width; height-=s.height; return *this;
      }

      /// adds a Point to the rects offset
      Rect32f& operator+=(const Point32f &p){
        x+=p.x; y+=p.y; return *this;
      }

      /// substracts a Point to the rects offset
      Rect32f& operator-=(const Point32f &p){
        x-=p.x; y-=p.y; return *this;
      }

      /// scales all rect params inplace
      Rect32f& operator*=(double d){
        x*=d;
        y*=d;
        width*=d;
        height*=d;
        return *this;
      }
      /// scales all rect params inplace
      Rect32f& operator/=(double d){
        x/=d;
        y/=d;
        width/=d;
        height/=d;
        return *this;
      }
      /// returns width*height
      float getDim() const {return width*height;}

      /// intersection of two Rect32fs
      Rect32f operator&(const Rect32f &r) const {
         Point32f ul (iclMax (x, r.x), iclMax (y, r.y));
         Point32f lr (iclMin (right(), r.right()), iclMin (bottom(), r.bottom()));
         Rect32f result (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
         if (result.width > 0 && result.height > 0) return result;
         else return null;
      }

      /// inplace intersection of two rects
      Rect32f &operator&=(const Rect32f &r){
        (*this)=(*this)&r;
        return *this;
      }

      /// union of two Rect32fs
      Rect32f operator|(const Rect32f &r) const {
         Point32f ul (iclMin (x, r.x), iclMin (y, r.y));
         Point32f lr (iclMax (right(), r.right()), iclMax (bottom(), r.bottom()));
         return Rect32f (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
      }

      /// inplace union of two rects
      Rect32f &operator|=(const Rect32f &r){
        (*this)=(*this)|r;
        return *this;
      }

      /// rects with negative sizes are normalized to Positive sizes
      /** e.g. the rect (5,5,-5,-5) is normalized to (0,0,5,5) */
      Rect32f normalized() const {
         Rect32f r (*this);
         if (r.width < 0) {r.x += r.width; r.width = -r.width; }
         if (r.height < 0) {r.y += r.height; r.height = -r.height; }
         return r;
      }

      /// returns if a Rect32f containes another rect
      bool contains(const Rect32f &r) const {
         return x<=r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
      }

      /// returns if the Rect32f contains a given point
      bool contains(float x, float y) const{
        return this->x<=x && right()>=x && this->y<=y && bottom()>=y;
      }

      /// let the rect grow by k pixles into each direction
      /** if k<0 the rect becomes smaller
          E.g. Rect32f(10,10,90,90).enlarge(10) creates a Rect32f (0,0,100,100)
          @param k amount of pixel the rectangle is enlarged by
          @return *this
          */
      Rect32f &enlarge(float k){
        x-=k; y-=k; width+=2*k; height+=2*k;
        return *this;
      }

      /// returns an enlarged instance of this rect
      /** @see enlarge(float)*/
      Rect32f enlarged(float k) const{
        return Rect32f(*this).enlarge(k);
      }


      /// returns upper left point of the rect
      Point32f ul() const {
        return Point32f(x,y);
      }
      /// returns lower left point of the rect
      Point32f ll() const {
        return Point32f(x,y+height);
      }
      /// returns upper right point of the rect
      Point32f ur() const {
        return Point32f(x+width,y);
      }
      /// returns lower right point of the rect
      Point32f lr() const {
        return Point32f(x+width,y+height);
      }

      /// returns the left border position
      float left() const { return x; }

      /// returns the right border position
      float right() const { return x+width; }

      /// returns the position of the bottom border
      float bottom() const { return y+height; }

      /// returns the position of the upper border
      float top() const { return y; }

      /// returns the size of the rect
      Size32f getSize() const { return Size32f(width,height); }

      /// returns the center point of the rect
      Point32f center() const {
        return Point32f(x+width/2,y+height/2);
      }

      /// multiplies the rect's x and width by xfac and y and height by yfac
      Rect32f transform(double xfac, double yfac) const {
        return Rect32f(x*xfac,y*yfac,width*xfac,height*yfac);
      }
    };

    /// ostream operator (x,y)wxy
    ICLUtils_API std::ostream &operator<<(std::ostream &s, const Rect32f &r);

    /// istream operator
    ICLUtils_API std::istream &operator>>(std::istream &s, Rect32f &r);


  } // namespace utils
} // namespace icl

