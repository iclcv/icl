#ifndef ICL_OBJECT_H
#define ICL_OBJECT_H

#include <iclImg.h>
#include <iclGeom.h>

/// The icl namespace
namespace icl{

  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */
  
  /// Object interface class
  /** Objects consist of:
      -# a set of 3D points
      -# a set of connections (lines) between this points
      -# a set of colors for each point
      -# a set of colors for each line
      -# (a set of triangles <b>not yet implemented!</b> ...)
      -# (a set of colors for each triangle <b>not yet implemented!</b>...)
      -# a homogeneous transformation 4x4 matrix <b>T</b> that transforms the objects
         points from the object coordinate system into the world coordinate system.
      
      Each Object is defined by a primitive of that object (e.g. the unity cube is the
      primitive for all cube objects as cubes and cuboids), and an
      associated transformation matrix. A unity cube at an arbitrary position (x,y,z)
      in the 3D Szene is thus defined by the unity cube \f$ [-1,1]^3 \f$ and an associated
      4x4 homogeneous transformation matrix T:
      \f[
      T=\left(\begin{array}{cccc}
      1 & 0 & 0 & x \\
      0 & 1 & 0 & y \\
      0 & 0 & 1 & z \\
      0 & 0 & 0 & 1 \\
      \end{array} \right)
      \f]
      The object can be transformed later on by just multiplicating its transformation
      matrix T with other transformation matrices. \n
      To enhance performance, each object has 3 different sets of points:
      -# the primitive (original) points (as described above 
         \f$ O=\{(i,j,k)|i,j,j\in\{-1,1\}\} \f$ for the unity cube)
      -# the transformed points (\f$ \{T o_i|o_i \in O\}\f$ for the unity cube)
      -# the projected points (\f$ \{T_{cam} t_i|t_i \in T\}\f$ for the unity cube, where \f$ T_{cam} \f$
         is the camera transformation matrix (@see Camera)
      
      In special object classes, objects must add all points lines (and triangles) by the
      protected add(..) functions. This will reserve e.g. one entry in each of the 3 point sets.
  **/
  class Object{
    public:
    /// typedef for more writing convenience
    typedef std::pair<int,int> Tuple;
    struct Triple{
      Triple():a(0),b(0),c(0){}
      Triple(int a, int b, int c):a(a),b(b),c(c){}
      int a,b,c;
    };
    
    /// Constructor
    Object();
    
    /// Destructor
    virtual ~Object();
      
    /// multiplies T with a homogeneous translation matrix and re-calculates all transformed points
    virtual void translate(float dx, float dy, float dz){ transform(Mat::trans(dx,dy,dz)); }
    
    /// multiplies T with a homogeneous rotation matrix and re-calculates all transformed points
    virtual void rotate(float rx, float ry, float rz){ transform(Mat::rot(rx,ry,rz)); }
    
    /// multiplies T with an arbitrary homogeneous matrix and re-.calculates all transformed points
    virtual void transform(const Mat &m);
    
    /// projects all points using the given camera matrix
    virtual void project(const Mat &cameramatrix);
    
    /// re-define a points color
    void tintPoint(int i, const Vec &color);
    
    /// re-define a lines color
    void tintLine(int i, const Vec &color);

    /// returns the count of points of this object
    inline int getPointCount() const { return (int)(m_vecPtsOrig.size()); }
    
    /// returns the count of lines of this object
    inline int getLineCount() const { return (int)(m_vecConnections.size()); }

    /// internally saves the objects current transformation matrix
    inline void pushState(){ TPushed = T; }
    
    /// internally restores the objects current transformation matrix 
    /** use pushState before */
    inline void popState() { T = TPushed; }
    
    /// returns the current transformation matrix
    const Mat &getT() const { return T; }

    /// sets the current transformation matrix
    void setT(const Mat &m){ T=m; }
    
    /// returns the internal original points vector (const)
    VecArray &getPointsOrig() { return m_vecPtsOrig; }

    /// returns the internal transformed points vector (const)
    VecArray &getPointsTrans() { return m_vecPtsTrans; }

    /// returns the internal projected points vector (const)
    VecArray &getPointsProj() { return m_vecPtsProj; }

    /// returns the internal original points vector (un-const)
    const VecArray &getPointsOrig() const{ return m_vecPtsOrig; }

    /// returns the internal transformed points vector (un-const)
    const VecArray &getPointsTrans() const{ return m_vecPtsTrans; }

    /// returns the internal projected points vector (un-const)
    const VecArray &getPointsProj() const{ return m_vecPtsProj; }
    
    /// renders the object into a draw widget (using the current projected points)
    virtual void render(ICLDrawWidget *widget) const;

    /// renders the object into an image (using the current projected points)
    virtual void render(Img32f *image) const;
    
    protected:
    /// adds a new point to this object
    void add(const Vec &p, const Vec &color=Vec(255,0,0,255));
    
    // adds a new line to this object
    void add(const Tuple &t, const Vec &color=Vec(100,100,100,255));
    
    private:   

    /// The current objects transformation matrix
    Mat T;
    /// Currently pushed objects transformation matrix
    Mat TPushed;

    /// vector of original points
    VecArray m_vecPtsOrig;
    
    /// vector of transformed points
    VecArray m_vecPtsTrans;
    
    /// vector of projected points
    VecArray m_vecPtsProj;
    
    /// vector of point colors
    VecArray m_vecPtsColors;
    
    /// vector of line colors
    VecArray m_vecLineColors;
    
    /// vector of lines (connections between points)
    std::vector<Tuple> m_vecConnections;
  };
}

#endif
