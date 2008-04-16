#ifndef OSD_CAPTURE_VIDEO_WIDGET_H
#define OSD_CAPTURE_VIDEO_WIDGET_H

#include <iclOSDWidget.h>
#include "iclWidgetCaptureMode.h"

namespace icl{
  /// implementation of special widget for recording framebuffer \ingroup UNCOMMON
  class OSDCaptureVideoWidget : public OSDWidget{
    public:
    /// Create new instance
    OSDCaptureVideoWidget(int id, int startID,int stopID, int pauseID, int sliderID, Rect r, ImageWidget* poIW , OSDWidget *poParent);
  };
}

#endif
