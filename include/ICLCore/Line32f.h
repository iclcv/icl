/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_LINE_32F_H
#define ICL_LINE_32f_H

#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect.h>
#include <vector>

namespace icl{

  /** \cond */
  class Line;
  /** \endcond */
  
  /// The ICLs abstract line class describing a line from Point "start" to Point "end" \ingroup TYPES
  /** @see icl::Line
  */
  class Line32f{
    public:
    /// Null line of length 0 with and and end point 0
    static const Line32f null;
   
    /// Creates a new line from point "start" to point "end"
    /** @param start start point
        @param end end point 
    */
    Line32f(Point32f start=Point::null, Point32f end=Point::null):
    start(start),end(end){}
    
    /// Creates a new line by given polar coordinates
    /** @param start start point of the line
        @param angle angle of the line 
        @param length length of the line 
    */
    Line32f(Point32f start, float angle, float length);
    
    /// Creates a line by a given integer line
    /** @param l interger line*/
    Line32f(const Line &l);

    /// translates a line by a given vector
    /** @param p translation vector
        @return the translated line
    */
    Line32f operator+(const Point32f &p) const { return Line32f(start+p,end+p); }
    
    /// translates a line by a given vector (negative direction)
    /** @param p translation vector
        @return the translated line
    */
    Line32f operator-(const Point32f &p) const { return Line32f(start-p,end-p); }
    
    /// calculates the euclidean norm of this line
    /** @return length of the line */
    float length() const;

    /// returns line angle [ atan2(dy,dx) ]
    float getAngle() const;
    
    /// returns the lines center point [ (start+end)/2 ] 
    Point32f getCenter() const;
    
    /// samples this line from start to end point regarding the given limiting rect
    /** @param limits each line point is check for being inside of this rect
                      the eases working e.g. on image planes, that have an finite
                      extend. If the limits rect has width*height == 0, the limits
                      are not regarded.
        @return vector of line Points 
    */
    std::vector<Point> sample( const Rect &limits=Rect::null ) const;
    
    /// samples this line from start to end point regarding the given limiting rect
    /** This function works essentially like the above function. In this case, the
        result is not returned, but it is stored into the given vector references.
        @param xs destination vector for x-coordinates (filled using push_back, so
                  it is not cleared before it is filled) 
        @param ys as xs but for the y-coordinates 
        @param limits (see above)*/
    void sample(std::vector<int> &xs,std::vector<int> &ys, const Rect &limits=Rect::null ) const;

    /// swaps the lines start and end point internally
    void swap() { Point32f x=start; start=end; end=x; }
    
    /// start point of this line
    Point32f start;

    /// end point of this line
    Point32f end;
  };

  /// ostream operator (start-x,start-y)(end-x,end-y)
  std::ostream &operator<<(std::ostream &s, const Line32f &l);
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Line32f &l);

}

#endif
