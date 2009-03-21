#include <iclMouseHandler.h>

namespace icl{
  void MouseHandler::handleEvent(const MouseEvent &event){
    process(event);
  } 
  void MouseHandler::process(const MouseEvent &event){
    if(m_handler)m_handler(event);
  }  
}
