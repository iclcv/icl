#include <ICLQuick/Common.h>
#include <ICLGeom/Camera.h>

#include <ICLGeom/Scene2.h>
#include <ICLFilter/MedianOp.h>
#include <ICLCore/ImgBorder.h>

GenericGrabber *grabber = 0;

GUI gui("hsplit");
Img32f IMAGE;
Camera CAM;
Point32f POS;
Scene2 scene;
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


struct ImageObj : public Object2{
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

    addTexture(s.getDim(),size.getDim()+1,size.getDim()+2,size.getDim()+3,image);
    
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

      m_vertices[size.getDim()+4] = cam.getPos();
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
  gui << ( GUI ("tab(2D,3D)[@handle=tab]")
           << "draw[@handle=draw@minsize=32x24]"
           << "draw3D[@handle=draw3D@minsize=32x24]"
         );
  
  GUI con("vbox[@maxsize=12x100]");
  con << "togglebutton(off,!on)[@out=grab@label=grab loop]";
  con << "togglebutton(off,!on)[@out=pp@label=median]";
  con << ( GUI("vbox[@label=3D vis]") 
           << "checkbox(points,on)[@out=points]"
           << "checkbox(lines,off)[@out=lines]"
           << "checkbox(fill,off)[@out=fill]"
           << "checkbox(image,off)[@out=imageOn]"
           << "spinner(1,10,2)[@out=pointSize@label=point size]"
           << "combo(solid,depth,intensity,confidence)[@handle=visColor@out=_]"
           << "button(reset pos)[@handle=resPos]"
         );

  //con << "camcfg";
  gui << con;
  gui.show();
  
  gui["draw"].install(new MouseHandler(mouse));
  
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setIgnoreDesiredParams(true);
  
  if(pa("-dist")){
    const ImgBase *image = grabber->grab();
    grabber->enableDistortion(DIST_FROM_PROGARG("-dist"),image->getSize());
  }
  if(pa("-props")){
    grabber->loadProperties(pa("-props"));
  }
                            
  CAM = Camera(*pa("-input",2));

  gui_DrawHandle(draw);
  gui_DrawHandle3D(draw3D);
  (*draw)->setRangeMode(ICLWidget::rmAuto);

  float data[] = {0,0,0,1000,1000,1000};
  Object2 *o = new Object2("cuboid",data);
  o->setVisible(Primitive::quad,false);
  o->setVisible(Primitive::triangle,false);

  scene.addObject(o);
  grid = new ImageObj(CAM.getViewPort().getSize());
  scene.addObject(grid);

  scene.addCamera(CAM);
  scene.addCamera(CAM);
  

  (*draw3D)->install(scene.getMouseHandler(0));


}


void run(){
  gui_DrawHandle(draw);
  gui_ButtonHandle(resPos);
  ICLDrawWidget &w = **draw;
  gui_bool(grab);
  gui_bool(pp);

  
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
  
  w.lock();
  w.reset();
  w.color(0,100,255,255);
  w.fill(0,100,255,100);
  w.rect(POS.x-1,POS.y-1,1,1);
  w.color(255,255,255,255);
  w.text("("+str(p3D[0])+","+str(p3D[1])+","+str(p3D[2]) + ")[VAL:"+str(p3D[3])+"]",POS.x+3,POS.y+3,-1,-1,10);
  w.unlock();

  draw.update();

  gui_TabHandle(tab);
  gui_DrawHandle3D(draw3D);
  gui_ComboHandle(visColor);
  gui_bool(points);
  gui_bool(lines);
  gui_bool(fill);
  gui_bool(imageOn);
  gui_int(pointSize);

  if(resPos.wasTriggered()){
    scene.getCamera(0) = scene.getCamera(1);
  }
  
  if((*tab)->currentIndex() == 1){


    grid->setVisible(Primitive::vertex,points);
    grid->setVisible(Primitive::line,lines);
    grid->setVisible(Primitive::quad,fill);

    grid->update(IMAGE,CAM,(VisMode)visColor.getSelectedIndex(),pointSize,imageOn);

    ICLDrawWidget3D &dw = **draw3D;
    dw.lock();
    dw.reset3D();
    dw.callback(scene.getGLCallback(0));
    dw.unlock();
    dw.update();
  }
  
  Thread::msleep(10);
}


int main(int n, char **ppc){
  paex
  ("-i","for input device selection")
  ("-d","given distortion parameters computed with icl-intrinsic-camera-calibration tool")
  ("-p","optionally define property-xml-file for the used camera");
  return ICLApplication(n,ppc,"[m]-input|-i(device,device-params,camera-xml-file) "
                        "-dist|-d(float,float,float,float) -props|-p(filename)",init,run).exec();
}
