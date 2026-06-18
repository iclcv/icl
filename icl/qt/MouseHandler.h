// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <QtCore/QObject>
#include <icl/qt/MouseEvent.h>


namespace icl::qt {
  /** \cond */
  class ICLWidget;
  /** \endcond */

  /// Result of MouseHandler::process — drives the handler-chain dispatch.
  /** A widget dispatches a mouse event to its installed handlers in
      registration order (first installed = highest priority) and stops at the
      first handler that returns Processed. Return Forward to let the next
      handler (e.g. a camera-navigation handler installed last) see the event. */
  enum class MouseResult {
    Processed,   //!< event consumed — stop the chain here
    Forward      //!< not handled / observe-only — pass to the next handler
  };

  /// MouseEvent Handler  \ingroup COMMON
  /** Here's a short example:
      \code
      #include <icl/qt/Common.h>
      #include <iterator>

      ImageSource *grabber;
      ICLWidget *widget;

      class Mouse : public MouseHandler{
        public:
        virtual MouseResult process(const MouseEvent &event){
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
          return MouseResult::Forward;
        }
      } mouse;

      void init(){
        widget = new ICLWidget(0);
        widget->setGeometry(200,200,640,480);
        widget->show();
        grabber = new ImageSource;
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
    using mouse_handler = void(*)(const MouseEvent &event);

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
    /// process a mouse event; return whether it was consumed
    /** Reimplement for custom mouse interaction. Return MouseResult::Processed
        when you act on the event and want to stop the handler chain, or
        MouseResult::Forward to let the next installed handler see it.
        If a plain function is enough, pass a mouse_handler to the constructor
        instead; the default implementation calls it (if non-null) and returns
        Forward (a bare callback is observe-only and never blocks the chain). */
    virtual MouseResult process(const MouseEvent &event);

    private:
    /// internal mouse handler function
    mouse_handler m_handler;
  };
  } // namespace icl::qt