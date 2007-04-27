#include <iclGLTextureMapPaintEngine.h>
#include <iclGLTextureMapBaseImage.h>
#include <map>
#include <vector>

using namespace std;
#include <iclGLTextureMapPaintEngine.h>
#include <iclGLTextureMapBaseImage.h>
#include <QGLWidget>
#include <iclQImageConverter.h>
#include <iclCC.h>

namespace icl{
  GLTextureMapPaintEngine::GLTextureMapPaintEngine(QGLWidget *widget) : 
    GLPaintEngine(widget), m_bSharedImage(0){
  }
  GLTextureMapPaintEngine::GLTextureMapPaintEngine(QGLWidget *widget, GLTextureMapBaseImage *sharedTMImage) : 
    GLPaintEngine(widget),m_bSharedImage(sharedTMImage) {   
  }
  
  GLTextureMapPaintEngine::~GLTextureMapPaintEngine(){}
  
  void GLTextureMapPaintEngine::image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode){
    ICLASSERT_RETURN(image);
    glColor4f(1,1,1,1);
    GLTextureMapBaseImage(image).drawTo(computeRect(r,image->getSize(),mode), Size(m_poWidget->width(),m_poWidget->height()));
  }
  void GLTextureMapPaintEngine::sharedImage(const Rect &r, PaintEngine::AlignMode mode){
    ICLASSERT_RETURN(m_bSharedImage);
    glColor4f(1,1,1,1);

    Rect r2 = computeRect(r,m_bSharedImage->getImageSize(),mode);
    Size s(m_poWidget->width(),m_poWidget->height());
    m_bSharedImage->drawTo(r2,s);
  }
  
  Rect GLTextureMapPaintEngine::computeRect(const Rect &rect, const Size &imageSize, PaintEngine::AlignMode mode){
    switch(mode){
      case PaintEngine::NoAlign: return Rect(rect.x, rect.y, imageSize.width, imageSize.height);
      case PaintEngine::Centered: {
        int cx  = rect.x+rect.width/2;
        int cy  = rect.y+rect.height/2;
        return Rect(cx-imageSize.width/2,cy-imageSize.height/2,imageSize.width,imageSize.height);
      }
      default:  return rect;
    }
  }

  void GLTextureMapPaintEngine::image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode){
    Img8u buf;    
    if(image.format()==QImage::Format_Indexed8){
      buf = Img8u(Size(image.width(),image.height()),formatGray);
    }else{
      buf = Img8u(Size(image.width(),image.height()),4);
    }
    interleavedToPlanar(image.bits(),&buf);
    this->image(r,&buf,mode);
  }

}



