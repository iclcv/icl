#include <ICLQuick/Common.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MedianOp.h>

#include <ICLGeom/CoplanarPointPoseEstimator.h>
#include <ICLGeom/Scene.h>


GUI gui("hsplit");
RegionDetector rd(6,2<<20,0,255,true);
GenericGrabber grabber;
LocalThresholdOp lt;
Scene scene;

static float cube_params[] = {0,0,-35,70};
struct Cube : public SceneObject{
  Cube():SceneObject("cube",cube_params){
    baseVertices = m_vertices;
  }
  void setPose(const Mat &T){
    for(unsigned int i=0;i<m_vertices.size();++i){
      m_vertices[i] = T * baseVertices[i];
    }
  }
  std::vector<Vec> baseVertices;
} cube;

void init(){
  if(pa("-create-marker-image")){
    const icl8u X=0;
    const icl8u O=255;
    static const icl8u p[11*11] = { O,O,O,O,O,O,O,O,O,O,O,
                                    O,X,X,X,X,X,X,X,X,X,O,
                                    O,X,O,O,O,O,O,O,O,X,O,
                                    O,X,O,X,X,X,X,X,O,X,O,
                                    O,X,O,X,O,X,O,X,O,X,O,
                                    O,X,O,X,X,X,X,X,O,X,O,
                                    O,X,O,O,O,O,O,O,O,X,O,
                                    O,X,O,X,O,X,O,X,O,X,O,
                                    O,X,O,O,O,O,O,O,O,X,O,
                                    O,X,X,X,X,X,X,X,X,X,O,
                                    O,O,O,O,O,O,O,O,O,O,O };
    Img8u im(Size(11,11),formatGray);
    std::copy(p,p+11*11,im.begin(0));
    im.scale(Size(1100,1100),interpolateNN);
    save<icl8u>(im,*pa("-create-marker-image"));
    std::cout << "wrote marker image to " << *pa("-create-marker-image") << std::endl;
    std::cout << "print out this marker so that each 'rect' is 10x10 mm" << std::endl;
    exit(0);
  }
  gui << "draw3D()[@handle=draw@minsize=32x24]" 
      << ( GUI("vbox[@minsize=10x1@maxsize=10x100]")
           << "combo(gray,thresh,median)[@handle=vis@label=visualization]"
           << "slider(1,80,40)[@label=mask size@out=masksize]"
           << "slider(-30,30,-9)[@label=threshold@out=thresh]"
           << ( GUI("vbox[@label=CSS Params]")
                << "fslider(1,180,150)[@out=angleThresh]"
                << "fslider(0.5,5,1.5)[@out=rcCoeff]"
                << "fslider(1,5,3)[@out=sigma]"
                << "fslider(10,300,100)[@out=curvCutoff]"
                << "fslider(0.01,0.5,0.1)[@out=straighLineThresh]"
              )
           << "checkbox(use corners only,checked)[@out=useCornersOnly]"
           << "togglebutton(stopped,!running)[@out=cap]"
         )
      << "!show";
  
  grabber.resetBus();
  grabber.init(FROM_PROGARG("-i"));
  grabber.setDesiredSize(Size(1280,960));
  grabber.setDesiredFormat(formatGray);
  
  Camera cam;
  if(pa("-cam")){
    cam = Camera(*pa("-cam"));
  }else{
    const ImgBase *image = grabber.grab();
    cam.setResolution(image->getSize());
    cam.setFocalLength(5);
  }
  
  scene.addCamera(cam);
  scene.addObject(&cube);
}

struct sort_by_sprod_ortho{
  Point32f p;
  sort_by_sprod_ortho(const Point32f &p):
    p(-p.y,p.x){}

  inline float sprod(Point32f a) const{
    return (a.x*p.x) + (a.y*p.y);
  }
  bool operator()(const Point32f &a,const Point32f &b) const{
    return sprod(a) < sprod(b);
  }
};


Point32f find_point_closest_to(const Point32f &p, const std::vector<Point32f> &ps){
  Point32f minp = ps[0];
  float minDist = minp.distanceTo(p);
  for(unsigned int i=1;i<ps.size();++i){
    float cDist = ps[i].distanceTo(p);
    if(cDist < minDist){
      minDist = cDist;
      minp = ps[i];
    }
  }
  return minp;
}

Point32f find_point_furthest_from(const Point32f &p, const std::vector<Point32f> &ps){
  Point32f maxp = ps[0];
  float maxDist = maxp.distanceTo(p);
  for(unsigned int i=1;i<ps.size();++i){
    float cDist = ps[i].distanceTo(p);
    if(cDist > maxDist){
      maxDist = cDist;
      maxp = ps[i];
    }
  }
  return maxp;
}

void run(){
  gui_DrawHandle3D(draw);
  gui_int(masksize);
  gui_int(thresh);
  gui_ComboHandle(vis);
  gui_bool(cap);
  
  while(!cap){
    Thread::msleep(10);
  }

  lt.setup(masksize,thresh);
  static MedianOp mo(Size(3,3));
  
  const ImgBase * imgs[] = {
    grabber.grab(),
    lt.apply(imgs[0]),
    mo.apply(imgs[1])
  };
  
  const std::vector<ImageRegion> &rs = rd.detect(imgs[2]);
  
  ImageRegion r; //null
  
  Point32f a[3],b[3],A,B,C,D;

  for(unsigned int i=0;i<rs.size();++i){
    if(rs[i].getVal() != 255) continue;
    if(rs[i].getSize() < 100) continue;
    if(rs[i].getFormFactor() < 1.3) continue;
    if(rs[i].getFormFactor() > 5) continue;

    const std::vector<ImageRegion> &srs = rs[i].getSubRegions(true);
    if(srs.size() != 4) continue;

    unsigned int ns[] = { 
      srs[0].getSubRegions(true).size(),
      srs[1].getSubRegions(true).size(),
      srs[2].getSubRegions(true).size(),
      srs[3].getSubRegions(true).size(),
    };
    
    std::sort(ns,ns+4);

    if(ns[0] || ns[1] || ns[2] || (ns[3] != 2) ) continue;
    
    int j=0;
    for(int k=0;k<4;++k){
      if(!srs[k].getSubRegions(true).size()){
        a[j++] = srs[k].getCOG();
      } else{
        b[0] = srs[k].getSubRegions(true)[0].getCOG();
        b[1] = srs[k].getSubRegions(true)[1].getCOG();
      }
    }
    A = (a[0]+a[1]+a[2])*(1.0/3.0);
    B = (b[0]+b[1])*0.5;
    C = (A+B)*0.5;
    D = A - B;
    
    std::sort(a,a+3,sort_by_sprod_ortho(D));
    std::sort(b,b+2,sort_by_sprod_ortho(D));

    r = rs[i].getParentRegion();
    break;
  }
  
  draw = imgs[(int)vis];
  draw->lock();
  draw->reset3D();
  draw->reset();

  if(r){
    const std::vector<Point32f> bs = r.getBoundaryCorners(gui["angleThresh"],
                                                          gui["rcCoeff"],
                                                          gui["sigma"],
                                                          gui["curvCutoff"],
                                                          gui["straighLineThresh"]);
    if(bs.size() == 4){

      static CoplanarPointPoseEstimator pe(CoplanarPointPoseEstimator::worldFrame);
      
      Point32f e[4] = {
        find_point_closest_to(a[0],bs),
        find_point_closest_to(a[2],bs),
        find_point_furthest_from(a[0],bs),
        find_point_furthest_from(a[2],bs)
      };
      
      if(gui["useCornersOnly"]){
        std::copy(bs.begin(),bs.end(),e);
        
      }
      
      const int N = 9;
      Point32f imagePoints[N] = {a[0],a[1],a[2],b[0],b[1],e[0],e[1],e[2],e[3]};
      static const Point32f modelPoints[N] = {
        Point32f(-20,-20),
        Point32f(0,-20),
        Point32f(20,-20),
        Point32f(-10,10),
        Point32f(10,10),
        Point32f(-45,-45),
        Point32f(45,-45),
        Point32f(45,45),
        Point32f(-45,45)
      };

      
      Mat T = gui["useCornersOnly"] ? pe.getPose(N-5,modelPoints+5,imagePoints+5,scene.getCamera(0)):
      pe.getPose(N,modelPoints,imagePoints,scene.getCamera(0));
      
      //SHOW(T);
      //SHOW(scene.getCamera(0).getCSTrasformationMatrix());
      
      cube.setPose(T);
      
      draw->color(255,0,0,255);
      draw->linewidth(2);
      draw->linestrip(r.getBoundary());
      
      draw->color(0,255,0,255);
      draw->symsize(20);
      
      // SHOW(bs.size());
      for(unsigned int i=0;i<bs.size();++i){
        draw->sym(bs[i],'+');
      }

      for(int i=0;i<4;++i){
        if(i<3){
          draw->color(0,100,255,255);
          draw->text("A-"+str(i),a[i].x,a[i].y,10);
          if(i<2) draw->text("B-"+str(i),b[i].x,b[i].y,10);
        }
        draw->color(255,0,255,255);
        draw->text("E-"+str(i),e[i].x,e[i].y,10);
      }
      
      draw->color(255,0,0,255);
      draw->text("_A_",A.x,A.y);
      draw->text("_B_",B.x,B.y);
      draw->text("_C_",C.x,C.y);
      draw->arrow(C,C+D);
      
      draw->callback(scene.getGLCallback(0));
      draw->linewidth(1);
    }
  }
  
  draw->unlock();
  draw.update();

}


int main(int n, char **ppc){
  paex
  ("-cam","optionally, a camera calibration file can be passed here\n"
   "obtain such a calibration file using 'icl-cam-calib' application")
  ("-input","ICL's default input device specification")
  ("-create-marker-image","if this flag is given, only a marker image is created");
  return ICLApp(n,ppc,"[m]-input|-i(2) -cam(1) -create-marker-image(output-filename)",init,run).exec();
}


