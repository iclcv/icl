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
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Array2D.h>
#include <ICLQt/GLImg.h>

namespace icl{
  /** \cond */
  class SceneObject;
  /** \endcond */
  
  /// Abastract base type for geoemtric primitives 
  /** Primitives are atomar geometric entities, that are used
      to build SceneObjects. Primitives must only* define, how
      they are rendered in OpenGL. For rendering, the primitives
      can access the parent objects data such as vertices and
      normals. By these means, several primitives can share
      resources such as vertices, normals or even textures. 
      Usually primitives will just define vertex indices that
      are then used to pick the correct vertices from the 
      parent SceneObject's vertex list.\n
      *) only is not completely correct, they also have to 
         implement a deep copy interface \see Primitive::copy
      */
  struct Primitive{

    /// primitive type for dynamic handling of different primitives
    enum Type{
      vertex   = 1<<0, //<! vertex
      line     = 1<<1, //<! line primitive (adressing two vertices -> start and end position of the line)
      triangle = 1<<2, //<! triange primitive (adressing three vertices)
      quad     = 1<<3, //<! quad primitve (adressing four vertices)
      polygon  = 1<<4, //<! polygon primitive (adressing at least 3 vertices)
      texture  = 1<<5, //<! texture primitive (using 4 vertices like a quad as textured rectangle)
      text     = 1<<6, //<! text primitive (internally implmented as texture or as billboard)
      nothing  = 1<<7, //<! internally used type
      custom   = 1<<20, //<! for custom primitives
      PRIMITIVE_TYPE_COUNT = 8,                //<! also for internal use only
      all      = (1<<PRIMITIVE_TYPE_COUNT)-1,    //<! all types
      faces    = triangle | quad | polygon | texture | text
    };
  
    Type type;        //!< the primitive type
    GeomColor color;  //!< the color of this primitive
    
    /// accumulated context information for rendering primitives
    /** the RenderContext contains all render information from the parent SceneObject instance.
        The Scene automatically creates the RenderContext structure for each object and exposes
        this to it's primitives render() calls;
    **/
    struct RenderContext{
      const std::vector<Vec> &vertices;            //!< list of shared vertices
      const std::vector<Vec> &normals;             //!< list of shared normals
      const std::vector<GeomColor> &vertexColors;  //!< list of vertex colors
      const std::vector<SmartPtr<GLImg> > &sharedTextures; //!< list of shared textures
      bool lineColorsFromVertices;                 //!< line coloring
      bool triangleColorsFromVertices;             //!< triangle coloring
      bool quadColorsFromVertices;                 //!< quad coloring
      bool polygonColorsFromVertices;              //!< polygon coloring
      SceneObject *object;                         //!< the parent object
    };

    /// Default constructor
    Primitive(Type type=nothing, const GeomColor &color=GeomColor(255,255,255,255)):type(type),color(color){}

    /// virtual render method, which is called by the parent scene object
    virtual void render(const Primitive::RenderContext &ctx) = 0;
    
    /// must be implemented in order to obtain a deep and independent copy
    virtual Primitive *copy() const = 0;
  };
  
  /// line primitive (the line references 2 vertices)
  struct LinePrimitive : public FixedColVector<int,2>, public Primitive{
    /// super type
    typedef  FixedColVector<int,2> super; 
    
    /// constructor
    LinePrimitive(int a, int b, const GeomColor &color):
      FixedColVector<int,2>(a,b),Primitive(Primitive::line,color){}
      
    /// render
    virtual void render(const Primitive::RenderContext &ctx);

    /// direct access to the i-th vertex/normal index
    inline int i(int idx) const { return super::operator[](idx); }
    
    /// deep copy implementation (trivial)
    virtual Primitive *copy() const { return new LinePrimitive(*this); }
  };

  /// triangle primitive
  struct TrianglePrimitive : public FixedColVector<int,6>, public Primitive{
    /// super type
    typedef  FixedColVector<int,6> super; 
    
    /// constructor
    TrianglePrimitive(int a, int b, int c, const GeomColor &color, int na=-1, int nb=-1, int nc=-1):
    super(a,b,c,na,nb,nc),Primitive(Primitive::triangle,color){}
    
    /// render method
    virtual void render(const Primitive::RenderContext &ctx);
    
    /// direct access to the i-th vertex/normal index
    inline int i(int idx) const { return super::operator[](idx); }
    
    /// deep copy implementation (trivial)
    virtual Primitive *copy() const { return new TrianglePrimitive(*this); }
  };

  /// quad primitive
  struct QuadPrimitive : public FixedColVector<int,8>, public Primitive{
    /// super type
    typedef  FixedColVector<int,8> super; 

    /// constructor
    QuadPrimitive(int a, int b, int c, int d, const GeomColor &color, int na=-1, int nb=-1, int nc=-1, int nd=-1):
    super(a,b,c,d,na,nb,nc,nd),Primitive(Primitive::quad,color){}
    
    /// render method
    virtual void render(const Primitive::RenderContext &ctx);

    /// direct access to the i-th vertex/normal index
    inline int i(int idx) const { return super::operator[](idx); }

    /// deep copy implementation (trivial)
    virtual Primitive *copy() const { return new QuadPrimitive(*this); }
  };

  /// polygon primitive
  /** The Array2D's first row contains the */
  struct PolygonPrimitive : public Primitive{
    
    /// vertex and texture primitives
    /** Layout: 
        - first row: column i -> vertex index i
        - 2nd row: (optional) column i -> normal index i
    */
    Array2D<int> idx;
    
    /// constructor
    PolygonPrimitive(int n,const int *vidx, const GeomColor &color,const int *nidx=0):
    Primitive(Primitive::polygon,color),idx(n,nidx?2:1){
      std::copy(vidx,vidx+n,idx.begin());
      if(nidx) std::copy(nidx,nidx+n,idx.begin()+n);
    }
    
    /// render method
    virtual void render(const Primitive::RenderContext &ctx);
    
    /// deep copy method
    virtual Primitive *copy() const { 
      PolygonPrimitive *p = new PolygonPrimitive(*this);
      p->idx.detach();
      return p;
    }
    
    /// direct access to number of vertices
    inline int getNumPoints() const { return idx.getWidth(); }
    
    /// direct access to i-th vertex index
    inline int getVertexIndex(int i) const { return idx(i,0); }

    /// direct access to i-th normal index
    /** This will crash, if there are no normals */
    inline int getNormalIndex(int i) const { return idx(i,1); }
    
    /// utility method to ask whether normal indices are available
    inline bool hasNormals() const { return idx.getHeight() == 2; }
  };
  
  /// Texture Primitive 
  /** Texture Primitives hare two modes: 
      -# createTextureOnce=true: In this case, the texture data that is
         given to the constructor is only copied once. This will result
         in a static texture, that is only transferred to the graphics
         card-memory once. This is very efficient, but the texture
         cannot be updated lateron automatically 
      -# createTextureOnce=false: in this case, the texture data
         will always be updated before the texture is drawn. In this way,
         one can easily create video textures. */
  struct TexturePrimitive : public QuadPrimitive{
    GLImg texture;         //!<< internal texture
    const ImgBase *image;  //!<< set if the texture shall be updated every time it is drawn

    /// create with given texture that is either copied once or everytime the primitive is rendered
    TexturePrimitive(int a, int b, int c, int d, 
                     const ImgBase *image=0, bool createTextureOnce=true, 
                     int na=-1, int nb=-1, int nc=-1, int nd=-1, scalemode sm=interpolateLIN):
    QuadPrimitive(a,b,c,d,na,nb,nc,nd), texture(image,sm),
      image(createTextureOnce ? 0 : image){
      type = Primitive::texture;
    }

    /// create with given texture, that is copied once
    TexturePrimitive(int a, int b, int c, int d, 
                     const Img8u &image,
                     int na=-1, int nb=-1, int nc=-1, int nd=-1, scalemode sm=interpolateLIN):
    QuadPrimitive(a,b,c,d,na,nb,nc,nd), texture(&image,sm), 
      image(0){
      type = Primitive::texture;
    }

    /// render method
    virtual void render(const Primitive::RenderContext &ctx);

    /// deep copy
    virtual Primitive *copy() const { 
      return new TexturePrimitive(i(0),i(1),i(2),i(3),
                                  image ? image : texture.extractImage(),
                                  image ? true : false,
                                  i(4),i(5),i(6),i(7),
                                  texture.getScaleMode());
    }

  };

  
  /// The shared texture primitive references a texture from the parent SceneObject
  /** Therefore, shared textures can be reused in order to avoid that identical textures
      have to be hold several times in the graphics hardware memory */
  struct SharedTexturePrimitive : public QuadPrimitive{
    int sharedTextureIndex;
    
    /// create with given texture that is either copied once or everytime the primitive is rendered
    SharedTexturePrimitive(int a, int b, int c, int d, 
                     int sharedTextureIndex,
                     int na=-1, int nb=-1, int nc=-1, int nd=-1):
    QuadPrimitive(a,b,c,d,na,nb,nc,nd), sharedTextureIndex(sharedTextureIndex){
      type = Primitive::texture;
    }

    /// render method
    virtual void render(const Primitive::RenderContext &ctx);

    /// deep copy
    virtual Primitive *copy() const { 
      return new SharedTexturePrimitive(*this);
    }
  };
  
  /// Text Texture
  /** The text texture is implemented by a static common texture */
  struct TextPrimitive : public TexturePrimitive{

    /// utility method to creat a text texture
    static Img8u create_texture(const std::string &text, const GeomColor &color, int textSize);
    
    /// used for billboard text
    /** if the value is > 0, the text-texture will always be oriented towards the camera.
        the billboardHeight value is used as text-height (in scene units) */
    int billboardHeight; 
    
    /// constructor
    TextPrimitive(int a, int b, int c, int d, 
                  const std::string &text,
                  int textSize=20,
                  const GeomColor &textColor=GeomColor(255,255,255,255),
                  int na=-1, int nb=-1, int nc=-1, int nd=-1,
                  int billboardHeight=0,
                  scalemode sm=interpolateLIN):
    TexturePrimitive(a,b,c,d,create_texture(text,textColor,textSize),na,nb,nc,nd, sm),
    billboardHeight(billboardHeight){
      type = Primitive::text;
    }
    
    /// render method
    virtual void render(const Primitive::RenderContext &ctx);

    /// deep copy method
    virtual Primitive *copy() const {
      Primitive *p = TexturePrimitive::copy();
      p->type = text;
      return p;
    }
  };
  
}

#endif




#if 0
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
      texture,  //<! texture primitive (using 4 vertices like a quad as textured rectangle)
      text,     //<! text primitive (internally implmented as texture or as billboard)
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

    /// Special constructor to create a billboard texture primitive
    /** There is not special TEXT-type: type remains 'texture' */
    Primitive(int a, const std::string &text, const GeomColor &color, 
              int textSize, float billboardHeight, scalemode mode=interpolateLIN);

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

    /// this flag is only used for text-texture primitives
    /** if the value is > 0, the text-texture will always be oriented towards the camera.
        the billboardHeight value is used as text-height (in scene units) */
    float billboardHeight;

  };
}

#endif
