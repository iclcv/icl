// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/DragRectangleMouseHandler.h>
#include <ICLQt/DrawWidget.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {
  DragRectangleMouseHandler::DragRectangleMouseHandler(int minDim):
    m_minDim(minDim),m_edge(0,100,255,255),m_fill(0,0,0,0),m_outer(0,100,255,150),
    m_edgeWhileDrag(255,0,0,255),m_fillWhileDrag(0,0,0,0),m_outerWhileDrag(255,0,0,50){
  }


  void DragRectangleMouseHandler::process(const MouseEvent &e){
    std::scoped_lock<std::recursive_mutex> l(getMutex());
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
    std::scoped_lock<std::recursive_mutex> l(getMutex());
    if(m_rect != Rect::null){
      vis_rect(w, m_rect, imagew, imageh, m_edge, m_fill, m_outer);
    }
    if(m_curr != Point::null || m_origin != Point::null){
      Rect r(m_origin, Size(m_curr.x-m_origin.x,m_curr.y-m_origin.y));
      vis_rect(w, r, imagew, imageh, m_edgeWhileDrag, m_fillWhileDrag, m_outerWhileDrag);
    }
  }

  bool DragRectangleMouseHandler::hasRect() const{
    std::scoped_lock<std::recursive_mutex> l(getMutex());
    return m_rect != Rect::null;
  }
  bool DragRectangleMouseHandler::hasDraggedRect() const{
    std::scoped_lock<std::recursive_mutex> l(getMutex());
    return m_curr != Point::null || m_origin != Point::null;
  }

  Rect DragRectangleMouseHandler::getRect() const{
    std::scoped_lock<std::recursive_mutex> l(getMutex());
    return m_rect;
  }

  Rect DragRectangleMouseHandler::getDragggedRect() const{
    std::scoped_lock<std::recursive_mutex> l(getMutex());
    return Rect(m_origin, Size(m_curr.x-m_origin.x,m_curr.y-m_origin.y));
  }


  } // namespace icl::qt