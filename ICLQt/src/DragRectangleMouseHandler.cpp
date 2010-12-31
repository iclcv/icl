/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/DragRectangleMouseHandler.cpp                **
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

#include <ICLQt/DragRectangleMouseHandler.h>
#include <ICLQt/DrawWidget.h>

namespace icl{

  DragRectangleMouseHandler::DragRectangleMouseHandler(int minDim):
    m_minDim(minDim),m_edge(0,100,255,255),m_fill(0,0,0,0),m_outer(0,100,255,150),
    m_edgeWhileDrag(255,0,0,255),m_fillWhileDrag(0,0,0,0),m_outerWhileDrag(255,0,0,50){
  }


  void DragRectangleMouseHandler::process(const MouseEvent &e){
    Mutex::Locker l(*this);
    if(e.isRight()){
      m_rect = Rect::null;
      return;
    }
    if(e.isPressEvent()){
      m_origin  = e.getPos();
      m_curr   = e.getPos();
    }else if(e.isDragEvent()){
      m_curr   = e.getPos();
    }
    if(e.isReleaseEvent()){
      if(m_origin != m_curr){
        m_rect = Rect(m_origin, Size(m_curr.x-m_origin.x,m_curr.y-m_origin.y )).normalized();
        if(m_rect.getDim() < m_minDim) {
          m_rect = Rect::null;
        }
      }
      m_origin = m_curr =  Point::null;
    }
  }
  
  static void vis_rect(ICLDrawWidget &w, const Rect &r, int imagew, int imageh, 
                       const Color4D & edgeColor, const Color4D &fillColor, 
                       const Color4D & outerColor){
    w.color(edgeColor);
    w.fill(fillColor);
    w.rect(r);
    
    if(outerColor[3]>0){
      w.color(0,0,0,0);
      w.fill(outerColor);
      w.rect(0,0,r.x,imageh);
      w.rect(r.right(),0,imagew-r.right(),imageh);
      w.rect(r.x,r.bottom(),r.width,imageh-r.bottom());
      w.rect(r.x,0,r.width,r.y);
    }
  }
  
  void DragRectangleMouseHandler::visualize(ICLDrawWidget &w){
    const Size s = w.getImageSize();
    const int &imagew = s.width;
    const int &imageh = s.height;
    Mutex::Locker l(this);
    if(m_rect != Rect::null){
      vis_rect(w, m_rect, imagew, imageh, m_edge, m_fill, m_outer);
    }
    if(m_curr != Point::null || m_origin != Point::null){
      Rect r(m_origin, Size(m_curr.x-m_origin.x,m_curr.y-m_origin.y));
      vis_rect(w, r, imagew, imageh, m_edgeWhileDrag, m_fillWhileDrag, m_outerWhileDrag);
    }
  }

  bool DragRectangleMouseHandler::hasRect() const{
    Mutex::Locker l(this);
    return m_rect != Rect::null; 
  }
  bool DragRectangleMouseHandler::hasDraggedRect() const{
    Mutex::Locker l(this);
    return m_curr != Point::null || m_origin != Point::null;
  }

  Rect DragRectangleMouseHandler::getRect() const{
    Mutex::Locker l(this);
    return m_rect;
  }

  Rect DragRectangleMouseHandler::getDragggedRect() const{
    Mutex::Locker l(this);
    return Rect(m_origin, Size(m_curr.x-m_origin.x,m_curr.y-m_origin.y));
  }


}
