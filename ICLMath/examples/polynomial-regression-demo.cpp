#include <ICLQt/Common.h>
#include <ICLMath/PolynomialRegression.h>
#include <ICLGeom/Geom.h>
#include <ICLGeom/GridSceneObject.h>
#include <ICLUtils/Random.h>

HSplit gui;
typedef float Scalar;
typedef DynMatrix<Scalar> Matrix;
typedef PolynomialRegression<Scalar> Reg;

Scene scene;
GenericGrabber grabber;

ImgQ approx(const ImgQ &image){
  static std::string lastF = gui["f"];
  std::string currF = gui["f"];
  static Reg *reg = new Reg(currF);
  gui["fused"] = reg->getFunctionString();
  gui["status"] = str("ok");

  static ImgQ rimage(image.getSize(),3);

  if(lastF != currF){
    try{
      Reg *rnew = new Reg(currF);
      reg = rnew;
      gui["fused"] = reg->getFunctionString();
      gui["status"] = str("ok");
      lastF = currF;
    }catch(ICLException &e){
      gui["status"] = str(e.what());
    }
  }

  try{
    const int N= image.getDim();
    Matrix xs(2,N);
    Matrix ys(3,N);
    
    for(int y=0,idx=0;y<image.getHeight();++y){
      for(int x=0;x<image.getWidth();++x,++idx){
        xs(0,idx) = x;
        xs(1,idx) = y;
        
        ys(0,idx) = image(x,y,0);
        ys(1,idx) = image(x,y,1);
        ys(2,idx) = image(x,y,2);
      }
    }
    
    const Reg::Result &result = reg->apply(xs,ys);
    
    const Matrix &z = result(xs);
    
    for(int i=0;i<3;++i){
      std::copy(z.col_begin(i),z.col_end(i), rimage.begin(i));
    }
  }catch(ICLException &e){
    gui["status"] = str(e.what());
  }
  return rimage;
}

// approximates a rgb image by color patches of size cellsize x cellsize
// each color patch is learned using polynomial regression
void init_2D_demo(){
  grabber.init(pa("-2D"));
  grabber.useDesired(formatRGB);
  grabber.useDesired(depth32f);
  grabber.useDesired(Size::QVGA);
  
  gui << Image().handle("input")
      << Image().handle("result")
      << ( VBox().minSize(14,1)
           << Slider(2,100,10,true).handle("cellsize").label("cell size")
           << String("1 + x0 + x1 + x0*x1").handle("f").label("function")
           << Label("--").handle("fused").label("current function")
           << Label("--").handle("status").label("status")
           )
      << Show();
}

void init_3D_demo(){
  randomSeed();

  Reg reg(*pa("-f"));
  static const int N = pa("-n-samples");

  Matrix xs(2,N);
  Matrix ys(1,N);

  std::cout << "Parsed Function: " << reg.getFunctionString() << std::endl;
  
  URand r(-6,6);
  GRand noise(0,pa("-n"));

  SceneObject *o = new SceneObject;
  
  for(int i=0;i<N;++i){
    xs(0,i) = r;
    xs(1,i) = r;
    ys[i] = 0.1 * ( sqr(xs(0,i)) + sqr(xs(1,i)) ) + noise;

    o->addVertex(Vec(xs(0,i), xs(1,i), ys[i], 1));
  }

  o->setColor(Primitive::vertex,geom_red());
  o->setVisible(Primitive::vertex,true);
  o->setPointSize(5);
  scene.addObject(o);

  const Reg::Result &result = reg.apply(xs,ys);
  
  int dim = 151;
  xs.setBounds(2,dim*dim);
  for(float x=-dim/2;x<=dim/2;++x){
    for(float y=-dim/2;y<=dim/2;++y){
      xs(0,(x+dim/2)+dim*(y+dim/2)) = x/10.0;
      xs(1,(x+dim/2)+dim*(y+dim/2)) = y/10.0;
    }
  }
  
  const Matrix &grid = result(xs);
  
  std::vector<Vec> ps(dim*dim);
  for(int i=0;i<dim*dim;++i){
    ps[i] = Vec(xs(0,i), xs(1,i), grid[i]);
  }
  
  
  SceneObject *ogrid = new GridSceneObject(dim,dim,ps,false,true);
  ogrid->createAutoNormals();
  ogrid->setVisible(Primitive::vertex,false);
  ogrid->setColor(Primitive::quad,GeomColor(0,100,255,200));


  scene.addObject(ogrid);
  
  scene.addCamera(Camera(Vec(16.6866,10.8957,21.3801,1),
                         Vec(-0.672659,-0.415045,-0.61259,1),
                         Vec(0.453578,0.431459,-0.779814,1),
                         3, Point32f(320,240),200,200,
                         0, Camera::RenderParams(Size(640,480),
                                                 1,10000, 
                                                 Rect(0,0,640,480),
                                                 0,1)));


  gui << Draw3D().handle("draw").minSize(32,24) << Show();
  
  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0));
  
  scene.getLight(0).setSpecular(GeomColor(0,200,255,255));
  scene.getLight(0).setSpecularEnabled(true);
  scene.setDrawCoordinateFrameEnabled(true,4);
}

void init(){
  if(pa("-2D")){
    init_2D_demo();
  }else{
    init_3D_demo();
  }
}

void run_2D(){
  ImgQ image = *grabber.grab()->as32f();
  int cellsize = gui["cellsize"];
  while(image.getWidth() % cellsize || image.getHeight() & cellsize) --cellsize;
  static ImgQ result(image.getSize(),formatRGB);
  static ImgQ tmpa(Size(cellsize,cellsize),formatRGB);
  tmpa.setSize(Size(cellsize,cellsize));
      
  tic();
  for(int y=0;y<image.getHeight()/cellsize;++y){
    for(int x=0;x<image.getWidth()/cellsize;++x){
      Rect r(x*cellsize,y*cellsize,cellsize,cellsize);
      image.setROI(r);
      result.setROI(r);
      
      image.deepCopyROI(&tmpa);
      roi(result) = approx(tmpa);
    }
  }
  toc();

  gui["input"] = &image;
  gui["result"] = &result;
  
}

void run_3D(){
  Thread::msleep(1000);
}

void run(){
  void(*run_func)() = pa("-2D") ? run_2D : run_3D;
  run_func();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-2D(input-type,input-ID) "
                "[m]-f(function-string) -noise|-n(float=0) -n-samples(n=100)",init,run).exec();
}
