#include <iclGUIEvent.h>


namespace icl{
  GUIEvent::GUIEvent(const std::string &id):
    m_bTriggered(false),m_sID(id){
  }
  void GUIEvent::trigger() { 
    m_bTriggered = true; 
    for(unsigned int i=0;i<m_vecCallbackFuncs.size();i++){
      m_vecCallbackFuncs[i]();
    }
    for(unsigned int i=0;i<m_vecCallbackObjs.size();i++){
      (*m_vecCallbackObjs[i])();
    }
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

  void  GUIEvent::registerCallback(GUIEvent::callback c, bool remove){
    if(remove){
      std::vector<callback>::iterator it =  find(m_vecCallbackFuncs.begin(),m_vecCallbackFuncs.end(),c);
      if(it !=  m_vecCallbackFuncs.end()){
        m_vecCallbackFuncs.erase(it);
      }
    }else{
      m_vecCallbackFuncs.push_back(c);
    }
    
  }
  

  void  GUIEvent::registerCallback(GUIEvent::Callback *c, bool remove){
    if(remove){
      std::vector<Callback*>::iterator it =  find(m_vecCallbackObjs.begin(),m_vecCallbackObjs.end(),c);
      if(it !=  m_vecCallbackObjs.end()){
        m_vecCallbackObjs.erase(it);
      }
    }else{
      m_vecCallbackObjs.push_back(c);
    }
  
  }

}
