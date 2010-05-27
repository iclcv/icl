/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#include <gtest/gtest.h>
#include <custom-assertions.h>
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
    xs_correct.push_back(Point32f(9250.9546,228.8122));
  }

  // virtual void TearDown() {}
  
  Camera cam;
  vector<Vec> Xs;
  vector<Point32f> xs_correct;
};

TEST(Camera, DecomposeQR) {
  FixedMatrix<icl32f,3,4> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);  
  FixedMatrix<icl32f,3,4> Q;
  FixedMatrix<icl32f,3,3> R;
  decompose_QR(A, Q, R);
  EXPECT_TRUE(isNear(A,Q*R,1e-6f));
}

TEST(Camera, DecomposeRQ) {
  FixedMatrix<icl32f,4,3> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  FixedMatrix<icl32f,4,3> Q;
  FixedMatrix<icl32f,3,3> R;
  decompose_RQ(A, R, Q);
  EXPECT_TRUE(isNear(A,R*Q,1e-6f));
}

TEST(Camera, SVD) {
  FixedMatrix<icl32f,3,4> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  FixedMatrix<icl32f,3,4> U;
  FixedColVector<icl32f,3> s;
  FixedMatrix<icl32f,3,3> V;
  svd_fixed<icl32f,3,4>(A,U,s,V);
  
  // singular values correctly sorted?
  EXPECT_GE(s[0],s[1]);
  EXPECT_GE(s[1],s[2]);
  // correct decomposition?
  FixedMatrix<icl32f,3,3> S(0.0);
  S(0,0) = s[0]; S(1,1) = s[1]; S(2,2) = s[2];
  EXPECT_TRUE(isNear(A,U*S*V.transp(),1e-6f));
  // orthogonal matrices?
  EXPECT_TRUE(isNear(V.transp()*V,FixedMatrix<icl32f,3,3>::id(),1e-6f));
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
    EXPECT_LE(xs_camera[i].distanceTo(xs_correct[i]), 0.001);
    //TODO: Make this more verbose!
  }
}

//class MyCameraTestSuite : public CxxTest::TestSuite {
//  void assertCameraSimilar(const MyCamera &cam_a, const MyCamera &cam_b, bool compare_render_params=false) {
//    assertDelta(cam_a.getPosition(), cam_b.getPosition(), 0.1);
//    assertDelta(cam_a.getNorm(), cam_b.getNorm(), 0.001);
//    assertDelta(cam_a.getUp(), cam_b.getUp(), 0.001);
//    TS_ASSERT_DELTA(cam_a.getFocalLength()*cam_a.getSamplingResolutionX(),
//                    cam_b.getFocalLength()*cam_b.getSamplingResolutionX(), 0.1);
//    TS_ASSERT_DELTA(cam_a.getFocalLength()*cam_a.getSamplingResolutionY(),
//                    cam_b.getFocalLength()*cam_b.getSamplingResolutionY(), 0.1);
//    TS_ASSERT_DELTA(cam_a.getPrincipalPointOffsetX(), cam_b.getPrincipalPointOffsetX(), 0.1);
//    TS_ASSERT_DELTA(cam_a.getPrincipalPointOffsetY(), cam_b.getPrincipalPointOffsetY(), 0.1);
//    TS_ASSERT_DELTA(cam_a.getSkew(), cam_b.getSkew(), 0.01);
//    if (compare_render_params) {
//      TS_ASSERT(cam_a.getRenderParams().chipSize == cam_b.getRenderParams().chipSize);
//      TS_ASSERT(cam_a.getRenderParams().viewport == cam_b.getRenderParams().viewport);
//      TS_ASSERT_DELTA(cam_a.getRenderParams().clipZFar, cam_b.getRenderParams().clipZFar, 0.001);
//      TS_ASSERT_DELTA(cam_a.getRenderParams().viewportZMin, cam_b.getRenderParams().viewportZMin, 0.001);
//      TS_ASSERT_DELTA(cam_a.getRenderParams().viewportZMax, cam_b.getRenderParams().viewportZMax, 0.001);
//    }
//  }
//  
//  void readPoints(const std::string &filename, vector<Vec> &world, vector<Point32f> &image) {
//    std::ifstream fs(filename.c_str());
//    if(!fs) throw ICLException("wrong filename: " + filename);
//    world.resize(40);
//    image.resize(40);
//    for (int i=0; i<40; i++) {
//      fs >> image[i] >> world[i];
//      //image.push_back(Point32f(800-p.x,600-p.y));
//    }
//  }

//  void getExampleSetup2(MyCamera &cam, vector<Vec> &world) {
//    cam.setPosition(Vec(0,0,150,1));
//    cam.setNorm(Vec(0,0,-1,1));
//    cam.setUp(Vec(0,1,0,1));
//    world.clear();
//    world.push_back(Vec(0,0,0,1));
//    world.push_back(Vec(100,0,0,1));
//    world.push_back(Vec(0,100,0,1));
//    world.push_back(Vec(0,0,100,1));
//  }
// 
//  float get_distance_point_line(Vec x, ViewRay l) {
//    Vec result = cross(l.offset-x,normalize3(l.direction));
//    result[3] = 0;
//    return result.length();    
//  }
//  
//  


//  

//  void test3DPositionEstimation(){
//    randomSeed();
//    static const int N=7;
//    std::vector<MyCamera*> CAMS(N);
//    std::vector<Point32f> projections(N);
//    
//    URand r(0,0.1);
//    
//    float nAll = 0;
//    float nWrong = 0;
//    for(int x=-10;x<10;x+=3){
//      for(int y=-10;y<10;y+=3){
//        for(int z=-10;z<10;z+=3){
//          
//          Vec pOrig(x,y,z,1);

//          for(int i=0;i<N;++i){
//            Vec norm(r,r,-1+r,0);
//            norm = norm * 1.0/norm.length();
//            norm[3] = 1;
//            CAMS[i] = new MyCamera(Vec(0,i,10,1), norm);
//            projections[i] = CAMS[i]->project(pOrig);
//          }
//          
//          try{
//            Vec pEst = MyCamera::estimate_3D_svd(CAMS,projections);
//            nAll++;
//            if((pOrig-pEst).length() > 0.001) nWrong++;
//          }catch(...){ nWrong++; }

//          for(int i=0;i<N;++i) delete CAMS[i];
//        }
//      }
//    }
//    if(nWrong/nAll > 0.1){
//      SHOW(nWrong);
//      SHOW(nAll);
//    }
//    TS_ASSERT_LESS_THAN(nWrong/nAll,0.1);
//  }
//  
//  void testProjectGL() {
//   // get example camera, 8 world points and the corresponding image points
//    vector<Vec> Xs;
//    MyCamera cam;
//    getExampleSetup2(cam, Xs);

//    std::cout << cam << std::endl;
//    cam.setPrincipalPointOffset(300,200);
//    cam.setSkew(-10);

////      Mat T = cam.getCSTransformationMatrix();
////      Mat P = cam.getProjectionMatrix();
////      Mat Tgl = cam.getCSTransformationMatrixGL();
////      Mat Pgl = cam.getProjectionMatrixGL();
////      Mat V = cam.getViewportMatrixGL();

////    DEBUG_LOG("Rot. matrix (Std): " << T);
////    DEBUG_LOG("Rot. matrix (OGL): " << Tgl);
//    
//    for (unsigned int i=0; i<Xs.size(); ++i) {
////      DEBUG_LOG("Transformation: ====================");
////      DEBUG_LOG("World coords.:     " << Xs[i].transp());
////      DEBUG_LOG("Cam. coords (Std): " << (T*Xs[i]).transp());
////      DEBUG_LOG("Cam. coords (OGL): " << (Tgl*Xs[i]).transp());
////      DEBUG_LOG("Projected (OGL):   " << (Pgl*Tgl*Xs[i]).transp());
////      DEBUG_LOG("Viewport (OGL):    " << (V*Pgl*Tgl*Xs[i]).transp());
////      DEBUG_LOG("Projected (Std):   " << (P*T*Xs[i]).transp());
////      DEBUG_LOG("Homogenized (Std): " << homogenize(P*T*Xs[i]).transp());
////      DEBUG_LOG("Homogenized (OGL): " << homogenize(V*Pgl*Tgl*Xs[i]).transp());
////      DEBUG_LOG("Std: " << Xs[i].transp() << " gets projected to " << cam.project(Xs[i]));
////      DEBUG_LOG("OGL: " << Xs[i].transp() << " gets projected to " << cam.projectGL(Xs[i]).transp());
////      TS_ASSERT_DELTA(cam.project(Xs[i]).x, cam.getRenderParams().viewport.width - cam.projectGL(Xs[i])[0], 0.001);
////      TS_ASSERT_DELTA(cam.project(Xs[i]).y, cam.getRenderParams().viewport.height - cam.projectGL(Xs[i])[1], 0.001);
//      TS_ASSERT_DELTA(cam.project(Xs[i]).x, cam.projectGL(Xs[i])[0], 0.001);
//      TS_ASSERT_DELTA(cam.project(Xs[i]).y, cam.projectGL(Xs[i])[1], 0.001);
//    }
//  }
//  
//  void testProjectsLikeRealCamera() {
//    // check that the projectGL projects the points to the same place as a real camera
//    // CAUTION: the up vector of the camera is actually looking downwards!
//    MyCamera cam(Vec(0,0,1000,1),Vec(0,0,-1,1),Vec(0,-1,0,1));
//    cam.getRenderParams().viewport = Rect(0,0,640,480);
//    cam.getRenderParams().chipSize = Size(640,480);
//    vector<Vec> Xws;
//    Xws.push_back(Vec(0,0,0,1));
//    Xws.push_back(Vec(50,0,0,1));
//    Xws.push_back(Vec(0,50,0,1));
//    Xws.push_back(Vec(0,0,50,1));
//    // project and test
//    vector<Vec> xis = cam.projectGL(Xws);
//    TS_ASSERT_DELTA(xis[0][0],320,1e-4);
//    TS_ASSERT_DELTA(xis[0][1],240,1e-4);
//    TS_ASSERT(xis[1][0] > 320);
//    TS_ASSERT_DELTA(xis[1][1],240,1e-4);
//    TS_ASSERT_DELTA(xis[2][0],320,1e-4);
//    TS_ASSERT(xis[2][1] < 240);
//    TS_ASSERT_DELTA(xis[3][0],320,1e-4);
//    TS_ASSERT_DELTA(xis[3][1],240,1e-4);
//  }


//  void testCalibrateWithRealData(){
//    std::vector<Vec> Xs;
//    std::vector<Point32f> xs;
//    readPoints("test/points.txt",Xs,xs);
//    
//    MyCamera cam_est = MyCamera::calibrate_pinv(Xs, xs);
//    MyCamera cam_est2 = MyCamera::calibrate(Xs, xs);
//    
//    SHOW(cam_est.getNorm());
//    SHOW(cam_est.getUp());
//    
//    SHOW(cam_est2.getNorm());
//    SHOW(cam_est2.getUp());

//    vector<Point32f> xs_est = cam_est.project(Xs);
//    vector<Point32f> xs_est2 = cam_est2.project(Xs);
//    for (unsigned int i=0; i<xs_est.size(); i++) {
//      TS_ASSERT_DELTA(xs[i].x, xs_est[i].x, 2);
//      TS_ASSERT_DELTA(xs[i].y, xs_est[i].y, 2);

//      TS_ASSERT_DELTA(xs[i].x, xs_est2[i].x, 1);
//      TS_ASSERT_DELTA(xs[i].y, xs_est2[i].y, 1);
//    }
//  }

//  
//  void testCalibrationPseudoinvers() {
//    // get example camera, 8 world points and the corresponding image points
//    MyCamera cam; vector<Vec> Xs; vector<Point32f> xs;
//    getExampleSetup(cam, Xs, xs);
//    cam.setSkew(10);
//    cam.setSamplingResolutionY(160);
//    xs = cam.project(Xs);
//  
//    // try to compute the camera from the corresponding points
//    MyCamera cam_est = MyCamera::calibrate_pinv(Xs, xs);
//    //assertCameraSimilar(cam_est,cam);
//    vector<Point32f> xs_est = cam_est.project(Xs);
//    for (unsigned int i=0; i<xs_est.size(); i++) {
//      TS_ASSERT_DELTA(xs[i].x, xs_est[i].x, 0.001);
//      TS_ASSERT_DELTA(xs[i].y, xs_est[i].y, 0.001);
//    }
//  }
//  
//  void testCalibrationSVD() {
//    // get example camera, 8 world points and the corresponding image points
//    MyCamera cam; vector<Vec> Xs; vector<Point32f> xs;
//    getExampleSetup(cam, Xs, xs);
//    cam.setSkew(10);
//    cam.setSamplingResolutionY(160);
//    xs = cam.project(Xs);
//  
//    // try to compute the camera from the corresponding points
//    MyCamera cam_est = MyCamera::calibrate(Xs, xs);
//    //assertCameraSimilar(cam_est,cam);
//    vector<Point32f> xs_est = cam_est.project(Xs);
//    for (unsigned int i=0; i<xs_est.size(); i++) {
//      TS_ASSERT_DELTA(xs[i].x, xs_est[i].x, 0.001);
//      TS_ASSERT_DELTA(xs[i].y, xs_est[i].y, 0.001);
//    }
//  }
//  
//  void testIO() {
//    MyCamera cam = getExampleCamera();
//    std::stringstream s;
//    s << cam;
//    MyCamera cam_from_stream(s);
//    assertCameraSimilar(cam, cam_from_stream,true);
//    TS_ASSERT_EQUALS(cam.getName(), cam_from_stream.getName());
//  }
//  
//  void testViewRay() {
//    // get example camera, 8 world points and the corresponding image points
//    MyCamera cam; vector<Vec> Xs; vector<Point32f> xs;
//    getExampleSetup(cam, Xs, xs);
//    // use project function of camera for higher accuracy
//    xs = cam.project(Xs);
//    // now check whether the viewray for each image point runs through the world point
//    for (unsigned int i=0; i<xs.size(); i++) {
//      ViewRay vr = cam.getViewRay(xs[i]);
//      TS_ASSERT_DELTA(get_distance_point_line(Xs[i],vr),0,0.1);
//    }
//  }
//};
