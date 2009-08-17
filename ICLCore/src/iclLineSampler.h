#ifndef ICL_LINE_SAMPLER_H
#define ICL_LINE_SAMPLER_H

#include <iclPoint.h>
#include <iclRect.h>

namespace icl{
  /// Utility class for efficient line sampling (providing only static functions)
  /** The LineSampler class is a bit different from the Line class, which is also provided 
      in the ICLCore package. The Line class is a really basic line structure, that ALSO provides
      a function to sample itsef into an image pixel grid. The LineSampler class uses a lot of
      optimizations to enhance sampling performance. This is achieved by a set of optimizations:
      
      - avoid data copies and dynamic memory allocations: Internally, a static (of given) buffer is 
        used to store the sampled point set. This provides a high performance benefit, however
        it also entails the main drawback: The LineSampler class cannot be instatiated, as it this 
        would mean, that this sampling buffers was either used by several instances, or that each
        instance would need an own buffer, which would nullify all the performance benefits, we got
        before
      - limiting the maximum line length: by this means, we can use a fixed size data buffer
      - internally we use a lot of templates, to suppress if-then statements if possible
      
      The facilitate buffer allocation and things like that, the SampledLine class should be used. 
      This class provides an Object-oriented handling of sampled lines
      
      \section PERF Performance Comparison
      In comparison to the Line.sample function, the LineSampler is nearly 30 times faster.
      Sampling a short line (from (23,40) to (20,20)) with given bounding rect 1 Million times 
      lasts about 120ms (compiled with -O4 and -march=native on a 2Ghz Core-2-duo)
  */
  class LineSampler{
    public:
    
    // maximum number of line pixels
    static const int MAX_LINE_LENGTH = 10000;

    private:
    /// Internel data pointers (wrapped shallowly)
    static Point *cur, *end;
    
    /// internal initialization function
    static inline void init(Point *cur, Point *end){
      LineSampler::cur = cur;
      LineSampler::end = end;
    }
    
    /// internal sampling function using bresenham line sampling algorithm
    template<bool steep, bool steep2, int ystep>
    static void bresenham_templ_2(int x0, int x1, int y0, int y1, Point *p);

    /// internal sampling function using bresenham line sampling algorithm
    template<bool steep, bool steep2, int ystep>
    static void bresenham_templ(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *p);

    /// internal sampling function using bresenham line sampling algorithm
    static void bresenham(int x0, int x1, int y0, int y1, int minX, int maxX, int minY, int maxY, Point *p);

    /// internal sampling function using bresenham line sampling algorithm
    static void bresenham(int x0, int x1, int y0, int y1, Point *p);

    /// internal point buffer
    static Point buf[MAX_LINE_LENGTH];
    
    static Point *get_buf(Point *userBuf){
      return userBuf ? userBuf : LineSampler::buf;
    }
    LineSampler(){}
    
    public:
    
    static void getPointers(Point **cur, Point **end){
      *cur = LineSampler::cur;
      *end = LineSampler::end;
    }

    /// create a SampledLine instance (only one instance is valid at a time)
    static void init(int aX, int aY, int bX, int bY, Point *userBuf=0){ bresenham(aX,bX,aY,bY,get_buf(userBuf)); }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    static void init(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY, Point *userBuf=0) {
      bresenham(aX,bX,aY,bY,minX,maxX,minY,maxY,get_buf(userBuf));
    }
    /// create a SampledLine instance (only one instance is valid at a time)
    static void init(const Point &a, const Point &b, Point *userBuf=0) { bresenham(a.x,b.x,a.y,b.y,get_buf(userBuf)); }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    static void init(const Point &a, const Point &b, const Rect &bounds, Point *userBuf=0){
      bresenham(a.x,b.x,a.y,b.y,bounds.x,bounds.y,bounds.right(),bounds.bottom(), get_buf(userBuf));
    }
    
    /// gets the next valid point (this function is not overflow-safe)
    /** calls to next must be protected by using hasNext() before*/
    static inline const Point &next(){ return *cur++; }
    
    /// returns whether this line has remaining points, that have not yet been extracted using next()
    static inline bool hasNext() { return cur != end; }

    /// returns the number of remaining points in this line
    static inline int remaining() { return (int)(end-cur); }
  };

}

#endif
