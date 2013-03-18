/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineQuadrangleMouseHandler.cpp       **
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

#include <ICLQt/DefineQuadrangleMouseHandler.h>
#include <vector>
#include <ICLCore/ConvexHull.h>
#include <ICLCore/Line.h>

namespace icl{
  
  namespace qt{

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
      Mutex::Locker lock(this);
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
    }

    void DefineQuadrangleMouseHandler::setQuadrangle(const utils::Point ps[4]) throw (ICLException){
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
      Mutex::Locker lock(this);
      m_data->ps.assign(ps,ps+4);
    }
    
    
    void DefineQuadrangleMouseHandler::setOffset(const Point &o){
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::setOffset(p) was called before it was initialized!");
      }

      m_data->xoffset = o.x;
      m_data->yoffset = o.y;
    }
    
    std::vector<Point> DefineQuadrangleMouseHandler::getQuadrangle() const{
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::getQuadrangle() was called before it was initialized!");
      }
      std::vector<Point> ps = m_data->ps;
      return ps;
    }
    
    void DefineQuadrangleMouseHandler::process(const MouseEvent &e){
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::process(MouseEvent) was called before it was initialized!");
      }
      
      Point p = e.getPos();
      p.x -= m_data->xoffset;
      p.y -= m_data->yoffset;
    
      if(!m_data->bounds.contains(p.x,p.y)){
        return;
      }
    
      if(e.isReleaseEvent()){
        m_data->dragged = -1;
        std::fill(m_data->handles,m_data->handles+4,0);
      }
      if(e.isPressEvent()){
        for(int i=0;i<4;++i){
          if(m_data->ps[i].distanceTo(p) < 8){
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
          if(m_data->ps[i].distanceTo(p) < 8){
            m_data->handles[i] = 1;
          }else{
            m_data->handles[i] = 0;
          }
        }
      }
    }

    VisualizationDescription DefineQuadrangleMouseHandler::vis() const{
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("DefineQuadrangleMouseHandler::vis() was called before it was initialized!");
      }
      
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
        d.fill(255,0,0,1+127*m_data->handles[i]);
        d.rect(pso[i].x-5,pso[i].y-5,11,11);
      }
      return d;
    }

  } 
}
