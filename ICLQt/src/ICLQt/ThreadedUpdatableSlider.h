/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ThreadedUpdatableSlider.h              **
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

#include <QSlider>
#include <QtCore/QThread>
#include <QApplication>
#include <ICLQt/SliderUpdateEvent.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Function.h>

namespace icl{
  namespace qt{

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
        utils::Function<void> f; //!< associated 'void f()' -function
      };

      /// internal list of callbacks
      std::vector<CB> callbacks;

      int m_stepping;

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
      void registerCallback(const utils::Function<void> &cb, const std::string &events = "value");

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

  } // namespace qt
}

