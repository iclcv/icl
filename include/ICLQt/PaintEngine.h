/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/PaintEngine.h                            **
** Module : ICLQt                                                  **
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

#ifndef ICL_PAINT_ENGINE_H
#define ICL_PAINT_ENGINE_H

#include <stdio.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Size32f.h>
#include <ICLUtils/Rect32f.h>
#include <ICLCore/Types.h>
#include <string>
#include <QImage>

namespace icl{

  /** \cond */
  class ImgBase;
  /** \endcond */  
  
  /// pure virtual Paint engine interface \ingroup UNCOMMON
  class PaintEngine{
    public:
    virtual ~PaintEngine(){}
    enum AlignMode {NoAlign, Centered, Justify};
    enum TextWeight {Light, Normal, DemiBold, Bold, Black};
    enum TextStyle {StyleNormal, StyleItalic, StyleOblique };

    virtual void color(float r, float g, float b, float a=255)=0;
    virtual void fill(float r, float g, float b, float a=255)=0;
    virtual void fontsize(float size)=0;
    virtual void font(std::string name, float size = -1, TextWeight weight = Normal, TextStyle style = StyleNormal)=0;

    virtual void linewidth(float w)=0;
    virtual void pointsize(float s)=0;
    virtual void line(const Point32f &a, const Point32f &b)=0;
    virtual void point(const Point32f &p)=0;
    virtual void image(const Rect32f &r,ImgBase *image, AlignMode mode = Justify, scalemode sm=interpolateNN)=0;
    virtual void image(const Rect32f &r,const QImage &image, AlignMode mode = Justify, scalemode sm=interpolateNN)=0;
    virtual void rect(const Rect32f &r)=0;
    virtual void triangle(const Point32f &a, const Point32f &b, const Point32f &c)=0;
    virtual void quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d)=0;
    virtual void ellipse(const Rect32f &r)=0;
    virtual void text(const Rect32f &r, const std::string text, AlignMode mode = Centered)=0;

    /// brightness-constrast intensity adjustment (for images only)
    virtual void bci(float brightness=0, float contrast=0, float floatensity=0)=0;
    virtual void bciAuto()=0;
    
    virtual void getColor(float *piColor)=0;
    virtual void getFill(float *piColor)=0;
    
    virtual float getFontSize() const =0;

  };
}// namespace

#endif

