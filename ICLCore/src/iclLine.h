#ifndef ICL_LINE_H
#define ICL_LINE_H

#include <iclPoint.h>
#include <iclRect.h>
#include <vector>

namespace icl{
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
    Line(Point start=Point::null, Point end=Point::null):
    start(start),end(end){}
    
    /// Creates a new line by given polar coordinates
    /** @param start start point of the line
        @param angle angle of the line 
        @param length length of the line 
    */
    Line(Point start, float angle, float length);
    
    /// translates a line by a given vector
    /** @param p translation vector
        @return the translated line
    */
    Line operator+(const Point &p) const { return Line(start+p,end+p); }
    
    /// translates a line by a given vector (negative direction)
    /** @param p translation vector
        @return the translated line
    */
    Line operator-(const Point &p) const { return Line(start-p,end-p); }
    
    /// calculates the euclidean norm of this line
    /** @return length of the line */
    float length() const;

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
    void swap() { Point x=start; start=end; end=x; }
    
    /// start point of this line
    Point start;

    /// end point of this line
    Point end;
  };

  /// ostream operator (start-x,start-y)(end-x,end-y)
  std::ostream &operator<<(std::ostream &s, const Line &l);
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Line &l);

}

#endif
