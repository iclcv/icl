#ifndef ICL_DRAW_WIDGET_3D_H
#define ICL_DRAW_WIDGET_3D_H

#include <iclDrawWidget.h>
#include <vector>

namespace icl{

  /// Extended ICLDrawWidget, able to draw 2D and 3D primitives using OpenGL
  /** TODO: Document*/
  class ICLDrawWidget3D : public ICLDrawWidget {
    public:
    
    /// internally used vector class
    struct vec{
      vec(float x=0,float y=0,float z=0):x(x),y(y),z(z){}
      float x,y,z;
    };

    /// creates a new ICLDrawWidget embedded into the parent component
    ICLDrawWidget3D(QWidget *parent);

    /// destructor
    ~ICLDrawWidget3D();
    
    virtual void customPaintEvent(PaintEngine *e);

    /// reset the command queue
    void reset3D();
    
    /// clears all buffers
    void clear3D();
    
    /// draw a 3D cube with border size d at given location (x,y,z)
    void cube3D(const vec &center, float d);

    /// sets the current color to (r,g,b,a)
    void color3D(float r, float g, float b, float a);
    
    /// sets up the current modelview-matrix
    void lookAt3D(const vec &eye, const vec &center, const vec &up);
    
    /// sets up the current projection-matrix
    void frustum3D(float left,float right,float bottom, float top,float zNear,float zFar);
    
    /// sets up current projection matrix using a perpective projection
    void perspective(float angle, float aspect, float near, float far);
    
    /// rotates the current matrix by given angles
    void rotate3D(float rx, float ry, float rz);

    /// translates teh current szene by given angles
    void translate3D(float rx, float ry, float rz);
    
    /// multiplies the current matrix with a given matrix
    void multMat3D(float *mat);
    
    /// sets the current matrix with a given matrix
    void setMat3D(float *mat);

    /// switches the current matrix to modelview matrix
    void modelview();
    
    /// switches the current matrix to projection matrix
    void projection();
    
    /// sets the current matrix to the identity matrix
    void id();

    /// forward declaration of the internally used DrawCommandClass
    class DrawCommand3D;

    protected:    

    /// draw command event queue
    std::vector<DrawCommand3D*> m_vecCommands3D;

  };
}

#endif
