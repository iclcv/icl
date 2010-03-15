/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/GLTextureMapPaintEngine.cpp                  **
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
*********************************************************************/

#include <ICLQt/GLTextureMapPaintEngine.h>
#include <ICLQt/GLTextureMapBaseImage.h>
#include <map>
#include <vector>

using namespace std;
#include <ICLQt/GLTextureMapPaintEngine.h>
#include <ICLQt/GLTextureMapBaseImage.h>
#include <QGLWidget>
#include <ICLQt/QImageConverter.h>
#include <ICLCC/CCFunctions.h>

namespace icl{
  GLTextureMapPaintEngine::GLTextureMapPaintEngine(QGLWidget *widget) : 
    GLPaintEngine(widget), m_bSharedImage(0){
  }
  GLTextureMapPaintEngine::GLTextureMapPaintEngine(QGLWidget *widget, GLTextureMapBaseImage *sharedTMImage) : 
    GLPaintEngine(widget),m_bSharedImage(sharedTMImage) {   
  }
  
  GLTextureMapPaintEngine::~GLTextureMapPaintEngine(){}
  
  void GLTextureMapPaintEngine::image(const Rect32f &r,ImgBase *image, PaintEngine::AlignMode mode){
    ICLASSERT_RETURN(image);
    glColor4f(1,1,1,1);
    GLTextureMapBaseImage(image).drawTo(computeRect(r,image->getSize(),mode), Size(m_widget->width(),m_widget->height()));
  }
  void GLTextureMapPaintEngine::sharedImage(const Rect32f &r, PaintEngine::AlignMode mode){
    ICLASSERT_RETURN(m_bSharedImage);
    glColor4f(1,1,1,1);

    Rect32f r2 = computeRect(r,m_bSharedImage->getSize(),mode);
    Size32f s(m_widget->width(),m_widget->height());
    m_bSharedImage->drawTo(r2,s);
  }
  
  Rect32f GLTextureMapPaintEngine::computeRect(const Rect32f &rect, const Size32f &imageSize, PaintEngine::AlignMode mode){
    switch(mode){
      case PaintEngine::NoAlign: return Rect32f(rect.x, rect.y, imageSize.width, imageSize.height);
      case PaintEngine::Centered: {
        float cx  = rect.x+rect.width/2;
        float cy  = rect.y+rect.height/2;
        return Rect32f(cx-imageSize.width/2,cy-imageSize.height/2,imageSize.width,imageSize.height);
      }
      default:  return rect;
    }
  }

  void GLTextureMapPaintEngine::image(const Rect32f &r,const QImage &image, PaintEngine::AlignMode mode){
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



