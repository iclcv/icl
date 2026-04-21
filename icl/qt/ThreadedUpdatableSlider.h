// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <QSlider>
#include <QtCore/QThread>
#include <QApplication>
#include <icl/qt/SliderUpdateEvent.h>
#include <icl/utils/Macros.h>
#include <atomic>
#include <functional>

namespace icl::qt {
  /// Compability class
  /** This class provides a compability function for asyncronous updating
      of a QWidget.
      As QWidget::update() has shown to be not as threadsafe as expected,
      the new updateFromOtherThread function fixes this problem.

      updateFromOtherThread used QApplication::postEvent, to post a UserType
      QEvent to this object, which is caught in the overloaded event() function
  */
  class ICLQt_API ThreadedUpdatableSlider : public QSlider{
    Q_OBJECT;

    struct EventFilter;
    friend struct EventFilter;
    //int m_stepping;

    /// internally callback type
    struct CB{
      /// associated event
      enum Event{ press,release,move,value,all } event;
      std::function<void()> f; //!< associated 'void f()' -function
    };

    /// internal list of callbacks
    std::vector<CB> callbacks;

    int m_stepping;

    /// Lock-free snapshot of the current slider value.  Written from the
    /// GUI thread in `collectValueChanged` (the slot Qt wires to
    /// `valueChanged(int)`); read from any thread via `atomicValue()`.
    /// Exists so that application-thread readers (`SliderHandle::getValue()`)
    /// don't have to touch `QSlider::value()` cross-thread — Qt widgets
    /// are not thread-safe for reads outside the GUI thread.
    std::atomic<int> m_atomicValue{0};

    public:

    /// Base constructor
    ThreadedUpdatableSlider(QWidget *parent = 0);

    ThreadedUpdatableSlider(Qt::Orientation o, QWidget *parent = 0);

    /// call this function to update a widget's UI from an external thread
    /** new, if this is called from the GUI thread, setValue is called directly
        without using Qt's signal mechanism*/
    void setValueFromOtherThread(int value){
      value = (value/m_stepping)*m_stepping;
      if(QThread::currentThread() == QCoreApplication::instance()->thread()){
        setValue(value);
      }else{
        QApplication::postEvent(this,new SliderUpdateEvent(value),Qt::HighEventPriority);
      }
    }

    /// Lock-free read of the current slider value.  Safe from any
    /// thread — returns the value cached by `collectValueChanged` the
    /// last time Qt fired `valueChanged(int)` on the GUI thread.
    /// Prefer this over `QSlider::value()` from non-GUI threads.
    int atomicValue() const noexcept {
      return m_atomicValue.load(std::memory_order_relaxed);
    }

    /// the given stepping is automatically clipped to [1,...]
    void setStepping(int stepping){
      if(m_stepping < 1) m_stepping = 1;
      m_stepping = stepping;
    }

    /// automatically called by Qt's event processing mechanism
    virtual bool event ( QEvent * event );

    /// registers a void-callback function to the given event names
    /** allowed event names are
        - press (when the slider is pressed)
        - release (when the slider is released)
        - move (when the slider is moved)
        - value (when the value is changed)
        - all (for all events)
    */
    void registerCallback(const std::function<void()> &cb, const std::string &events = "value");

    /// removes all callbacks associated to this slider component
    void removeCallbacks();

    protected Q_SLOTS:
    /// for collecting slider singnals
    void collectValueChanged(int);

    /// for collecting slider singnals
    void collectSliderPressed();

    /// for collecting slider singnals
    void collectSliderMoved(int);

    /// for collecting slider singnals
    void collectSliderReleased();

  };

  } // namespace icl::qt