/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_BUTTON_HANDLE_H
#define ICL_BUTTON_HANDLE_H

#include <string>
#include <vector>
#include <ICLQt/GUIHandle.h>

/**\cond */
class QPushButton;
/**\endcond */


namespace icl{
  
  /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
  class ButtonHandle : public GUIHandle<QPushButton>{
    public:
    
    friend class ButtonGUIWidget;
    friend class ToggleButtonGUIWidget;
    
    /// typedefinition for a callback function
    //typedef void (*callback)(void);
    
    /// Interface for callback objects (functors)
    /*
    struct Callback{
      /// Destructor
      virtual ~Callback(){}
      /// call back function (pure virtual)
      virtual void operator()() = 0;
    };
    */
    
    /// creates a n empty button handle
    ButtonHandle();

    /// create a new event with a given button id
    ButtonHandle(QPushButton *b, GUIWidget *w);

    /// check if this event/button was triggered
    /** @param reset if set to true the internal boolen variable
                     is set to false, so wasTriggered returns true
                     only if the button was triggered again */
    bool wasTriggered(bool reset=true);
    
    /// trigger this event (sets the internal boolean variable to true)
    void trigger(bool execCallbacks=true){
      m_bTriggered = true;
      if(execCallbacks){
        cb();
      }
    }
    
    /// sets the internal boolean variable to false
    void reset();
    
    /// returns this buttons id (uncommon)
    const std::string &getID() const ;
    
    /// register/unregister a new/current callback Function
    //void registerCallback(callback c, bool remove=false);
    
    /// register/unregister a new/current callback Object
    //void registerCallback(Callback *c, bool remove=false);
    
    private:
    
    bool m_bTriggered; //!< internal boolean variable
    std::string m_sID; //!< corresponding id
    //   std::vector<callback> m_vecCallbackFuncs; //!< internal vector of callback functions
    //std::vector<Callback*> m_vecCallbackObjs; //!< internal vector of callback Object
  };  
}

#endif
