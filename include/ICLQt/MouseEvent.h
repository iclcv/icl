/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/MouseEvent.h                             **
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

#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include <vector>
#include <QtCore/Qt>
#include <ICLCore/Img.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  
  /** \cond */
  class ICLWidget;
  /** \endcond */
  
  /// list of supported mouse event types
  enum MouseEventType { 
    MouseMoveEvent    = 0,                           //!< mouse moved
    MouseDragEvent    = 1,                           //!< mouse button pressed down and held
    MousePressEvent   = 2,                           //!< mouse button pressed
    MouseReleaseEvent = 3,                           //!< mouse button released
    MouseEnterEvent   = 4,                           //!< mouse entered area
    MouseLeaveEvent   = 5,                           //!< mouse left area
    MouseWheelEvent   = 6,                           //!< mouse wheel
    MAX_MOUSE_EVENT   = MouseWheelEvent              //!< highest enum value (enum value count = MAX_MOUSE_EVENT + 1)
  };

  /// list of supported mouse buttons
  /** when ever you'll find a bool downmaks[3],
      buttons are arranged by this enum order*/
  enum MouseButton{
    LeftMouseButton   = 0,                           //!< left mouse button
    MiddleMouseButton = 1,                           //!< middle mouse button
    RightMouseButton  = 2,                           //!< right moouse button
    MAX_MOUSE_BUTTON  = RightMouseButton             //!< highest enum value (enum value count = MAX_MOUSE_BUTTON + 1)
  };

  enum KeyboardModifier{
      NoModifier = Qt::NoModifier,                   //!< No modifier key is pressed.
      ShiftModifier = Qt::ShiftModifier,             //!< A Shift key on the keyboard is pressed.
      ControlModifier = Qt::ControlModifier,         //!< A Ctrl key on the keyboard is pressed.
      AltModifier = Qt::AltModifier,                 //!< An Alt key on the keyboard is pressed.
      MetaModifier = Qt::MetaModifier,               //!< A Meta key on the keyboard is pressed.
      KeypadModifier = Qt::KeypadModifier,           //!< A keypad button is pressed.
      GroupSwitchModifier = Qt::GroupSwitchModifier  //!< X11 only. A Mode_switch key on the keyboard is pressed.
  };

  /// Provided by interface MouseGrabber \ingroup COMMON
  /** Most commonly, mouse events are processed wrt. the current image coordinate frame
      of the ICLWidget where the mouse event occurs. So, getX(), getY() and getPos()
      return the pixel coordinate of a mouse event.
      If mouse interaction shall be visualized, this coordinates can directly be passed
      to the drawing functions of ICLDrawWidget's.
  */
  class MouseEvent{
    /// event location in widget coordinates
    Point m_widgetPos;
    
    /// event location in image coordinates (common)
    Point m_imagePos;
    
    /// relative image coordinates
    Point32f m_relImagePos;
    
    /// wheel delta (x: horizontal wheel, y: vertical wheel)
    Point m_wheelDelta;
   
    /// button downMask
    bool m_downMask[3];
    
    /// clicked color
    std::vector<double> m_color;
    
    /// ICLWidget where the event occured
    ICLWidget *m_widget;
    
    /// Type of this event (press, release, move, ...)
    MouseEventType m_type;
    
    /// ored list of active keyboard modifiers
    int m_keyboardModifiers;
    
    
    /// private constructor, mouse events are created by the ICLWidget only
    MouseEvent(const Point &widgetPos,
               const Point &imagePos,
               const Point32f &relImagePos,
               const bool downMask[3],
               const std::vector<double> &color,
	       const Point &wheelDelta,
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
    
    /// wheel delta (x: horizontal wheel, y: vertical wheel (common))
    /** We use the unit of Qt's QWheelEvent's delta:
    \code
    MouseEvent event = ...;
    int numDegrees = event.getWheelDelta().y / 8;
    int numSteps = numDegrees / 15;
    \endcode
    A positive delta value means that the wheel was rotated forward (for y) 
    and to the right (for x) */
    inline const Point &getWheelDelta() const { return m_wheelDelta; }
    
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
    
    /// convenience function for special event type
    inline bool isMoveEvent() const { return m_type == MouseMoveEvent; }

    /// convenience function for special event type
    inline bool isDragEvent() const { return m_type == MouseDragEvent; }

    /// convenience function for special event type
    inline bool isPressEvent() const { return m_type == MousePressEvent; }

    /// convenience function for special event type
    inline bool isReleaseEvent() const { return m_type == MouseReleaseEvent; }

    /// convenience function for special event type
    inline bool isEnterEvent() const { return m_type == MouseEnterEvent; }

    /// convenience function for special event type
    inline bool isLeaveEvent() const { return m_type == MouseLeaveEvent; }
    
    /// convenience function for special event type
    inline bool isWheelEvent() const { return m_type == MouseWheelEvent; }
    
    /// returns the ICLWidget, which produced this event
    inline ICLWidget *getWidget() const { return m_widget; }
    
    /// returns all active keyboard modifiers (ored)
    /** A certain KeyboardModifier m can be checked for presence by 
    \code
    bool is_m_present = event.getKeyboardModifiers() & m;
    \endcode
    or by using the isModifierActive(KeyboardModifier) method directly.
    */
    inline int getKeyboardModifiers() const { return m_keyboardModifiers; }
    
    /// returns whether a certain modifier is currently active
    inline bool isModifierActive(KeyboardModifier m) const { return m & m_keyboardModifiers; }
  };
}

#endif
