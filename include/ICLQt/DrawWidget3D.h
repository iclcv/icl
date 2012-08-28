/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#pragma once

#include <ICLQt/DrawWidget.h>
#include <vector>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  namespace qt{
  
    /// Extended ICLDrawWidget, able to draw 2D and 3D primitives using OpenGL
    /** Even though, it is possible to use the ICLDrawWidget3D for rendering
        3D stuff on top of an image directly, it is strongly recommended to use
        an instance of ICLGeom::Scene to manage camera, scene objects and lights.
        A scene instance provides a single ICLDrawWidget3D::GLCallback* that can easily
        be linked to the DrawWidget3D by using the ICLDrawWidget3D::link method.
        
        Most older methods got the deprecated status. They will be removed soon, since
        it turned out, that using ICLGeom's Scene class is much easier and less error-prone.
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
  
  #if 0
      /// reset the command queue  (deprecated)
      ICL_DEPRECATED void reset3D();
      
      /// clears all buffers (deprecated)
      ICL_DEPRECATED void clear3D();
      
      /// draws a 3D cube with size d at given location (x,y,z) (deprecated)
      ICL_DEPRECATED void cube3D(float cx, float cy, float cz, float d);
  
      /// draws a very nice 3D cube with size d at given location (x,y,z) (deprecated)
      ICL_DEPRECATED void supercube3D(float cx, float cy, float cz, float d);
  
      /// draws a 3D cube with size d at given location (x,y,z) (deprecated)
      ICL_DEPRECATED void imagecube3D(float cx, float cy, float cz, float d, const core::ImgBase *image);
  
      /// draws a 3D cube with size d at given location (x,y,z) (deprecated)
      /** @param images array of images to draw on each face of the cube*/
      ICL_DEPRECATED void imagecube3D(float cx, float cy, float cz, float d, const core::ImgBase *images[6]);
  
      /// draw a 3D image texture into the parallelogram defined by the base vector c and two direction vector a and b (deprecated)
      ICL_DEPRECATED void image3D(float cX,float cY,float cZ,float aX, float aY,float aZ,float bX,float bY,float bZ, const core::ImgBase *image);
  
      /// sets the current color to (r,g,b,a) (deprecated)
      ICL_DEPRECATED void color3D(float r, float g, float b, float a);
      
      /// sets up the current modelview-matrix (deprecated)
      /** - eye position of the camera in the scene
          - center center of the szene (where the camera is looking at)
          - up up-vector 
      */
      ICL_DEPRECATED void lookAt3D(float eyeX, float eyeY, float eyeZ, float cX, float cY, float cZ, float upX, float upY, float upZ);
      
      /// sets up the current projection-matrix (deprecated)
      ICL_DEPRECATED void frustum3D(float left,float right,float bottom, float top,float zNear,float zFar);
      
      /// sets up the gl view port (deprecated)
      ICL_DEPRECATED void viewport3D(float x,float y, float width, float height);
      
      /// sets up current projection matrix using a perpective projection (deprecated)
      ICL_DEPRECATED void perspective(float angle, float aspect, float near, float far);
      
      /// rotates the current matrix by given angles (deprecated)
      ICL_DEPRECATED void rotate3D(float rx, float ry, float rz);
  
      /// translates the current szene by given angles (deprecated)
      ICL_DEPRECATED void translate3D(float rx, float ry, float rz);
      
      /// calls glScalef multiply the current matrix with a scaling matrix (deprecated)
      ICL_DEPRECATED void scale3D(float sx, float sy, float sz);
      
      /// multiplies the current matrix with a given matrix (deprecated)
      ICL_DEPRECATED void multMat3D(float *mat);
      
      /// sets the current matrix with a given matrix (deprecated)
      ICL_DEPRECATED void setMat3D(float *mat);
  
      /// switches the current matrix to modelview matrix (deprecated)
      ICL_DEPRECATED void modelview();
      
      /// switches the current matrix to projection matrix (deprecated)
      ICL_DEPRECATED void projection();
      
      /// creates a glPushMatrix command in the command queue (deprecated)
      ICL_DEPRECATED void pushMatrix();
      
      /// creates a glPopMatrix command in the command queue (deprecated)
      ICL_DEPRECATED void popMatrix();
  
      /// sets the current matrix to the identity matrix (deprecated)
      ICL_DEPRECATED void id();
      
      /// creates a callback function draw command (deprecated)
      /** This method locks the internal recursive mutex automatically */
      ICL_DEPRECATED void callback(GLCallbackFunc, void *data=0);
      
      /// create a callback object draw comamnd (ownership is not passed!) (deprecated)
      /** This method locks the internal recursive mutex automatically */
      ICL_DEPRECATED void callback(GLCallback *cb);
      
      /// create a callback object draw command using a smart ptr (so ownership is passed smartly) (deprecated)
      /** Using smart-pointer callback is recommended, if given callback objects are endangered to be
          released in some other thread than that one, which fills and clears the DrawWidget3D's 
          draw command queue. Here it would be possible, that the command queue contains already
          released objects, which leads to run-time errors, if Qt's GUI-threads tries to execute this
          callback objects asynchronously.\\
          This method locks the internal recursive mutex automatically
      */
      ICL_DEPRECATED void callback(SmartPtr<GLCallback> smartCB);
  
  #endif
      /// add a single 3D callback, that is rendered (here, no queue swapping is neccessary)
      /** the linked callback can be removed by calling link(0). The ownership of the linked 
          callback is never passed. */
      void link(GLCallback *cb);
      
  #if 0
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
          - property: "core::depth-test" value "on|off" (default on)
          
          Further properties will follow!
          
          Note: specular light is not working yet, maybe the object 
          material has to be set up for specular light as well.
      */
      ICL_DEPRECATED void setProperty(const std::string &property, const std::string &value);
  
      protected:    
  
      virtual void swapQueues();
      
      /// draw command event queue
      std::vector<DrawCommand3D*> *m_commands[2];
  #endif
  
      protected:
      /// special single callback linked to the 3D visualisation
      GLCallback *m_linkedCallback;
  
    };
  } // namespace qt
}

