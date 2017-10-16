/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Line32f.h                          **
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
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect.h>
#include <vector>

namespace icl{
  namespace core{

    /** \cond */
    class Line;
    /** \endcond */

    /// The ICLs abstract line class describing a line from Point "start" to Point "end" \ingroup TYPES
    /** @see icl::Line
    */
    class ICLCore_API Line32f{
      public:
      /// Null line of length 0 with and and end point 0
      static const Line32f null;

      /// Creates a new line from point "start" to point "end"
      /** @param start start point
          @param end end point
      */
      Line32f(utils::Point32f start=utils::Point::null,
              utils::Point32f end=utils::Point::null):
      start(start),end(end){}

      /// Creates a new line by given polar coordinates
      /** @param start start point of the line
          @param angle angle of the line
          @param length length of the line
      */
      Line32f(utils::Point32f start, float angle, float length);

      /// Creates a line by a given integer line
      /** @param l interger line*/
      Line32f(const Line &l);

      /// translates a line by a given vector
      /** @param p translation vector
          @return the translated line
      */
      Line32f operator+(const utils::Point32f &p) const { return Line32f(start+p,end+p); }

      /// translates a line by a given vector (negative direction)
      /** @param p translation vector
          @return the translated line
      */
      Line32f operator-(const utils::Point32f &p) const { return Line32f(start-p,end-p); }

      /// calculates the euclidean norm of this line
      /** @return length of the line */
      float length() const;

      /// returns line angle [ atan2(dy,dx) ]
      float getAngle() const;

      /// returns the lines center point [ (start+end)/2 ]
      utils::Point32f getCenter() const;

      /// return whether the line intersects with the given other line
      /** Optionally, the actual intersection point can be calculated and
          stored into a non-null p argument. dstr and dsts are filled
          with the interpolation factors between start and end for
          the actual intersection point if not null.
          If there is no intersection, p, dstr and dsts are not used */
      bool intersects(const core::Line32f &other, utils::Point32f *p=0,
                      float *dstr=0, float *dsts=0) const;

      /// samples this line from start to end point regarding the given limiting rect
      /** @param limits each line point is check for being inside of this rect
                        the eases working e.g. on image planes, that have an finite
                        extend. If the limits rect has width*height == 0, the limits
                        are not regarded.
          @return vector of line Points
      */
      std::vector<utils::Point> sample(const utils::Rect &limits=utils::Rect::null ) const;

      /// samples this line from start to end point regarding the given limiting rect
      /** This function works essentially like the above function. In this case, the
          result is not returned, but it is stored into the given vector references.
          @param xs destination vector for x-coordinates (filled using push_back, so
                    it is not cleared before it is filled)
          @param ys as xs but for the y-coordinates
          @param limits (see above)*/
      void sample(std::vector<int> &xs,std::vector<int> &ys, const utils::Rect &limits=utils::Rect::null ) const;

      /// swaps the lines start and end point internally
      void swap() { utils::Point32f x=start; start=end; end=x; }

      /// start point of this line
      utils::Point32f start;

      /// end point of this line
      utils::Point32f end;
    };

    /// ostream operator (start-x,start-y)(end-x,end-y)
    ICLCore_API std::ostream &operator<<(std::ostream &s, const Line32f &l);

    /// istream operator
    ICLCore_API std::istream &operator>>(std::istream &s, Line32f &l);

  } // namespace core
}

