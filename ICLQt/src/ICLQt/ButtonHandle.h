// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <memory>
#include <ICLQt/GUIHandle.h>
#include <string>
#include <vector>
#include <QPushButton>

//#include <QPushButton>

/**\cond */
class QPushButton;
/**\endcond */


namespace icl::qt {
    /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
    class ButtonHandle : public GUIHandle<QPushButton>{
      public:

      friend class ButtonGUIWidget;
      friend class ToggleButtonGUIWidget;

      /// creates a n empty button handle
      ICLQt_API ButtonHandle();

      /// create a new event with a given button id
      ICLQt_API ButtonHandle(QPushButton *b, GUIWidget *w);

      /// check if this event/button was triggered
      /** @param reset if set to true the internal boolen variable
                       is set to false, so wasTriggered returns true
                       only if the button was triggered again */
      ICLQt_API bool wasTriggered(bool reset = true);

      /// trigger this event (sets the internal boolean variable to true)
      void trigger(bool execCallbacks = true){
        *m_triggered = true;
        if(execCallbacks){
          cb();
        }
      }

	  ICLQt_API void setButtonText(std::string const &text) {
		  (**this)->setText(text.c_str());
	  }

      /// sets the internal boolean variable to false
      ICLQt_API void reset();

      /// returns this buttons id (uncommon)
      ICLQt_API const std::string &getID() const;

      private:

      std::shared_ptr<bool> m_triggered; //!< internal boolean variable
      std::string m_sID; //!< corresponding id
    };
  } // namespace icl::qt