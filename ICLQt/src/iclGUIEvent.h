#ifndef ICLGUI_EVENT_H
#define ICLGUI_EVENT_H

#include <string>

namespace icl{
  class GUIEvent{
    public:
    GUIEvent(const std::string &id="undefined");


    bool wasTriggered(bool reset=true);
    
    void trigger();
    
    void reset();
    
    const std::string &getID() const ;
    
    private:
    bool m_bTriggered;
    std::string m_sID;
  };  
}

#endif
