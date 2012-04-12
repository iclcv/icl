/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/DrawHandle.h                             **
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

#ifndef ICL_DRAW_HANDLE_H
#define ICL_DRAW_HANDLE_H

#include <ICLQt/GUIHandle.h>
#include <ICLUtils/Exception.h>

namespace icl{
  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class DrawHandle : public GUIHandle<ICLDrawWidget>{
    public:
    /// create an empty handle
    DrawHandle(){}

    /// create a new ImageHandel
    DrawHandle(ICLDrawWidget *w, GUIWidget *guiw):GUIHandle<ICLDrawWidget>(w,guiw){}
    
    /// make the wrapped ICLWidget show a given image
    void setImage(const ImgBase *image);
    
    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase *image) { setImage(image); }

    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase &image) { setImage(&image); }
    
    /// re-renders the draw-component
    void render();

    /// passes callback registration to wrapped ICLWidget instance)
    virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all");

    /// complex callbacks are not allowed for image-components (this method will throw an exception)
    virtual void registerCallback(const GUI::ComplexCallback&, const std::string &){
      throw ICLException("ImageHandle::registerCallback: you cannot register "
                         "GUI::ComplexCallback instances to an image GUI component");
    }
    
    /// passes callback registration to wrapped ICLWidget instance)
    virtual void removeCallbacks();

  };
  
}

#endif
