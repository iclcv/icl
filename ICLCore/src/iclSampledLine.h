#ifndef ICL_SAMPLED_LINE_H
#define ICL_SAMPLED_LINE_H

#include <iclPoint.h>
#include <iclRect.h>

namespace icl{
  /// Utility class for efficient line sampling (not reentrant)
  /** The SampledLine class is a bit different from the Line class, which is also provided 
      in the ICLCore package. The Line class is a really basic line structure, that ALSO provides
      a function to sample itsef into an image pixel grid. The SampledLine class uses a lot of
      optimizations to enhance sampling performance. This is achieved by a set of optimizations:
      
      - avoid data copies and dynamic memory allocations: Internally, we use a single static
        buffer to store the sampled point set. This provides a hight performance benefit, however
        it also entails the main drawback: Only one instance of this class can be used at one 
        time
      - limiting the maximum line length: by this means, we can use a fixed size data buffer
      - internal use of a lot of templates, to remove if-then statements if possible
      
      \section PERF Performance Comparison
      In comparison to the Line.sample function, the SampledLine is nearly 30 times faster.
      Sampling a short line (from (23,40) to (20,20)) with given bounding rect 1 Million times 
      lasts about 120ms (compiled with -O4 and -march=native on a 2Ghz Core-2-duo)
  */
  class SampledLine{
    public:
    
    // maximum number of line pixels
    static const int MAX_LINE_LENGTH = 10000;

    private:
    /// Internel data pointers (wrapped shallowly)
    Point *cur, *end;
    
    /// internal initialization function
    inline void init(Point *cur, Point *end){
      this->cur = cur;
      this->end = end;
    }
    
    /// internal sampling function using bresenham line sampling algorithm
    template<bool steep, bool steep2, int ystep>
    void bresenham_templ_2(int x0, int x1, int y0, int y1, Point *p);

    /// internal sampling function using bresenham line sampling algorithm
    template<bool steep, bool steep2, int ystep>
    void bresenham_templ(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *p);

    /// internal sampling function using bresenham line sampling algorithm
    void bresenham(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY);

    /// internal sampling function using bresenham line sampling algorithm
    void bresenham(int x0, int x1, int y0, int y1);

    /// internal point buffer
    static Point buf[MAX_LINE_LENGTH];
    public:

    /// create an empty Sampled line instance (only one instance is valid at a time)
    SampledLine():cur(0),end(0){}

    /// create a SampledLine instance (only one instance is valid at a time)
    SampledLine(int aX, int aY, int bX, int bY){ bresenham(aX,bX,aY,bY); }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    SampledLine(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY) {
      bresenham(aX,bX,aY,bY,minX,maxX,minY,maxY);
    }
    /// create a SampledLine instance (only one instance is valid at a time)
    SampledLine(const Point &a, const Point &b) { bresenham(a.x,b.x,a.y,b.y); }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    SampledLine(const Point &a, const Point &b, const Rect &bounds){
      bresenham(a.x,b.x,a.y,b.y,bounds.x,bounds.y,bounds.right(),bounds.bottom());
    }
    
    /// gets the next valid point (this function is not overflow-safe)
    /** calls to next must be protected by using hasNext() before*/
    inline const Point &next(){ return *cur++; }
    
    /// returns whether this line has remaining points, that have not yet been extracted using next()
    inline bool hasNext() const { return cur != end; }

    /// returns the number of remaining points in this line
    inline int remaining() const{ return (int)(end-cur); }
  };

}

#endif
