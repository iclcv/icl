/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ThreadedUpdatableSlider.h                **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_THREADED_UPDATABLE_SLIDER_H
#define ICL_THREADED_UPDATABLE_SLIDER_H

#include <QtGui/QSlider>
#include <QtGui/QApplication>
#include <ICLQt/SliderUpdateEvent.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Function.h>

namespace icl{
  
  /// Compability class 
  /** This class provides a compability function for asyncronous updating
      of a QWidget.
      As QWidget::update() has shown to be not as threadsafe as expected, 
      the new updateFromOtherThread function fixes this problem.
      
      updateFromOtherThread used QApplication::postEvent, to post a UserType
      QEvent to this object, which is caught in the overloaded event() function
  */
  class ThreadedUpdatableSlider : public QSlider{
    Q_OBJECT;
    
    /// internally callback type
    struct CB{
      /// associated event 
      enum Event{ press,release,move,value,all } event;
      Function<void> f; //!< associated 'void f()' -function
    };
    
    /// internal list of callbacks
    std::vector<CB> callbacks;
    
    public:
    
    /// Base constructor
    ThreadedUpdatableSlider(QWidget *parent = 0);

    ThreadedUpdatableSlider(Qt::Orientation o, QWidget *parent = 0);
    
    /// call this function to update a widget's UI from an external thread
    void setValueFromOtherThread(int value){
      QApplication::postEvent(this,new SliderUpdateEvent(value),Qt::HighEventPriority);
    }
    
    /// automatically called by Qt's event processing mechanism
    virtual bool event ( QEvent * event ){
      ICLASSERT_RETURN_VAL(event,false);
      if(event->type() == SliderUpdateEvent::EVENT_ID){
        setValue(reinterpret_cast<SliderUpdateEvent*>(event)->value);
        return true;
      }else{
        return QSlider::event(event);
      }
    } 
    
    /// registers a void-callback function to the given event names
    /** allowed event names are
        - press (when the slider is pressed)
        - release (when the slider is released)
        - move (when the slider is moved)
        - value (when the value is changed)
        - all (for all events)
    */
    void registerCallback(const Function<void> &cb, const std::string &events="value");

    /// removes all callbacks associated to this slider component
    void removeCallbacks();

    protected slots:
    /// for collecting slider singnals
    void collectValueChanged(int);

    /// for collecting slider singnals
    void collectSliderPressed();

    /// for collecting slider singnals
    void collectSliderMoved(int);

    /// for collecting slider singnals
    void collectSliderReleased();
    
  };

}

#endif
