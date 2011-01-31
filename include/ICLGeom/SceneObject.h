/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/SceneObject.h                          **
** Module : ICLGeom                                                **
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

#ifndef ICL_SCENE_OBJECT_H
#define ICL_SCENE_OBJECT_H

#include <ICLGeom/Primitive.h>
#include <ICLGeom/ViewRay.h>

namespace icl{

  /** \cond */
  class Scene;
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
      
      \section DYN Dynamic SceneObjects
      Custome extensions of the SceneObject-interface can implement the SceneObject's 
      virtual method SceneObject::prepareForRendering which is calle every time before
      the object is acutally rendered. Here, the custom SceneObject can be adapted 
      dynamically. \n
      <b>Please note:</b> When then you want to change the vertex-, primitive- or 
      children count at runtime, you have to implement the virtual methods 
      SceneObject::lock() and SceneObject::unlock() appropriately. Usually this will
      look like this:
      \code
      class MySceneObject : public SceneObject{
        Mutex mutex;
        public:
        void lock() { mutex.lock(); }
        void unlock() { mutex.unlock(); }
        ...
      };
      \endcode

      \section CFV Colors From Vertices
      Sometimes, you might want to draw primtives that use different colors 
      for different corners and interpolate between these. This can be achieved
      by using SceneObject::setColorsFromVertices(true). 
  */
  class SceneObject{
    public:
    
    /// provides direct access for the Scene class
    friend class Scene;
    
    /// create an object
    SceneObject();
    
    /// create by string:
    /** currently allowed:
        "cube" params: [x,y,z,radius];
        "cuboid" params: [x,y,z,dx,dy,dz]
        "sphere" params: [x,y,z,radius,rzSteps,xySlices]
        "spheroid" params: [x,y,z,rx,ry,rz,rzSteps,xySlices]
    */
    SceneObject(const std::string &type,const float *params);


    /// creates a scene object from given .obj file
    SceneObject(const std::string &objFileName) throw (ICLException);

    /// deep copy of SceneObject instance
    /** The new instance's parent is set to null, i.e. it must
        be added to other's parent explicitly if this is necessary. */
    SceneObject(const SceneObject &other) { 
      *this = other; 
      m_parent = 0;
    }

    /// assignment operator for deep copy
    /** This instances parent is not changed. I.e. it must
        be added to other's parent explicitly if this is necessary. */
    SceneObject &operator=(const SceneObject &other);
    
    /// Empty destructor (but virtual)
    virtual ~SceneObject(){}
    
    /// returns object vertices
    std::vector<Vec> &getVertices();

    /// returns object vertices (const)
    const std::vector<Vec> &getVertices() const;

    /// returns object's primitives (lines, quads, etc...)
    std::vector<Primitive> &getPrimitives();

    /// returns object's primitives (lines, quads, etc...) (const)
    const std::vector<Primitive> &getPrimitives() const;

    /// changes visibility of given primitive type
    void setVisible(Primitive::Type t, bool visible, bool recursive=true);
    
    /// returns visibility of given primitive type
    bool isVisible(Primitive::Type t) const;
    
    /// adds a new vertex to this object
    void addVertex(const Vec &p, const GeomColor &color=GeomColor(255,0,0,255));
    
    /// adds a new normal to this object
    void addNormal(const Vec &n);
    
    /// adds a new line to this object
    /** If the given normal indices (na and nb) are -1, no normals are used for this primitives */
    void addLine(int x, int y, int na, int nb, const GeomColor &color=GeomColor(100,100,100,255));

    /// convenience method for creation of a line with auto-normals
    inline void addLine(int x, int y, const GeomColor &color=GeomColor(100,100,100,255)){
      addLine(x,y,-1,-1,color);
    }
    
    /// adds a new triangle to this onject
    /** If the given normal indices (na,nb and nc) are -1, auto-normal are computed using cross-product */
    void addTriangle(int a, int b, int c, int na, int nb, int nc,
                     const GeomColor &color=GeomColor(0,100,250,255));
    
    /// convenience method for creation of a triangle with auto-normals
    inline void addTriangle(int a, int b, int c, const GeomColor &color=GeomColor(0,100,250,255)){
      addTriangle(a,b,c,-1,-1,-1,color);
    }


    /// adds a new triangle to this onject
    /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product */
    void addQuad(int a, int b, int c, int d, int na, int nb, int nc, int nd, 
                 const GeomColor &color=GeomColor(0,100,250,255)); 

    /// convenience method for creation of a quad with auto-normals
    inline void addQuad(int a, int b, int c, int d, const GeomColor &color=GeomColor(0,100,250,255)){
      addQuad(a,b,c,d,-1,-1,-1,-1,color);
    }

    /// add a polygon to this object (note triangles and quads are slower here)
    /** If the given normal indices's size is 0, auto-normal are computed using cross-product */
    void addPolygon(const std::vector<int> &vertexIndices, 
                    const std::vector<int> &normalIndices,
                    const GeomColor &color=GeomColor(0,100,250,255)); 
    
    /// convenience method for creation of a polygon with auto-normals
    inline void addPolygon(const std::vector<int> &vertexIndices, 
                           const GeomColor &color=GeomColor(0,100,250,255)){
      addPolygon(vertexIndices,std::vector<int>(),color);
    }

    
    /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product */
    void addTexture(int a, int b, int c, int d, 
                    const Img8u &texture, 
                    int na, int nb, int nc, int nd,
                    bool deepCopy=false);

    /// convenience method for creation of a texture with auto-normals
    inline void addTexture(int a, int b, int c, int d, 
                           const Img8u &texture, 
                           bool deepCopy=false){
      addTexture(a,b,c,d,texture,-1,-1,-1,-1,deepCopy);
    }

    /// adds text-texture quad -primitive to this object
    /** If the given normal indices (na,nb,nc and nd) are -1, auto-normal are computed using cross-product.
        Please note, that the text aspect ratio might not be preserved 
        @param holdTextAR not supported yet! */
    void addTextTexture(int a, int b, int c, int d, const std::string &text,
                        const GeomColor &color, 
                        int na, int nb, int nc, int nd,
                        int textSize, bool holdTextAR);

    /// convenience method for creation of a text-texture with auto-normals
    inline void addTextTexture(int a, int b, int c, int d, const std::string &text,
                        const GeomColor &color=GeomColor(255,255,255,255), 
                        int textSize=30, bool holdTextAR=true){
      addTextTexture(a,b,c,d,text,color,-1,-1,-1,-1,textSize,holdTextAR);
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
    SceneObject *addCuboid(float x, float y, float z, float dx, float dy, float dz);

    /// adds a cuboid child-object with given parameters
    /** returns a pointer to the cube added. This can be used to adapt
        further properties of that object */
    SceneObject *addSphere(float x, float y, float z, float r,int rzSteps, int xySlices){
      return addSpheroid(x,y,z,r,r,r,rzSteps,xySlices);
    }

    /// adds a cuboid child-object with given parameters
    /** returns a pointer to the cube added. This can be used to adapt
        further properties of that object */
    SceneObject *addSpheroid(float x, float y, float z, float rx, float ry, float rz, int rzSteps, int xySlices);
    
    /// tints all Primitives with given type in given color
    void setColor(Primitive::Type t,const GeomColor &color,bool recursive=true);
    
    /// sets point size
    void setPointSize(float pointSize, bool recursive=true);

    /// sets point size
    void setLineWidth(float lineWidth, bool recursive=true);

    /// sets the interpolation mode for textures
    /** allowed values are interpolateNN and interpolateLIN*/
    void setTextureInterpolation(scalemode mode, bool recursive=true) throw (ICLException);
    
    /// this function can be implemented by subclasses that need an eplicit locking
    /** E.g. if an objects data is updated from another thread, you can sub-class 
        this class and implement a locking mechanism for it*/
    virtual void lock(){}

    /// this function can be implemented by subclasses that need an eplicit locking
    /** E.g. if an objects data is updated from another thread, you can sub-class 
        this class and implement a locking mechanism for it*/
    virtual void unlock(){}
    
    /// performs a deep copy of this object
    virtual SceneObject *copy() const;

    /// called by the renderer before the object is rendered
    /** here, dynamic object types can adapt e.g. their vertices or colors*/
    virtual void prepareForRendering() {}

    /// sets how 2D-geom colors are set 
    void setColorsFromVertices(Primitive::Type t, bool on, bool recursive=true);
    
    /// returns wheather smooth shading is activated
    bool getSmoothShading() const;
    
    /// sets whether to use smoothshading (default is false)
    void setSmoothShading(bool on, bool recursive=true);


    /** @{ @name methods for creation of a scene graph **/
    /// Sets a transformation matrix 
    /** All vertices are transformed with this matrix before rendering. If the
        SceneObject instance has a parent-Scene object, then the parent's
        SceneObject's transformation pre-multiplied */
    void setTransformation(const Mat &m);
    
    /// sets the internal transformation to the identity matrix
    void removeTransformation();

    /// multiplies the current transformation matrix by given matrix 
    void transform(const Mat &m);

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
    void rotate(float rx, float ry, float rz);

    /// utility wrapper for vector based rotation 
    template<class T>
    inline void rotate(const T &t) { rotate((float)t[0],(float)t[1],(float)t[2]); }
    
    /// translates the scene object (this affects it's translates matrix)
    void translate(float dx, float dy, float dz);

    /// utility wrapper for vector based translation
    template<class T>
    inline void translate(const T &t) { translate((float)t[0],(float)t[1],(float)t[2]); }

    /// transformes the current transformation matrix by a scale matrix
    void scale(float sx, float sy, float sz);
    
    /// utility wrapper for vector based scaling
    template<class T>
    inline void scale(const T &t) { scale((float)t[0],(float)t[1],(float)t[2]); }

    /// returns the current transformation matrix
    /** If the relative flag is true, only this objects transformation matrix is returned.
        If it is set to false (which is default), also the parent SceneObjects absolute
        transformation matrix is queried and premultiplied */
    Mat getTransformation(bool relative=false) const;
    
    /// returns whether the SceneObject has currently a non-ID-transformation
    /** Here also the parent transformation is regarded if relative is false */
    bool hasTransformation(bool relative=false) const;
    
    /// returns the parent scene object
    SceneObject *getParent();
    
    /// returns the parent scene object (const version)
    const SceneObject *getParent() const;

    /// adds a new child to this scene object
    /** If the child's owner ship is passed, it is deleted automatically when it is removed or
        if the parent is deleted. Otherwise, the caller has to manage the passed child's 
        memory. 
        <b>Note:</b> there is no cycle detection in the SceneObject class. Adding A to B and
        B to A leads to unknown results and most likely to programm errors.
    */
    void addChild(SceneObject *child, bool passOwnerShip=true);
    
    /// removes given child
    /** no errors if the child was not found */
    void removeChild(SceneObject *child);

    /// removes all children
    void removeAllChildren();
    
    /// returns whether the SceneObject has children at all
    bool hasChildren() const;
    
    /// returns the number of children
    int getChildCount() const;
    
    /// returns child at given index
    SceneObject *getChild(int index);
    
    /// returns child at given index (const)
    const SceneObject *getChild(int index) const;
    
    /** @} **/

    /// returns whether this object is hit by the given viewray
    /** If contactPos is not 0, it is set to the actual contact 
        pos with the given primitive 
        Please note: only faces (i.e. quads, triangles and polygons
        are checked)
        The method returns the hit scene object or null, if it was not hit.
        If recursive is true, the scene-graph is traversed from this
        object on and the actually hit child (or child of child etc.) 
        might also be returned.
    */
    SceneObject *hit(const ViewRay &v, Vec *contactPos=0, bool recursive=true);
    
    /// returns whether this object is hit by the given viewray (const)
    const SceneObject *hit(const ViewRay &v, Vec *contactPos=0, bool recursive=true) const{
      return const_cast<SceneObject*>(this)->hit(v,contactPos,recursive);
    }
    
    /// returns all vertices in their final world coordinates
    std::vector<Vec> getTransformedVertices() const;
    
    /// returns the vertex, that is closest to the given point in wold coordinates
    /** If relative is true, the vertex is returned in object-coordinates, otherwise
        it is returned in world coordinates */
    Vec getClosestVertex(const Vec &pWorld, bool relative=false) throw (ICLException);
    
    protected:
    
    /// recursive picking method
    static void collect_hits_recursive(SceneObject *obj, const ViewRay &v, 
                                       std::vector<Vec> &hits, 
                                       std::vector<SceneObject*> &objects,
                                       bool recursive);
    
    std::vector<Vec> m_vertices;
    std::vector<Vec> m_normals;
    
    std::vector<GeomColor> m_vertexColors;
    std::vector<Primitive> m_primitives;
    bool m_visible[Primitive::PRIMITIVE_TYPE_COUNT];

    bool m_lineColorsFromVertices;
    bool m_triangleColorsFromVertices;
    bool m_quadColorsFromVertices;
    bool m_polyColorsFromVertices;

    float m_pointSize;
    float m_lineWidth;
    
    bool m_useSmoothShading;
    
    /// for the scene graph implementation
    Mat m_transformation;
    bool m_hasTransformation;
    SceneObject *m_parent;
    std::vector<SmartPtr<SceneObject> > m_children;
  };
}

#endif
