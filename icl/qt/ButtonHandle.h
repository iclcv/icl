// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>

#include <atomic>
#include <memory>
#include <string>
#include <type_traits>
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
      m_triggered->store(true, std::memory_order_relaxed);
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

    /// Explicit readback — returns the wrapped button's current checked
    /// state static-cast to T.  Provider-only (buttons don't accept
    /// assignment; they're read for their state).
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>((*this)->isChecked()); }

    private:

    /// Lock-free "was triggered" flag — set by `trigger()` on the GUI
    /// thread (Qt signal) and read-reset by `wasTriggered()` on the
    /// app thread.  shared_ptr so copies of the handle observe the
    /// same flag; std::atomic so reads/writes don't race on bool.
    std::shared_ptr<std::atomic<bool>> m_triggered;
    std::string m_sID; //!< corresponding id
  };
  } // namespace icl::qt