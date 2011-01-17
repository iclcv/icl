/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/CalibrationGrid.cpp                        **
** Module : ICLGeom                                                **
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

#include <ICLGeom/CalibrationGrid.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StraightLine2D.h>

#include <fstream>

namespace icl{
  static std::vector<float> compute_center_brightness(const std::vector<Point32f> &cogs, 
                                                      const std::vector<Rect> &bbs, 
                                                      const Img8u &maskedImage){
    std::vector<float> v(cogs.size());
    const Img8u &im = maskedImage; 
    const Channel8u &ci = im[0];
    Rect ir = im.getImageRect();
    const float cf = 0.3; // center fraction
    const float lr = (1.0 - cf)/2; // fraction for left and right of center
    for(unsigned int i=0;i<cogs.size();++i){
      Rect r = bbs[i] & ir;
      Rect rs(r.x+r.width*lr,r.y+r.height*lr,r.width*cf,r.height*cf);
      int buf = 0;
      for(int x=rs.x;x<rs.right();++x){
        for(int y=rs.y;y<rs.bottom();++y){
          buf += ci(x,y);
        }
      }
      int dim = rs.getSize().getDim();
      if(dim){
        v[i] = buf/dim;
      }
    }
    return v;
  }
  namespace{
    struct IdxPoint32f{
      explicit IdxPoint32f(const std::vector<Point32f> *cogs, int idx):
        cogs(cogs),idx(idx),distToLine(0.0f){}
      IdxPoint32f(){}
      IdxPoint32f(const std::vector<Point32f> *cogs,int idx, 
                  const StraightLine2D *line):
        cogs(cogs),idx(idx){
        distToLine = line->signedDistance(StraightLine2D::Pos(p().x,p().y));
      }
      IdxPoint32f(const IdxPoint32f &ip,const StraightLine2D *otherLine):
        cogs(ip.cogs),idx(ip.idx){
        distToLine = otherLine->signedDistance(StraightLine2D::Pos(p().x,p().y));
      }
      const std::vector<Point32f> *cogs;
      int idx;
      float distToLine;
      /*float centerBrightness;*/
      const Point32f &p() const { 
        return cogs->at(idx); 
      }
      bool operator<(const IdxPoint32f &p) const{
        return distToLine < p.distToLine;
      }
      bool operator==(const IdxPoint32f &p) const { return idx == p.idx; }
      
      static bool cmp_less(const IdxPoint32f &a, const IdxPoint32f &b){
        return a<b;
      }
      static bool cmp_greater(const IdxPoint32f &a, const IdxPoint32f &b){
        return !(a<b);
      }
      static bool cmp_less_abs(const IdxPoint32f &a, const IdxPoint32f &b){
        return fabs(a.distToLine) < fabs(b.distToLine);
      }
      friend std::ostream &operator<<(std::ostream &s, const IdxPoint32f &p){
        std::string valid=p.idx>=(int)p.cogs->size() ? "invalid!" : "ok";
        if(valid == "ok"){
          return std::cout << "IdxPoint32f(index=" << p.idx << ", valid= " << valid << ", Point=" << p.p() << ")";
        }else{
          return std::cout << "IdxPoint32f(index=" << p.idx << ", valid= " << valid << ", Point=" << "xxx" << ")";
        }
      }

    };
    struct DistToLineCompare{
      const StraightLine2D &line;
      DistToLineCompare(const StraightLine2D &line):line(line){}
      bool operator()(const IdxPoint32f &a, const IdxPoint32f &b) const{
        float da = line.distance(StraightLine2D::Pos(a.p().x,a.p().y));
        float db = line.distance(StraightLine2D::Pos(b.p().x,b.p().y));
        return da < db;
      }
    };
  
  }

  bool is_straight(const std::vector<IdxPoint32f> &col){
    if(col.size() < 2) throw ICLException("is_straight cannot work with less than 2 points...");
    Point32f a = col.front().p(), b = col.back().p();
    StraightLine2D line(StraightLine2D::Pos(a.x,a.y),StraightLine2D::Pos(b.x-a.x,b.y-a.y));
    for(unsigned int i=1;i<col.size()-1;++i){
      if(line.distance(StraightLine2D::Pos(col[i].p().x,col[i].p().y) ) > 5){
        return false;
      }
    }
    return true;
  }
  
  
  /**
      this function grabs one next column (ny points) from the given set of remaining points 'all'. First, it trys
      the trivial solution: Here, the furthest ny points are used to build the next column. Only if it turns out,
      that these points are not collinear (is_straight), a *new* heuristics is used:
      - The main assumption of this heuristics is that at least the first two furthest points are part of the 
        next column
      - These points are used to create a StraightLine2D instance. 
      - Now, the next points of all are extracted and sorted by their distance to that line
      - Since we assumed that the line was correct, the first ny points (including the support points for the line)
        can be moved from all and are returned as complete last column
      - Only if also these points are not collinear, an execption is thrown
  */
  static std::vector<IdxPoint32f> grab_next_ny_points(std::vector<IdxPoint32f> &all, int ny, const StraightLine2D *line){
    // 1st: try trivial solution
    std::vector<IdxPoint32f> col(ny);
    for(int i=0;i<ny;++i){
      col[i] = IdxPoint32f(all[i], line);
    }
    std::sort(col.begin(),col.end(),IdxPoint32f::cmp_less);

    if(!is_straight(col)){
      // assumption: at least the 2 furthest points belong to the line
      StraightLine2D tmpline(StraightLine2D::Pos(all[0].p().x, all[0].p().y),
                             StraightLine2D::Pos(all[1].p().x-all[0].p().x, 
                                                 all[1].p().y-all[0].p().y));
      // sort up to ny*ny-2 by their distance to the line
      std::vector<IdxPoint32f> use(iclMin(ny*ny,(int)all.size()));
      std::copy(all.begin(), all.begin()+use.size(),use.begin());
      std::sort(use.begin(),use.end(),DistToLineCompare(tmpline));
      for(int i=0;i<ny;++i){
        col[i] = IdxPoint32f(use[i],line);
      }
      std::sort(col.begin(),col.end(),IdxPoint32f::cmp_less);      
      // remove entries from column from all
      for(int i=0;i<ny;++i){
        std::vector<IdxPoint32f>::iterator it = std::find(all.begin(),all.end(),col[i]);
        if(it == all.end()) throw ICLException("error in grab_next_ny_points: grabbed a point "
                                               " that did not exist (which should not happen)");
        all.erase(it);
      }  
      if(!is_straight(col)) throw ICLException("error in grab_next_ny_points: line was not straight");
    }else{
      all.erase(all.begin(),all.begin()+ny);
    }
      return col;
  }

  /**
      adapted version of partition_and_sort_points, where each next ny points (each next column)
      is checked for being straight. If not, the heuristics implemented in grab_next_ny_points
      is used!
  */
  static std::vector<std::vector<IdxPoint32f> > partition_and_sort_points_2(std::vector<IdxPoint32f> all, 
                                                                            int ny, const StraightLine2D *line,
                                                                            bool sortReverse){
    std::vector<std::vector<IdxPoint32f> > columns(all.size() / ny);
    for(unsigned int i=0;i<columns.size();++i){
      columns[i] = grab_next_ny_points(all, ny, line);
    }
    if(sortReverse){
      std::reverse(columns.begin(),columns.end());
    }
    return columns;
  }

#if USE_OLD_VERSION
  static std::vector<std::vector<IdxPoint32f> > partition_and_sort_points(const std::vector<IdxPoint32f> &all, 
                                                                          int ny, const StraightLine2D *line,
                                                                          bool sortReverse){
    std::vector<std::vector<IdxPoint32f> > columns(all.size() / ny);
    std::vector<IdxPoint32f>::const_iterator it = all.begin();
    for(unsigned int i=0;i<columns.size();++i){
      columns[i].resize(ny);
      for(int j=0;j<ny;++j){
        columns[i][j] = IdxPoint32f(*it++, line);
      }
      std::sort(columns[i].begin(),columns[i].end(),IdxPoint32f::cmp_less);
      // set last line ...
      
    }
    if(sortReverse){
      std::reverse(columns.begin(),columns.end());
    }
    return columns;
  }
#endif

  static std::pair<int,int> count_distance_signs(const std::vector<IdxPoint32f> &cogsi){
    std::pair<int,int> ds(0,0);
    for(unsigned int i=0;i<cogsi.size();++i){
      if(fabs(cogsi[i].distToLine) > 0.1){
        if(cogsi[i].distToLine>0) ds.second++;
        else ds.first++;
      }
    }
    return ds;
  }

  
  
  struct DistToPointCompare{
    Point32f p;
    DistToPointCompare(const IdxPoint32f &p):p(p.p()){}
    bool operator()(const IdxPoint32f &a, const IdxPoint32f &b) const{
      return a.p().distanceTo(p) < b.p().distanceTo(p);
    }
  };
  
  IdxPoint32f find_closest_point_to_line(const StraightLine2D &line, const std::vector<IdxPoint32f> &ps){
    if(!ps.size()) throw ICLException("find_closest_point_to_line: given point set size was 0");
    return *min_element(ps.begin(),ps.end(),DistToLineCompare(line));
  }
  
  std::vector<IdxPoint32f> find_n_closest_points_to_point(int n, std::vector<IdxPoint32f> list, const IdxPoint32f &X){
    //if((int)list.size() < n) throw ICLException(str("error in grid fitting process: in find_n_closest_points_to_point\n")+
    //                                            str("searching for at least ")+str(n)+str(" points in a list of size ") + 
    //                                            str(list.size()));
    // we dont need this, since at the end, only ny points are left ..
    if(!list.size()) throw ICLException("find_n_closest_points_to_point: list size was null");
    std::sort(list.begin(),list.end(),DistToPointCompare(X));
    list.erase(list.begin());
    if((int)list.size()>n) list.resize(n);
    return list;
  }

  int find_index_of_point_with_closest_slope(const StraightLine2D &line, const IdxPoint32f &X, const std::vector<IdxPoint32f> &ys){
    std::vector<float> ds(ys.size());
    for(unsigned int i=0;i<ys.size();++i){
      StraightLine2D ly(StraightLine2D::Pos(X.p().x, X.p().y),
                        StraightLine2D::Pos(ys[i].p().x, ys[i].p().y)-
                        StraightLine2D::Pos(X.p().x, X.p().y));
      ds[i] = fabs(ly.v.element_wise_inner_product(line.v));
    }
    return (int)(max_element(ds.begin(),ds.end())-ds.begin());
  }

  struct SortPointsByDistanceToPerpendicularLine{
    StraightLine2D perpLine;
    SortPointsByDistanceToPerpendicularLine(const StraightLine2D &line){
      perpLine.v = StraightLine2D::Pos(line.v[1], -line.v[0]);
      perpLine.o = line.o;
    }
    inline bool operator() (const IdxPoint32f &a, const IdxPoint32f &b) const {
      return perpLine.signedDistance(StraightLine2D::Pos(a.p().x,a.p().y)) 
           > perpLine.signedDistance(StraightLine2D::Pos(b.p().x,b.p().y));
    }
  };

  bool is_almost(const Point32f &a, const Point32f &b){
    return (fabs(a.x-b.x) + fabs(a.y-b.y)) < 0.01;
  }

  StraightLine2D fix_line_direction_and_sort_first_points(StraightLine2D line, std::vector<IdxPoint32f> &points, 
                                                          const Point32f &s1,const Point32f &s2){
    std::sort(points.begin(),points.end(),SortPointsByDistanceToPerpendicularLine(line));
    Point32f top;
    if(is_almost(points.front().p(),s1) || is_almost(points.back().p(),s1)){
      top = s1;
    }else{
      top = s2;
    }
    if(is_almost(top,points.front().p())){
      line.v = -line.v;
      std::reverse(points.begin(),points.end());
    }
    return line;
  }

  std::vector<std::vector<IdxPoint32f> > match_grid(const std::vector<Point32f> &cogs, int s1, int s2, int nx, int ny){
    std::vector<std::vector<IdxPoint32f> > columns(cogs.size() / ny);
    
    
    StraightLine2D line(StraightLine2D::Pos(cogs[s1].x,cogs[s1].y),
                        StraightLine2D::Pos(cogs[s2].x,cogs[s2].y)-
                        StraightLine2D::Pos(cogs[s1].x,cogs[s1].y));
    
    std::vector<IdxPoint32f> cogsi(cogs.size());
    for(unsigned int i=0;i<cogs.size();++i){
      cogsi[i] = IdxPoint32f(&cogs,i,&line);
    }
    std::sort(cogsi.begin(),cogsi.end(),IdxPoint32f::cmp_less_abs);
    std::copy(cogsi.begin(),cogsi.begin()+ny, back_inserter(columns[nx]));
    cogsi.erase(cogsi.begin(),cogsi.begin()+ny);

    //std::pair<int,int> ds = count_distance_signs(cogsi);
    //if(ds.first < ds.second){
    //  line.v = -line.v;
    //}
    
    line = fix_line_direction_and_sort_first_points(line, columns[nx], cogs[s1], cogs[s2]);
    
    /// now sort points to the left and to the right
    std::vector<IdxPoint32f> left,right;
    for(unsigned int i=0;i<cogsi.size();++i){
      (cogsi[i].distToLine < 0 ? left : right).push_back(cogsi[i]); 
    }
    if((int)left.size() != nx*ny) throw ICLException("Left and Right size was wrong");
    
    StraightLine2D lastLine = line;

    /// sort left points
    for(int i=0;i<nx;++i){
      // hint -> xxx
      IdxPoint32f X = find_closest_point_to_line(lastLine,left);
      std::vector<IdxPoint32f> Y = find_n_closest_points_to_point(3,left,X); // X will be removed automatically
      int yi = find_index_of_point_with_closest_slope(lastLine, X, Y);
      StraightLine2D lastLineSave = lastLine;
      lastLine = StraightLine2D(StraightLine2D::Pos(X.p().x,X.p().y),
                                StraightLine2D::Pos(Y[yi].p().x,Y[yi].p().y)-
                                StraightLine2D::Pos(X.p().x,X.p().y));
      
      std::sort(left.begin(),left.end(),DistToLineCompare(lastLine));
      std::copy(left.begin(),left.begin()+ny, std::back_inserter(columns[nx-1-i]));
      left.erase(left.begin(),left.begin()+ny);
      std::sort(columns[nx-1-i].begin(),columns[nx-1-i].end(),SortPointsByDistanceToPerpendicularLine(lastLineSave));
    }
      
    lastLine = line;
    // sort right points
    for(int i=0;i<nx-1;++i){
      // hint -> xxx
      IdxPoint32f X = find_closest_point_to_line(lastLine,right);
      std::vector<IdxPoint32f> Y = find_n_closest_points_to_point(3
,right,X); 
      int yi = find_index_of_point_with_closest_slope(lastLine, X, Y);
      StraightLine2D lastLineSave = lastLine;
      lastLine = StraightLine2D(StraightLine2D::Pos(X.p().x,X.p().y),
                                StraightLine2D::Pos(Y[yi].p().x,Y[yi].p().y)-
                                StraightLine2D::Pos(X.p().x,X.p().y));
      
      std::sort(right.begin(),right.end(),DistToLineCompare(lastLine));
      std::copy(right.begin(),right.begin()+ny, std::back_inserter(columns[nx+1+i]));
      right.erase(right.begin(),right.begin()+ny);
      std::sort(columns[nx+1+i].begin(),columns[nx+1+i].end(),SortPointsByDistanceToPerpendicularLine(lastLineSave));
    }
    return columns;
  }
      
      
  // xxx
  // find the next ny points that are 
  // - closest to the lastLine 
  // - AND almost collinear (most important)
  // - AND more or less parallel to last line
  /**
      * start mit nächstem punkt zu last line X
      * suche den punkt Yi aus [Y1,Y2,Y3|Yi closest to X] zu X mit dem
      * die linie X->Yi möglichst parallel zu lastLine sind
      * lastLine = X->Yi oder Yi->X ??
      * sortiere left nach abstand zu lastLine (X->Yi)
      * kopiere die ersten ny punkte und lösche sie aus 'left'
      */
  // store results in columns[nx-1-i] (sorted from left to right)


  CalibrationGrid::CalibrationGrid():inputDataReady(false){}
  
  
  static inline float round1(float x){
    return float((int)round(x*10))/10.;
  }
  
  void CalibrationGrid::visualize2D(ICLDrawWidget &w){
    w.color(0,255,0,200);
    w.fill(0,0,0,0);//255,0,100);
    for(unsigned int i=0;i<lastBoundingBoxes.size();++i){
      w.rect(lastBoundingBoxes[i]);
    }
    
    if(!inputDataReady) return;
    w.color(0,100,255,255);
    w.linewidth(1);
    w.grid(A.img.data(),nx,ny);
    w.linewidth(3);
    w.color(0,100,255,200);
    w.arrow(A.p1,A.p2);
    w.arrow(A.p1,A.p3);
    w.color(255,0,0,255);
    w.linewidth(1);
    w.grid(B.img.data(),nx,ny);
    w.linewidth(3);
    w.color(255,0,0,200);
    w.arrow(B.p1,B.p2);
    w.arrow(B.p1,B.p3);
    w.linewidth(1);
    

    for(int h=0;h<2;++h){
      const Point32f *ps = (h?B:A).img.data();
      const FixedColVector<float,3> *vs = (h?B:A).world.data();
      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          float vx = round1(vs[x+nx*y][0]);
          float vy = round1(vs[x+nx*y][1]);
          float vz = round1(vs[x+nx*y][2]);
          w.color(255,255,255,255);
          w.text(str("(")+str(vx)+", "+str(vy)+", "+str(vz)+str(")"),ps[x+nx*y].x+1, ps[x+nx*y].y+1, 9);
          w.color(0,0,0,255);
          w.text(str("(")+str(vx)+", "+str(vy)+", "+str(vz)+str(")"),ps[x+nx*y].x, ps[x+nx*y].y, 9);

        }
      }
    }
  }

  CalibrationGrid::CalibrationGrid(const std::string &configFileName) : inputDataReady(false){
    loadFromFile(configFileName);
  }

  void CalibrationGrid::loadFromFile(const std::string &configFileName){
    ConfigFile f(configFileName);
    Vec3 wo = parse<Vec3>(f["config.world-offset"]);
    nx = f["config.calibration-object.nx"];
    ny = f["config.calibration-object.ny"];
    
    f.setPrefix("config.calibration-object.part-A.");
    
    Vec3 A1 = wo + parse<Vec3>(f["offset"]);
    Vec3 adx = parse<Vec3>(f["dx"])/(nx-1);
    Vec3 ady = parse<Vec3>(f["dy"])/(ny-1);
    
    f.setPrefix("config.calibration-object.part-B.");
    
    Vec3 B1 = wo + parse<Vec3>(f["offset"]);
    Vec3 bdx = parse<Vec3>(f["dx"])/(nx-1);
    Vec3 bdy =  parse<Vec3>(f["dy"])/(ny-1);
    
    for(int y=0;y<ny;++y){    
      for(int x=0;x<nx;++x){
        A.world.push_back( A1 + adx * (nx-1-x) + ady * (ny-1-y));
        B.world.push_back( B1 + bdx * (   x  ) + bdy * (ny-1-y));
      }
    }
  }
  
  void CalibrationGrid::initializeSceneObject(SceneObject &so){
    static const GeomColor red(255,0,0,255);
    GeomColor blue(0,100,255,255);
    GeomColor green(0,255,0,255);
    GeomColor white(255,255,255,255);
    
    /// coordinate frame RGB-> x,y,z-axis
    so.addVertex(Vec(0,0,0,1),geom_white());
    so.addVertex(Vec(100,0,0,1),geom_red());
    so.addVertex(Vec(0,100,0,1),geom_green());
    so.addVertex(Vec(0,0,100,1),geom_blue());
    so.addLine(0,1,geom_red());
    so.addLine(0,2,geom_green());
    so.addLine(0,3,geom_blue());
    
    for(int i=0;i<nx*ny;++i){
      so.addVertex(A.world[i].resize<1,4>(1),geom_blue());        
    }
    for(int i=0;i<nx*ny;++i){
      so.addVertex(B.world[i].resize<1,4>(1),geom_red());
    }
    
    for(int x=0;x<nx-1;++x){
      for(int y=0;y<ny-1;++y){
        so.addLine(a(x,y),a(x+1,y),geom_blue());
        so.addLine(a(x,y),a(x,y+1),geom_blue());
        
        so.addLine(b(x,y),b(x+1,y),geom_red());
        so.addLine(b(x,y),b(x,y+1),geom_red());
      }
    }
    
    for(int x=0;x<nx-1;++x){
      so.addLine(a(x,ny-1),a(x+1,ny-1),geom_blue());
      so.addLine(b(x,ny-1),b(x+1,ny-1),geom_red());
    }
    for(int y=0;y<ny-1;++y){
      so.addLine(a(nx-1,y),a(nx-1,y+1),geom_blue());
      so.addLine(b(nx-1,y),b(nx-1,y+1),geom_red());
    }
    
    so.setPointSize(6);
    so.setLineWidth(3);
    so.setVisible(Primitive::vertex,true);
    so.setVisible(Primitive::line,true);
  }

  static inline Vec vec3to4(const CalibrationGrid::Vec3 &v){
    return v.resize<1,4>(1);
  }

  float CalibrationGrid::get_RMSE_on_image(const std::vector<Vec> &ws, const std::vector<Point32f> &is, const Camera &cam) {
    float result = 0;
    for (unsigned int i=0; i<is.size(); i++) {
      result += pow(cam.project(ws[i]).distanceTo(is[i]),2);
    }
    return sqrt(result/is.size());
  }

  std::pair<int,int> CalibrationGrid::findMarkedPoints(const std::vector<Point32f> &cogs, 
                                                       const std::vector<Rect> &bbs, 
                                                       const Img8u *hintImage){
    if(!hintImage) throw ICLException("CalibrationGrid::findMarkedPoints: given hintImage is null");
    if(hintImage->getChannels() != 1) throw ICLException("CalibrationGrid::findMarkedPoints: given hintImages channel count is not 1");
    std::vector<float> cbs = compute_center_brightness(cogs,bbs,*hintImage);
    int s1 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    float cbs_1 = cbs[s1];
    cbs[s1] = 0;
    int s2 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    cbs[s1] = cbs_1;
    return std::pair<int,int>(s1,s2);
  }
  

  void CalibrationGrid::update(const std::vector<Point32f> &cogs, const std::vector<Rect> &bbs, const Img8u *hintImage){
    lastBoundingBoxes = bbs;
    
    if((int)cogs.size() != 2*nx*ny || (int)bbs.size() != 2*nx*ny){
      inputDataReady = false;
      return;
    }
    const std::pair<int,int> si = findMarkedPoints(cogs,bbs,hintImage);
    const int &s1 = si.first;
    const int &s2 = si.second;
    
    // check for wrongly detected marked points 
    if( (s1 < 0) || (s1 >= (int)cogs.size()) || (s2 < 0) || (s2 >= (int)cogs.size()) || (s1 == s2)) return;


#if USE_OLD_VERSION
    StraightLine2D line(StraightLine2D::Pos(cogs[s1].x,cogs[s1].y),
                        StraightLine2D::Pos(cogs[s2].x,cogs[s2].y)-
                        StraightLine2D::Pos(cogs[s1].x,cogs[s1].y));
    
    std::vector<IdxPoint32f> cogsi(cogs.size());
    for(unsigned int i=0;i<cogs.size();++i){
      cogsi[i] = IdxPoint32f(&cogs,i,&line/*, cbs[i]*/);
    }
    std::sort(cogsi.begin(),cogsi.end());
    std::pair<int,int> ds = count_distance_signs(cogsi);
    StraightLine2D perpLine = line;
    
    if(ds.first > ds.second){
      perpLine.v = StraightLine2D::Pos(line.v[1], -line.v[0]);
    }else{
      perpLine.v = StraightLine2D::Pos(-line.v[1], line.v[0]);
    }
    std::vector<std::vector<IdxPoint32f> > columns = partition_and_sort_points(cogsi, ny, &perpLine,
                                                                               ds.first > ds.second);              
#elif 1
    StraightLine2D line(StraightLine2D::Pos(cogs[s1].x,cogs[s1].y),
                        StraightLine2D::Pos(cogs[s2].x,cogs[s2].y)-
                        StraightLine2D::Pos(cogs[s1].x,cogs[s1].y));
    
    std::vector<IdxPoint32f> cogsi(cogs.size());
    for(unsigned int i=0;i<cogs.size();++i){
      cogsi[i] = IdxPoint32f(&cogs,i,&line/*, cbs[i]*/);
    }
    std::sort(cogsi.begin(),cogsi.end());
    std::pair<int,int> ds = count_distance_signs(cogsi);
    StraightLine2D perpLine = line;
    
    if(ds.first > ds.second){
      perpLine.v = StraightLine2D::Pos(line.v[1], -line.v[0]);
    }else{
      perpLine.v = StraightLine2D::Pos(-line.v[1], line.v[0]);
    }
    
    std::vector<std::vector<IdxPoint32f> > columns;
    try{
      columns = partition_and_sort_points_2(cogsi, ny, &perpLine,
                                            ds.first > ds.second);              
    }catch(ICLException &ex){
      WARNING_LOG(ex.what());
      return;
    }
#else
    std::vector<std::vector<IdxPoint32f> > columns;
    try{
      columns = match_grid(cogs,s1,s2,nx,ny);
    }catch(const ICLException &ex){
      SHOW(ex.what());
      return;
    }
    
#endif
    A.img.resize(nx*ny);
    B.img.resize(nx*ny);
    for(int i=0;i<nx;++i){
      for(int j=0;j<ny;++j){
        A.img[i+nx*j] = columns[i][j].p();
        B.img[i+nx*j] = columns[i+nx][j].p();
      }
    }
    A.p1 = A.img.back();
    A.p2 = A.img[(ny-1)*nx];
    A.p3 = A.img[nx-1];
    
    B.p1 = B.img[(ny-1)*nx];
    B.p2 = B.img.back();
    B.p3 = B.img.front();
    
    inputDataReady = true;
  }
  
  float CalibrationGrid::applyCalib(const Size &imageSize, Camera &cam){
    if(!inputDataReady) return -1;
    // create appropriate point sets
    std::vector<Point32f> imgPts;
    std::copy(A.img.begin(),A.img.end(),back_inserter(imgPts));
    std::copy(B.img.begin(),B.img.end(),back_inserter(imgPts));
    
    std::vector<Vec> worldPts;
    std::transform(A.world.begin(),A.world.end(),back_inserter(worldPts),vec3to4);
    std::transform(B.world.begin(),B.world.end(),back_inserter(worldPts),vec3to4);
    
    cam = Camera::calibrate(worldPts, imgPts);
    cam.getRenderParams().viewport = Rect(Point::null,imageSize);
    cam.getRenderParams().chipSize = imageSize;
    return get_RMSE_on_image(worldPts,imgPts, cam);
  }

  void CalibrationGrid::create_empty_configuration_file(const std::string &filename){
    std::ofstream stream(filename.c_str());
#define LINE(x) << x <<  std::endl

    stream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl
    LINE("<config>")
    LINE("<!-- Layout:")
    LINE("0 -- 1 -- 2 -- 3  |  16 - 17 - 18 - 19")
    LINE("4 -- 5 -- 6 -- 7  |  20 - 21 - 22 - 23")
    LINE("8 -- 9 - 10 - 11  |  24 - 25 - 26 - 27")
    LINE("12 - 13 - 14 - 15  |  28 - 29 - 30 - 31")
    LINE("                                                  ___")
    LINE(" a------a------a------A3 B3-----b-----b-----b      |")
    LINE(" |      |      |      |   |     |     |     |      |")
    LINE(" a------a------a------a   b-----b-----b-----b      | ")
    LINE(" |      |      |      |   |     |     |     |     ny (cells)")
    LINE(" a------a------a------a   b-----b-----b-----b      |")
    LINE(" |      |      |      |   |     |     |     |      |")
    LINE("A2------a------a------A1 B1-----b-----b-----B2    _|_")
    LINE(" ")
    LINE(" |-------- nx --------|")
    LINE("         (cells)  ")
    LINE(" ")
    LINE("A1 = partA.offset")
    LINE("A2 = partA.dx + partA.offset")
    LINE(" A3 = partA.dy + partA.offset")
    LINE(" ")
    LINE("B1 = partB.offset")
    LINE(" B2 = partB.dx + partB.offset")
    LINE("B3 = partB.dy + partB.offset")
    LINE("-->")
    LINE("<title>Camera Calibration Config-File</title>")
    LINE("<data id='world-offset' type='string'>0,266,0</data>")
    LINE("<section id='calibration-object'>")
    LINE("  <data id='nx' type='int'>5</data>")
    LINE("  <data id='ny' type='int'>4</data>")
    LINE("  <section id='part-A'>")
    LINE("    <data id='offset' type='string'>-21.21 30.8 282.29</data>")
    LINE("    <data id='dx' type='string'>-200.12 0 -200.12</data>")
    LINE("    <data id='dy' type='string'>0 212.25 0</data>")
    LINE("  </section>")
    LINE("  <section id='part-B'>")
    LINE("    <data id='offset' type='string'>21.92 29.5 281.58</data>")
    LINE("    <data id='dx' type='string'>200.12 0 -200.12</data>")
    LINE("    <data id='dy' type='string'>0 212.25 0</data>")
    LINE("  </section>")
    LINE("</section>")
    LINE("</config>");
#undef LINE
  }

  Size CalibrationGrid::getDimension() const{
    return Size(nx,ny);
  }
  
  
}
