#ifndef ICLGUI_EVENT_H
#define ICLGUI_EVENT_H

#include <string>

namespace icl{
  
  /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup COMMON
  class GUIEvent{
    public:
    /// create a new event with a given button id
    GUIEvent(const std::string &id="undefined");

    /// check if this event/button was triggered
    /** @param reset if set to true the internal boolen variable
                     is set to false, so wasTriggered returns true
                     only if the button was triggered again */
    bool wasTriggered(bool reset=true);
    
    /// trigger this event (sets the internal boolean variable to true)
    void trigger();
    
    /// sets the internal boolean variable to false
    void reset();
    
    /// returns this buttons id (uncommon)
    const std::string &getID() const ;
    
    private:
    bool m_bTriggered; //!< internal boolean variable
    std::string m_sID; //!< corresponding id
  };  
}

#endif
