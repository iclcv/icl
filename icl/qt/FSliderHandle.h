// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/ThreadedUpdatableSlider.h>

/** \cond */
class QSlider;
class QLCDNumber;
/** \endcond */

namespace icl::qt {
  /// Handle class for slider componets \ingroup HANDLES
  class ICLQt_API FSliderHandle : public GUIHandle<ThreadedUpdatableSlider>{
    /// associated display
    QLCDNumber *lcd;
    public:
    /// Create an empty slider handle
    FSliderHandle();

    /// create a slider handle
    FSliderHandle(ThreadedUpdatableSlider *sl,float *minV, float *maxV, float *M, float *B,int range, GUIWidget *w, QLCDNumber *lcd=0);

    /// retuns the QLCDNumber that is used as display
    /** result is null, if the slider was created without display */
    inline QLCDNumber *getDisplay() { return lcd; }

    /// set the min value
    void setMin(float min);

    /// set the max value
    void setMax(float max);

    /// set the range of the slider
    void setRange(float min, float max){ setMin(min); setMax(max); }

    /// set the current value of the slider
    void setValue(float val);

    /// sets all parameters of a slider
    void setAll(float min ,float max, float val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the slider
    float getMin() const;

    /// returns the current max. of the slider
    float getMax() const;

    /// returns the current value of the slider
    float getValue() const;

    /// assigns a new value to the slider (equal to setValue)
    void operator=(float val) { setValue(val); }

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

    private:

    /// internal utiltiy function which update the param for the slider equation
    void updateMB();

    /// current min value
    float *m_fMin;

    /// current max value
    float *m_fMax;

    /// utility function tranlating from valued to slider coordinates
    int f2i(float f) const{
      return static_cast<int>((f-*m_fB)/ *m_fM);
    }

    /// utility function tranlating from slider to value coordinates
    float i2f(int i) const{
      return *m_fM*i+*m_fB;
    }

    /// Slope for the internal slider equation
    float *m_fM;

    /// Bias for the internal slider equation
    float *m_fB;

    /// accumulator of the current slider range
    int m_iSliderRange;
  };

  } // namespace icl::qt