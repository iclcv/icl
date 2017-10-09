/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Primitive.h                        **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Img.h>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/Array2D.h>
#include <ICLQt/GLImg.h>

namespace icl{
  namespace geom{
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
        const std::vector<utils::SmartPtr<qt::GLImg> > &sharedTextures; //!< list of shared textures
        bool lineColorsFromVertices;                 //!< line coloring
        bool triangleColorsFromVertices;             //!< triangle coloring
        bool quadColorsFromVertices;                 //!< quad coloring
        bool polygonColorsFromVertices;              //!< polygon coloring
        SceneObject *object;                         //!< the parent object
      };

      /// Default constructor
      Primitive(Type type=nothing, const GeomColor &color=GeomColor(255,255,255,255)):type(type),color(color){}

      /// virtual, but empty destructor
      virtual ~Primitive() {}

      /// virtual render method, which is called by the parent scene object
      virtual void render(const Primitive::RenderContext &ctx) = 0;

      /// must be implemented in order to obtain a deep and independent copy
      virtual Primitive *copy() const = 0;
    };

    /// line primitive (the line references 2 vertices)
    struct LinePrimitive : public math::FixedColVector<int,2>, public Primitive{
      /// super type
      typedef math::FixedColVector<int,2> super;

      /// constructor
      LinePrimitive(int a, int b, const GeomColor &color):
      math::FixedColVector<int,2>(a,b),Primitive(Primitive::line,color){}

      /// render
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// direct access to the i-th vertex/normal index
      inline int i(int idx) const { return super::operator[](idx); }

      /// deep copy implementation (trivial)
      virtual Primitive *copy() const { return new LinePrimitive(*this); }
    };

    /// triangle primitive
    struct TrianglePrimitive : public math::FixedColVector<int,6>, public Primitive{
      /// super type
      typedef  math::FixedColVector<int,6> super;

      /// constructor
      TrianglePrimitive(int a, int b, int c, const GeomColor &color, int na=-1, int nb=-1, int nc=-1):
      super(a,b,c,na,nb,nc),Primitive(Primitive::triangle,color){}

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// direct access to the i-th vertex/normal index
      inline int i(int idx) const { return super::operator[](idx); }

      /// deep copy implementation (trivial)
      virtual Primitive *copy() const { return new TrianglePrimitive(*this); }

      /// computes the normal for this triangle
      /** Given the parent SceneObject's vertex vector*/
      ICLGeom_API Vec computeNormal(const std::vector<Vec> &vertices) const;

    };

    /// quad primitive
    struct QuadPrimitive : public math::FixedColVector<int,8>, public Primitive{
      /// super type
      typedef math::FixedColVector<int,8> super;

      /// visualization optimization flag
      bool trySurfaceOptimization;

      /// number of sub-quads to render for better lighting
      /** for non-texture quads only. Interferes with quadColorsFromVertices.
          If both is activated, quadColorsFromVertices is not used!*/
      int tesselationResolution;

      /// constructor
      QuadPrimitive(int a, int b, int c, int d, const GeomColor &color, int na=-1, int nb=-1, int nc=-1, int nd=-1,
                    bool trySurfaceOptimization=false,int tesselationResolution=1):
      super(a,b,c,d,na,nb,nc,nd),Primitive(Primitive::quad,color),trySurfaceOptimization(trySurfaceOptimization),
      tesselationResolution(tesselationResolution){}

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// direct access to the i-th vertex/normal index
      inline int i(int idx) const { return super::operator[](idx); }

      /// deep copy implementation (trivial)
      virtual Primitive *copy() const { return new QuadPrimitive(*this); }

      /// computes the normal for this quad
      /** Given the parent SceneObject's vertex vector*/
      ICLGeom_API Vec computeNormal(const std::vector<Vec> &vertices) const;
    };

    /// polygon primitive
    /** The Array2D's first row contains the */
    struct PolygonPrimitive : public Primitive{

      /// vertex and texture primitives
      /** Layout:
          - first row: column i -> vertex index i
          - 2nd row: (optional) column i -> normal index i
      */
      utils::Array2D<int> idx;

      /// constructor
      PolygonPrimitive(int n,const int *vidx, const GeomColor &color,const int *nidx=0):
      Primitive(Primitive::polygon,color),idx(n,nidx?2:1){
        std::copy(vidx,vidx+n,idx.begin());
        if(nidx) std::copy(nidx,nidx+n,idx.begin()+n);
      }

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

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

    /// extra base class for primitives, that use a special alpha function (in particular textures)
    struct AlphaFuncProperty{
      /// base constructor setting up to GL_GREATER 0.1
      ICLGeom_API AlphaFuncProperty();
      AlphaFuncProperty(int alphaFunc, float alphaValue):alphaFunc(alphaFunc),alphaValue(alphaValue){}

      int alphaFunc;         //!<< used for glAlphaFunc call glAlphaFunc((GLenum)alphaFunc,alphaValue)
      float alphaValue;      //!<< used for glAlphaFunc call glAlphaFunc((GLenum)alphaFunc,alphaValue)

      /// used for setting up the alpha func, that is used to render this texture primitive
      void setAlphaFunc(int func, float value){
        alphaFunc = func;
        alphaValue = value;
      }

      ICLGeom_API void restoreAlphaDefaults();
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
    struct TexturePrimitive : public QuadPrimitive, public AlphaFuncProperty{
      qt::GLImg texture;         //!<< internal texture
      const core::ImgBase *image;  //!<< set if the texture shall be updated every time it is drawn

      /// create with given texture that is either copied once or everytime the primitive is rendered
      TexturePrimitive(int a, int b, int c, int d,
                       const core::ImgBase *image=0, bool createTextureOnce=true,
                       int na=-1, int nb=-1, int nc=-1, int nd=-1, core::scalemode sm=core::interpolateLIN):
      QuadPrimitive(a,b,c,d,geom_white(),na,nb,nc,nd), texture(image,sm),
        image(createTextureOnce ? 0 : image){
        type = Primitive::texture;
      }

      /// create with given texture, that is copied once
      TexturePrimitive(int a, int b, int c, int d,
                       const core::Img8u &image,
                       int na=-1, int nb=-1, int nc=-1, int nd=-1, core::scalemode sm=core::interpolateLIN):
      QuadPrimitive(a,b,c,d,geom_white(),na,nb,nc,nd), texture(&image,sm),
        image(0){
        type = Primitive::texture;
      }

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// deep copy
      virtual Primitive *copy() const {
        return new TexturePrimitive(i(0),i(1),i(2),i(3),
                                    image ? image : texture.extractImage(),
                                    !image,
                                    i(4),i(5),i(6),i(7),
                                    texture.getScaleMode());
      }


    };

    /// Special texture Primitive for single textures spread over a regular grid of vertices
    /** For more details look at ICLQt's icl::GLImg::drawToGrid method */
    class TextureGridPrimitive : public Primitive, public AlphaFuncProperty{
      protected:
      friend class SceneObject;
      int w,h;
      qt::GLImg texture;
      const icl32f *px, *py, *pz, *pnx,  *pny, *pnz;
      int stride;
      const core::ImgBase *image;

      public:
      TextureGridPrimitive(int w, int h, const core::ImgBase *image,
                           const icl32f *px, const icl32f *py, const icl32f *pz,
                           const icl32f *pnx=0, const icl32f *pny=0, const icl32f *pnz=0,
                           int stride = 1,bool createTextureOnce=true,core::scalemode sm=core::interpolateLIN):
      Primitive(Primitive::texture),w(w),h(h),texture(image,sm),px(px),py(py),pz(pz),
      pnx(pnx),pny(pny),pnz(pnz),stride(stride),image(createTextureOnce ? 0 : image){}

      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      virtual Primitive *copy() const {
        return new TextureGridPrimitive(w,h,image ? image : texture.extractImage(),
                                        px,py,pz,pnx,pny,pnz,stride,!image,
                                        texture.getScaleMode());
      }
      ICLGeom_API void getAABB(utils::Range32f aabb[3]);

      inline Vec getPos(int x, int y) const {
        const int idx = stride*(x + w*y);
        return Vec(px[idx],py[idx],pz[idx],1);
      }
    };

    class TwoSidedTextureGridPrimitive : public TextureGridPrimitive{
      qt::GLImg back;
      const core::ImgBase *iback;
      public:
      TwoSidedTextureGridPrimitive(int w, int h, const core::ImgBase *front, const core::ImgBase *back,
                                   const icl32f *px, const icl32f *py, const icl32f *pz,
                                   const icl32f *pnx=0, const icl32f *pny=0, const icl32f *pnz=0,
                                   int stride = 1,bool createFrontOnce=true,
                                   bool createBackOnce=true, core::scalemode sm=core::interpolateLIN):
      TextureGridPrimitive(w,h,front,px,py,pz,pnx,pny,pnz,stride,createFrontOnce,sm),back(back,sm),
      iback(createBackOnce ? 0 : back){}

      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// sets new textures
      ICLGeom_API void setTextures(const core::ImgBase *front, const core::ImgBase *back);
    };


    /// Grid primitive that renders a two-sided grid (sides have different colors)
    class TwoSidedGridPrimitive: public Primitive{
      public:
      int w,h;
      const Vec *vertices, *normals;
      GeomColor front,back,lines;
      bool drawLines,drawQuads;

      inline int getIdx(int x, int y) const { return x+w*y; }
      TwoSidedGridPrimitive(int w, int h, const Vec *vertices, const Vec *normals=0,
                            const GeomColor &frontColor=GeomColor(0,100,255,255),
                            const GeomColor &backColor=GeomColor(255,0,100,255),
                            const GeomColor &lineColor=GeomColor(0,255,100,255),
                            bool drawLines=false, bool drawQuads=true):
      Primitive(Primitive::quad), w(w), h(h), vertices(vertices), normals(normals),
      front(frontColor*(1./255)),back(backColor*(1./255)),lines(lineColor*(1./255)),
      drawLines(drawLines),drawQuads(drawQuads){}

      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      virtual Primitive *copy() const{
        return new TwoSidedGridPrimitive(w,h,vertices, normals, front*255, back*255,
                                         lines*255,drawLines,drawQuads);
      }
      inline const Vec &getPos(int x, int y) const {
        return vertices[x+w*y];
      }
    };

    /// The shared texture primitive references a texture from the parent SceneObject
    /** Therefore, shared textures can be reused in order to avoid that identical textures
        have to be hold several times in the graphics hardware memory */
    struct SharedTexturePrimitive : public QuadPrimitive, public AlphaFuncProperty{
      int sharedTextureIndex;

      /// create with given texture that is either copied once or everytime the primitive is rendered
      SharedTexturePrimitive(int a, int b, int c, int d,
                       int sharedTextureIndex,
                       int na=-1, int nb=-1, int nc=-1, int nd=-1):
      QuadPrimitive(a,b,c,d,geom_white(),na,nb,nc,nd), sharedTextureIndex(sharedTextureIndex){
        type = Primitive::texture;
      }

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// deep copy
      virtual Primitive *copy() const {
        return new SharedTexturePrimitive(*this);
      }
    };

    /// Texture Primitive for rendering textures with arbitrary texture coordinates
    struct GenericTexturePrimitive : public Primitive, public AlphaFuncProperty{
      utils::SmartPtr<qt::GLImg> texture;
      const core::ImgBase *image;

      std::vector<Vec> ps;
      std::vector<utils::Point32f> texCoords;
      std::vector<Vec> normals;

      /// if these are given (size > 0), ps and normals are not used!
      std::vector<int> vertexIndices;
      std::vector<int> normalIndices;

      /// Generic version, where the given values are copied deeply into the internal buffers for rendering
      ICLGeom_API GenericTexturePrimitive(const core::ImgBase *image, int numPoints,
                              const float *xs, const float *ys, const float *zs, int xyzStride,
                              const utils::Point32f *texCoords, const float *nxs=0, const float *nys=0,
                              const float *nzs=0, int nxyzStride=1, bool createTextureOnce=true);

      /// less generic Constructor, that uses index-pointers for referencing vertices and normals of the parent SceneObject
      ICLGeom_API GenericTexturePrimitive(const core::ImgBase *image, int numPoints, const int *vertexIndices,
                              const utils::Point32f *texCoords, const int *normalIndices = 0,
                              bool createTextureOnce=true);

      //// custom render implementation
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// deep copy method
      virtual Primitive *copy() const {
        GenericTexturePrimitive *cpy = new GenericTexturePrimitive(*this);
        cpy->texture = new qt::GLImg(image ? image : texture->extractImage());
        return cpy;
      }
    };

    /// Text Texture
    /** The text texture is implemented by a static common texture */
    struct TextPrimitive : public TexturePrimitive{
      /// internal memory for the text size
      int textSize;

      /// internal memory for the text color
      GeomColor textColor;

      /// utility method to creat a text texture
      ICLGeom_API static core::Img8u create_texture(const std::string &text, const GeomColor &color, int textSize);

      /// used for billboard text
      /** if the value is > 0, the text-texture will always be oriented towards the camera.
          the billboardHeight value is used as text-height (in scene units) */
      float billboardHeight;

      /// constructor
      ICLGeom_API TextPrimitive(int a, int b, int c, int d,
                    const std::string &text,
                    int textSize=20,
                    const GeomColor &textColor=GeomColor(255,255,255,255),
                    int na=-1, int nb=-1, int nc=-1, int nd=-1,
                    float billboardHeight=0,
                    core::scalemode sm=core::interpolateLIN);
      ICLGeom_API ~TextPrimitive();

      /// render method
      ICLGeom_API virtual void render(const Primitive::RenderContext &ctx);

      /// deep copy method
      virtual Primitive *copy() const {
        Primitive *p = TexturePrimitive::copy();
        p->type = text;
        return p;
      }

      /// sets new text
      inline void updateText(const std::string &newText){
        core::Img8u t = create_texture(newText,textColor,textSize);
        texture.update(&t);
      }

    };


  } // namespace geom
}

