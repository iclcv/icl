/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImageRenderer.h                    **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>


namespace icl{
  namespace core{
    
    class ImageRenderer : public utils::Uncopyable{
      struct Data;
      Data *m_data;
      
      public:
      ImageRenderer(ImgBase *image=0);
      ~ImageRenderer();
      
      void setImage(ImgBase *image);
      
      void color(int r, int g, int b, int a);
      void fill(int r, int g, int b, int a);
      void symsize(float size);
      void fontsize(float size);

      void sym(char sym, int x, int y);
      void rect(int x, int y, int w, int h);
      void triangle(int x1, int y1, int x2, int y2, int x3, int y3);
      void line(int x1, int y1, int x2, int y2);
      void linestrip(int n, int *xs, int *ys, int xStride=1, int yStride=1);
      void pix(int x1, int x2);
      void pix(int n, int *xs, int *ys, int xStride=1, int yStride=1);
      void circle(int cx, int cy, int r);
      void ellipse(int x, int y, int w, int h);
      void text(int x, int y, const std::string &text);
    };
    
  }
}
