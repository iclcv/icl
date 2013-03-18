/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/MouseEvent.cpp                         **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLQt/MouseEvent.h>
#include <QtGui/QApplication>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace qt{
    
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
      
      m_keyboardModifiers = (int)QApplication::keyboardModifiers();
    }
  
    MouseEvent::MouseEvent():
      m_widgetPos(Point::null),m_imagePos(Point::null),
      m_imagePos32f(Point32f::null),m_relImagePos(Point32f::null),
      m_widget(0),m_type(MouseLeaveEvent){
      
      std::fill(m_downMask,m_downMask+3,false);
    }
  } // namespace qt
}
