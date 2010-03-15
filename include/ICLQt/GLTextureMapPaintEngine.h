/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/GLTextureMapPaintEngine.h                **
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

#ifndef ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H
#define ICL_GL_TEXTURE_MAP_PAINT_ENGINE_H

#include <ICLQt/GLPaintEngine.h>


namespace icl{

  /** \cond */
  class GLTextureMapBaseImage;
  /** \endcond */
  /// Paint Engine implementation which uses OpenGL's texture mapping abilities for drawing images \ingroup UNCOMMON
  /** In addition to the default PaintEngine, this class can handle an addition so called 
      "shared-image". If this image is set, it is drawn. The use of a shared image avoids a 
      deep copy call of that image internally 
  */
  class GLTextureMapPaintEngine : public GLPaintEngine{
    public:
    /// create a new GLTextureMapPaintEngine with given parent widget
    GLTextureMapPaintEngine(QGLWidget *widget);
    
    /// create a new GLTextureMapPaintEngine with given parent widget and given shared image
    GLTextureMapPaintEngine(QGLWidget *widget, GLTextureMapBaseImage *sharedTMImage);

    /// Destructor
    virtual ~GLTextureMapPaintEngine();

    /// reimplementation of the image drawing function (using texture mapping)
    virtual void image(const Rect32f &r,ImgBase *image, PaintEngine::AlignMode mode = PaintEngine::Justify);

    /// reimplementation of the image drawing function (using texture mapping)
    virtual void image(const Rect32f &r,const QImage &image, PaintEngine::AlignMode mode = PaintEngine::Justify);

    /// <b>additional</b> shared image drawing function (draws the internal shared image)
    void sharedImage(const Rect32f &r, PaintEngine::AlignMode mode = PaintEngine::Justify);

    private:
    /// internal utility function which helps to calculate the current image rect
    Rect32f computeRect(const Rect32f &rect, const Size32f &imageSize,PaintEngine::AlignMode mode );

    /// wrapped shared image (or null)
    GLTextureMapBaseImage *m_bSharedImage;
    
  };  
}


#endif
