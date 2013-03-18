/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Line.h                             **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <vector>

namespace icl{
  namespace core{
    /// The ICLs abstract line class describing a line from Point "start" to Point "end" \ingroup TYPES
    /** This Line class provides basic abilities for the description of an abstract line.
        A line is defined by two Points "start" and "end" where each of this points is
        a 2D integer vector. Lines can be translated by using the "+"- and "-"-operators.
        In addition it is possible to sample a line into the discrete grid. Internally this
        sampling procedure is highly optimized by an implementation of Bresenhams line-
        algorithm, which draws lines without any floating point operation at all.
    */
    class Line{
      public:
      /// Null line of length 0 with and and end point 0
      static const Line null;
     
      /// Creates a new line from point "start" to point "end"
      /** @param start start point
          @param end end point 
      */
      Line(utils::Point start=utils::Point::null,
           utils::Point end=utils::Point::null):
      start(start),end(end){}
      
      /// Creates a new line by given polar coordinates
      /** @param start start point of the line
          @param angle angle of the line 
          @param length length of the line 
      */
      Line(utils::Point start, float angle, float length);
      
      /// translates a line by a given vector
      /** @param p translation vector
          @return the translated line
      */
      Line operator+(const utils::Point &p) const { return Line(start+p,end+p); }
      
      /// translates a line by a given vector (negative direction)
      /** @param p translation vector
          @return the translated line
      */
      Line operator-(const utils::Point &p) const { return Line(start-p,end-p); }
      
      /// calculates the euclidean norm of this line
      /** @return length of the line */
      float length() const;

      /// returns the point on the line closest to the given point
      utils::Point findClosestPoint(const utils::Point &p) const;

      /// returns the minimum distance of the line to a given point 
      /** The distance can never be longer then the max-distance 
          to start and end of the line */
      inline float getMinDist(const utils::Point &p) const{
        return findClosestPoint(p).distanceTo(p);
      }

      /// return whether the line intersects with the given other line
      /** Optionally, the actual intersection point can be calculated and
          stored into a non-null p argument. dstr and dsts are filled
          with the interpolation factors between start and end for
          the actual intersection point if not null.
          If there is no intersection, p, dstr and dsts are not used */
      bool intersects(const core::Line &other, utils::Point32f *p=0,
                      float *dstr=0, float *dsts=0) const;


      /// samples this line from start to end point regarding the given limiting rect
      /** @param limits each line point is check for being inside of this rect
                        the eases working e.g. on image planes, that have an finite
                        extend. If the limits rect has width*height == 0, the limits
                        are not regarded. 
          @return vector of line Points 
          
          Please note: for more efficient line sampling, the core::LineSampler class can be used!
      */
      std::vector<utils::Point> sample( const utils::Rect &limits=utils::Rect::null ) const;
  
      /// swaps the lines start and end point internally
      void swap() { utils::Point x=start; start=end; end=x; }
      
      /// start point of this line
      utils::Point start;
  
      /// end point of this line
      utils::Point end;
    };
  
    /// ostream operator (start-x,start-y)(end-x,end-y)
    std::ostream &operator<<(std::ostream &s, const Line &l);
    
    /// istream operator
    std::istream &operator>>(std::istream &s, Line &l);
  
  } // namespace core
}

