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
#include <QtWidgets/QCheckBox>

namespace icl::qt {
  /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
  class ICLQt_API CheckBoxHandle : public GUIHandle<QCheckBox>{
    public:

    /// creates a n empty button handle
    CheckBoxHandle();

    /// create a new event with a given button id
    CheckBoxHandle(QCheckBox *cb, GUIWidget *w, bool *stateRef);

    /// checks this checkbox
    void check(bool execCallbacks=true);

    // unchecks this checkbox
    void uncheck(bool execCallbacks=true);

    /// defines the check-state
    inline void doCheck(bool on, bool execCallbacks=true){
      if(on) check(execCallbacks);
      else uncheck(execCallbacks);
    }

    // returns whether this the checkbox is currently checked
    bool isChecked() const;

    /// assigns the check-state (true → checked, false → unchecked).
    /// Also accepts arithmetic sources via implicit conversion to bool.
    void operator=(bool on) { doCheck(on); }

    /// parses `s` as a bool and sets the check-state.  Throws on bad input.
    void operator=(const std::string &s);

    /// Explicit readback.  Arithmetic specialization casts `isChecked()`
    /// to T; string specialization formats it as "0" or "1".
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(isChecked()); }

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;

    private:

    /// internal state reference variable
    bool *m_stateRef;

    /// Lock-free snapshot of the checkbox state.  Written from the
    /// GUI thread by a `stateChanged(int)` lambda installed in the
    /// primary ctor; read from any thread via `isChecked()`.
    /// Coexists with `m_stateRef` (the old `.out()`-allocated bool)
    /// during the thread-safety transition — both track the same
    /// value but `m_stateRef` reads are unsynchronized.
    std::shared_ptr<std::atomic<bool>> m_cache;

  };
  } // namespace icl::qt