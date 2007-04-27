#ifndef MOUSE_INTERACTION_INFO_H
#define MOUSE_INTERACTION_INFO_H

#include <vector>

namespace icl{
  /// for mouse interaction slots and signals
  struct MouseInteractionInfo{
    MouseInteractionInfo(){
      widgetX = -1;
      widgetY = -1;
      imageX = 0;
      imageY = 0;
      downmask[0] = downmask[1] = downmask[2] = 0;
      color.push_back(0);
      color.push_back(0);
      color.push_back(0);
      type = leaveEvent;
    }
    enum Type { moveEvent, dragEvent, pressEvent, releaseEvent, enterEvent, leaveEvent};
    int widgetX, widgetY;
    int imageX, imageY;
    int downmask[3];
    std::vector<float> color;
    Type type;
  };
}

#endif
