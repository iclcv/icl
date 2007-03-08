#include <vector>
#ifndef MOUSE_INTERACTION_INFO_H
#define MOUSE_INTERACTION_INFO_H


namespace icl{
  /// for mouse interaction slots and signals
  struct MouseInteractionInfo{
    enum Type { moveEvent, dragEvent, pressEvent, releaseEvent, enterEvent, leaveEvent};
    int widgetX, widgetY;
    int imageX, imageY;
    int downmask[3];
    std::vector<float> color;
    Type type;
  };
}

#endif
