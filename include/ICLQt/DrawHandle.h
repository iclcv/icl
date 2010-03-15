/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#ifndef ICL_DRAW_HANDLE
#define ICL_DRAW_HANDLE

#include <ICLQt/GUIHandle.h>

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
    
    /// calles updated internally
    void update();

    /// passes callback registration to wrapped ICLWidget instance)
    virtual void registerCallback(GUI::CallbackPtr cb, const std::string &events="all");
    
    /// passes callback registration to wrapped ICLWidget instance)
    virtual void removeCallbacks();

  };
  
}

#endif
