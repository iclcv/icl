// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DefineQuadrangleMouseHandler.h>
#include <icl/qt/Widget.h>
#include <vector>
#include <icl/core/ConvexHull.h>
#include <icl/core/Line.h>
#include <mutex>

namespace icl::qt {
    using namespace utils;
    using namespace core;

    struct DefineQuadrangleMouseHandler::Data{
      std::vector<Point> ps;
      Rect bounds;
      int handles[4];
      int dragged;
      bool convexOnly;
      int xoffset;
      int yoffset;
      float handleSize;
    };

    static bool is_convex(const std::vector<Point> &ps){
      bool a = convexHull(ps).size() == 5;
      bool b = !Line(ps[0],ps[1]).intersects(Line(ps[2],ps[3]));
      bool c = !Line(ps[1],ps[2]).intersects(Line(ps[3],ps[0]));
      return a && b && c;
    }

    DefineQuadrangleMouseHandler::DefineQuadrangleMouseHandler():m_data(0){
    }

    DefineQuadrangleMouseHandler::DefineQuadrangleMouseHandler(const Size &maxSize, bool convexOnly):m_data(0){
      init(maxSize, convexOnly);
    }

    DefineQuadrangleMouseHandler::~DefineQuadrangleMouseHandler(){
      ICL_DELETE(m_data);
    }

    void DefineQuadrangleMouseHandler::init(const Size &maxSize, bool convexOnly){
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      ICL_DELETE(m_data);
      m_data = new Data;

      m_data->ps.resize(4);
      m_data->bounds = Rect(Point::null,maxSize-Size(1,1));
      m_data->xoffset = m_data->yoffset = 0;
      m_data->convexOnly = convexOnly;

      Rect r = m_data->bounds.enlarged(-20);
      m_data->ps[0] = r.ul();
      m_data->ps[1] = r.ur();
      m_data->ps[2] = r.lr();
      m_data->ps[3] = r.ll();

      std::fill(m_data->handles,m_data->handles+4,0);
      m_data->dragged = -1;

      m_data->handleSize = 8;
    }

    void DefineQuadrangleMouseHandler::setQuadrangle(const utils::Point ps[4]){
      Rect r(Point::null,Size(m_data->bounds.width+1,m_data->bounds.height+1));

      for(int i=0;i<4;++i){
        if(!r.contains(ps[i].x,ps[i].y)){
          throw ICLException("DefineQuadrangleMouseHandler::setQuadrangle: at least one of the "
                             "given points is not contained by the defined image bounds");
        }
      }
      std::vector<Point> psn(ps,ps+4);
      if(m_data->convexOnly && !is_convex(psn)){
        throw ICLException("DefineQuadrangleMouseHandler::setQuadrangle: given quadrangle is not convex "
                           "or twisted");
      }
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      m_data->ps.assign(ps,ps+4);
    }


    void DefineQuadrangleMouseHandler::setOffset(const Point &o){
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::setOffset(p) was called before it was initialized!");
      }

      m_data->xoffset = o.x;
      m_data->yoffset = o.y;
    }

    std::vector<Point> DefineQuadrangleMouseHandler::getQuadrangle() const{
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::getQuadrangle() was called before it was initialized!");
      }
      std::vector<Point> ps = m_data->ps;
      return ps;
    }

    void DefineQuadrangleMouseHandler::process(const MouseEvent &e){
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::process(MouseEvent) was called before it was initialized!");
      }

      Point p = e.getPos();
      p.x -= m_data->xoffset;
      p.y -= m_data->yoffset;

      if(!m_data->bounds.contains(p.x,p.y)){
        return;
      }

      // Click detection uses widget-pixel distance so the tolerance is
      // independent of image/widget scale. Rendering stays in image coords
      // (so the handle size scales visually with zoom). We pick the larger
      // of handleSize-projected-to-widget and MIN_CLICK_PX as the threshold.
      constexpr float MIN_CLICK_PX = 10.0f;
      const int iw = m_data->bounds.width + 1;
      const int ih = m_data->bounds.height + 1;
      ICLWidget *w = e.getWidget();
      Rect r = w ? w->getImageRect(true) : Rect(0,0,iw,ih);
      const float sx = iw > 0 && r.width  > 0 ? float(r.width)  / iw : 1.f;
      const float sy = ih > 0 && r.height > 0 ? float(r.height) / ih : 1.f;
      const Point pw = e.getWidgetPos();
      auto handleWidgetPos = [&](int i) {
        return Point32f((m_data->ps[i].x + m_data->xoffset) * sx + r.x,
                        (m_data->ps[i].y + m_data->yoffset) * sy + r.y);
      };
      auto hitsHandle = [&](int i){
        const float projected = m_data->handleSize * std::min(sx, sy);
        const float thresh = std::max(projected, MIN_CLICK_PX);
        return handleWidgetPos(i).distanceTo(Point32f(pw.x, pw.y)) < thresh;
      };

      if(e.isReleaseEvent()){
        m_data->dragged = -1;
        std::fill(m_data->handles,m_data->handles+4,0);
      }
      if(e.isPressEvent()){
        for(int i=0;i<4;++i){
          if(hitsHandle(i)){
            m_data->dragged = i;
            m_data->handles[i] = 2;
          }
        }
      }
      if(e.isDragEvent()){
        if(m_data->dragged != -1){
          Point tmp = m_data->ps[m_data->dragged];
          m_data->ps[m_data->dragged] = p;
          if(m_data->convexOnly && !is_convex(m_data->ps)){
            m_data->ps[m_data->dragged] = tmp;
          }
        }
      }else if(e.isMoveEvent()){
        for(int i=0;i<4;++i){
          m_data->handles[i] = hitsHandle(i) ? 1 : 0;
        }
      }
    }

    void DefineQuadrangleMouseHandler::setHandleSize(float size){
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::setHandlesize() was called before it was initialized!");
      }
      m_data->handleSize = size;
    }

    VisualizationDescription DefineQuadrangleMouseHandler::vis() const{
      std::scoped_lock<std::recursive_mutex> lock(getMutex());
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::vis() was called before it was initialized!");
      }

      const float r = m_data->handleSize;
      VisualizationDescription d;
      d.color(255,0,0,255);
      d.linewidth(2);
      d.fill(255,0,0,40);
      std::vector<Point> pso = m_data->ps;
      for(int i=0;i<4;++i){
        pso[i].x += m_data->xoffset;
        pso[i].y += m_data->yoffset;
      }
      d.polygon(pso);
      d.linewidth(1);
      for(int i=0;i<4;++i){
        d.fill(255,0,0,1+60*m_data->handles[i]);
        d.rect(pso[i].x-r,pso[i].y-r,2*r+1,2*r+1);
      }
      return d;
    }

  }