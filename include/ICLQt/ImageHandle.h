#ifndef ICL_IMAGE_HANDLE
#define ICL_IMAGE_HANDLE


#include <ICLQt/GUIHandle.h>

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

    /// passes callback registration to wrapped ICLWidget instance)
    virtual void registerCallback(GUI::CallbackPtr cb, const std::string &events="all");
    
    /// passes callback registration to wrapped ICLWidget instance)
    virtual void removeCallbacks();
                  
  };
  
}

#endif
