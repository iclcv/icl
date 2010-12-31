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
      IdxPoint32f(){}
      IdxPoint32f(const std::vector<Point32f> *cogs,int idx, 
                  const StraightLine2D *line, float centerBrightness):
        cogs(cogs),idx(idx),centerBrightness(centerBrightness){
        distToLine = line->signedDistance(StraightLine2D::Pos(p().x,p().y));
      }
      IdxPoint32f(const IdxPoint32f &ip,const StraightLine2D *otherLine):
        cogs(ip.cogs),idx(ip.idx),centerBrightness(ip.centerBrightness){
        distToLine = otherLine->signedDistance(StraightLine2D::Pos(p().x,p().y));
      }
      const std::vector<Point32f> *cogs;
      int idx;
      float distToLine;
      float centerBrightness;
      const Point32f &p() const { 
        return cogs->at(idx); 
      }
      bool operator<(const IdxPoint32f &p) const{
        return distToLine < p.distToLine;
      }
      static bool cmp_less(const IdxPoint32f &a, const IdxPoint32f &b){
        return a<b;
      }
      static bool cmp_greater(const IdxPoint32f &a, const IdxPoint32f &b){
        return !(a<b);
      }
    };
  }
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
      
    }
    if(sortReverse){
      std::reverse(columns.begin(),columns.end());
    }
    return columns;
  }

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





  CalibrationGrid::CalibrationGrid():inputDataReady(false){}
  
  void CalibrationGrid::visualize2D(ICLDrawWidget &w){
    w.color(0,255,0,200);
    w.fill(0,255,0,100);
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


  void CalibrationGrid::update(const std::vector<Point32f> &cogs, const std::vector<Rect> &bbs, const Img8u &maskedImage){
    lastBoundingBoxes = bbs;
    
    if((int)cogs.size() != 2*nx*ny || (int)bbs.size() != 2*nx*ny){
      inputDataReady = false;
      return;
    }
    std::vector<float> cbs = compute_center_brightness(cogs,bbs,maskedImage);
    int s1 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    float cbs_1 = cbs[s1];
    cbs[s1] = 0;
    int s2 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    cbs[s1] = cbs_1;
    
    StraightLine2D line(StraightLine2D::Pos(cogs[s1].x,cogs[s1].y),
                        StraightLine2D::Pos(cogs[s2].x,cogs[s2].y)-
                        StraightLine2D::Pos(cogs[s1].x,cogs[s1].y));
    
    std::vector<IdxPoint32f> cogsi(cogs.size());
    for(unsigned int i=0;i<cogs.size();++i){
      cogsi[i] = IdxPoint32f(&cogs,i,&line, cbs[i]);
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

  
  
}
