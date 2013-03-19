/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ThreadedUpdatableWidget.h              **
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

#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QEvent>
#include <ICLUtils/Macros.h>

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
    class ThreadedUpdatableWidget : public QWidget{
      public:
      
      /// Base constructor
      ThreadedUpdatableWidget(QWidget *parent = 0): QWidget(parent){}
      
      /// call this function to update a widget's UI from an external thread
      void updateFromOtherThread(){
        QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
      }
      
      /// automatically called by Qt's event processing mechanism
      virtual bool event ( QEvent * event ){
        ICLASSERT_RETURN_VAL(event,false);
        if(event->type() == QEvent::User){
          update();
          return true;
        }else{
          return QWidget::event(event);
        }
      } 
    };
  
  } // namespace qt
}

