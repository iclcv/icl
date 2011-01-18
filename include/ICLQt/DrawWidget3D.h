/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/DrawWidget3D.h                           **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_DRAW_WIDGET_3D_H
#define ICL_DRAW_WIDGET_3D_H

#include <ICLQt/DrawWidget.h>
#include <vector>
#include <ICLUtils/SmartPtr.h>

namespace icl{

  /// Extended ICLDrawWidget, able to draw 2D and 3D primitives using OpenGL
  /** Even though, it is possible to use the ICLDrawWidget3D for rendering
      3D stuff on top of an image directly, it is strongly recommended to use
      an instance of ICLGeom::Scene to manage camera, scene objects and lights.
      A scene instance provides a single ICLDrawWidget3D::GLCallback instance 
      that can be passed to the DrawWidget3D instace using the 
      DrawWidget3D::callback method.
      
      still todo: more documentation and examples 
  */
  class ICLDrawWidget3D : public ICLDrawWidget {
    /// Internal structure for hidden data
    struct Properties;

    /// Internal structure for hidden data
    Properties *m_properties;
    
    public:
    /// internally used callback function type
    typedef void (*GLCallbackFunc)(void*);
    
    /// internally used callback class type
    class GLCallback{
      protected:
      /// create a callback with given displaylist flag
      GLCallback(bool useDisplayList = false);
      
      public:
      /// empty destructor (doing nothing)
      virtual ~GLCallback(){}
      /// pure virtual drawing function
      virtual void draw()=0;

      /// 2nd draw functions that can access the parent draw-widget
      virtual void drawSpecial(ICLDrawWidget3D *widget){}

      /// this function is called externally eventually using display list
      void draw_extern(ICLDrawWidget3D *widget);

      private:
      bool m_bUseDisplayList;
      unsigned int m_uiListHandle;
      bool m_bFirst;
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
    
    /// draws a 3D cube with size d at given location (x,y,z)
    void cube3D(float cx, float cy, float cz, float d);

    /// draws a very nice 3D cube with size d at given location (x,y,z)
    void supercube3D(float cx, float cy, float cz, float d);

    /// draws a 3D cube with size d at given location (x,y,z)
    void imagecube3D(float cx, float cy, float cz, float d, const ImgBase *image);

    /// draws a 3D cube with size d at given location (x,y,z)
    /** @param images array of images to draw on each face of the cube*/
    void imagecube3D(float cx, float cy, float cz, float d, const ImgBase *images[6]);

    /// draw a 3D image texture into the parallelogram defined by the base vector c and two direction vector a and b
    void image3D(float cX,float cY,float cZ,float aX, float aY,float aZ,float bX,float bY,float bZ, const ImgBase *image);

    /// sets the current color to (r,g,b,a)
    void color3D(float r, float g, float b, float a);
    
    /// sets up the current modelview-matrix
    /** - eye position of the camera in the scene
        - center center of the szene (where the camera is looking at)
        - up up-vector 
    */
    void lookAt3D(float eyeX, float eyeY, float eyeZ, float cX, float cY, float cZ, float upX, float upY, float upZ);
    
    /// sets up the current projection-matrix
    void frustum3D(float left,float right,float bottom, float top,float zNear,float zFar);
    
    /// sets up the gl view port
    void viewport3D(float x,float y, float width, float height);
    
    /// sets up current projection matrix using a perpective projection
    void perspective(float angle, float aspect, float near, float far);
    
    /// rotates the current matrix by given angles
    void rotate3D(float rx, float ry, float rz);

    /// translates the current szene by given angles
    void translate3D(float rx, float ry, float rz);
    
    /// calls glScalef multiply the current matrix with a scaling matrix
    void scale3D(float sx, float sy, float sz);
    
    /// multiplies the current matrix with a given matrix
    void multMat3D(float *mat);
    
    /// sets the current matrix with a given matrix
    void setMat3D(float *mat);

    /// switches the current matrix to modelview matrix
    void modelview();
    
    /// switches the current matrix to projection matrix
    void projection();
    
    /// creates a glPushMatrix command in the command queue
    void pushMatrix();
    
    /// creates a glPopMatrix command in the command queue
    void popMatrix();

    /// sets the current matrix to the identity matrix
    void id();
    
    /// creates a callback function draw command
    void callback(GLCallbackFunc, void *data=0);
    
    /// create a callback object draw comamnd (ownership is not passed!)
    void callback(GLCallback *cb);
    
    /// create a callback object draw command using a smart ptr (so ownership is passed smartly)
    /** Using smart-pointer callback is recommended, if given callback objects are endangered to be
        released in some other thread than that one, which fills and clears the DrawWidget3D's 
        draw command queue. Here it would be possible, that the command queue contains already
        released objects, which leads to run-time errors, if Qt's GUI-threads tries to execute this
        callback objects asynchronously.
    */
    void callback(SmartPtr<GLCallback> smartCB);

    /// forward declaration of the internally used DrawCommandClass
    class DrawCommand3D;

    /// sets internal openGL rendering properties (important: please use the ICLGeom::Scene instead)
    /** <b>Please note:</b> These light properties are just used if
        you use the DrawWidget3D with out an instance of ICLGeom::Scene.
        The Scene has it's own much more sophisticated light handling.
        
        Currently available: 
        - property "diffuse" value "on|off" (default is on)
        - property "ambient" value "on|off" (default is off)
        - property "specular" value "on|off" (default is off) 
        - property: "light-X" value "on|off"
        - property: "light-X-pos" value "{x,y,z,h}"
        - property: "light-X-ambient" value "{r,g,b,factor}"
        - property: "light-X-diffuse" value "{r,g,b,factor}"
        - property: "light-X-specular" value "{r,g,b,factor}"
        - property: "lighting" value "on|off" (default on) 
        - property: "color-material" value "on|off" (default on)
        - property: "depth-test" value "on|off" (default on)
        
        Further properties will follow!
        
        Note: specular light is not working yet, maybe the object 
        material has to be set up for specular light as well.
    */
    void setProperty(const std::string &property, const std::string &value);

    protected:    

    /// draw command event queue
    std::vector<DrawCommand3D*> m_vecCommands3D;

  };
}

#endif
