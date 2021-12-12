/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/SliderHandle.h                         **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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
