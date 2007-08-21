#ifndef ICL_BORDERED_OBJECT_H
#define ICL_BORDERED_OBJECT_H

#include <iclObject.h>

/// The icl namespace
namespace icl{

  /// class of objects with dedicated "Border"-points
  /** In many applications, not all object-points are interesting at one time,
      but only these ones, which will produce visible borders in an image. E.g.
      an application, which trys to find some sphere object in a szene, just needs
      the spheres surrounding (if illumination, reflexes and so on remain discounted)
      fit the the "sphere-model" into the szene. For convex objects, this surrounding
      can be estimated by calculating each point, which belongs to the convex hull
      of that object. Hence the BorderedObject class provides an efficient implementation
      for calculating an objects convex hull using the "Graham-Scan" algorithm.\n
      In addition, the class provides an interface to define additional edges which will
      help to match an object into a szene: The addNewBorder() function will internally
      create a new border int the private border array.\n
      Borders are stored internally in a list (calls LineStripArray) of line-strips
      (type LineStrip) (Point--Point--Point--...)
      which implies connections between adjacent points. 
  */
  class BorderedObject : public Object{
    public:
    /// critical parameter which defines the maximal count of border elements per object
    static const int MAX_BORDERS = 100;
    
    /// typedef for LineStrip
    typedef std::vector<Vec*> LineStrip;
    
    /// typedef for all borders of an object
    typedef std::vector<LineStrip> LineStripArray;
    
    /// Constructor
    BorderedObject();
    
    /// Destructor
    virtual ~BorderedObject();
    
    /// extended projection function (projects and updates all border points)
    virtual void project(const Mat &cameramatrix);
    
    /// extended rendering function (renders the object and visualizes the border points)
    virtual void render(ICLDrawWidget *widget) const;

    /// extended rendering function (renders the object and visualizes the border points)
    virtual void render(Img32f *image) const;

    /// sets whether border points should be explicitly visualized or not
    static void enableBorderVisualization(bool enable){
      VISUALIZE_BORDERS = enable;
    }
    protected:
    /// internally creates a new border buffer (which is deleted automatically)
    /** The returned pointer may serve as shortcut for special borders */
    LineStrip *addNewBorder();
    
    /// returns the convex-hull line-strip (const)
    const LineStrip *getConvexHull() const{ return m_poConvexHullLineStrip; }
    
    /// returns the all borders (const)
    const LineStripArray &getBorders() const{ return m_vecBorders; }

    /// virtual function which updates all borders
    /** This function can be reimplemented by special objects. If this is done,
        do not forget to call BorderedObject::updateBorders(). Otherwise, the
        convex hull is not updated internally.\n
        The function is called automatically when the project(const Mat&) function 
        is called.
    **/
    virtual void updateBorders(); // 

    /// internally checks wheter a Line, a Triangle or a Quad references an invalid point
    virtual bool check() const;
    private:   
  
    /// internally calculates the objects convex using the "Graham-Scan" algorithm
    void calculateConvexHull(LineStrip &dst);

    /// All borders are stored here 
    LineStripArray m_vecBorders; 
    
    /// The general border (surrounding=convex hull is stored here!)
    LineStrip *m_poConvexHullLineStrip;

    /// static flag 
    static bool VISUALIZE_BORDERS;
  };
}

#endif
