/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineRectanglesMouseHandler.cpp       **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#include <ICLQt/AdjustGridMouseHandler.h>
#include <ICLCore/ConvexHull.h>
#include <ICLCore/Line.h>
#include <ICLCore/Color.h>
#include <ICLMath/Projective4PointTransform.h>
#include <ICLUtils/CompatMacros.h>



namespace icl{
  namespace qt{

    using namespace utils;
    using namespace core;

    struct AdjustGridMouseHandler::Data{
      struct Handle{
        Point pos;
        int state;
        Handle():state(0){}
      };

      struct Grid;

      struct Texture{
        Size32f size;
        std::vector<Line32f> lines;
        mutable math::Projective4PointTransform t4;

        /*math::Mat3 Ainv;

            static math::Mat3 create_mapping_matrix(const Point32f &a, const Point32f &b,
            const Point32f &c, const Point32f &d){
            // see http://math.stackexchange.com/questions/296794/finding-the-transform-matrix-from-4-projected-points-with-javascript
            math::DynMatrix<float> M(3,3), B(1,3);
            M(0,0) = a.x;
            M(1,0) = b.x;
            M(2,0) = c.x;

            M(0,1) = a.y;
            M(1,1) = b.y;
            M(2,1) = c.y;

            M(0,2) = M(1,2) = M(2,2) = 1;

            B[0] = d.x;
            B[1] = d.y;
            B[2] = 1;

            math::DynMatrix<float> x = M.solve(B,"svd");

            const float &l1 = x[0], &l2 = x[1], &l3 = x[2];
            return math::Mat3(l1*a.x,l2*b.x,l3*c.x,
            l1*a.y,l2*b.y,l3*c.y,
            l1,    l2,    l3);
            }
            */

        Texture(const Size32f &size, const std::vector<core::Line32f> &lines):
          size(size),lines(lines){

          t4.init(Rect32f(0,0,size.width,size.height));
          /*
              Ainv = create_mapping_matrix(Point32f(0,0), Point32f(size.width,0),
              Point32f(size.width,size.height),
              Point32f(0,size.height)).pinv();
              */
        }
        Point ref[4];
        std::vector<Line32f> linesToRender;

        /*
            static Point32f map(const math::Mat3 &C, const Point32f &p){
            math::Vec3 m = C * math::Vec3(p.x,p.y,1);
            return Point32f(m[0]/m[2],m[1]/m[2]);
            }
            */

        /* math::Mat3 create_projective_transform_matrix(const Data::Grid &g) const{
            const Point32f a = g.handles[0].pos;
            const Point32f b = g.handles[1].pos;
            const Point32f c = g.handles[2].pos;
            const Point32f d = g.handles[3].pos;

            return create_mapping_matrix(a,b,c,d) * Ainv;
            }
            */

        std::vector<Point32f> mapPoints(const Data::Grid &g, const std::vector<Point32f> &srcPoints) const{
          t4.setDstQuad(g.handles[0].pos, g.handles[1].pos,
                        g.handles[2].pos, g.handles[3].pos);
          return t4.map(srcPoints);
          /*
              const math::Mat3 C = create_projective_transform_matrix(g);
              std::vector<Point32f> dst(srcPoints.size());
              for(size_t i=0;i<dst.size();++i){
              dst[i] = map(C, srcPoints[i]);
              }
              return dst;*/
        }

        void computeInterpolations(const Data::Grid &g){
          t4.setDstQuad(g.handles[0].pos, g.handles[1].pos,
                        g.handles[2].pos, g.handles[3].pos);
          //        const math::Mat3 C = create_projective_transform_matrix(g);

          linesToRender.resize(lines.size());
          for(size_t i=0;i<lines.size();++i){
            linesToRender[i].start = t4.mapPoint(lines[i].start);
            linesToRender[i].end = t4.mapPoint(lines[i].end);
          }
        }

        void checkForUpdate(const Grid &g){
          if(ref[0] != g.handles[0].pos ||
             ref[1] != g.handles[1].pos ||
             ref[2] != g.handles[2].pos ||
             ref[3] != g.handles[3].pos){
            for(int i=0;i<4;++i){
              ref[i] = g.handles[i].pos;
              computeInterpolations(g);
            }
          }
        }

        void vis(VisualizationDescription &d){
          for(size_t i=0;i<linesToRender.size();++i){
            d.line(linesToRender[i].start,linesToRender[i].end);
          }
        }



      };

      struct Grid{
        SmartPtr<Texture> texture;
        Handle handles[4];
        int draggedHandle;
        int state; // 0: nothing, 1: fully hovered, 2: fully dragged



        Grid():draggedHandle(-1),state(0){}

        std::vector<Point32f> mapPoints(const std::vector<Point32f> &srcPoints) const{
          if(!texture) throw std::logic_error("Grid::mapPoint is only supported if a texture is given");
          return texture->mapPoints(*this, srcPoints);
        }

        void reset(){
          for(int i=0;i<4;++i){
            handles[i].state = 0;
          }
          this->draggedHandle = -1;
          this->state = 0;
        }
        std::vector<Point> get() const {
          std::vector<Point> v(4);
          for(int i=0;i<4;++i) v[i] = handles[i].pos;
          return v;
        }
        void set(const std::vector<Point> &ps){
          for(int i=0;i<4;++i) handles[i].pos = ps[i];
        }
        bool contains(const Point &p) const{
          std::vector<Point> ps = get();
          ps.push_back(p);
          std::vector<Point> ch = convexHull(ps);
          if(ch.size() != ps.size()) return false;
          for(int i=0;i<4;++i){
            const Point &x = ps[i];
            if(ch[0] != x && ch[1] != x && ch[2] != x && ch[3] != x){
              return false;
            }
          }
          return true;
        }
        bool isFullyDragged() const {
          return state == 2;
        }
        bool isFullyHovered() const {
          return state > 0;
        }

      };
      std::vector<Grid> grids;
      int draggedGrid;
      Rect bounds;
      float handleSize;
      bool convexOnly;


      std::vector<Point> fullyDraggedGridOrig;
      Point fullyDraggedGridMouseAnchor;


      int findFullyDraggedGrid(const Point &p) const{
        for(size_t i=0;i<grids.size();++i){
          if(grids[i].contains(p)){
            return (int)i;
          }
        }
        return -1;
      }

      Data():draggedGrid(-1),handleSize(2),convexOnly(false){

      }
    };

    static bool is_convex(const Point psIn[4]){
      std::vector<Point> ps(psIn,psIn+4);
      bool a = convexHull(ps).size() == 5;
      if(!a) return false;
      bool b = !Line(ps[0],ps[1]).intersects(Line(ps[2],ps[3]));
      if(!b) return false;
      bool c = !Line(ps[1],ps[2]).intersects(Line(ps[3],ps[0]));
      return c;
    }

    AdjustGridMouseHandler::AdjustGridMouseHandler():utils::Lockable(true),m_data(0){

    }

    AdjustGridMouseHandler::AdjustGridMouseHandler(const Rect &bounds, bool convexOnly):
      utils::Lockable(true),m_data(0){
      init(bounds, convexOnly);
    }

    AdjustGridMouseHandler::~AdjustGridMouseHandler(){
      ICL_DELETE(m_data);
    }

    size_t AdjustGridMouseHandler::getNumGrids() const{
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::getNumGrids() was called before it was initialized!");
      }
      return m_data->grids.size();
    }

    std::vector<utils::Point32f> AdjustGridMouseHandler::mapPoints(size_t idx,
                                                                   const std::vector<Point32f> &srcPoints) const{
      if(idx >= m_data->grids.size()){
        throw ICLException("AdjustGridMouseHandler: mapPoints invalid index");
      }
      return m_data->grids[idx].mapPoints(srcPoints);
    }

    void AdjustGridMouseHandler::init(const Rect &bounds, bool convexOnly){
      Mutex::Locker lock(this);
      ICL_DELETE(m_data);
      m_data = new Data;

      m_data->bounds = bounds;
      m_data->convexOnly = convexOnly;

      Rect r = m_data->bounds.enlarged(-20);
      Data::Grid g;
      g.handles[0].pos = r.ul();
      g.handles[1].pos = r.ur();
      g.handles[2].pos = r.lr();
      g.handles[3].pos = r.ll();

      m_data->grids.push_back(g);
    }

    void AdjustGridMouseHandler::clear(){
      Mutex::Locker lock(this);
      ICL_DELETE(m_data);
    }

    void AdjustGridMouseHandler::init(const utils::Rect &bounds,
                                      const std::vector<std::vector<Point> > &grids,
                                      bool convexOnly){
      Mutex::Locker lock(this);
      ICL_DELETE(m_data);
      m_data = new Data;

      m_data->bounds = bounds;
      m_data->convexOnly = convexOnly;

      m_data->grids.resize(grids.size());
      for(size_t i=0;i<grids.size();++i){
        if(grids[i].size() != 4){
          throw ICLException("AdjustGridMouseHandler::init: error given grids must have 4 corners");
        }
        setGrid(i,grids[i].data());
      }
    }

    const utils::Rect &AdjustGridMouseHandler::getBounds() const{
      return m_data->bounds;
    }

    void AdjustGridMouseHandler::setGrid(size_t idx, const utils::Point psIn[4]) throw (ICLException){
      if(idx >= m_data->grids.size()){
        throw ICLException("AdjustGridMouseHandler: setGrid invalid index");
      }
      Rect r = m_data->bounds;

      Point ps[4] = { psIn[0], psIn[1], psIn[2], psIn[3] };
      const Point corners[4] = { r.ul(), r.ur(), r.lr(), r.ll() };

      for(int i=0;i<4;++i){
        if(!r.contains(ps[i].x,ps[i].y)){
          // find nearest corner
          int closest = 0;
          float closestDist = ps[i].distanceTo(corners[0]);
          for(int j=1;j<4;++j){
            float d = ps[i].distanceTo(corners[j]);
            if(d < closestDist) {
              closestDist = d;
              closest = j;
            }
          }
          ps[i] = corners[closest];
        }
      }
      if(m_data->convexOnly && !is_convex(ps)){
        throw ICLException("AdjustGridMouseHandler::setQuadrangle: given quadrangle is not convex "
                           "or twisted");
      }
      Mutex::Locker lock(this);
      Data::Grid &g = m_data->grids[idx];
      for(int i=0;i<4;++i){
        g.handles[i].pos = ps[i];
      }
    }

    std::vector<Point> AdjustGridMouseHandler::getGrid(size_t idx) const throw (ICLException){
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::getGrid() was called before it was initialized!");
      }
      if(idx >= m_data->grids.size()){
        throw ICLException("AdjustGridMouseHandler::getGrid() index out of range");
      }
      std::vector<Point> ps(4);
      const Data::Grid &g = m_data->grids[idx];
      for(int i=0;i<4;++i){
        ps[i] = g.handles[i].pos;
      }
      return ps;
    }

    void AdjustGridMouseHandler::process(const MouseEvent &e){
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::process(MouseEvent) was called before it was initialized!");
      }

      Point p = e.getPos();

      if(!m_data->bounds.contains(p.x,p.y)){
        return;
      }

      if(e.isReleaseEvent()){
        m_data->draggedGrid = -1;
        for(size_t i=0;i<m_data->grids.size();++i){
          m_data->grids[i].reset();
        }
      }
      if(e.isPressEvent()){
        bool hit = false;
        for(size_t gi=0;!hit && gi<m_data->grids.size();++gi){
          Data::Grid &g = m_data->grids[gi];
          for(int i=0;i<4;++i){
            if(g.handles[i].pos.distanceTo(p) < m_data->handleSize){
              g.draggedHandle = i;
              g.handles[i].state = 2;
              hit = true;
              m_data->draggedGrid = gi;
              break;
            }
          }
        }
        if(!hit){
          int fullyHitIndex = m_data->findFullyDraggedGrid(p);
          if(fullyHitIndex != -1){
            m_data->fullyDraggedGridMouseAnchor = p;
            m_data->fullyDraggedGridOrig = m_data->grids[fullyHitIndex].get();
            m_data->draggedGrid = fullyHitIndex;
            m_data->grids[fullyHitIndex].state = 2;
          }
        }
      }
      if(e.isDragEvent()){
        if(m_data->draggedGrid != -1){
          Data::Grid &g = m_data->grids[m_data->draggedGrid];
          if(g.state == 2){ // fully dragged!
            Point delta = p - m_data->fullyDraggedGridMouseAnchor;
            std::vector<Point> newPoints = m_data->fullyDraggedGridOrig;
            for(int i=0;i<4;++i){
              newPoints[i] += delta;
            }
            Rect r(Point::null,Size(m_data->bounds.width+1,m_data->bounds.height+1));
            bool out = false;
            for(int i=0;i<4;++i){
              if(!r.contains(newPoints[i].x,newPoints[i].y)){
                out = true;
                break;
              }
            }
            if(!out){
              g.set(newPoints);
            }
          }else{
            Point test[4] = { g.handles[0].pos, g.handles[1].pos,
                              g.handles[2].pos, g.handles[3].pos };
            test[g.draggedHandle] = p;
            if(!m_data->convexOnly || is_convex(test)){
              g.handles[g.draggedHandle].pos = p;
            }
          }
        }
      }else if(e.isMoveEvent()){
        bool any = false;
        for(size_t gi=0;gi<m_data->grids.size();++gi){
          Data::Grid &g = m_data->grids[gi];
          for(int i=0;i<4;++i){
            Data::Handle &h = g.handles[i];
            if(!any && h.pos.distanceTo(p) < m_data->handleSize){
              h.state = 1;
              any = true;
            }else{
              h.state = 0;
            }
          }
        }
        if(!any){
          int fullyHitIndex = m_data->findFullyDraggedGrid(p);
          for(size_t i=0;i<m_data->grids.size();++i){
            m_data->grids[i].state = (int)i == fullyHitIndex ? 1 : 0;
          }
        }

      }
    }

    void AdjustGridMouseHandler::setHandleSize(float size){
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::setHandlesize() was called before it was initialized!");
      }
      m_data->handleSize = size;
    }

    VisualizationDescription AdjustGridMouseHandler::vis() const{
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::vis() was called before it was initialized!");
      }

      static const Color colors[3] = { Color(255,0,0), Color(0,255,0), Color(0,100,255) };


      const float r = m_data->handleSize;
      VisualizationDescription d;

      for(size_t gi=0;gi<m_data->grids.size();++gi){
        Data::Grid &g = m_data->grids[gi];
        const Color &c = colors[gi%3];
        d.color(c[0],c[1],c[2],255);
        d.linewidth(2);
        d.fill(c[0],c[1],c[2],40 + 60 * g.state);
        std::vector<Point> ps = g.get();
        d.polygon(ps);
        d.linewidth(1);
        for(int i=0;i<4;++i){
          d.fill(c[0],c[1],c[2],1+60*g.handles[i].state);
          d.rect(ps[i].x-r,ps[i].y-r,2*r+1,2*r+1);
        }

        if(g.texture){
          g.texture->checkForUpdate(g);
          g.texture->vis(d);
        }
      }
      return d;
    }

    void AdjustGridMouseHandler::defineGridTexture(size_t idx, const Size32f &dim,
                                                   const std::vector<Line32f> &lines){
      Mutex::Locker lock(this);
      if(!m_data) {
        throw ICLException("AdjustGridMouseHandler::defineGridTexture() was called before it was initialized!");
      }
      if(idx >= m_data->grids.size()){
        throw ICLException("AdjustGridMouseHandler::defineGridTexture() invalid index");
      }
      m_data->grids[idx].texture = new Data::Texture(dim,lines);
    }
  }
}
