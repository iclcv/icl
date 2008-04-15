#ifndef OSD_CAPTURE_VIDEO_BUTTON_H
#define OSD_CAPTURE_VIDEO_BUTTON_H

#include <iclOSDButton.h>

namespace icl{
  /// implementation of special button for recording framebuffer \ingroup UNCOMMON
  class OSDCaptureVideoButton : public OSDButton{
    public:
    OSDCaptureVideoButton(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent);
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);

  };
}

#endif
