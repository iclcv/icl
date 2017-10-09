/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/swiss-ranger/swiss-ranger.cpp            **
** Module : ICLGeom                                                **
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

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>

#include <ICLGeom/Scene.h>
#include <ICLFilter/MedianOp.h>
#include <ICLCore/ImgBorder.h>

GenericGrabber *grabber = 0;

HSplit gui;
Img32f IMAGE;
Camera CAM;
Point32f POS;
Scene scene;
enum VisMode{ SOLID,DEPTH,INTEN,CONFI};

Vec estimate_3D_pos(const Point32f &p, const Img32f &image, const Camera &cam){
  ViewRay vr = cam.getViewRay(p);
  Vec o = vr.offset;
  Vec v = vr.direction;
  v = v/v.length();

  float val = image.subPixelLIN(p.x,p.y,0);
  Vec r = o + v*val;

  r[3] = val;

  return r;
}

Vec estimate_3D_pos(const Point32f &p, float val, const Camera &cam){
  ViewRay vr = cam.getViewRay(p);
  Vec o = vr.offset;
  Vec v = vr.direction;
  v = v/v.length();

  Vec r = o + v*val;

  r[3] = val;

  return r;
}


struct ImageObj : public SceneObject{
  Size size;
  Mutex mutex;
  Img8u image;

  virtual void lock(){ mutex.lock(); }
  virtual void unlock(){ mutex.unlock(); }

  inline int idx(int x, int y){
    return x+y*size.width;
  }

  inline Vec &node(int x, int y){
    return m_vertices[idx(x,y)];
  }
  inline GeomColor &col(int x, int y){
    return m_vertexColors[idx(x,y)];
  }

  ImageObj(const Size &s):size(s){
    image = Img8u(size,formatRGB);

    for(int x=0;x<s.width;++x){
      for(int y=0;y<s.height;++y){
        addVertex(Vec(0,0,0,1));
      }
    }

    addVertex(Vec(0,0,0,1),GeomColor(0,0,0,0));
    addVertex(Vec(0,0,0,1),GeomColor(0,0,0,0));
    addVertex(Vec(0,0,0,1),GeomColor(0,0,0,0));
    addVertex(Vec(0,0,0,1),GeomColor(0,0,0,0));
    addVertex(Vec(0,0,0,1),GeomColor(0,0,0,0));

    addTexture(s.getDim(),size.getDim()+1,size.getDim()+2,size.getDim()+3,&image,false);

    for(int x=0;x<s.width-1;++x){
      for(int y=0;y<s.height-1;++y){
        addLine(idx(x,y),idx(x+1,y));
        addLine(idx(x,y),idx(x,y+1));

        addQuad(idx(x,y),idx(x+1,y),
                idx(x+1,y+1),idx(x,y+1));
      }
    }
    for(int x=0;x<s.width-1;++x){
      addLine(idx(x,s.height-1),idx(x+1,s.height-1));
    }
    for(int y=0;y<s.height-1;++y){
      addLine(idx(s.width-1,y),idx(s.width-1,y+1),GeomColor(255,255,255,40));
    }

    addLine(size.getDim(),size.getDim()+4,GeomColor(255,255,255,255));
    addLine(size.getDim()+1,size.getDim()+4,GeomColor(255,255,255,255));
    addLine(size.getDim()+2,size.getDim()+4,GeomColor(255,255,255,255));
    addLine(size.getDim()+3,size.getDim()+4,GeomColor(255,255,255,255));

    setColorsFromVertices(Primitive::quad,true);
  }
  void update(const Img32f &image, const Camera &cam, VisMode m, int pointSize, bool imageOn){
    setPointSize(pointSize);
    std::vector<Range32f> rs(image.getChannels());
    for(unsigned int i=0;i<rs.size();++i){
      rs[i] = image.getMinMax(i);
    }
    lock();

    setVisible(Primitive::texture,imageOn);
    if(imageOn){
      image.convert(&this->image);
      float maxDepth = rs[0].maxVal;
      m_vertices[size.getDim()] = estimate_3D_pos(Point(0,0),maxDepth,cam);
      m_vertices[size.getDim()+1] = estimate_3D_pos(Point(size.width,0),maxDepth,cam);
      m_vertices[size.getDim()+2] = estimate_3D_pos(Point(size.width,size.height),maxDepth,cam);
      m_vertices[size.getDim()+3] = estimate_3D_pos(Point(0,size.height),maxDepth,cam);

      m_vertices[size.getDim()+4] = cam.getPosition();
    }




    for(int x=0;x<size.width;++x){
      for(int y=0;y<size.height;++y){
        node(x,y) = estimate_3D_pos(Point32f(x,y),image,cam);
        switch(m){
          case SOLID:
            col(x,y) = GeomColor(0,100,255,255);
            break;
          case DEPTH:
            if(image.getChannels() > 0){
              float c = 255-(255*(image(x,y,0)-rs[0].minVal)/rs[0].getLength());
              col(x,y) = GeomColor(c,c,c,255);
            }
            break;
          case INTEN:
            if(image.getChannels() > 1){
              float c = 255*(image(x,y,1)-rs[1].minVal)/rs[1].getLength();
              col(x,y) = GeomColor(c,c,c,255);
              //col(x,y) = image(x,y,1);
            }
            break;
          case CONFI:
            if(image.getChannels() > 2){
              float c = 255*(image(x,y,2)-rs[1].minVal)/rs[2].getLength();
              col(x,y) = GeomColor(c,c,c,255);
            }
            break;
        }
      }
    }
    unlock();
  }
};

ImageObj *grid = 0;

void mouse(const MouseEvent &evt){
  if(evt.isPressEvent() || evt.isDragEvent()){
    POS = evt.getPos();
  }
}

void init(){

  gui << ( Tab("2D,3D").handle("tab")
           << Draw().handle("draw").minSize(32,24)
           << Draw3D().handle("draw3D").minSize(32,24)
         )
      << ( VBox().maxSize(12,100)
           << Button("off","!on").out("grab").label("grab loop")
           << Button("off","!on").out("pp").label("median")
           << ( VBox().label("3D vis ")
                << CheckBox("points",true).out("points")
                << CheckBox("lines").out("lines")
                << CheckBox("fill").out("fill")
                << CheckBox("image").out("imageOn")
                << Spinner(1,10,2).out("pointSize").label("point size")
                << Combo("solid,depth,intensity,confidence").handle("visColor")
                << Button("reset pos").handle("resPos")
                )
           )
      << Show();

  gui["draw"].install(mouse);

  grabber = new GenericGrabber();
  grabber -> init(pa("-i"));

  CAM = Camera(*pa("-input",2));

  static DrawHandle draw = gui["draw"];
  static DrawHandle3D draw3D = gui["draw3D"];
  (*draw)->setRangeMode(ICLWidget::rmAuto);

  float data[] = {0,0,0,1000,1000,1000};
  SceneObject *o = new SceneObject("cuboid",data);
  o->setVisible(Primitive::quad,false);
  o->setVisible(Primitive::triangle,false);

  scene.addObject(o);
  grid = new ImageObj(CAM.getRenderParams().viewport.getSize());
  scene.addObject(grid);

  scene.addCamera(CAM);
  scene.addCamera(CAM);


  (*draw3D)->install(scene.getMouseHandler(0));
  (*draw3D)->setViewPort(Size(176,144));


}


void run(){
  DrawHandle draw = gui["draw"];
  static ButtonHandle resPos = gui["resPos"];
  static ICLDrawWidget &w = **draw;
  bool grab = gui["grab"];
  bool pp = gui["pp"];

  if(grab){
    const ImgBase *image = grabber->grab();
    if(pp){
      static MedianOp mo(Size(3,3));
      mo.setClipToROI(false);
      IMAGE=cvt(mo.apply(image));
      ImgBorder::copy(&IMAGE);
      IMAGE.setFullROI();
    }else{
      IMAGE=cvt(image);
    }
  }
  draw = IMAGE;

  Vec p3D = estimate_3D_pos(POS,IMAGE,CAM);

  w.color(0,100,255,255);
  w.fill(0,100,255,100);
  w.rect(POS.x-1,POS.y-1,1,1);
  w.color(255,255,255,255);
  w.text("("+str(p3D[0])+","+str(p3D[1])+","+str(p3D[2]) + ")[VAL:"+str(p3D[3])+"]",POS.x+3,POS.y+3,-1,-1,10);

  draw.render();


  static TabHandle tab = gui["tab"];
  static DrawHandle3D draw3D = gui["draw3D"];
  static ComboHandle visColor = gui["visColor"];
  bool points = gui["points"];
  bool lines = gui["lines"];
  bool fill = gui["fill"];
  bool imageOn = gui["imageOn"];
  int pointSize = gui["pointSize"];

  if(resPos.wasTriggered()){
    scene.getCamera(0) = scene.getCamera(1);
  }

  if((*tab)->currentIndex() == 1){


    grid->setVisible(Primitive::vertex,points);
    grid->setVisible(Primitive::line,lines);
    grid->setVisible(Primitive::quad,fill);

    grid->update(IMAGE,CAM,(VisMode)visColor.getSelectedIndex(),pointSize,imageOn);

    ICLDrawWidget3D &dw = **draw3D;
    dw.link(scene.getGLCallback(0));
    dw.render();
  }

  Thread::msleep(10);
}


int main(int n, char **ppc){
  pa_explain("-i","for input device selection");
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params,camera-xml-file)",init,run).exec();
}
