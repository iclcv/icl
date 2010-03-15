/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/MouseEvent.cpp                               **
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
*********************************************************************/

#include <ICLQt/MouseEvent.h>

namespace icl{
  
  MouseEvent::MouseEvent(const Point &widgetPos,
                         const Point &imagePos,
                         const Point32f &relImagePos,
                         const bool downMask[3],
                         const std::vector<double> &color,
                         MouseEventType type,
                         ICLWidget* widget):
    m_widgetPos(widgetPos),m_imagePos(imagePos),
    m_relImagePos(relImagePos),
    m_color(color),m_widget(widget),m_type(type){
    
    std::copy(downMask,downMask+3,m_downMask);
  }

  MouseEvent::MouseEvent():
    m_widgetPos(Point::null),m_imagePos(Point::null),
    m_relImagePos(Point32f::null),
    m_widget(0),m_type(MouseLeaveEvent){
    
    std::fill(m_downMask,m_downMask+3,false);
  }
}
