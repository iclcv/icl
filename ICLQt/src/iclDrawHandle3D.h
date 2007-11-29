#ifndef ICL_DRAW_HANDLE_3D_H
#define ICL_DRAW_HANDLE_3D_H

#include <iclGUIHandle.h>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget3D;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class DrawHandle3D : public GUIHandle<ICLDrawWidget3D>{
    public:
    /// create a new ImageHandel
    DrawHandle3D(ICLDrawWidget3D *w=0):GUIHandle<ICLDrawWidget3D>(w){}
    
    /// make the wrapped ICLWidget show a given image
    void setImage(const ImgBase *image);
    
    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase *image) { setImage(image); }
    
    /// calles updated internally
    void update();
  };
  
}

#endif
