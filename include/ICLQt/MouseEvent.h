#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include <vector>
#include <ICLCore/Img.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  
  /** \cond */
  class ICLWidget;
  /** \endcond */
  
  /// list of supported mouse event types
  enum MouseEventType { 
    MouseMoveEvent, 
    MouseDragEvent, 
    MousePressEvent, 
    MouseReleaseEvent, 
    MouseEnterEvent, 
    MouseLeaveEvent
  };
  
  /// list of supported mouse buttons
  /** when ever you'll find a bool downmaks[3],
      buttons are arranged by this enum order*/
  enum MouseButton{
    LeftMouseButton=0,
    MiddleMouseButton=1,
    RightMouseButton=2
  };
  
  /// Provided by interface MouseGrabber \ingroup COMMON
  /** Most commonly, mouse events are processed wrt. the current image coordinate frame
      of the ICLWidget where the mouse event occurs. So, getX(), getY() and getPos()
      return the pixel coordinate of a mouse event.
      If mouse interaction shall be visualized, this coordinates can directly be passed
      to the drawing functions of ICLDrawWidget's.
  */
  class MouseEvent{
    // event location in widget coordinates
    Point m_widgetPos;
    
    // event location in image coordinates (common)
    Point m_imagePos;
    
    // relative image coordinates
    Point32f m_relImagePos;
    
    // button downMask
    bool m_downMask[3];
    
    /// clicked color
    std::vector<double> m_color;
    
    /// ICLWidget where the event occured
    ICLWidget *m_widget;
    
    /// Type of this event (press, release, move, ...)
    MouseEventType m_type;
    
    /// private constructor, mouse events are created by the ICLWidget only
    MouseEvent(const Point &widgetPos,
               const Point &imagePos,
               const Point32f &relImagePos,
               const bool downMask[3],
               const std::vector<double> &color,
               MouseEventType type,
               ICLWidget *widget);
    
    public:
    /// Create an empty mouse event
    MouseEvent();
        
    /// ICLWidget is allowed to create mouse events
    friend class ICLWidget;
    
    /// returns event's x coordinate wrt. widget frame
    inline int getWidgetX() const { return m_widgetPos.x; }

    /// returns event's y coordinate wrt. widget frame
    inline int getWidgetY() const { return m_widgetPos.y; }

    /// returns event's location wrt. widget frame
    inline const Point &getWidgetPos() const { return m_widgetPos; }

    /// returns event's x coordinate wrt. image frame
    inline int getX() const { return m_imagePos.x; }

    /// returns event's y coordinate wrt. image frame
    inline int getY() const { return m_imagePos.y; }

    /// returns event's location wrt. image frame
    inline const Point &getPos() const { return m_imagePos; }
    
    /// returns event's relative x coordinate wrt. image frame
    inline float getRelX() const { return m_relImagePos.x; }

    /// returns event's relative y coordinate wrt. image frame
    inline float getRelY() const { return m_relImagePos.y; }

    /// returns event's relative location wrt. image frame
    inline const Point32f getRelPos() const { return m_relImagePos; }

    /// returns clicked pixels color (or a zero length vector, if there was no pixel)
    inline const std::vector<double> &getColor() const { return m_color; }
    
    /// returns if the widget's image was hit (and a color is available)
    inline bool hitImage() const { return m_color.size(); }
    
    /// returns the downmask in order [left, middle, right]- button
    inline std::vector<bool> getDownMask() const { return std::vector<bool>(m_downMask,m_downMask+3); }

    /// convenience function for left button
    inline bool isLeft() const { return m_downMask[LeftMouseButton]; }

    /// convenience function for middle button
    inline bool isMiddle() const { return m_downMask[MiddleMouseButton]; }

    /// convenience function for right button
    inline bool isRight() const { return m_downMask[RightMouseButton]; }

    /// convenience function for left button
    inline bool isLeftOnly() const { return (isLeft() && !isMiddle()) || (!isRight()); }

    /// convenience function for middle button
    inline bool isMiddleOnly() const { return (!isLeft() && isMiddle()) || (!isRight()); }

    /// convenience function for right button
    inline bool isRightOnly() const { return (!isLeft() && !isMiddle()) || (isRight()); }

    /// returns the event type
    inline const MouseEventType getType() const { return m_type; }
    
    /// conenience function for special event type
    inline bool isMoveEvent() const { return m_type == MouseMoveEvent; }

    /// conenience function for special event type
    inline bool isDragEvent() const { return m_type == MouseDragEvent; }

    /// conenience function for special event type
    inline bool isPressEvent() const { return m_type == MousePressEvent; }

    /// conenience function for special event type
    inline bool isReleaseEvent() const { return m_type == MouseReleaseEvent; }

    /// conenience function for special event type
    inline bool isEnterEvent() const { return m_type == MouseEnterEvent; }

    /// conenience function for special event type
    inline bool isLeaveEvent() const { return m_type == MouseLeaveEvent; }
    
    /// returns the ICLWidget, which produced this event
    inline ICLWidget *getWidget() const { return m_widget; }
  };
}

#endif
