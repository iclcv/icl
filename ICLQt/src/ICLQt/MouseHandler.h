/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/MouseHandler.h                         **
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

#include <QtCore/QObject>
#include <ICLQt/MouseEvent.h>


namespace icl{
  namespace qt{

    /** \cond */
    class ICLWidget;
    /** \endcond */


    /// MouseEvent Handler  \ingroup COMMON
    /** Here's a short example:
        \code
        #include <ICLQt/Common.h>
        #include <iterator>

        GenericGrabber *grabber;
        ICLWidget *widget;

        class Mouse : public MouseHandler{
          public:
          virtual void process(const MouseEvent &event){
            std::cout << "image location: " << event.getPos() << std::endl;
            std::cout << "widget location: " << event.getWidgetPos() << std::endl;

            std::string eventNames[] = {"MoveEvent","DragEvent","PressEvent","ReleaseEvent", "EnterEvent","LeaveEvent"};
            std::cout << "type: " << eventNames[event.getType()] << std::endl;

            if(event.isPressEvent()){
              if(event.getColor().size()){
                std::cout << "color:";
                std::copy(event.getColor().begin(),event.getColor().end(),std::ostream_iterator<icl64f>(std::cout,","));
                std::cout << endl;
              }else{
                std::cout << "no color here!" << std::endl;
              }
            }
          }
        } mouse;

        void init(){
          widget = new ICLWidget(0);
          widget->setGeometry(200,200,640,480);
          widget->show();
          grabber = new GenericGrabber;
          widget->install(&mouse);
        }
        void run(){
          widget->setImage(grabber->grab());
          widget->update();
        }

        int main(int n, char **ppc){
          return ICLApp(n,ppc,"",init,run).exec();
        }
        \endcode
        */
    class ICLQt_API MouseHandler : public QObject{
      Q_OBJECT
      public:

      /// ICLWidget is allowed to connect to the private slot handleEvent
      friend class ICLWidget;

      /// mouse_handler function type
      typedef void (*mouse_handler)(const MouseEvent &event);

      /// Create a mouse handler with given callback function
      /** In most cases a MouseHandler can be used directly by passing
          a mouse_handler function to it's constructor. The mouse
          handler function is called by virtual void process
          automatically */
      explicit MouseHandler(mouse_handler handler):
      m_handler(handler){}

      protected:

      /// This constructor can be called from derived classes
      /** derived classes will reimplement
          virtual void process(const MouseEvent &event), so in
          this case, no external mouse_handler function needs to
          be passed */
      MouseHandler():
      m_handler(0){}


      public Q_SLOTS:

      /// connected to the ICLWidget's signal mouseEventOccured
      void handleEvent(const MouseEvent &event);

      public:
      /// this function is called automatically when the handleEvent slot is invoked
      /** It can be reimplemented for custom mouse interaction.
          If a mouse handling function is enough for your purpose, you can also
          pass a function of type mouse_handler to the constructor of a
          MouseHandler instance. The default implementation of process
          calls the handler function if it's not null.
          **/
      virtual void process(const MouseEvent &event);

      private:
      /// internal mouse handler function
      mouse_handler m_handler;
    };
  } // namespace qt
}

