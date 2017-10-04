

#include <ICLQt/Common.h>

#include <ICLIO/GenericGrabber.h>
#include <ICLIO/GenericImageOutput.h>

#include <ICLFilter/ColorDistanceOp.h>

#include <ICLGeom/GenericPointCloudOutput.h>
#include <ICLGeom/PointCloudCreator.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/Scene.h>

// /////////////////////////////////////////////////////////////////////////////////////////////////

// GUI
GUI gui;
Scene scene;

Camera c_cam;
Camera d_cam;

// IO
GenericGrabber grabber_c;
GenericGrabber grabber_d;
GenericPointCloudOutput cloud_out;

PointCloudCreator pcc;

// others
PointCloudObject *pc_obj = 0;

Img32f depth_img;
Img8u color_img;
Img8u color_mapped;

// /////////////////////////////////////////////////////////////////////////////////////////////////

void init_icl() {

  // IO
  grabber_c.init(pa("-ic"));
  grabber_d.init(pa("-id"));

  
  if (pa("-pco")) {
    cloud_out.init(pa("-pco"));
  }

  const int camera_index = 0;
  c_cam = Camera(*pa("-icc"));
  d_cam = Camera(*pa("-idc"));
  scene.addCamera(d_cam);
  scene.addCamera(c_cam);

  std::string unit = *pa("-depth-image-unit");
  pcc.init(d_cam,c_cam,
           ( unit == "raw" ? PointCloudCreator::KinectRAW11Bit : 
             unit == "distToCamCenter" ? PointCloudCreator::DistanceToCamCenter :
             PointCloudCreator::DistanceToCamPlane) );
  
  grabber_c.setDesiredSizeInternal(c_cam.getResolution());
  grabber_d.setDesiredSizeInternal(d_cam.getResolution());

  // GUI
  gui << ( HBox()
           << Draw().handle("depth-view").label("Depth Image")
           << Draw().handle("color-view").label("Color Image")
           << Draw3D().handle("main-view").label("Main View")
           );
  gui << Show();

  // scene
  gui["main-view"].install(scene.getMouseHandler(camera_index));
  gui["main-view"].link(scene.getGLCallback(camera_index));

  pc_obj = new PointCloudObject(d_cam.getResolution().width,d_cam.getResolution().height);
  pc_obj->setLockingEnabled(true);
  scene.addObject(pc_obj);

}

void run_icl() {

  depth_img = *grabber_d.grab()->as32f();
  color_img = *grabber_c.grab()->as8u();

  pc_obj->lock();
  pcc.create(depth_img,*pc_obj);
  pcc.mapImage(&color_img,bpp(color_mapped));
  pc_obj->setColorsFromImage(color_mapped);
  pc_obj->unlock();

  if (!cloud_out.isNull())
    cloud_out.send(*pc_obj);

  gui["color-view"] = &color_img;
  gui["depth-view"] = &depth_img;

  gui["color-view"].render();
  gui["depth-view"].render();
  gui["main-view"].render();
}

// /////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

  pa_explain("-du","sets the expected unit of the input depth images one of raw, distToCamCenter or distToCamPlane.\n"
             "Note that a standard kinect input source produces images in distToCamPlane mode (which is also the default here).\n"
             "The non-raw formats use mm as units!");
  std::string args_init = ("-id(type=kinectd,device=0) "
                           "-ic(type=kinectc,device=0) "
                           "[m]-idc(1) "
                           "[m]-icc(1) "
                           "-pco(2) "
                           "-depth-image-unit|-du(depthImageUnit=distToCamPlane)");
  
  ICLApplication app(argc,argv,args_init,init_icl,run_icl);
  
  return app.exec();
}
