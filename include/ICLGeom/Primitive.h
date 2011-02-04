/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Primitive.h                            **
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

#ifndef ICL_PRIMITIVE_H
#define ICL_PRIMITIVE_H

#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Img.h>

namespace icl{
  /// Storage class for 3D geometrical primitive used in the SceneObject class
  /** Instances of the class SceneObject consist basically of a set of vertices 
      and of a set of so called Primitive's. These are geometical primitives,
      that are basically defined by their type and by references to the parent
      SceneObject's vertices. E.g. a line-typed Primitive uses 2 vertex references
      (start- and end-position) 
  */
  struct Primitive{
    
    /// Primitive type flags
    enum Type{
      vertex,   //<! very basic type, that is currently not used since a SceneObjects vertices are treated in a special way
      line,     //<! line primitive (adressing two vertices -> start and end position of the line)
      triangle, //<! triange primitive (adressing three vertices)
      quad,     //<! quad primitve (adressing four vertices)
      polygon,  //<! polygon primitive (adressing at least 3 vertices)
      texture,  //<! texture primitive (using 4 vertices like a quad as textured rectangle, note: text has also type texture)
      nothing,  //<! intenally used type
      PRIMITIVE_TYPE_COUNT //<! also for internal use only
    };

    /// Base constructor creating an empty primitive
    Primitive();
    
    /// Line-Constructor
    Primitive(int a, int b, const GeomColor &color=GeomColor());

    /// Line-Constructor
    Primitive(int a, int b, const GeomColor &color, int na, int nb);

    /// Triangle constructor
    Primitive(int a, int b, int c, const GeomColor &color=GeomColor());
    
    /// Triangle constructor
    Primitive(int a, int b, int c, const GeomColor &color,int na, int nb, int nc);
    
    /// Quad constructor
    Primitive(int a, int b, int c, int d,const GeomColor &color=GeomColor());

    /// Quad constructor
    Primitive(int a, int b, int c, int d,const GeomColor &color, int na, int nb, int nc, int nd);
    
    /// texture constructor
    Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy=false, scalemode mode=interpolateNN);

    /// texture constructor
    Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy, scalemode mode,
              int na, int nb, int nc, int nd);
    
    /// Special constructor to create a texture primitive that contains 3D text
    /** There is not special TEXT-type: type remains 'texture' */
    Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
              int textSize=30, scalemode mode=interpolateNN);
    
    /// Special constructor to create a texture primitive that contains 3D text
    /** There is not special TEXT-type: type remains 'texture' */
    Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
              int textSize, scalemode mode, int na, int nb, int nc, int nd);
    

    /// Creates a polygon primitive
    Primitive(const std::vector<int> &polyData, const GeomColor &color=GeomColor());

    /// Creates a polygon primitive
    Primitive(const std::vector<int> &polyData, const GeomColor &color, const std::vector<int> &normalIndices);

    /// detaches deep copied textures
    void detachTextureIfDeepCopied();
    
    /// Creates a deep copy (in particular deep copy of the texture image)
    //Primitive(const Primitive &other);

    /// Creates a deep copy in assignment
    //Primitive &operator=(const Primitive &other);
    
    /// static utility method that allows for simple creation of text-textures
    static Img8u create_text_texture(const std::string &text,const GeomColor &color, int textSize=30);
    
    /// 4 vertex references for lines, quads triangles and textures
    int a() const { return vertexIndices[0]; }
    int b() const { return vertexIndices[1]; }
    int c() const { return vertexIndices[2]; }
    int d() const { return vertexIndices[3]; }

    int na() const { return normalIndices[0]; }
    int nb() const { return normalIndices[1]; }
    int nc() const { return normalIndices[2]; }
    int nd() const { return normalIndices[3]; }

    int &a() { return vertexIndices[0]; }
    int &b() { return vertexIndices[1]; }
    int &c() { return vertexIndices[2]; }
    int &d() { return vertexIndices[3]; }

    int &na() { return normalIndices[0]; }
    int &nb() { return normalIndices[1]; }
    int &nc() { return normalIndices[2]; }
    int &nd() { return normalIndices[3]; }
    
    /// vertex indices
    std::vector<int> vertexIndices;
    
    /// vertex normal indices -> if size is 0, auto-normals are created using cross-product
    std::vector<int> normalIndices;
    
    /// primitve color
    GeomColor color;
    
    /// optional texture
    Img8u tex;
    
    /// is the texture referenced or was is copied deeply
    bool texDeepCopied;
    
    /// primitve type
    Type type;
    
    /// interpolation state for textures
    scalemode mode;
    
    /// flag that indicates whether a normal was defined
    bool hasNormals;
  };
  
}

#endif
