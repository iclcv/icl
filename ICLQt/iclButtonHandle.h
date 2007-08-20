#ifndef ICLGUI_EVENT_H
#define ICLGUI_EVENT_H

#include <string>
#include <vector>

namespace icl{
  
  /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup COMMON
  class GUIEvent{
    public:
    
    /// typedefinition for a callback function
    typedef void (*callback)(void);
    
    /// Interface for callback objects (functors)
    struct Callback{
      /// Destructor
      virtual ~Callback(){}
      /// call back function (pure virtual)
      virtual void operator()() = 0;
    };
    
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
    
    /// register/unregister a new/current callback Function
    void registerCallback(callback c, bool remove=false);
    
    /// register/unregister a new/current callback Object
    void registerCallback(Callback *c, bool remove=false);
    
    private:
    bool m_bTriggered; //!< internal boolean variable
    std::string m_sID; //!< corresponding id
    std::vector<callback> m_vecCallbackFuncs; //!< internal vector of callback functions
    std::vector<Callback*> m_vecCallbackObjs; //!< internal vector of callback Object
  };  
}

#endif
