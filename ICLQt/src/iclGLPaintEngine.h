#ifndef GLPAINT_ENGINE_H
#define GLPAINT_ENGINE_H

#define NOMINMAX // needed for Win32 in order to not define min, max as macros

#include <iclPaintEngine.h>
#include <QFont>

// forward declaration of the parent class
class QGLWidget;

namespace icl{
  
  /// Paint engine to be used in QGLWigets for a high performance image visualization \ingroup UNCOMMON
  /** The GLPaintEngine provides a QPainter like interface for drawing
      2D-OpenGL primitives directly into a OpenGL graphics context. 
      It provides the following functionalities:
      - line and fill color state machniene
      - antialized Font rendering with implicit scaling if desired
      - drawing primitives using pixel coordinates (lines, points, rects, ellipese)
      - drawing QImages and ICL-img images directly without buffering, flipping or 
        converting image data in software. 
      - brightness / (contrast / intestity) adjustment (in hardware)
  
      @see ICL/trunk/ICLQt/test/ICLDrawDemo for more details
  */
  class GLPaintEngine : public PaintEngine{
    public:
    
    GLPaintEngine(QGLWidget *widget);
    virtual ~GLPaintEngine();
    
    virtual void color(int r, int g, int b, int a=255);
    virtual void fill(int r, int g, int b, int a=255);
    virtual void fontsize(int size);
    virtual void font(std::string name, 
                      int size = -1, 
                      PaintEngine::TextWeight weight = PaintEngine::Normal, 
                      PaintEngine::TextStyle style = PaintEngine::StyleNormal);

    virtual void line(const Point &a, const Point &b);
    virtual void point(const Point &p);
    virtual void image(const Rect &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify);
    virtual void image(const Rect &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify);
    virtual void rect(const Rect &r);
    virtual void triangle(const Point &a, const Point &b, const Point &c);
    virtual void quad(const Point &a, const Point &b, const Point &c, const Point &d);
    virtual void ellipse(const Rect &r);
    virtual void text(const Rect &r, const std::string text, PaintEngine::AlignMode mode = PaintEngine::Centered);

    /// brightness-constrast intensity adjustment (for images only)
    virtual void bci(int brightness=0, int contrast=0, int intensity=0);
    virtual void bciAuto();
    
    virtual void getColor(int *piColor);
    virtual void getFill(int *piColor);

    virtual int getFontSize() const{
      return m_oFont.pixelSize();
    }

    protected:
    void setupRasterEngine(const Rect& r, const Size &s, PaintEngine::AlignMode mode);
    void setPackAlignment(depth d, int linewidth);
    void setupPixelTransfer(depth d, int brightness, int contrast, int intensity);

    QGLWidget *m_poWidget;
    
    float m_afLineColor[4];
    float m_afFillColor[4];
    int m_aiBCI[3];
    bool m_bBCIAutoFlag;

    QFont m_oFont;

    private:
    ImgBase *m_poImageBufferForIncompatibleDepth;
  };
}// namespace
#endif

