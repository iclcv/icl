#ifndef GLPAINT_ENGINE_H
#define GLPAINT_ENGINE_H

#include <stdio.h>
#include <stdlib.h>

#include "Point.h"
#include "Size.h"
#include "Rect.h"
#include "ICLCore.h"

#include <string>
#include <QImage>
#include <QFont>

using std::string;

class QGLWidget;
namespace icl{

  /// Forward declaration of the ImgI class
  class ImgI;
    
  /// Paint engine to be used in QGLWigets for a high performance image visualization
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
  class GLPaintEngine{
    public:
    enum AlignMode {NoAlign, Centered, Justify};
    enum TextWeight {Light, Normal, DemiBold, Bold, Black};
    enum TextStyle {StyleNormal, StyleItalic, StyleOblique };
    GLPaintEngine(QGLWidget *widget);
    ~GLPaintEngine();
    
    void color(int r, int g, int b, int a=255);
    void fill(int r, int g, int b, int a=255);
    void fontsize(int size);
    void font(string name, int size = -1, TextWeight weight = Normal, TextStyle style = StyleNormal);

    void line(const Point &a, const Point &b);
    void point(const Point &p);
    void image(const Rect &r,ImgI *image, AlignMode mode = Justify);
    void image(const Rect &r,const QImage &image, AlignMode mode = Justify);
    void rect(const Rect &r);
    void ellipse(const Rect &r);
    void text(const Rect &r, const string text, AlignMode mode = Centered);

    /// brightness-constrast intensity adjustment (for images only)
    void bci(int brightness, int contrast=0, int intensity=0);
    
    void getColor(int *piColor);
    void getFill(int *piColor);

    protected:
    void setupRasterEngine(const Rect& r, const Size &s, AlignMode mode);
    void setPackAlignment(depth d, int linewidth);
    void setupPixelTransfer(depth d, int brightness, int contrast, int intensity);

    QGLWidget *m_poWidget;
    
    float m_afLineColor[4];
    float m_afFillColor[4];
    int m_aiBCI[3];

    QFont m_oFont;
  };
}// namespace
#endif

