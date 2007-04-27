#ifndef ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H
#define ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H

#include <iclGLPaintEngine.h>


namespace icl{

  class GLTextureMapBaseImage;

  /// ...
  /** . if a shared image is given, this image is drawn !!*/
  class GLTextureMapPaintEngine : public GLPaintEngine{
    public:
    GLTextureMapPaintEngine(QGLWidget *widget);
    GLTextureMapPaintEngine(QGLWidget *widget, GLTextureMapBaseImage *sharedTMImage);
    virtual ~GLTextureMapPaintEngine();
    virtual void image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify);
    virtual void image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify);

    void sharedImage(const Rect &r, PaintEngine::AlignMode mode = PaintEngine::Justify);
    private:
    Rect computeRect(const Rect &rect, const Size &imageSize,PaintEngine::AlignMode mode );
    GLTextureMapBaseImage *m_bSharedImage;
    
  };  
}


#endif
