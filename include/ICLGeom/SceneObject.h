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
      Normals are used for realistic lighting. Therefore, it is recommended to use one
      of the provided NormalMode-modes. <b>Please note:</b> In you custom extension 
      of the SceneObject class you have to set the normal mode appropriately <b>before</b>
      you add vertices and primitives. The normal-mode can only be set in a SceneObject's
      implementation using the protected method SceneObject::setNormalMode.
      
      \subsection AN AutoNormals
      If the AutoNormals mode is activated (which is also be used by default), the normals
      passed to SceneObject::addVertex and the other primitive methods like
      SceneObject::addTriangle or SceneObject::addQuad are not stored internally. In this
      mode normals are created by the following method:
      
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
    
    /// Enermeration for normal creation modes
    enum NormalMode{
      AutoNormals,      //!< normals are generated automatically using cross-product
      NormalsPerVertex, //!< normals are defined per vertex
      NormalsPerFace    //!< normals are defined per face
    };
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
    /** The given normal is only used if the normal mode is set to NormalsPerVertex */
    void addVertex(const Vec &p, const GeomColor &color=GeomColor(255,0,0,255), const Vec &normal=Vec(0.0f));
    
    /// adds a new line to this object
    /** The given normal is only used if the normal mode is set to NormalsPerFace */
    void addLine(int x, int y, const GeomColor &color=GeomColor(100,100,100,255), const Vec &normal=Vec(0.0f));
    
    /// adds a new triangle to this onject
    /** The given normal is only used if the normal mode is set to NormalsPerFace */
    void addTriangle(int a, int b, int c, const GeomColor &color=GeomColor(0,100,250,255),const Vec &normal=Vec(0.0f));

    /// adds a new triangle to this onject
    /** The given normal is only used if the normal mode is set to NormalsPerFace */
    void addQuad(int a, int b, int c, int d, const GeomColor &color=GeomColor(0,100,250,255), const Vec &normal=Vec(0.0f)); 

    /// add a polygon to this object (note triangles and quads are slower here)
    /** The given normal is only used if the normal mode is set to NormalsPerFace */
    void addPolygon(const std::vector<int> &vertexIndices, const GeomColor &color=GeomColor(0,100,250,255), const Vec &normal=Vec(0.0f)); 
    
    /// adds a textured quad to this object
    void addTexture(int a, int b, int c, int d, const Img8u &texture, bool deepCopy=false);

    /// adds text-texture quad -primitive to this object
    /** Please note, that the text aspect ratio might not be preserved 
        @param holdTextAR not supported yet! */
    void addTextTexture(int a, int b, int c, int d, const std::string &text,
                        const GeomColor &color=GeomColor(255,255,255,255), 
                        int textSize=30, bool holdTextAR=true);

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
    
    /// returns the Objects normal mode
    NormalMode getNormalMode() const;
    
    protected:
    
    void setNormalMode(NormalMode mode);
    
    std::vector<Vec> m_vertices;

    NormalMode m_normalMode;
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
