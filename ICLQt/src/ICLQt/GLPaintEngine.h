/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GLPaintEngine.h                        **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#define NOMINMAX // needed for Win32 in order to not define min, max as macros

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/PaintEngine.h>
#include <QtGui/QFont>

// forward declaration of the parent class
class QGLWidget;

namespace icl{
  namespace qt{
    
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
    class ICLQt_API GLPaintEngine : public PaintEngine{
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
      
      virtual void line(const utils::Point32f &a, const utils::Point32f &b);
      virtual void point(const utils::Point32f &p);
      virtual void image(const utils::Rect32f &r,core::ImgBase *image, 
                         PaintEngine::AlignMode mode = PaintEngine::Justify, 
                         core::scalemode sm=core::interpolateNN);
      virtual void image(const utils::Rect32f &r,const QImage &image, 
                         PaintEngine::AlignMode mode = PaintEngine::Justify, 
                         core::scalemode sm=core::interpolateNN);
      virtual void rect(const utils::Rect32f &r);
      virtual void triangle(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c);
      virtual void quad(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c, const utils::Point32f &d);
      virtual void ellipse(const utils::Rect32f &r);
      virtual void text(const utils::Rect32f &r, const std::string text, PaintEngine::AlignMode mode = PaintEngine::Centered);
  
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
      // estimates the size of a given text
      utils::Size estimateTextBounds(const std::string &text) const;
  
  
      protected:
      void setupRasterEngine(const utils::Rect32f& r, const utils::Size32f &s, PaintEngine::AlignMode mode);
      void setPackAlignment(core::depth d, int linewidth);
      void setupPixelTransfer(core::depth d, float brightness, float contrast, float intensity);
  
      QGLWidget *m_widget;
      
      float m_linewidth;
      float m_pointsize;
      float m_linecolor[4];
      float m_fillcolor[4];
      float m_bci[3];
      bool m_bciauto;
  
      QFont m_font;
  
      private:
      core::ImgBase *m_incompDepthBuf;
    };
  } // namespace qt
}// namespace
