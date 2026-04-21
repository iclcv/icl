// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <QtWidgets/QSpinBox>

#include <atomic>
#include <memory>
#include <string>
#include <type_traits>

namespace icl::qt {
  /// Handle class for spinner components \ingroup HANDLES
  class ICLQt_API SpinnerHandle : public GUIHandle<QSpinBox>{

    public:

    /// Create an empty spinner handle
    SpinnerHandle() = default;

    /// create a new SpinnerHandle wrapping `sb`.  Seeds the lock-free
    /// value cache and installs a Qt connection that writes to it on
    /// every `valueChanged(int)` — runs on the GUI thread.  The cache
    /// is held by shared_ptr, captured by value into the lambda, so it
    /// outlives all handle copies.  The connection's context is `sb`,
    /// so it's dropped automatically when the widget is destroyed.
    SpinnerHandle(QSpinBox *sb, GUIWidget *w);

    /// set the min value
    void setMin(int min);

    /// set the max value
    void setMax(int max);

    /// set the range of the spin-box
    void setRange(int min, int max) { setMin(min); setMax(max); }

    /// set the current value of the spin-box
    void setValue(int val);

    /// sets all parameters of a spin-box
    void setAll(int min ,int max, int val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the spin-box
    int getMin() const;

    /// returns the current max. of the spin-box
    int getMax() const;

    /// returns the current value of the spin-box
    int getValue() const;

    /// assigns a new value to the spin-box (equal to setValue)
    void operator=(int val) { setValue(val); }

    /// parses `s` as an int and sets the spin-box.  Throws on bad input.
    void operator=(const std::string &s);

    /// Explicit readback.  Arithmetic specialization static-casts
    /// the current value; string specialization formats it.
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(getValue()); }

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;

    private:
    /// internally used utility function
    QSpinBox *sb() { return **this; }

    /// internally used utility function
    const QSpinBox *sb() const{ return **this; }

    /// Lock-free snapshot of `valueChanged(int)`.  Written from the
    /// GUI thread by a lambda wired to the signal; read from any
    /// thread via `getValue()` / `as<T>()`.  Held by shared_ptr so
    /// copies of the handle observe the same cache.  Null on a
    /// default-constructed handle (no widget).
    std::shared_ptr<std::atomic<int>> m_cache;
  };
  } // namespace icl::qt