/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/test/test-camera.cpp                           **
** Module : ICLGeom                                                **
** Authors: Erik Weitnauer                                         **
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

#include <gtest/gtest.h>
#include <ICLUtils/TestAssertions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/FixedMatrixUtils.h>
#include <ICLCore/Random.h>
#include <fstream>

using namespace icl;
using namespace std;
using namespace testing;

class CameraTest : public Test {
 protected:
  virtual void SetUp() {
    // initialize camera
    cam.setName("Example Camera");
    cam.setPosition(Vec(0,50,50,1));
    cam.setRotation(Vec(40.*M_PI/180.,-10.*M_PI/180.,0.));
    cam.setFocalLength(3);
    cam.setPrincipalPointOffset(Point32f(320,240));
    cam.setSamplingResolution(200,200);
    cam.setSkew(0);
    cam.getRenderParams().viewport = Rect(0,0,640,480);
    cam.getRenderParams().chipSize = Size(640,480);
    
    // initialize example world points
    Xs.push_back(Vec(406,941,1183,1));
    Xs.push_back(Vec(266,937,1062,1));
    Xs.push_back(Vec(-1,795,814,1));
    Xs.push_back(Vec(89,890,1139,1));
    Xs.push_back(Vec(223,692,1174,1));
    Xs.push_back(Vec(428,776,1072,1));
    Xs.push_back(Vec(432,924,1103,1));
    Xs.push_back(Vec(29,887,1097,1));
    
    // initialize corresponding image points
    xs_correct.push_back(Point32f(402.2513,207.6056));
    xs_correct.push_back(Point32f(357.7913,244.0200));
    xs_correct.push_back(Point32f(243.9973,289.5791));
    xs_correct.push_back(Point32f(275.6749,215.6134));
    xs_correct.push_back(Point32f(331.3610,126.2544));
    xs_correct.push_back(Point32f(433.0986,175.7763));
    xs_correct.push_back(Point32f(422.9612,220.7864));
    xs_correct.push_back(Point32f(250.9546,228.8122));
  }

  // virtual void TearDown() {}
  
  Camera cam;
  vector<Vec> Xs;
  vector<Point32f> xs_correct;
};

float get_distance_point_line(Vec x, ViewRay l) {
  Vec result = cross(l.offset-x,normalize3(l.direction));
  result[3] = 0;
  return result.length();    
}

TEST(Camera, set_rotation) {
  Camera c;
  c.setPosition(Vec(1,2,3,0));
  c.setRotation(Vec(40.*M_PI/180.,-10.*M_PI/180.,5.*M_PI/180.));
  Mat T_corr( 0.9811,-0.0858,-0.1736,-0.2885,
             -0.0444, 0.7729,-0.6330, 0.3978,
              0.1885, 0.6287, 0.7544,-3.7093,
                   0,      0,      0,      1);
  Mat T_cam = c.getCSTransformationMatrix();
  EXPECT_TRUE(isNear(T_corr, T_cam, 0.001f));
}

TEST_F(CameraTest, project_points) {
  // calculate the image points by using the camera object
  vector<Point32f> xs_camera = cam.project(Xs);
  ASSERT_EQ(xs_correct.size(), xs_camera.size());
  for (unsigned int i=0; i<xs_camera.size(); ++i) {
    SCOPED_TRACE("i is " + str(i));
    EXPECT_TRUE(isNear(xs_camera[i], xs_correct[i], 0.001));
  }
}

TEST_F(CameraTest, view_ray) {
  // use project function of camera for higher accuracy
  vector<Point32f> xs = cam.project(Xs);
  // now check whether the viewray for each image point runs through the world point
  for (unsigned int i=0; i<xs.size(); i++) {
    ViewRay vr = cam.getViewRay(xs[i]);
    EXPECT_NEAR(get_distance_point_line(Xs[i],vr),0,0.1);
  }
}

void expectCameraSimilar(const Camera &cam_expected, const Camera &cam_actual, bool compare_render_params=false) {
  EXPECT_TRUE(isNear(cam_expected.getPosition(), cam_actual.getPosition(), 0.1f));
  EXPECT_TRUE(isNear(cam_expected.getNorm(), cam_actual.getNorm(), 0.001f));
  EXPECT_TRUE(isNear(cam_expected.getUp(), cam_actual.getUp(), 0.001f));
  EXPECT_NEAR(cam_expected.getFocalLength()*cam_expected.getSamplingResolutionX(),
              cam_actual.getFocalLength()*cam_actual.getSamplingResolutionX(), 0.1);
  EXPECT_NEAR(cam_expected.getFocalLength()*cam_expected.getSamplingResolutionY(),
              cam_actual.getFocalLength()*cam_actual.getSamplingResolutionY(), 0.1);
  EXPECT_NEAR(cam_expected.getPrincipalPointOffsetX(), cam_actual.getPrincipalPointOffsetX(), 0.1);
  EXPECT_NEAR(cam_expected.getPrincipalPointOffsetY(), cam_actual.getPrincipalPointOffsetY(), 0.1);
  EXPECT_NEAR(cam_expected.getSkew(), cam_actual.getSkew(), 0.01);
  if (compare_render_params) {
    EXPECT_EQ(cam_expected.getRenderParams().chipSize, cam_actual.getRenderParams().chipSize);
    EXPECT_EQ(cam_expected.getRenderParams().viewport, cam_actual.getRenderParams().viewport);
    EXPECT_NEAR(cam_expected.getRenderParams().clipZFar, cam_actual.getRenderParams().clipZFar, 0.001);
    EXPECT_NEAR(cam_expected.getRenderParams().viewportZMin, cam_actual.getRenderParams().viewportZMin, 0.001);
    EXPECT_NEAR(cam_expected.getRenderParams().viewportZMax, cam_actual.getRenderParams().viewportZMax, 0.001);
  }
}


void readPoints(const std::string &filename, vector<Vec> &world, vector<Point32f> &image) {
  std::ifstream fs(filename.c_str());
  if(!fs) throw ICLException("wrong filename: " + filename);
  world.resize(40);
  image.resize(40);
  for (int i=0; i<40; i++) {
    fs >> image[i] >> world[i];
    //image.push_back(Point32f(800-p.x,600-p.y));
  }
}

TEST_F(CameraTest, IO) {
  // fixture provides a camera 'cam' and 8 world points 'Xs'
  stringstream s;
  s << cam;
  Camera cam_from_stream(s);
  expectCameraSimilar(cam, cam_from_stream,true);
  EXPECT_EQ(cam.getName(), cam_from_stream.getName());
}

TEST_F(CameraTest, calibration_SVD) {
  // fixture provides a camera 'cam' and 8 world points 'Xs'
  cam.setSkew(10);
  cam.setSamplingResolutionY(160);
  vector<Point32f> xs = cam.project(Xs);

  // try to compute the camera from the corresponding points
  Camera cam_est = Camera::calibrate(Xs, xs);
  expectCameraSimilar(cam, cam_est);
  vector<Point32f> xs_est = cam_est.project(Xs);
  for (unsigned int i=0; i<xs_est.size(); i++) {
    EXPECT_NEAR(xs[i].x, xs_est[i].x, 0.001);
    EXPECT_NEAR(xs[i].y, xs_est[i].y, 0.001);
  }
}

TEST_F(CameraTest, calibration_pseudo_inverse) {
  // fixture provides a camera 'cam' and 8 world points 'Xs'
  cam.setSkew(10);
  cam.setSamplingResolutionY(160);
  vector<Point32f> xs = cam.project(Xs);

  // try to compute the camera from the corresponding points
  Camera cam_est = Camera::calibrate_pinv(Xs, xs);
  expectCameraSimilar(cam, cam_est);
  vector<Point32f> xs_est = cam_est.project(Xs);
  for (unsigned int i=0; i<xs_est.size(); i++) {
    EXPECT_NEAR(xs[i].x, xs_est[i].x, 0.01);
    EXPECT_NEAR(xs[i].y, xs_est[i].y, 0.01);
  }
}

TEST(Camera, calibrate_with_real_data) {
  std::vector<Vec> Xs;
  std::vector<Point32f> xs;
  readPoints("points.txt",Xs,xs);
  
  Camera cam_est = Camera::calibrate(Xs, xs);
  Camera cam_est2 = Camera::calibrate_pinv(Xs, xs);
  
  vector<Point32f> xs_est = cam_est.project(Xs);
  vector<Point32f> xs_est2 = cam_est2.project(Xs);
  for (unsigned int i=0; i<xs_est.size(); i++) {
    EXPECT_NEAR(xs[i].x, xs_est[i].x, 1);
    EXPECT_NEAR(xs[i].y, xs_est[i].y, 1);

    EXPECT_NEAR(xs[i].x, xs_est2[i].x, 2);
    EXPECT_NEAR(xs[i].y, xs_est2[i].y, 2);
  }
}

TEST(Camera, projects_like_real_camera) {
  // check that the projectGL projects the points to the same place as a real camera
  // CAUTION: the up vector of the camera is actually looking downwards!
  Camera cam(Vec(0,0,1000,1),Vec(0,0,-1,1),Vec(0,-1,0,1));
  cam.getRenderParams().viewport = Rect(0,0,640,480);
  cam.getRenderParams().chipSize = Size(640,480);
  vector<Vec> Xws;
  Xws.push_back(Vec(0,0,0,1));
  Xws.push_back(Vec(50,0,0,1));
  Xws.push_back(Vec(0,50,0,1));
  Xws.push_back(Vec(0,0,50,1));
  // project and test
  vector<Vec> xis = cam.projectGL(Xws);
  EXPECT_NEAR(xis[0][0],320,1e-4);
  EXPECT_NEAR(xis[0][1],240,1e-4);
  EXPECT_GT(xis[1][0], 320);
  EXPECT_NEAR(xis[1][1],240,1e-4);
  EXPECT_NEAR(xis[2][0],320,1e-4);
  EXPECT_LT(xis[2][1], 240);
  EXPECT_NEAR(xis[3][0],320,1e-4);
  EXPECT_NEAR(xis[3][1],240,1e-4);
}

TEST(Camera, projectGL) {
  // get example camera, 8 world points and the corresponding image points
  Camera cam;
  cam.setPosition(Vec(0,0,150,1));
  cam.setNorm(Vec(0,0,-1,1));
  cam.setUp(Vec(0,1,0,1));
  vector<Vec> Xs;
  Xs.push_back(Vec(0,0,0,1));
  Xs.push_back(Vec(100,0,0,1));
  Xs.push_back(Vec(0,100,0,1));
  Xs.push_back(Vec(0,0,100,1));

  cam.setPrincipalPointOffset(300,200);
  cam.setSkew(-10);

  //Mat T = cam.getCSTransformationMatrix();
  //Mat P = cam.getProjectionMatrix();
  //Mat Tgl = cam.getCSTransformationMatrixGL();
  //Mat Pgl = cam.getProjectionMatrixGL();
  //Mat V = cam.getViewportMatrixGL();

  //DEBUG_LOG("Rot. matrix (Std): " << T);
  //DEBUG_LOG("Rot. matrix (OGL): " << Tgl);

  for (unsigned int i=0; i<Xs.size(); ++i) {
    //DEBUG_LOG("Transformation: ====================");
    //DEBUG_LOG("World coords.:     " << Xs[i].transp());
    //DEBUG_LOG("Cam. coords (Std): " << (T*Xs[i]).transp());
    //DEBUG_LOG("Cam. coords (OGL): " << (Tgl*Xs[i]).transp());
    //DEBUG_LOG("Projected (OGL):   " << (Pgl*Tgl*Xs[i]).transp());
    //DEBUG_LOG("Viewport (OGL):    " << (V*Pgl*Tgl*Xs[i]).transp());
    //DEBUG_LOG("Projected (Std):   " << (P*T*Xs[i]).transp());
    //DEBUG_LOG("Homogenized (Std): " << homogenize(P*T*Xs[i]).transp());
    //DEBUG_LOG("Homogenized (OGL): " << homogenize(V*Pgl*Tgl*Xs[i]).transp());
    //DEBUG_LOG("Std: " << Xs[i].transp() << " gets projected to " << cam.project(Xs[i]));
    //DEBUG_LOG("OGL: " << Xs[i].transp() << " gets projected to " << cam.projectGL(Xs[i]).transp());
    EXPECT_NEAR(cam.project(Xs[i]).x, cam.projectGL(Xs[i])[0], 0.001);
    EXPECT_NEAR(cam.project(Xs[i]).y, cam.projectGL(Xs[i])[1], 0.001);
  }
  
  // TODO: remove DEBUG_LOG() calls, when I don't need them anymore  for wrting
  // the Camera documentation.
}

TEST(Camera, 3D_position_estimation) {
  // TODO: Is Camera::estimate_3D_svd still broken as commented?
  randomSeed();
  static const int N=7;
  std::vector<Camera*> CAMS(N);
  std::vector<Point32f> projections(N);
  
  URand r(0,0.1);
  
  float nAll = 0;
  float nWrong = 0;
  for(int x=-10;x<10;x+=3){
    for(int y=-10;y<10;y+=3){
      for(int z=-10;z<10;z+=3){
        
        Vec pOrig(x,y,z,1);

        for(int i=0;i<N;++i){
          Vec norm(r,r,-1+r,0);
          norm = norm * 1.0/norm.length();
          norm[3] = 1;
          CAMS[i] = new Camera(Vec(0,i,10,1), norm);
          projections[i] = CAMS[i]->project(pOrig);
        }
        
        try{
          Vec pEst = Camera::estimate_3D_svd(CAMS,projections);
          nAll++;
          if((pOrig-pEst).length() > 0.001) nWrong++;
        }catch(...){ nWrong++; }

        for(int i=0;i<N;++i) delete CAMS[i];
      }
    }
  }
  if(nWrong/nAll > 0.1){
    SHOW(nWrong);
    SHOW(nAll);
  }
  EXPECT_LT(nWrong/nAll,0.1);
}

