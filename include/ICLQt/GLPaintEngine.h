/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GLPaintEngine.h                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#ifndef ICL_GL_PAINT_ENGINE_H
#define ICL_GL_PAINT_ENGINE_H

#define NOMINMAX // needed for Win32 in order to not define min, max as macros

#include <ICLQt/PaintEngine.h>
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
    
    virtual void color(float r, float g, float b, float a=255);
    virtual void fill(float r, float g, float b, float a=255);
    virtual void fontsize(float size);
    virtual void font(std::string name, 
                      float size = -1, 
                      PaintEngine::TextWeight weight = PaintEngine::Normal, 
                      PaintEngine::TextStyle style = PaintEngine::StyleNormal);

    virtual void linewidth(float w);
    virtual void pointsize(float s);
    
    virtual void line(const Point32f &a, const Point32f &b);
    virtual void point(const Point32f &p);
    virtual void image(const Rect32f &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify, scalemode sm=interpolateNN);
    virtual void image(const Rect32f &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify, scalemode sm=interpolateNN);
    virtual void rect(const Rect32f &r);
    virtual void triangle(const Point32f &a, const Point32f &b, const Point32f &c);
    virtual void quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d);
    virtual void ellipse(const Rect32f &r);
    virtual void text(const Rect32f &r, const std::string text, PaintEngine::AlignMode mode = PaintEngine::Centered);

    /// brightness-constrast intensity adjustment (for images only)
    virtual void bci(float brightness=0, float contrast=0, float intensity=0);
    virtual void bciAuto();
    
    virtual void getColor(float *piColor);
    virtual void getFill(float *piColor);

    virtual float getFontSize() const{
      return m_font.pointSize();
    }
    virtual float getLineWidth() const {
      return m_linewidth;
    }
    virtual float getPointSize() const {
      return m_pointsize;
    }


    protected:
    void setupRasterEngine(const Rect32f& r, const Size32f &s, PaintEngine::AlignMode mode);
    void setPackAlignment(depth d, int linewidth);
    void setupPixelTransfer(depth d, float brightness, float contrast, float intensity);

    QGLWidget *m_widget;
    
    float m_linewidth;
    float m_pointsize;
    float m_linecolor[4];
    float m_fillcolor[4];
    float m_bci[3];
    bool m_bciauto;

    QFont m_font;

    private:
    ImgBase *m_incompDepthBuf;
  };
}// namespace
#endif

