#ifndef ICL_IMAGE_HANDLE
#define ICL_IMAGE_HANDLE


#include <iclGUIHandle.h>

namespace icl{
  
  /** \cond */
  class ICLWidget;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class ImageHandle : public GUIHandle<ICLWidget>{
    public:
    /// Create an empty handle
    ImageHandle(){}
    
    /// create a new ImageHandel
    ImageHandle(ICLWidget *w, GUIWidget *guiw):GUIHandle<ICLWidget>(w,guiw){}
    
    /// make the wrapped ICLWidget show a given image
    void setImage(const ImgBase *image);
    
    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase *image) { setImage(image); }

    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase &image) { setImage(&image); }
    
    /// calles updated internally
    void update();
                  
  };
  
}

#endif
