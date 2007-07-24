#ifndef ICL_BORDERED_OBJECT_H
#define ICL_BORDERED_OBJECT_H

#include <iclObject.h>

/// The icl namespace
namespace icl{

  class BorderedObject : public Object{
    public:
    static const int MAX_BORDERS = 100;
    
    friend class Szene;
    typedef std::vector<Vec*> LineStrip;
    typedef std::vector<LineStrip> LineStripArray;
    
    BorderedObject();
    virtual ~BorderedObject();
    
    virtual void project(const Mat &cameramatrix);
    
    virtual void render(ICLDrawWidget *widget) const;
    virtual void render(Img32f *image) const;

    protected:
    LineStrip *addNewBorder();
    
    const LineStrip *getConvexHull() { return m_poConvexHullLineStrip; }
    const LineStripArray &getBorders() const{ return m_vecBorders; }

    virtual void updateBorders(); // can be reimplemented by special objects!
    private:   
  

    void calculateConvexHull(LineStrip &dst);

    /// All borders are stored here 
    LineStripArray m_vecBorders; 
    
    /// The general border (surrounding=convex hull is stored here!)
    LineStrip *m_poConvexHullLineStrip;

    
    static const bool VISUALIZE_BORDERS = true;
  };
}

#endif
