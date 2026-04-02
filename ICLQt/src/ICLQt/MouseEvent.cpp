// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/MouseEvent.h>
#include <QApplication>

using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {
    MouseEvent::MouseEvent(const Point &widgetPos,
                           const Point &imagePos,
                           const Point32f &imagePos32f,
                           const Point32f &relImagePos,
                           const bool downMask[3],
                           const std::vector<double> &color,
  			 const Point &wheelDelta,
                           MouseEventType type,
                           ICLWidget* widget):
      m_widgetPos(widgetPos),m_imagePos(imagePos),m_imagePos32f(imagePos32f),
      m_relImagePos(relImagePos),
      m_wheelDelta(wheelDelta),m_color(color),m_widget(widget),m_type(type){

      std::copy(downMask,downMask+3,m_downMask);

      m_keyboardModifiers = static_cast<int>(QApplication::keyboardModifiers());
    }

    MouseEvent::MouseEvent():
      m_widgetPos(Point::null),m_imagePos(Point::null),
      m_imagePos32f(Point32f::null),m_relImagePos(Point32f::null),
      m_widget(0),m_type(MouseLeaveEvent){

      std::fill(m_downMask,m_downMask+3,false);
    }
  } // namespace icl::qt