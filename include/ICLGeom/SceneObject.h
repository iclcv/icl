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
 
  class SceneObject{
    public:
    
    /// scene2 is able to work directly with object's data
    friend class Scene;
    
    /// create an object
    SceneObject();
    
    /// create by string:
    /** currently allowed:
        "cube" params: [x,y,z,radius];
        "cuboid" params: [x,y,z,dx,dy,dz]
        "sphere" params: [x,y,z,radius,slices,steps]
    */
    SceneObject(const std::string &type,const float *params);
    
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
    void setVisible(Primitive::Type t, bool visible);
    
    /// returns visibility of given primitive type
    bool isVisible(Primitive::Type t) const;
    
    /// adds a new vertex to this object
    void addVertex(const Vec &p, const GeomColor &color=GeomColor(255,0,0,255));
    
    /// adds a new line to this object
    void addLine(int x, int y, const GeomColor &color=GeomColor(100,100,100,255));
    
    /// adds a new triangle to this onject
    void addTriangle(int a, int b, int c, const GeomColor &color=GeomColor(0,100,250,255));

    /// adds a new triangle to this onject
    void addQuad(int a, int b, int c, int d, const GeomColor &color=GeomColor(0,100,250,255)); 
    
    /// adds a textured quad to this object
    void addTexture(int a, int b, int c, int d, const Img8u &texture, bool deepCopy=false);
    
    /// tints all Primitives with given type in given color
    void setColor(Primitive::Type t,const GeomColor &color);
    
    /// sets point size
    void setPointSize(float pointSize) { m_pointSize = pointSize; }

    /// sets point size
    void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }

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

    /// returns current z value estimation
    float getZ() const { return m_z; }

    /// calculates mean z of all primitives
    void updateZFromPrimitives();
    
    /// called by the renderer before the object is rendered
    /** here, dynamic object types can adapt e.g. their vertices or colors*/
    virtual void prepareForRendering() {}

    /// sets how 2D-geom colors are set 
    void setColorsFromVertices(Primitive::Type t, bool on);
    
    protected:
    std::vector<Vec> m_vertices;
    std::vector<GeomColor> m_vertexColors;
    std::vector<Primitive> m_primitives;
    bool m_visible[Primitive::PRIMITIVE_TYPE_COUNT];
    float m_z;

    bool m_lineColorsFromVertices;
    bool m_triangleColorsFromVertices;
    bool m_quadColorsFromVertices;

    float m_pointSize;
    float m_lineWidth;
  };
}

#endif
