// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/ThreadedUpdatableSlider.h>

#include <string>
#include <type_traits>


/** \cond */
class QLCDNumber;
/** \endcond */

namespace icl::qt {
  /** \cond */
  class ThreadedUpdatableSlider;
  /** \endcond */

  /// Handle class for slider componets \ingroup HANDLES
  class ICLQt_API SliderHandle : public GUIHandle<ThreadedUpdatableSlider>{

    /// associated display
    QLCDNumber *lcd;

    public:
    /// Creates and empty slider handle
    SliderHandle():lcd(0){}

    /// create a slider handle
    SliderHandle(ThreadedUpdatableSlider *sl, GUIWidget *w, QLCDNumber *lcd=0):GUIHandle<ThreadedUpdatableSlider>(sl,w),lcd(lcd){}

    /// retuns the QLCDNumber that is used as display
    /** result is null, if the slider was created without display */
    inline QLCDNumber *getDisplay() { return lcd; }

    /// set the min value
    void setMin(int min);

    /// set the max value
    void setMax(int max);

    /// set the range of the slider
    void setRange(int min, int max);

    /// set the current value of the slider
    void setValue(int val);

    /// sets all parameters of a slider
    void setAll(int min ,int max, int val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the slider
    int getMin() const;

    /// returns the current max. of the slider
    int getMax() const;

    /// returns the current value of the slider
    int getValue() const;

    /// assigns a new value to the slider (equal to setValue).
    /// Accepts any arithmetic source via the implicit conversion to `int`,
    /// so `slider = 3.7f` and `slider = 99L` work out of the box.
    void operator=(int val) { setValue(val); }

    /// parses `s` as an int and sets the slider.  Throws if `s` is not
    /// a valid int representation.
    void operator=(const std::string &s);

    /// Explicit readback — `int v = slider.as<int>()`,
    /// `double v = slider.as<double>()`, …  Arithmetic specialization
    /// returns the current value static-cast to T.  No implicit
    /// conversion operators on this class, on purpose: implicit casts
    /// cause subtle overload-resolution bugs that are very hard to
    /// diagnose later.  Callers who want the raw integer can also use
    /// the named `getValue()`.
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(getValue()); }

    /// String readback — formats the current value via `utils::str()`.
    /// (Defined out-of-line in `SliderHandle.cpp`.)
    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const;

    /// overloaded method for registering callbacks to specific slider events
    /** <b>Please note:</b> Only this callback mechanism is overloaded for the slider class
        Simple GUI callbacks are stored within the ThreadedUpdatableSlider class, while
        complex callbacks (those that get the GUI components handle name as parameters) are
        stored and handled within the GUIHandleBase class.
        Allowed values for the event parameter are comma-separated lists that consist of
        the following tokes:
        - press (slider is pressed)
        - release (slider is released)
        - move (slider is moved)
        - value (the slider value is changed)
        - all (all event types)
    */
    virtual void registerCallback(const GUI::Callback &cb, const std::string &events="value"){
      (***this).registerCallback(cb,events);
    }

    /// import the other register callback method
    using GUIHandleBase::registerCallback;

    /// empties both callback locations (GUIHandleBase and ThreadedUpdatableSlider)
    virtual void removeCallbacks(){
      GUIHandleBase::removeCallbacks();
      (***this).removeCallbacks();
    }

  };

  } // namespace icl::qt