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

  class ImgI;
  
  class GLPaintEngine{
    public:
    enum AlignMode {NoAlign, Centered, Justify};
    GLPaintEngine(QGLWidget *widget);
    ~GLPaintEngine();
    
    void color(int r, int g, int b, int a=255);
    void fill(int r, int g, int b, int a=255);
    void fontsize(int size);
    void font(string name, int size = -1);

    void line(int x1, int y1, int x2, int y2);
    void point(int x,int y);
    void image(const Rect &r,ImgI *image, AlignMode mode = Justify);
    void image(const Rect &r,const QImage &image, AlignMode mode = Justify);
    void rect(const Rect &r);
    void ellipse(const Rect &r);
    void text(const Rect &r, const string text, AlignMode mode = Centered);
    
    protected:
    void setupRasterEngine(const Rect& r, const Size &s, AlignMode mode);
    void setPackAlignment(depth d, int linewidth);
    QGLWidget *m_poWidget;
    
    float m_afLineColor[4];
    float m_afFillColor[4];

    QFont m_oFont;
  };
}// namespace
#endif

