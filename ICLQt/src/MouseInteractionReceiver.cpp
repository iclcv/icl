#include <MouseInteractionReceiver.h>

namespace icl{
  void MouseInteractionReceiver::mouseInteraction(MouseInteractionInfo *info){
    processMouseInteraction(info);
  } 
  void MouseInteractionReceiver::processMouseInteraction(MouseInteractionInfo *info){
    (void)info;
  }  
}
