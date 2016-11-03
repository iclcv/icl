/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneObject.h                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/ICLConfig.h>
#ifndef ICL_HAVE_OPENGL
#if WIN32
#pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined");
#else
#warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
#endif
#endif

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Mutex.h>
#include <ICLGeom/Primitive.h>
#include <ICLGeom/ViewRay.h>
#include <ICLGeom/Hit.h>
#include <ICLQt/GLFragmentShader.h>

namespace icl{
  namespace geom{
  
    /** \cond */
    class Scene;
    class ShaderUtil;
    /** \endcond */
   
    
    /// The SceneObject class defines visible objects in scenes or scene graph nodes
    /** SceneObject instances are used in combination with the icl::Scene class. You
        can add SceneObjects into a Scene and then render these as an image overlay.
        
        A scene object is defined by
        - a list of 3D-homogeneous vertices
        - a list of primitives that use indices to the vertex list 
        - a transformation matrix
        - a list of children that are rendered relatively to their parent object 
        
        SceneGraph objects can also have no vertices. In this case they are invisible
        nodes within a scene graph.
        
        \section CREATION Creation of SceneObjects
        Usually special SceneObject instances are created by subclassing the SceneObject
        class. Subclasses can either simply add other SceneObjects e.g. using the
        utility methods SceneObject::addCube or SceneObject::addSphere or they can also
        define a custom geometry by adding vertices and primitives using 
        SceneObject::addVertex and e.g. SceneObject::addLine or SceneObject::addQuad
        
  
        \section NORMALS Normals 
        Normals are used for realistic lighting. Therefore, it is recommended to use 
        normals when objects are defined. Normals are also stored in a list. Each face-vertex
        references one of the normals of this list.
        
        \subsection AN AutoNormals
        If no normals are provided, the normals are computed automatically at run-time using
        cross-product:
        
        - lines: no auto normals
        - triangles (vertices a,b,c) -> (a-c) x (b-c)
        - quads (vertices a,b,c,d) -> (d-c) x (b-c)
        - polygons: no auto normals supported
        - textures: here we always use auto-normals
        
        \section DYN Dynamic SceneObjects and Locking
        Custome extensions of the SceneObject-interface can implement the SceneObject's 
        virtual method SceneObject::prepareForRendering which is calle every time before
        the object is acutally rendered. Here, the custom SceneObject can be adapted 
        dynamically. \n
        <b>Please note:</b> When then you want to change the vertex-, primitive- or 
        you'll have to enable the SceneObjects locking mechanism using
        SceneObject::setLockingEnabled(true).
  
        
        For compatibility with former version of the SceneObject class, you can 
        also re-implement the virtual methods 
        SceneObject::lock() and SceneObject::unlock() appropriately. Usually this will
        look like this:
        \code
        class MySceneObject : public SceneObject{
          utils::Mutex mutex;
          public:
          void lock() { mutex.lock(); }
          void unlock() { mutex.unlock(); }
          ...
        };
        \endcode
        
        
  
        \section _COLORS_ Colors
        In the object specification (when you add vertices and other primitives, colors
        are always expected to be in ICL's commong [0,255]^3 range. However, the colors
        are scaled by 1/255 to range [0,1]^3 internally, since this is how OpenGL can access
        the colors more easily. Please keep in mind, that the colors you can find in
        m_vertexColors and also in primitive-instances is alaws in [0,1]^3 range
        
        \section CFV Colors From Vertices
        Sometimes, you might want to draw primtives that use different colors 
        for different corners and interpolate between these. This can be achieved
        by using SceneObject::setColorsFromVertices(true). 
  
        \section _DISPLAY_LISTS_ Display Lists
        
        For static objects (or objects that are not so frequently changed), display lists
        can be created using SceneObject::createDisplayList(). This will speed up the 
        object rendering significantly. Please note, that a display list is always created
        in the next render cycle, which is why SceneObject::createDisplayList can also
        be called from the working thread.
    */
    class SceneObject{
      public:
      
      /// provides direct access for the Scene class
      friend class Scene;
      
      /// create an object
      ICLGeom_API SceneObject();
      
      /// create by string:
      /** currently allowed:
          - "cube" params: [x,y,z,radius];
          - "cuboid" params: [x,y,z,dx,dy,dz]
          - "cone" params: [x,y,z,dx,dy,dz,steps]
          - "cylinder" params: [x,y,z,dx,dy,dz,steps]
          - "sphere" params: [x,y,z,radius,rzSteps,xySlices]
          - "spheroid" params: [x,y,z,rx,ry,rz,rzSteps,xySlices]
          - "superquadric" params: [x,y,z,rx,ry,rz,dx,dy,dz,e1,e2,rzSteps,xySlices] where \n
             - (x,y,z)^T is the center position
             - (rx,ry,rz)^T are the rotation euler angles
             - (dx,dy,dz)^T are the diameters into x-, y- and z-direction
             - (e1,and e2) are the roundness parameters
             - (rzSlices,rxSlices) is used for the number of steps the create nodes
      */
      ICLGeom_API SceneObject(const std::string &type,const float *params);
      
      /// create a cube scene object
      static inline SceneObject *cube(float x, float y, float z, float r){
        const float p[] = { x,y,z,r };
        return new SceneObject("cube",p);
      }
  
      /// create a cuboid scene object
      static inline SceneObject *cuboid(float x, float y, float z, float dx, float dy, float dz){
        const float p[] = { x,y,z,dx,dy,dz };
        return new SceneObject("cuboid",p);
      }
  
      /// create a shere scene object
      static inline SceneObject *sphere(float x, float y, float z, float r, int rzSteps, int xySlices){
        const float p[] = { x,y,z,r, float(rzSteps), float(xySlices) };
        return new SceneObject("sphere",p);
      }
  
      /// create a shere scene object
      static inline SceneObject *spheroid(float x, float y, float z, float rx, float ry, float rz, int rzSteps, int xySlices){
        const float p[] = { x,y,z,rx, ry, rz, float(rzSteps), float(xySlices) };
        return new SceneObject("spheroid",p);
      }
  
      /// create a superquadric scene object
      static inline SceneObject *superquadric(float x, float y, float z, float rx, float ry, float rz, 
                                              float dx, float dy, float dz, float e1, float e2, int rzSteps, int xySlices){
        const float p[] = { x,y,z,rx, ry, rz, dx, dy, dz, e1, e2, float(rzSteps), float(xySlices) };
        return new SceneObject("superquadric",p);
      }
  
      
      /// creates a scene object from given .obj file
      ICLGeom_API SceneObject(const std::string &objFileName) throw (utils::ICLException);
  
      /// deep copy of SceneObject instance
      /** The new instance's parent is set to null, i.e. it must
          be added to other's parent explicitly if this is necessary. */
      SceneObject(const SceneObject &other) { 
        m_displayListHandle = 0;
        m_fragmentShader = 0;
        *this = other; 
        m_parent = 0;
      }
  
      /// assignment operator for deep copy
      /** This instances parent is not changed. I.e. it must
          be added to other's parent explicitly if this is necessary. */
      ICLGeom_API SceneObject &operator=(const SceneObject &other);
      
      /// Empty destructor (but virtual)
      ICLGeom_API virtual ~SceneObject();
      
      /// returns object vertices
      /** If the vertex count is changed, the object needs to be
          locked */
      ICLGeom_API std::vector<Vec> &getVertices();
  
      /// returns object vertices (const)
      ICLGeom_API const std::vector<Vec> &getVertices() const;

			/// returns object normals (const)
			ICLGeom_API const std::vector<Vec> &getNormals() const;
  
      /// returns object vertex colors
      /** If the number of vertex colors is changed, the object needs to be
          locked */
      ICLGeom_API std::vector<GeomColor> &getVertexColors();
  
      /// returns object vertex colors (const)
      ICLGeom_API const std::vector<GeomColor> &getVertexColors() const;
  
  
      /// returns object's primitives (lines, quads, etc...)
      ICLGeom_API std::vector<Primitive*> &getPrimitives();
  
      /// returns object's primitives (lines, quads, etc...) (const)
      ICLGeom_API const std::vector<Primitive*> &getPrimitives() const;
  
      /// changes visibility of given primitive type
      ICLGeom_API void setVisible(int oredTypes, bool visible, bool recursive = true);

      /// convenience method that allows for setting several visibility properties at once
      /** 'what' must be a comma separated list of A=B tokens, where A is a primitive type name
          and B is a boolean (e.g. either true or 1 or false or 0). The "=B" part can be
          left out and is then automatically interpreted as "=true"
          Supported primitive type names are:
          - vertex
          - line
          - triangle
          - quad
          - polygon
          - texture
          - text
          - faces (this is a meta type and refers to triangle, quad, polygon, text and texture)
          - all (refers to all primitive types and to this)
          - this (this is a special type that refers to the whole object)
          
          Here are some examples
          <pre>
          obj.setVisible("vertex,line,faces=false"); // shows only lines and points
          obj.setVisible("this");                    // shows the whole object
          obj.setVisible("this=0");                  // hides the whole object
          obj.setVisible("vertex=false,line=true,triangles=false,quads=true"); // ...
          </pre>

          In case of referencing a given primitive more than once, and with different
          visiblity values, the result is undefined.
     */
      ICLGeom_API void setVisible(const std::string &what, bool recursive=true);

      /// explicit version for const char pointer to avoid an explicit cast to bool/int
      inline void setVisible(const char *what, bool recursive=true){
        setVisible(std::string(what),recursive);
      }
      
      /// returns visibility of given primitive type
      ICLGeom_API bool isVisible(Primitive::Type t) const;

      /// this can be overwrittern in subclasses to hide an object for given camera indices
      /** Please note that this mechanism does not work in case of optimizing object rendering using
          display-lists */
      ICLGeom_API virtual bool isInvisibleForCamera(int camIndex) const { return false; }
      
      /// adds a new vertex to this object
      /** Please note, that colors are defined in ICL's commong [0,255] range,
          but they are stored internally in [0,1] range, since this is how
          OpenGL expects colors */
      ICLGeom_API void addVertex(const Vec &p, const GeomColor &color = GeomColor(255, 0, 0, 255));
  
      /// adds a GLImg as shared texture
      ICLGeom_API void addSharedTexture(utils::SmartPtr<qt::GLImg> gli);
      
      /// adds an core::ImgBase * as shared texutre
      ICLGeom_API void addSharedTexture(const core::ImgBase *image, core::scalemode sm = core::interpolateLIN);
  
  
      /// adds a new normal to this object
      ICLGeom_API void addNormal(const Vec &n);
      
      /// adds a new line to this object
      /** If the given normal indices (na and nb) are -1, no normals are used for this primitives */
      ICLGeom_API void addLine(int x, int y, const GeomColor &color = GeomColor(100, 100, 100, 255));
  
      /// adds a new triangle to this onject
      /** If the given normal indices (na,nb and nc) are -1, auto-normal are computed using cross-product */
      ICLGeom_API void addTriangle(int a, int b, int c, int na, int nb, int nc,
                       const GeomColor &color=GeomColor(0,100,250,255));
      
      /// convenience method for creation of a triangle with auto-normals
      inline void addTriangle(int a, int b, int c, const GeomColor &color=GeomColor(0,100,250,255)){
        addTriangle(a,b,c,-1,-1,-1,color);
      }
  
  
      /// adds a new triangle to this onject
      /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product */
      ICLGeom_API void addQuad(int a, int b, int c, int d, int na, int nb, int nc, int nd,
                   const GeomColor &color=GeomColor(0,100,250,255)); 
  
      /// convenience method for creation of a quad with auto-normals
      inline void addQuad(int a, int b, int c, int d, const GeomColor &color=GeomColor(0,100,250,255)){
        addQuad(a,b,c,d,-1,-1,-1,-1,color);
      }
  
      /// add a polygon to this object (note triangles and quads are slower here)
      /** If the given normal indices's size is 0, auto-normal are computed using cross-product */
      ICLGeom_API void addPolygon(int nPoints, const int *vertexIndices, const GeomColor &color = GeomColor(0, 100, 250, 255),
                      const int *normalIndices=0);
  
      
      /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product */
      ICLGeom_API void addTexture(int a, int b, int c, int d,
                      const core::ImgBase *texture, 
                      int na, int nb, int nc, int nd,
                      bool createTextureOnce=true,
                      core::scalemode sm = core::interpolateLIN);
  
      /// convenience method for creation of a texture with auto-normals
      inline void addTexture(int a, int b, int c, int d, 
                             const core::ImgBase *texture, 
                             bool createTextureOnce=true,
                             core::scalemode sm = core::interpolateLIN){
        addTexture(a,b,c,d,texture,-1,-1,-1,-1,createTextureOnce,sm);
      }
  
      /// adds are shared texture primitive
      /** The sharedTextureIndex references a shared texture that has been added
          by using SceneObject::addSharedTexture */
      ICLGeom_API void addTexture(int a, int b, int c, int d,
                      int sharedTextureIndex,
                      int na=-1, int nb=-1, int nc=-1, int nd=-1);
  
      /// adds a GenericTexturePrimitive for custom texCoords
      ICLGeom_API void addTexture(const core::ImgBase *image, int numPoints, const int *vertexIndices,
                      const utils::Point32f *texCoords, const int *normalIndices = 0,
                      bool createTextureOnce=true);
                 
  
      /// adds a texture that is drawn on a 2D grid of vertices in 3D space
      ICLGeom_API void addTextureGrid(int w, int h, const core::ImgBase *image,
                          const icl32f *px, const icl32f *py, const icl32f *pz,
                          const icl32f *pnx=0, const icl32f *pny=0, const icl32f *pnz=0,
                          int stride = 1,bool createTextureOnce=true,core::scalemode sm=core::interpolateLIN);
  
      /// adds a texture grid that has two different texture for the two faces
      /** Internally, the TwoSidedTextureGridPrimitive is used */
      ICLGeom_API void addTwoSidedTextureGrid(int w, int h, const core::ImgBase *front, const core::ImgBase *back,
                             const icl32f *px, const icl32f *py, const icl32f *pz,
                             const icl32f *pnx=0, const icl32f *pny=0, const icl32f *pnz=0,
                             int stride = 1,bool createFrontOnce=true,
                                  bool createBackOnce=true, core::scalemode sm=core::interpolateLIN);
  
      
      /// adds a two sided grid (sides may have different colors)
      ICLGeom_API void addTwoSidedTGrid(int w, int h, const Vec *vertices, const Vec *normals = 0,
                            const GeomColor &frontColor=GeomColor(0,100,255,255), 
                            const GeomColor &backColor=GeomColor(255,0,100,255),
                            const GeomColor &lineColor=GeomColor(0,255,100,255),
                            bool drawLines=false, bool drawQuads=true);
  
  
      /// adds text-texture quad -primitive to this object
      /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product.
          Please note, that the text aspect ratio might not be preserved 
	  @param a
	  @param b
	  @param c
	  @param d
	  @param text
	  @param color
	  @param na
	  @param nb
	  @param nc
	  @param nd
	  @param textSize the text size
	  @param sm the scale mode
*/
      ICLGeom_API void addTextTexture(int a, int b, int c, int d, const std::string &text,
                          const GeomColor &color, 
                          int na, int nb, int nc, int nd,
                          int textSize,core::scalemode sm = core::interpolateLIN);
                          
  
      /// convenience method for creation of a text-texture with auto-normals
      inline void addTextTexture(int a, int b, int c, int d, const std::string &text,
                          const GeomColor &color=GeomColor(255,255,255,255), 
                                 int textSize=30, core::scalemode sm = core::interpolateLIN){
        addTextTexture(a,b,c,d,text,color,-1,-1,-1,-1,textSize, sm);
      }
      
      /// adds a billboard text-texture attached to given node index a
      /** the billboardHeight parameters defines the actual height in world
          units. The text is always centered at m_vertices[a] and it will
          always be oriented towards the camera. The textRenderSize parameter
          defines the pixel resolution of the text
      */
      ICLGeom_API void addText(int a, const std::string &text, float billboardHeight = 10,
                   const GeomColor &color=GeomColor(255,255,255,255),
                   int textRenderSize=30, core::scalemode sm=core::interpolateLIN);
  
      /// adds a custom primitive
      /** This should only be used for non-directly supported primitives 
          Note: right now, there is no 'hit' checking for non standard primitives
      */
      inline void addCustomPrimitive(Primitive *p){
        m_primitives.push_back(p);
      }
      
      /// adds a cube child-object with given parameters
      /** returns a pointer to the cube added. This can be used to adapt
          further properties of that object */
      SceneObject *addCube(float x, float y, float z, float d){
        return addCuboid(x,y,z,d,d,d);
      }
  
      /// adds a cuboid child-object with given parameters
      /** returns a pointer to the cube added. This can be used to adapt
          further properties of that object */
      ICLGeom_API SceneObject *addCuboid(float x, float y, float z, float dx, float dy, float dz);
  
      /// adds a cuboid child-object with given parameters
      /** returns a pointer to the cube added. This can be used to adapt
          further properties of that object */
      SceneObject *addSphere(float x, float y, float z, float r,int rzSteps, int xySlices){
        return addSpheroid(x,y,z,r,r,r,rzSteps,xySlices);
      }
  
      /// adds a cuboid child-object with given parameters
      /** returns a pointer to the cube added. This can be used to adapt
          further properties of that object */
      ICLGeom_API SceneObject *addSpheroid(float x, float y, float z, float rx, float ry, float rz, int rzSteps, int xySlices);
      
      /// adds a cylindical child object with given parameters
      /** returns a pointer to the cylinder added. This can be used to adapt
          further properties of that object */
      ICLGeom_API SceneObject *addCylinder(float x, float y, float z, float rx, float ry, float h, int steps);
  
      /// adds a conical child object with given parameters
      /** returns a pointer to the cone added. This can be used to adapt
          further properties of that object */
      ICLGeom_API SceneObject *addCone(float x, float y, float z, float rx, float ry, float h, int steps);
  
      
      /// tints all Primitives with given type in given color
      ICLGeom_API void setColor(Primitive::Type t, const GeomColor &color, bool recursive = true);
      
      /// sets point size
      ICLGeom_API void setPointSize(float pointSize, bool recursive = true);
  
      /// sets point size
      ICLGeom_API void setLineWidth(float lineWidth, bool recursive = true);

      /// if set, only custom render is used 
      ICLGeom_API void setUseCustomRender(bool use, bool recursive = true);   
      
      /// performs a deep copy of this object
      ICLGeom_API virtual SceneObject *copy() const;
  
      /// called by the renderer before the object is rendered
      /** here, dynamic object types can adapt e.g. their vertices or colors*/
      virtual void prepareForRendering() {}
  
      /// this function is called when an object is rendered
      /** The function can be used to draw something in Object coordinates using
          OpenGL commands directly. When customRender is called, the OpenGL matrices is 
          already prepared correctly. Custom render is always called <b>before</b> 
          the SceneObject's primitives are rendered */
      virtual void customRender() {}
      
      /// this function is called when an object is rendered
      /** The function can be used to draw something in Object coordinates using
          OpenGL commands directly. When complexCustomRender is called, the OpenGL matrices is 
          already prepared correctly. Custom render is always called <b>before</b> 
          the SceneObject's primitives are rendered. The complexCustomRender function is given
          a ShaderUtil to enable rendering with the complex shaders from the Scene. */
      virtual void complexCustomRender(icl::geom::ShaderUtil* util) {customRender();}
  
      /// sets how 2D-geom colors are set 
      ICLGeom_API void setColorsFromVertices(Primitive::Type t, bool on, bool recursive = true);
      
      /// returns wheather smooth shading is activated
      ICLGeom_API bool getSmoothShading() const;
      
      /// sets whether to use smoothshading (default is false)
      ICLGeom_API void setSmoothShading(bool on, bool recursive = true);
  
  
      /** @{ @name methods for creation of a scene graph **/
      /// Sets a transformation matrix 
      /** All vertices are transformed with this matrix before rendering. If the
          SceneObject instance has a parent-Scene object, then the parent's
          SceneObject's transformation pre-multiplied */
      ICLGeom_API virtual void setTransformation(const Mat &m);
      
      /// sets the internal transformation to the identity matrix
      ICLGeom_API void removeTransformation();
  
      /// multiplies the current transformation matrix by given matrix 
      ICLGeom_API virtual void transform(const Mat &m);
  
      /// utility method for passing arbitrary matrix classes 
      /** Note: the given T instance m, needs to have a function-operator(x,y)*/
      template<class T>
      void transform(const T &m){
        transform(Mat(m(0,0),m(1,0),m(2,0),m(3,0),
                      m(0,1),m(1,1),m(2,1),m(3,1),
                      m(0,2),m(1,2),m(2,2),m(3,2),
                      m(0,3),m(1,3),m(2,3),m(3,3)));
      } 
      
      /// rotates the scene object (this affects it's transformation matrix)
      ICLGeom_API virtual void rotate(float rx, float ry, float rz, 
                          icl::math::AXES axes=icl::math::AXES_DEFAULT);
  
      /// utility wrapper for vector based rotation 
      template<class T>
      inline void rotate(const T &t, icl::math::AXES axes=icl::math::AXES_DEFAULT) 
        { rotate((float)t[0],(float)t[1],(float)t[2]); }
      
      /// translates the scene object (this affects it's translates matrix)
      ICLGeom_API virtual void translate(float dx, float dy, float dz);
  
      /// utility wrapper for vector based translation
      template<class T>
      inline void translate(const T &t) { translate((float)t[0],(float)t[1],(float)t[2]); }
  
      /// transformes the current transformation matrix by a scale matrix
      ICLGeom_API virtual void scale(float sx, float sy, float sz);
      
      /// utility wrapper for vector based scaling
      template<class T>
      inline void scale(const T &t) { scale((float)t[0],(float)t[1],(float)t[2]); }
  
      /// returns the current transformation matrix
      /** If the relative flag is true, only this objects transformation matrix is returned.
          If it is set to false (which is default), also the parent SceneObjects absolute
          transformation matrix is queried and premultiplied */
      ICLGeom_API Mat getTransformation(bool relative = false) const;
      
      /// returns whether the SceneObject has currently a non-ID-transformation
      /** Here also the parent transformation is regarded if relative is false */
      ICLGeom_API bool hasTransformation(bool relative = false) const;
      
      /// returns the parent scene object
      ICLGeom_API SceneObject *getParent();
      
      /// returns the parent scene object (const version)
      ICLGeom_API const SceneObject *getParent() const;
  
      /// adds a new child to this scene object
      /** If the child's owner ship is passed, it is deleted automatically when it is removed or
          if the parent is deleted. Otherwise, the caller has to manage the passed child's 
          memory. 
          <b>Note:</b> there is no cycle detection in the SceneObject class. Adding A to B and
          B to A leads to unknown results and most likely to programm errors.
      */
      ICLGeom_API void addChild(SceneObject *child, bool passOwnerShip = true);
      
      /// directly passes a smart pointer as a child
      /** By passing a smart pointer to an object, pointer-sharing can
          also be extended to the caller scope.
          Please note, that adding a child to an object o, will always set
          the child's parent to o */
      void addChild(utils::SmartPtr<SceneObject> child);
      
      /// removes given child
      /** no errors if the child was not found */
      ICLGeom_API void removeChild(SceneObject *child);
  
      /// removes all children
      ICLGeom_API void removeAllChildren();
      
      /// returns whether the SceneObject has children at all
      ICLGeom_API bool hasChildren() const;
      
      /// returns the number of children
      ICLGeom_API int getChildCount() const;
      
      /// returns child at given index
      ICLGeom_API SceneObject *getChild(int index);

      /// returns child at given index (const)
      ICLGeom_API const SceneObject *getChild(int index) const;

      /// returns a shared pointer to the child at given index
      utils::SmartPtr<SceneObject> getChildPtr(int index);
      
      /// returns whether the given object is a child of this one
      ICLGeom_API bool hasChild(const SceneObject *o) const;
      /** @} **/

      /// automatically creates precomputed normals
      /** in smooth mode, for each vertex a surface normal is created.
          The normals are defined by the mean normal of all adjacent
          faces (triangles and quads). This does only work for
          sphere-like object, where a "mean normal" makes sense.
          This does not make sense for a cube.
          
          The non-smooth mode pre-computes normals for each 
          face, but here, each face-vertex uses the same normal
          leading to a flat-shaded result. The non-smooth-mode
          looks identical to the Primitives automatically created
          online normals, but reduces processor load.
      */
      ICLGeom_API void createAutoNormals(bool smooth = true);
      
      /// can be reimplemented by subclass to provide and interface for setting default vertex color
      /** The default vertex color is used if no color information is available (m_vertexColors.size()
          is 0 */
      virtual GeomColor getDefaultVertexColor() const{ return GeomColor(255,0,0,255); }
      
      /// returns whether this object is hit by the given viewray
      /** Please note: only faces (i.e. quads, triangles and polygons
          are checked)
          The method returns the hit scene object that was closest to
          the given view-rays origin or null, if it was not hit.
          If recursive is true, the scene-graph is traversed from this
          object on and the actually hit child (or child of child etc.) 
          might also be returned.
      */
      ICLGeom_API Hit hit(const ViewRay &v, bool recursive = true);
      
      /// returns whether this object is hit by the given viewray (const)
      const Hit hit(const ViewRay &v, bool recursive=true) const{
        return const_cast<SceneObject*>(this)->hit(v,recursive);
      }
      
      /// returns all hits with SceneObjects form the given viewray
      ICLGeom_API std::vector<Hit> hits(const ViewRay &v, bool recursive = true);
  
      /// returns all vertices in their final world coordinates
      ICLGeom_API std::vector<Vec> getTransformedVertices() const;
      
      /// returns the vertex, that is closest to the given point in wold coordinates
      /** If relative is true, the vertex is returned in object-coordinates, otherwise
          it is returned in world coordinates */
      ICLGeom_API Vec getClosestVertex(const Vec &pWorld, bool relative = false) throw (utils::ICLException);
      
      /// sets the visibility of this object
      ICLGeom_API void setVisible(bool visible, bool recursive = true);
      
      /// returns whether this object is currently visible
      bool isVisible() const { return m_isVisible; }
  
      /// calls setVisible(false)
      void hide(bool recursive=true){ setVisible(false); }
  
      /// calls setVisible(true)
      void show(bool recursive=true){ setVisible(true); }
  
  
      /// sets locking enabled or disabled
      /** Note, that the method itself locks the internal mutex
          to prevent, that m_enableLocking is disabled while
          the mutex is locked somewhere else */
      inline void setLockingEnabled(bool enabled) { 
        m_mutex.lock();
        m_enableLocking = enabled; 
        m_mutex.unlock();
      }
      
      /// returns whether locking is current enabled for this object
      bool getLockingEnabled() const {
        return m_enableLocking;
      }
  
      /// locks the internal mutex if locking enabled is set to true
      /** This function can be re implemented by subclasses that need an eplicit locking.
          Note, that explicit locking can be enabled/disabled using setLockingEnabled\n 
          E.g. if an objects data is updated from another thread, you can sub-class 
          this class and implement a locking mechanism for it*/
      virtual void lock() const{
        if(!m_enableLocking) return;
        m_mutex.lock();
      }
      
      /// unlocks the internal mutex if locking enabled is set to true
      /** This function can be re implemented by subclasses that need an eplicit locking.
          Note, that explicit locking can be enabled/disabled using setLockingEnabled\n 
          E.g. if an objects data is updated from another thread, you can sub-class 
          this class and implement a locking mechanism for it*/
      virtual void unlock() const{
        if(!m_enableLocking) return;
        m_mutex.unlock();
      }
      
      friend struct Primitive;
      
  
      /// sets whether points are visualized in a smoothed manner
      /** This might not be supported by the graphics hardware or driver.
          Default value is true */
      inline void setPointSmoothingEnabled(bool enabled=true){
        m_pointSmoothingEnabled = enabled;
      }
  
      /// sets whether lines are visualized in a smoothed manner
      /** This might not be supported by the graphics hardware or driver
          Default value is true */
      inline void setLineSmoothingEnabled(bool enabled=true){
        m_lineSmoothingEnabled = enabled;
      }
   
      /// sets whether faces are visualized in a smoothed manner
      /** This might not be supported by the graphics hardware or driver
          Default value is true */
      inline void setPolygonSmoothingEnabled(bool enabled=true){
        m_polygonSmoothingEnabled = enabled;
      }
  
      /// deletes and removes all primitives
      ICLGeom_API void clearAllPrimitives();
      
      /// deletes all primitive and all vertex, color and normal content (and optionally also the children)
      ICLGeom_API virtual void clearObject(bool deleteAndRemoveChildren=true, bool resetTransform=false);
      
      /// creates a displaylist in the next render cycle
      /** if the displaylist was already created, it is updated */
      ICLGeom_API void createDisplayList();
      
      /// frees the displaylist in the next render cycle
      ICLGeom_API void freeDisplayList();
      
      /// sets a fragment shader to use for this object
      /** use set fragment shader (0) in order to delete the fragment shader */
      ICLGeom_API void setFragmentShader(qt::GLFragmentShader *shader);
      
      /// returns the current fragment shader (or NULL if non was given)
      inline qt::GLFragmentShader *getFragmentShader() { return m_fragmentShader; }
      
      /// returns the current fragment shader (or NULL if non was given, const version)
      inline const qt::GLFragmentShader *getFragmentShader() const{ return m_fragmentShader; }

      inline void setCastShadowsEnabled(bool castShadows = true) { m_castShadows = castShadows; }

      inline bool getCastShadowsEnabled() { return m_castShadows; }

      inline void setReceiveShadowsEnabled(bool receiveShadows = true) { m_receiveShadows = receiveShadows; }

      inline bool getReceiveShadowsEnabled() { return m_receiveShadows; }

      /// sets the material shininess (default is 128)
      inline void setShininess(icl8u value){
        m_shininess = value;
      }
      
      /// sets the materials specular reflectance
      /** given color ranges are expected in range [0,255] */
      inline void setSpecularReflectance(const GeomColor &values){
        m_specularReflectance = values*(1.0/255);
      }
      
      /// returns whether depth test is enabled for this object
      inline bool getDepthTestEnabled() const{ return m_depthTestEnabled;  }
      
      /// sets whether depth test is enabled for this object
      inline void setDepthTestEnabled(bool enabled, bool recursive = true){
        m_depthTestEnabled = enabled;
        if(recursive) {
            for(int i = 0; i < this->getChildCount(); i++) {
                this->getChild(i)->setDepthTestEnabled(enabled,true);
            }
        }
      }

      /// returns the maximum distance to a pointing viewraw 
      /** When calling scene.find(ViewRay) in order to click at an object,
          all object primitives are checked for the test. In addition, 
          all object vertices are also checked here, but since here,
          an actual hit is statistically impossible, the "pointHitMaxDistance 
          property is used **/
      inline float getPointHitMaxDistance() const {
        return m_pointHitMaxDistance;
      }

      /// sets the maximum distance to a pointing view-ray
      /** @see getPointHitMaxDistance() */
      inline void setPointHitMaxDistance(float d){
        m_pointHitMaxDistance = d;
      }

      protected:
      /// recursive picking method
      static void collect_hits_recursive(SceneObject *obj, const ViewRay &v, 
                                         std::vector<Hit> &hits, 
                                         bool recursive);
      
      std::vector<Vec> m_vertices;
      std::vector<Vec> m_normals;
      
      std::vector<GeomColor> m_vertexColors;
      std::vector<Primitive*> m_primitives;
      std::vector<utils::SmartPtr<qt::GLImg> > m_sharedTextures;
      int m_visibleMask;
  
      bool m_lineColorsFromVertices;
      bool m_triangleColorsFromVertices;
      bool m_quadColorsFromVertices;
      bool m_polyColorsFromVertices;

      bool m_useCustomRender;
  
      float m_pointSize;
      float m_lineWidth;
      
      bool m_useSmoothShading;
      bool m_isVisible;
      
      /// for the scene graph implementation
      Mat m_transformation;
      bool m_hasTransformation;
      SceneObject *m_parent;
      std::vector<utils::SmartPtr<SceneObject> > m_children;
  
      mutable utils::Mutex m_mutex; //!< for asynchronous updates
      bool m_enableLocking; //!< can be enabled
  
      bool m_pointSmoothingEnabled;
      bool m_lineSmoothingEnabled;
      bool m_polygonSmoothingEnabled;
      bool m_depthTestEnabled; //!< default is true
      
      icl8u m_shininess;
      GeomColor m_specularReflectance;

      private:
  
      /// internally used flag
      void *m_displayListHandle;
      /// internal flag
      /** - 0: no change.
          - 1: create/update display list in next render cycle 
          - 2: free display-list (if there is any) in the next render cycle */
      int m_createDisplayListNextTime;
  
      /// internal optionally given fragment shader
      qt::GLFragmentShader *m_fragmentShader;
      
      bool m_castShadows;
      bool m_receiveShadows;

      float m_pointHitMaxDistance;
    };
  } // namespace geom
}

