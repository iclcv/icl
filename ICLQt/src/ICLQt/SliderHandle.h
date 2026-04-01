// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ThreadedUpdatableSlider.h>


/** \cond */
class QLCDNumber;
/** \endcond */

namespace icl{
  namespace qt{

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

      /// assigns a new value to the slider (equal to setValue)
      void operator=(int val) { setValue(val); }

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

  } // namespace qt
}
