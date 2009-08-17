#ifndef ICL_SAMPLED_LINE_H
#define ICL_SAMPLED_LINE_H

#include <iclPoint.h>
#include <iclRect.h>

namespace icl{
  /// Shallow wrapper class to make the LineSampler class instantiable
  /** Due to the high optimization level of the  class, it might not be appropriate for the 
      user, e.g. because of the fact, that it cannot be instantiated directly. The 
      SampledLine class provides some intelligent data buffer handling to wrap an
      instantiable interface around the LineSampler class.
  */
  class SampledLine{
    /// Internel data pointers (wrapped shallowly)
    Point *m_cur;
    Point *m_end;
    Point *m_bufBegin;
    Point *m_bufEnd;
    
    /// internal initialization function
    void init(int aX, int aY, int bX, int bY);

    /// internal initialization function
    void init(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY);
    
    public:

    /// Creates a NULL sampled line
    SampledLine():m_bufBegin(0),m_bufEnd(0){}
    
    /// Destruktor
    ~SampledLine(){
      ICL_DELETE_ARRAY(m_bufBegin);
    }
    
    /// copy constructor
    SampledLine(const SampledLine &other):m_bufBegin(0),m_bufEnd(0){
      *this = other;
    }
    
    /// create a SampledLine instance (only one instance is valid at a time)
    SampledLine(int aX, int aY, int bX, int bY){ 
      init(aX,aY,bX,bY);
    }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    SampledLine(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY) {
      init(aX,bX,aY,bY,minX,maxX,minY,maxY);
    }
    /// create a SampledLine instance (only one instance is valid at a time)
    SampledLine(const Point &a, const Point &b) { 
      init(a.x,b.x,a.y,b.y); 
    }

    /// create a SampledLine instance (only one instance is valid at a time) with given boundig rect parameters
    SampledLine(const Point &a, const Point &b, const Rect &bounds){
      init(a.x,b.x,a.y,b.y,bounds.x,bounds.y,bounds.right(),bounds.bottom());
    }
    
    /// sets the curr pointer back to the buffer start
    inline void reset(){
      m_cur = m_bufBegin;
    }
    
    /// returns the size of the internal buffer
    inline int getBufferSize() const {
      return m_bufEnd - m_bufBegin;
    }
    
    /// returns the count of already-extracted points
    inline int getBufferOffset() const {
      return m_cur - m_bufBegin;
    }
    
    /// returns the next point of this line
    inline const Point &next() { return *m_cur++; }
    
    /// returns whether this line has remaining points, that have not yet been extracted using next()
    inline bool hasNext() const { return m_cur != m_end; }

    /// returns the number of remaining points in this line
    inline int remaining() const { return (int)(m_end-m_cur); }
    
    /// returns whether this line has been initialized non-trivially 
    inline bool isNull() const { return !m_bufBegin; }
    
    // returns current point
    inline const Point &operator*() const { return *m_cur; }

    // pre-increment operator (jumps to the next point of this line)
    inline SampledLine &operator++() { ++m_cur; return *this; }

    /// equal to hasNext()
    operator bool() const { return hasNext(); }
    
    /// deep copy constructor
    SampledLine &operator=(const SampledLine &other);

  };

}

#endif
