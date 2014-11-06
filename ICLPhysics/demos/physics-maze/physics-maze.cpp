#include <ICLGeom/Geom.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/PhysicsMouseHandler.h>
#include <ICLPhysics/SixDOFConstraint.h>
#include <MazeObject.h>
#include <HoleObject.h>

using namespace geom;
using namespace physics;
GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(200,0,70,1), Vec(-1,0,0,1), Vec(0,0,-1,1));
PhysicsScene scene;
PhysicsMouseHandler handler(0,&scene);
MazeObject* maze;

void init(){
  scene.addCamera(cam);
  maze = new MazeObject();
  maze->addToWorld(&scene);
  gui << Draw3D().handle("draw") << Show();
  gui["draw"].install(&handler);
  gui["draw"].link(scene.getGLCallback(0));
  scene.setGravity(Vec(0,0,-10000));
}

double fun(double val) {
  return max(min(1.,val*5),-1.);
}

utils::Time start = utils::Time::now();
void run()
{
  scene.Scene::lock();
  double time_val = (utils::Time::now() - start).toSecondsDouble();
  maze->setTransformation(Mat::id());
  maze->rotate(M_PI/180.f*fun(sin(time_val)),0,0);
  maze->rotate(0,M_PI/180.f*fun(cos(time_val)),0);
  Mat ball_trans = maze->mazeBall->getTransformation();
  Vec v(ball_trans(3,0),ball_trans(3,1),ball_trans(3,2));
  std::cout<<maze->getWorldToMazeTransform()*v<<std::endl;
  scene.step(-1,50,1/120.);
  scene.Scene::unlock();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
