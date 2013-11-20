/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/demos/canvas/canvas.cpp                        **
** Module : ICLCore                                                **
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

#include <ICLQt/Common.h>
#include <ICLCore/AbstractCanvas.h>

struct Canvas : public AbstractCanvas{
  union{
    icl8u *c8u[4];
    icl16s *c16s[4];
    icl32s *c32s[4];
    icl32f *c32f[4];
    icl64f *c64f[4];
    void *data[4];
  };
  Size size;
  depth d;
  
  Canvas(ImgBase *image){
    ICLASSERT_THROW(image,ICLException("Canvas::Canvas: image was null"));
    std::fill(c8u,c8u+4,(icl8u*)0);
    for(int i=0;i<image->getChannels() && i<=4;++i){
      data[i] = image->getDataPtr(i);
    }
    this->size = image->getSize();
    this->d = image->getDepth();
  }
  
  virtual void draw_point_internal(const utils::Point32f &p){
  
  }
  virtual void draw_line_internal(const utils::Point32f &a, 
                                  const utils::Point32f &b){
  }
  virtual void fill_triangle_internal(const utils::Point32f &a, 
                                      const utils::Point32f &b,
                                      const utils::Point32f &c){
  
  }
  virtual void draw_ellipse_internal(const utils::Point32f &c,
                                     const utils::Point32f &axis1,
                                     const utils::Point32f &axis2){
  
  }
  virtual void draw_image_internal(const utils::Point32f &ul, 
                                   const utils::Point32f &ur, 
                                   const utils::Point32f &lr, 
                                   const utils::Point32f &ll,
                                   float alpha, scalemode sm){
  
  }
  
};

int main(){
  Img8u image = cvt8u(create("lena"));
  Canvas c(&image);
  
  ERROR_LOG("this application has not been implemented yet");
  return 0;
}
