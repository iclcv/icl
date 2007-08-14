#include <iclGUIEvent.h>


namespace icl{
  GUIEvent::GUIEvent(const std::string &id):
    m_bTriggered(false),m_sID(id){
  }
  void GUIEvent::trigger() { 
    m_bTriggered = true; 
  }

  bool GUIEvent::wasTriggered(bool reset) {
    if(m_bTriggered){
      if(reset) m_bTriggered = false;
      return true;
    }
    return false;
  }

  void GUIEvent::reset() { 
    m_bTriggered = false; 
  }
  const std::string &GUIEvent::getID() const { 
    return m_sID; 
  }
  
}
