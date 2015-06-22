/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ThreadedUpdatableSlider.cpp            **
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

#include <ICLQt/ThreadedUpdatableSlider.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>

#include <QtGui/QMouseEvent>
#include <QtWidgets/QStyle>

using namespace icl::utils;

namespace icl{
  namespace qt{

    struct ThreadedUpdatableSlider::EventFilter : public QObject{
      ThreadedUpdatableSlider *m_parent;
      EventFilter(ThreadedUpdatableSlider *parent):
        QObject(parent), m_parent(parent){}
      bool eventFilter(QObject* watched, QEvent* event){
        QMouseEvent *m = dynamic_cast<QMouseEvent*>(event);
	if(m && watched == m_parent){
          //QObject::eventFilter(watched, event);
          //Qt::MouseButton b = m->button();
          QFlags<Qt::MouseButton> bs = m->buttons();
          if(m && (bs & Qt::LeftButton || 
                   bs & Qt::RightButton || 
                   bs & Qt::MidButton)){
            int val = QStyle::sliderValueFromPosition(m_parent->minimum(), 
                                                      m_parent->maximum(), 
                                                      m->x(), 
                                                      m_parent->width());
            val = (val/m_parent->m_stepping)*m_parent->m_stepping;
            m_parent->setValue(val);
            m_parent->update();
            return true;
          }
        }
        //DEBUG_LOG("received event of type " << (int)event->type());
	return QObject::eventFilter(watched, event);
      }
    };

  
    ThreadedUpdatableSlider::ThreadedUpdatableSlider(QWidget *parent): QSlider(parent), m_stepping(1){
      QObject::connect(this,SIGNAL(valueChanged(int)),this,SLOT(collectValueChanged(int)));
      QObject::connect(this,SIGNAL(sliderMoved(int)),this,SLOT(collectSliderMoved(int)));
      QObject::connect(this,SIGNAL(sliderPressed()),this,SLOT(collectSliderPressed()));
      QObject::connect(this,SIGNAL(sliderReleased()),this,SLOT(collectSliderReleased()));
      installEventFilter(new EventFilter(this));
    }
  
    ThreadedUpdatableSlider::ThreadedUpdatableSlider(Qt::Orientation o, QWidget *parent): QSlider(o, parent), m_stepping(1){
      QObject::connect(this,SIGNAL(valueChanged(int)),this,SLOT(collectValueChanged(int)));
      QObject::connect(this,SIGNAL(sliderMoved(int)),this,SLOT(collectSliderMoved(int)));
      QObject::connect(this,SIGNAL(sliderPressed()),this,SLOT(collectSliderPressed()));
      QObject::connect(this,SIGNAL(sliderReleased()),this,SLOT(collectSliderReleased()));
      installEventFilter(new EventFilter(this));
    }

    bool ThreadedUpdatableSlider::event(QEvent *event){
      ICLASSERT_RETURN_VAL(event,false);
      if(event->type() == SliderUpdateEvent::EVENT_ID){
        setValue(reinterpret_cast<SliderUpdateEvent*>(event)->value);
        return true;
      }else{
          return QSlider::event(event);
      }
    } 
  
    void ThreadedUpdatableSlider::registerCallback(const Function<void> &cb, const std::string &eventList){
      std::vector<std::string> ts = tok(eventList,",");
      for(unsigned int i=0;i<ts.size();++i){
        const std::string &events = ts[i];
        CB c = { events == "all" ? CB::all :
                 events == "move" ? CB::move :
                 events == "press" ? CB::press :
                 events == "release" ? CB::release :
                 events == "value" ? CB::value : (CB::Event)-1,
                 cb };
        if(c.event < 0) throw ICLException("ThreadedUpdatableSlider::registerCallback: invalid event type " + events);
        callbacks.push_back(c);
      }
    }
  
    void ThreadedUpdatableSlider::removeCallbacks(){
      callbacks.clear();
    }
    
    void ThreadedUpdatableSlider::collectValueChanged(int value){
      int stepped = (value/m_stepping)*m_stepping;
      if(stepped != value){
        setValue(stepped); // this will re-call this method with an appropriate value
                           // which then actually calls the callbacks ...
      }else{
        for(unsigned int i=0;i<callbacks.size();++i){
          if(callbacks[i].event == CB::all || callbacks[i].event == CB::value) callbacks[i].f();
        }
      }
    }
  
    void ThreadedUpdatableSlider::collectSliderPressed(){
      for(unsigned int i=0;i<callbacks.size();++i){
        if(callbacks[i].event == CB::all || callbacks[i].event == CB::press) callbacks[i].f();
      }  
    }
  
    void ThreadedUpdatableSlider::collectSliderMoved(int){
      for(unsigned int i=0;i<callbacks.size();++i){
        if(callbacks[i].event == CB::all || callbacks[i].event == CB::move) callbacks[i].f();
      }  
    }
  
    void ThreadedUpdatableSlider::collectSliderReleased(){
      for(unsigned int i=0;i<callbacks.size();++i){
        if(callbacks[i].event == CB::all || callbacks[i].event == CB::release) callbacks[i].f();
      }  
    }
      
  } // namespace qt
}
