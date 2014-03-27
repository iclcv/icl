/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/PaintEngine.h                          **
** Module : ICLQt                                                  **
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
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Size32f.h>
#include <ICLUtils/Rect32f.h>
#include <ICLCore/Types.h>
#include <string>
#include <stdio.h>
#include <QImage>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{
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
      virtual void line(const utils::Point32f &a, const utils::Point32f &b)=0;
      virtual void point(const utils::Point32f &p)=0;
      virtual void image(const utils::Rect32f &r,core::ImgBase *image, AlignMode mode = Justify, 
                         core::scalemode sm=core::interpolateNN)=0;
      virtual void image(const utils::Rect32f &r,const QImage &image, AlignMode mode = Justify, 
                         core::scalemode sm=core::interpolateNN)=0;
      virtual void rect(const utils::Rect32f &r)=0;
      virtual void triangle(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c)=0;
      virtual void quad(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c, const utils::Point32f &d)=0;
      virtual void ellipse(const utils::Rect32f &r)=0;
      virtual void text(const utils::Rect32f &r, const std::string text, AlignMode mode = Centered)=0;
  
      /// brightness-constrast intensity adjustment (for images only)
      virtual void bci(float brightness=0, float contrast=0, float floatensity=0)=0;
      virtual void bciAuto()=0;
      
      virtual void getColor(float *piColor)=0;
      virtual void getFill(float *piColor)=0;
      
      virtual float getFontSize() const =0;
  
    };
  } // namespace qt
}// namespace

