/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Camera.cpp                         **
** Module : ICLGeom                                                **
** Authors: Erik Weitnauer, Christof Elbrechter                    **
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

#include <ICLUtils/ConfigFile.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLUtils/XML.h>
#include <ICLIO/ImageUndistortion.h>
#include <ICLUtils/File.h>
#include <ICLGeom/Camera.h>
#include <fstream>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;

namespace icl {
  namespace geom{

    static inline float sprod_3(const Vec &a, const Vec &b){
      return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
    }


    void Camera::setRotation(const Vec &rot) {
      // see http://en.wikipedia.org/wiki/Rotation_matrix#Dimension_three
      float sin_a = sin(rot[0]); float cos_a = cos(rot[0]);
      Mat3x3 R_x(1, 0, 0,
              0, cos_a, -sin_a,
              0, sin_a, cos_a);
      float sin_b = sin(rot[1]); float cos_b = cos(rot[1]);
      Mat3x3 R_y(cos_b, 0, sin_b,
              0, 1, 0,
              -sin_b, 0, cos_b);
      float sin_c = sin(rot[2]); float cos_c = cos(rot[2]);
      Mat3x3 R_z(cos_c, -sin_c, 0,
              sin_c,  cos_c, 0,
              0, 0, 1);
      setRotation(R_x*R_y*R_z);
    }

    Mat Camera::createTransformationMatrix(const Vec &norm, const Vec &up, const Vec &pos) {
      // see http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness
      /* [ --- hh --- | -hh.p ]
         [ --- up --- | -up.p ]
         [ -- norm -- | -no.p ]
         [ 0   0   0  |   1   ]
      */
      // double cross product to ensure that they are all orthogonal
      Vec hh = cross(up,norm);
      Vec uu = cross(norm,hh);

      Mat T;
      T.row(0) = hh;
      T.row(1) = uu;
      T.row(2) = norm;
      T.row(3) = Vec(0.0);
      T.col(3) = Vec(0.0);

      T(3,0) = -sprod_3(hh, pos);
      T(3,1) = -sprod_3(uu, pos);
      T(3,2) = -sprod_3(norm, pos);
      T(3,3) = 1;

      return T;
    }

    void Camera::orthogonalizeRotationMatrix(){
      Mat3 R = getCSTransformationMatrix().part<0,0,3,3>();
      Mat3 Ro = gramSchmidtOrtho(R);
      setRotation(Ro);
    }


    Mat Camera::getInvCSTransformationMatrix() const{
      Vec hh = cross(m_up,m_norm);
      Vec uu = cross(m_norm,hh);

      return Mat(hh[0], uu[0], m_norm[0], m_pos[0],
                 hh[1], uu[1], m_norm[1], m_pos[1],
                 hh[2], uu[2], m_norm[2], m_pos[2],
                 0,0,0,1);
    }


    Mat Camera::getCSTransformationMatrix() const {
      return createTransformationMatrix(m_norm, m_up, m_pos);
    }

    Mat Camera::getCSTransformationMatrixGL() const {
      // for OpenGL, the camera looks in negative z direction
      return createTransformationMatrix(-m_norm, m_up, m_pos);
    }

    FixedMatrix<icl32f,4,3> Camera::getQMatrix() const {
      Mat K = getProjectionMatrix();
      Mat CS = getCSTransformationMatrix();

      FixedMatrix<icl32f,4,3> cs(CS.begin());
      FixedMatrix<icl32f,3,3> k( K(0,0), K(1,0), K(2,0),
                                 K(0,1), K(1,1), K(2,1),
                                 K(0,3), K(1,3), K(2,3) );
      return k*cs;
    }


    FixedMatrix<icl32f,3,4> Camera::getInvQMatrix() const{
      // (P*T).inv = T.inv * P.inv
      Mat Tinv = getInvCSTransformationMatrix();
      Mat P = getProjectionMatrix();
      Mat II = Tinv * P.pinv(true, std::numeric_limits<icl32f>::epsilon());
      return FixedMatrix<icl32f,3,4> (II(0,0),II(1,0),II(3,0),
                                      II(0,1),II(1,1),II(3,1),
                                      II(0,2),II(1,2),II(3,2),
                                      II(0,3),II(1,3),II(3,3));
    }


    Mat Camera::getProjectionMatrix() const {
      return Mat(m_f * m_mx,   m_skew, m_px, 0,
                          0, m_f*m_my, m_py, 0,
                          0,        0, 0,    0,
                          0,        0, 1,    0);
    }

    Mat Camera::getProjectionMatrixGL() const {
      float clipZNear = m_renderParams.clipZNear == 0 ? m_f : m_renderParams.clipZNear;
      float A = (m_renderParams.clipZFar + clipZNear)/(clipZNear - m_renderParams.clipZFar);
      float B = (2*m_renderParams.clipZFar*clipZNear)/(clipZNear - m_renderParams.clipZFar);
      float w2 = m_renderParams.chipSize.width/2;
      float h2 = m_renderParams.chipSize.height/2;
      // Because OpenGL will automatically flip the y-coordinates in the end,
      // we need switch the sign of the skew and py components of the matrix.
      // Also we need switch the sign back when doing the projection by hand.
      return Mat( -m_f*m_mx/w2,     m_skew/w2,  -(m_px-w2)/w2,  0,
                            0,     -m_f*m_my/h2, (m_py-h2)/h2,  0,
                            0,               0,             A,  B,
                            0,               0,            -1,  0);
    }

    Mat Camera::getViewportMatrixGL() const {
      float w2 = m_renderParams.viewport.width/2;
      float h2 = m_renderParams.viewport.height/2;
      const float &zFar = m_renderParams.viewportZMax;
      const float &zNear = m_renderParams.viewportZMin;
      return Mat(w2,  0, 0, w2 + m_renderParams.viewport.x,
                  0, h2, 0, h2 + m_renderParams.viewport.y,
                  0,  0, (zFar-zNear)/2, (zFar+zNear)/2,
                  0,  0, 0,    1);
    }

    // Projects a world point to the screen
    Point32f Camera::project(const Vec &Xw) const {
      Mat T = getCSTransformationMatrix();
      Mat P = getProjectionMatrix();

      //#warning "testing project fix here!"
      //P(1,0) *= -1; P(2,1) *= -1;

      Vec xi = homogenize(P*T*Xw);

      return Point32f(xi[0],xi[1]);
    }


    // Projects a set of points
    void Camera::project(const std::vector<Vec> &Xws, std::vector<Point32f> &dst) const{
      dst.resize(Xws.size());
      Mat T = getCSTransformationMatrix();
      Mat P = getProjectionMatrix();

      //#warning "testing project fix here!"
      //P(1,0) *= -1; P(2,1) *= -1;

      Mat M = P*T;
      for(unsigned int i=0;i<Xws.size();++i){
        Vec xi = homogenize(M*Xws[i]);
        dst[i].x = xi[0];
        dst[i].y = xi[1];
      }
    }

    // Projects a set of points
    const std::vector<Point32f> Camera::project(const std::vector<Vec> &Xws) const{
      std::vector<Point32f> xis;
      project(Xws,xis);
      return xis;
    }

    /// Project a world point onto the image plane.
    Vec Camera::projectGL(const Vec &Xw) const {
      Mat T = getCSTransformationMatrix();
      Mat P = getProjectionMatrixGL();
      // correct the sign of skew and y-offset component
      P(1,0) *= -1; P(2,1) *= -1;
      Mat V = getViewportMatrixGL();
      return homogenize(V*P*T*Xw);
    }

    /// Project a vector of world points onto the image plane.
    void Camera::projectGL(const std::vector<Vec> &Xws, std::vector<Vec> &dst) const {
      dst.resize(Xws.size());
      Mat T = getCSTransformationMatrix();
      Mat P = getProjectionMatrixGL();
      // correct the sign of skew and y-offset component
      P(1,0) *= -1; P(2,1) *= -1;
      Mat V = getViewportMatrixGL();
      Mat M = V*P*T;
      for(unsigned int i=0;i<Xws.size();++i) {
        dst[i] = homogenize(M*Xws[i]);
      }
    }

    /// Project a vector of world points onto the image plane.
    const std::vector<Vec> Camera::projectGL(const std::vector<Vec> &Xws) const {
      std::vector<Vec> dst;
      projectGL(Xws, dst);
      return dst;
    }

    void Camera::setRotation(const Mat3x3 &rot) {
      m_norm = Vec(rot(0,2), rot(1,2), rot(2,2), 0).normalized();
      m_norm[3] = 1;
      m_up = Vec(rot(0,1), rot(1,1), rot(2,1), 0).normalized();
      m_up[3] = 1;
      // we need to check the handedness
      Vec v = cross(m_up,m_norm);
      // get the component with biggest abs. value of v
      int idx = 0;
      if (abs(v[1]) > abs(v[idx])) idx = 1;
      if (abs(v[2]) > abs(v[idx])) idx = 2;
      // check if the signs are same
      if (v[idx] * rot(idx,0) < 0) {
        // not the same sign -- switch directions to make right handed cs
        m_norm *= -1; m_norm[3] = 1;
        m_up *= -1; m_up[3] = 1;
      }
    }

    void Camera::setWorldFrame(const Mat &m){
      //setWorldTransformation(m.inv() * getCSTransformationMatrix().inv() );
      setWorldTransformation(m.inv() * getInvCSTransformationMatrix() );
    }

    void Camera::setWorldTransformation(const Mat &m){
      FixedMatrix<float,3,3> R = m.part<0,0,3,3>();
      setRotation(R.transp());

      m_pos[0] = m(3,0);
      m_pos[1] = m(3,1);
      m_pos[2] = m(3,2);
    }


    void Camera::setTransformation(const Mat &m){
      FixedMatrix<float,3,3> R = m.part<0,0,3,3>();
      setRotation(R);

      // since the transformation matrix T is [ R | -Rt ], we have to undo -Rt by -R^1t
      // and since R is orthogonal, R^-1 is equal to R.transp()
      FixedColVector<float,3> t = m.part<3,0,1,3>();
      t = -R.transp()*t;
      m_pos[0] = t[0];
      m_pos[1] = t[1];
      m_pos[2] = t[2];
    }


    Camera Camera::createFromProjectionMatrix(const FixedMatrix<icl32f,4,3> &Q,
                                                  float focalLength) {
      FixedMatrix<float,3,3> M = Q.part<0,0,3,3>();
      FixedMatrix<float,1,3> c4 = Q.col(3);

      FixedMatrix<float,3,3> K; // intrinsic parameters
      FixedMatrix<float,3,3> R; // extrinsic (rotation matrix)
      FixedMatrix<float,1,3> T; // extrinsic (tranlation vector)

      M.decompose_RQ(K,R);
      K = K/K(2,2); // normalize K
      T = -M.inv() * c4;
      Camera cam;
      cam.setPosition(Vec(T[0],T[1],T[2],1));
      cam.setRotation(R);
      cam.setFocalLength(focalLength);
      cam.setSamplingResolution(K(0,0)/focalLength,K(1,1)/focalLength);
      cam.setPrincipalPointOffset(Point32f(K(2,0),K(2,1)));
      cam.setSkew(K(1,0));
      return cam;
    }

    Camera Camera::calibrate_extrinsic(const std::vector<Vec> &Xws, const std::vector<utils::Point32f> &xis,
                                       const Camera &intrinsicCamValue, const RenderParams &renderParams,
                                       bool performLMAbasedOptimiziation)
      throw (NotEnoughDataPointsException,SingularMatrixException) {
      return calibrate_extrinsic(Xws, xis, intrinsicCamValue.getProjectionMatrix(), renderParams, performLMAbasedOptimiziation);
    }


    Camera Camera::calibrate_extrinsic(const std::vector<Vec> &Xws, const std::vector<utils::Point32f> &xis,
                                       const Mat &camIntrinsicProjectionMatrix, const RenderParams &renderParams,
                                       bool performLMAbasedOptimiziation)
      throw (NotEnoughDataPointsException,SingularMatrixException) {
      const Mat &k = camIntrinsicProjectionMatrix;
      return calibrate_extrinsic(Xws, xis, k(0,0), k(1,1), k(1,0), k(2,0), k(2,1), renderParams, performLMAbasedOptimiziation);
    }

    template<class T, unsigned int N, unsigned int M>
    inline T norm3_local(const FixedMatrix<T, N, M> &m){
      return ::sqrt(sqr(m[0]) + sqr(m[1]) + sqr(m[2]) );
    }

    Camera Camera::calibrate_extrinsic(std::vector<Vec> Xws, std::vector<utils::Point32f> xis,
                                       float fx, float fy, float s, float px ,float py,
                                       const RenderParams &renderParams, bool performLMAbasedOptimiziation)
    throw (NotEnoughDataPointsException,SingularMatrixException) {
      checkAndFixPoints(Xws,xis);

      int n = (int)Xws.size();

      DynMatrix<double> M(12,2*n);
      for(int i=0;i<n;++i){
        double x = Xws[i].x, y = Xws[i].y, z = Xws[i].z, u = xis[i].x, v = xis[i].y;
        double du = px-u, dv = py-v;
        const double F[12] = {x,y,z,1,x,y,z,1,x,y,z,1};
        const double A[12] = { fx,fx,fx,fx, s, s, s, s, du,du,du,du };
        const double B[12] = { 0, 0, 0, 0, fy,fy,fy,fy, dv,dv,dv,dv };
        double *a = &M(0,2*i), *b = &M(0,2*i+1);
        for(int j=0;j<12;++j){
          a[j] = A[j] * F[j];
          b[j] = B[j] * F[j];
        }
      }

      DynMatrix<double> _U,_s,V;
      M.svd(_U,_s,V);

      typedef math::FixedMatrix<double,4,4> DMat;
      typedef math::FixedMatrix<double,3,3> DMat3;
      typedef math::FixedMatrix<double,1,3> DVec3;

      DMat RT = Mat::id();
      std::copy(V.col_begin(11), V.col_end(11), RT.begin());

      DMat3 R = RT.part<0,0,3,3>();
      DVec3 t = RT.part<3,0,1,3>();

      DMat3 r,q;
      R.decompose_RQ(r,q);
      DMat3 Ri = q.transp();

      double norm = 3./(r(0,0) + r(1,1) + r(2,2));
      t = -Ri * ( t * norm );

      Camera cam(Vec(t[0],    t[1],    t[2],    1),
                 Vec(Ri(2,0), Ri(2,1), Ri(2,2), 1),
                 Vec(Ri(1,0), Ri(1,1), Ri(1,2), 1),
                 1, Point32f(px,py), fx, fy, s, renderParams);
      if(performLMAbasedOptimiziation){
        return optimize_camera_calibration_lma(Xws, xis, cam);
      }else{
        return cam;
      }
    }
      /** @} @{ @name utils::projection functions */

    namespace{
      struct LMAOptUtil{
        typedef math::LevenbergMarquardtFitter<double> LMA;
        typedef LMA::Vector vec;
        typedef LMA::Params params;
        typedef LMA::Matrix mat;

        Camera cam;
        Mat P,T;
        LMAOptUtil(const Camera &cam):cam(cam){
          P = cam.getProjectionMatrix();
          T = cam.getCSTransformationMatrix();
        }

        static Mat m(const params &p){
          return create_hom_4x4<float>(p[0],p[1],p[2],p[3]*1000,p[4]*1000,p[5]*1000);
        }

        Camera fixCam(const params &par){
          Mat3 R = T.part<0,0,3,3>();
          Vec3 pOrig =  cam.getPosition().resize<1,3>(); //T.part<3,0,1,3>();
          Mat d = m(par);
          Mat3 dR = d.part<0,0,3,3>();
          Vec3 dt = d.part<3,0,1,3>();

          Mat3 dRR = dR * R;

          Camera c = cam;
          c.setUp(Vec(dRR(0,1), dRR(1,1), dRR(2,1), 1));
          c.setNorm(Vec(dRR(0,2), dRR(1,2), dRR(2,2), 1));
          c.setPosition( (pOrig - dRR.transp() * dt).resize<1,4>(1));

          return c;
        }
        Mat getFixedCSMatrix(const params &p){
          return  m(p) * T;
        }

        vec f(const params &p, const vec &x) const{
          Mat Q = P * m(p) * T;
          Vec y = Q * Vec(x[0],x[1],x[2],1);
          vec r(2);
          r[0] = y[0]/y[3];
          r[1] = y[1]/y[3];
          return r;
        }

        static Camera optimize(const Camera &init,
                               const std::vector<Vec> &Xws,
                               const std::vector<Point32f> &xis){

          DEBUG_LOG("staring optimization at cam position " << init.getPosition().transp());
          LMAOptUtil u(init);
          LMA lma(function(u,&LMAOptUtil::f), 2, std::vector<LMA::Jacobian>(),
                  0.1, 1000);
          //lma.setDebugCallback();

          int n = (int)Xws.size();
          mat xs(3,n), ys(2,n);
          for(int i=0;i<n;++i){
            xs(0,i) = Xws[i].x;
            xs(1,i) = Xws[i].y;
            xs(2,i) = Xws[i].z;

            ys(0,i) = xis[i].x;
            ys(1,i) = xis[i].y;
          }

          LMA::Result r = lma.fit(xs, ys, params(6,0.0));
          std::cout << "LMA Optimization Results: " << std::endl << r << std::endl;

          return u.fixCam(r.params);
        }
      };
    }
    Camera Camera::optimize_camera_calibration_lma(const std::vector<Vec> &Xws,
                                                   const std::vector<utils::Point32f> xis,
                                                   const Camera &init){
      return LMAOptUtil::optimize(init, Xws, xis);
    }

    Camera Camera::calibrate_pinv(std::vector<Vec> Xws,
                                  std::vector<Point32f> xis,
                                  float focalLength,
                                  bool performLMAbasedOptimiziation)
      throw (NotEnoughDataPointsException,SingularMatrixException) {
      // TODO: normalize points
      checkAndFixPoints(Xws,xis);

      int N = (int)Xws.size();

      DynMatrix<double> U(1,2*N);
      for(int i=0;i<N;++i){
        U[2*i] = xis[i].x;
        U[2*i+1] = xis[i].y;
      }

      DynMatrix<double> B(11,2*N);
      for(int i=0;i<N;++i){
        float x=Xws[i][0], y=Xws[i][1],z=Xws[i][2], u=-xis[i].x,v=-xis[i].y;
        float r1[11] = {x,y,z,1,0,0,0,0,u*x,u*y,u*z};
        float r2[11] = {0,0,0,0,x,y,z,1,v*x,v*y,v*z};

        std::copy(r1,r1+11,B.row_begin(2*i));
        std::copy(r2,r2+11,B.row_begin(2*i+1));
      }

      DynMatrix<double> Cv = B.pinv(true) * U;
      FixedMatrix<float,4,3> Q(Cv[0],Cv[1],Cv[2],Cv[3],
                               Cv[4],Cv[5],Cv[6],Cv[7],
                               Cv[8],Cv[9],Cv[10],1);
      Camera cam = Camera::createFromProjectionMatrix(Q, focalLength);
      if(performLMAbasedOptimiziation){
        return optimize_camera_calibration_lma(Xws, xis, cam);
      }else{
        return cam;
      }
    }

    Camera Camera::calibrate(std::vector<Vec> Xws,
                             std::vector<Point32f> xis,
                             float focalLength,
                             bool performLMAbasedOptimiziation)
      throw (NotEnoughDataPointsException,SingularMatrixException) {

      //  #ifndef ICL_HAVE_MKL
      // 	return calibrate_pinv(Xws,xis,focalLength);
      //#else
      // TODO: normalize points
      // TODO: check whether we have svd (IPP) available
      checkAndFixPoints(Xws,xis);

      unsigned int n = Xws.size();
      DynMatrix<icl32f> A(12,2*n);

      for (unsigned int k=0; k<n; ++k) {
        int i = 2*k;
        float x = xis[k].x; float y = xis[k].y;
        float X = Xws[k][0]; float Y = Xws[k][1]; float Z = Xws[k][2]; float W = Xws[k][3];
        A(0,i) = 0; A(1,i) = 0; A(2,i) = 0; A(3,i) = 0;
        A(4,i) = -X; A(5,i) = -Y; A(6,i) = -Z; A(7,i) = -W;
        A(8,i) = y*X; A(9,i) = y*Y; A(10,i) = y*Z; A(11,i) = y*W;
        i = 2*k+1;
        A(0,i) = X; A(1,i) = Y; A(2,i) = Z; A(3,i) = W;
        A(4,i) = 0; A(5,i) = 0; A(6,i) = 0; A(7,i) = 0;
        A(8,i) = -x*X; A(9,i) = -x*Y; A(10,i) = -x*Z; A(11,i) = -x*W;
      }

      DynMatrix<icl32f> U,s,V;
      svd_dyn(A,U,s,V);

      FixedMatrix<float,4,3> Q;
      for (int i=0; i<4; i++) for (int j=0; j<3; j++) {
        Q(i,j) = V(11,j*4+i);
      }

      Camera cam = Camera::createFromProjectionMatrix(Q, focalLength);
      if(performLMAbasedOptimiziation){
        return optimize_camera_calibration_lma(Xws, xis, cam);
      }else{
        return cam;
      }
    }

    void Camera::checkAndFixPoints(std::vector<Vec> &Xws, std::vector<Point32f> &xis) throw (NotEnoughDataPointsException) {
      if(Xws.size() > xis.size()){
        ERROR_LOG("got more world points than image points (erasing additional world points)");
        Xws.resize(xis.size());
      } else if(Xws.size() < xis.size()){
        ERROR_LOG("got less world points than image points (erasing additional image points)");
        xis.resize(Xws.size());
      }
      if(Xws.size() < 6){
        throw NotEnoughDataPointsException();
      }
      for(unsigned int i=0;i<Xws.size();++i){
        Xws[i][3]=1;
      }
    }

    void Camera::load_camera_from_stream(std::istream &is, const std::string &prefix,
                                           Camera &cam){
      cam = Camera(); // load default values
      utils::XMLDocument *doc = new utils::XMLDocument;
      doc->loadNext(is);

      ConfigFile f(doc);
      f.setPrefix(prefix);

      #define LOAD_FROM_STREAM(KEY,FNAME) \
      if (f.contains("camera." #KEY)) cam.set##FNAME(f["camera." #KEY]); \
      else WARNING_LOG("No " #KEY " found in configuration (using default: '" << cam.get##FNAME() << "')");
      LOAD_FROM_STREAM(name,Name);
      LOAD_FROM_STREAM(position,Position);
      LOAD_FROM_STREAM(norm,Norm);
      LOAD_FROM_STREAM(up,Up);
      LOAD_FROM_STREAM(f,FocalLength);
      LOAD_FROM_STREAM(principal-point-offset,PrincipalPointOffset);
      LOAD_FROM_STREAM(sampling-resolution-x,SamplingResolutionX);
      LOAD_FROM_STREAM(sampling-resolution-y,SamplingResolutionY);
      LOAD_FROM_STREAM(skew,Skew);
      #undef LOAD_FROM_STREAM

      f.setPrefix(prefix+"camera.render-params.");
      #define LOAD_FROM_STREAM(KEY,ATTR) \
      if (f.contains(#KEY)) cam.getRenderParams().ATTR = f[#KEY]; \
      else WARNING_LOG("No " #KEY " found in configuration (using default: '" << cam.getRenderParams().ATTR << "')");
      LOAD_FROM_STREAM(chip-size, chipSize);
      LOAD_FROM_STREAM(clip-z-near, clipZNear);
      LOAD_FROM_STREAM(clip-z-far, clipZFar);
      LOAD_FROM_STREAM(viewport, viewport);
      LOAD_FROM_STREAM(viewport-z-min, viewportZMin);
      LOAD_FROM_STREAM(viewport-z-max, viewportZMax);
      #undef LOAD_FROM_STREAM
    }

    std::string Camera::toString() const {
      std::ostringstream os;
      os << "Position: " << getPosition().transp() << std::endl;
      os << "Norm: " << getNorm().transp() << std::endl;
      os << "Up: " << getUp().transp() << std::endl;
      os << "Focal Length: " << getFocalLength() << std::endl;
      os << "Principal Point Offset: " << getPrincipalPointOffset() << std::endl;
      os << "Sampling Resolution: " << getSamplingResolutionX() << ", "
                                    << getSamplingResolutionY() << std::endl;
      os << "Skew: " << getSkew() << std::endl;
      return os.str();
    }



    /// ostream operator
    std::ostream &operator<<(std::ostream &os, const Camera &cam){
      ConfigFile f;
      f.setPrefix("config.camera.");

      #define WRITE_TO_STREAM(KEY,FNAME) \
      f[#KEY] = cam.get##FNAME();
      WRITE_TO_STREAM(name,Name);
      WRITE_TO_STREAM(position,Position);
      WRITE_TO_STREAM(norm,Norm);
      WRITE_TO_STREAM(up,Up);
      WRITE_TO_STREAM(f,FocalLength);
      WRITE_TO_STREAM(principal-point-offset,PrincipalPointOffset);
      WRITE_TO_STREAM(sampling-resolution-x,SamplingResolutionX);
      WRITE_TO_STREAM(sampling-resolution-y,SamplingResolutionY);
      WRITE_TO_STREAM(skew,Skew);
      #undef WRITE_TO_STREAM

      f["render-params.chip-size"] = cam.getRenderParams().chipSize;
      f["render-params.clip-z-near"] = cam.getRenderParams().clipZNear;
      f["render-params.clip-z-far"] = cam.getRenderParams().clipZFar;
      f["render-params.viewport"] = cam.getRenderParams().viewport;
      f["render-params.viewport-z-min"] = cam.getRenderParams().viewportZMin;
      f["render-params.viewport-z-max"] = cam.getRenderParams().viewportZMax;

      std::ostringstream str;
      const Vec &p = cam.getPosition(), &n = cam.getNorm(), &u = cam.getUp();
      float fl = cam.getFocalLength(), px = cam.getPrincipalPointOffset().x,
            py = cam.getPrincipalPointOffset().y, s = cam.getSkew(),
            sx = cam.getSamplingResolutionX(), sy = cam.getSamplingResolutionY();

      const Camera::RenderParams &r = cam.getRenderParams();
      str << std::endl
          << "Camera(Vec(" << p[0] << "," << p[1] << "," << p[2] << ",1)," << std::endl
          << "       Vec(" << n[0] << "," << n[1] << "," << n[2] << ",1)," << std::endl
          << "       Vec(" << u[0] << "," << u[1] << "," << u[2] << ",1)," << std::endl
          << "      " << fl << ", Point32f(" << px << "," << py << ")," << sx << "," << sy << "," << std::endl
          << "      " << s << ", Camera::RenderParams(Size(" << r.chipSize.width <<  "," << r.chipSize.height << ")," << std::endl
          << "                         " << r.clipZNear << ","  << r.clipZFar << ", " << std::endl
          << "                         Rect(" << r.viewport.x << "," << r.viewport.y << ","
          << r.viewport.width << "," << r.viewport.height << ")," << std::endl
          << "                         " << r.viewportZMin << "," << r.viewportZMax << "));" << std::endl;

      f["for-direct-embedding-in-C++"] = str.str();

      return os << f;
    }

    /// istream operator parses a camera from an XML-string
    std::istream &operator>>(std::istream &is, Camera &cam) throw (ParseException) {
      cam = Camera(is,"config.");
      return is;
    }

    Camera::Camera(const std::string &filename, const std::string &prefix) throw (ParseException){
      if(!File(filename).exists()) throw ParseException("Camera(filename) given file '" + filename + "' does not exist!");
      std::ifstream is(filename.c_str());
      load_camera_from_stream(is,prefix,*this);
    }

    Camera::Camera(std::istream &is, const std::string &prefix) throw (ParseException){
      load_camera_from_stream(is,prefix,*this);
    }

    /// utility function called from all getViewRay methods
    inline ViewRay create_view_ray(const FixedMatrix<icl32f,3,4> &Qi, float x, float y, const Vec &p){
      Vec dir = Qi*FixedColVector<icl32f,3>(x, y, 1);
      return ViewRay(p, dir, true);
    }

    ViewRay Camera::getViewRay(const Point32f &pixel) const {
      return create_view_ray(getInvQMatrix(),pixel.x,pixel.y,m_pos);
    }


    std::vector<ViewRay> Camera::getViewRays(const std::vector<Point32f> &pixels) const{
      std::vector<ViewRay> vs(pixels.size());
      FixedMatrix<icl32f,3,4> Qi = getInvQMatrix();

      for(unsigned int i=0;i<pixels.size();++i){
        vs[i] = create_view_ray(Qi,pixels[i].x,pixels[i].y,m_pos);
      }
      return vs;
    }

    Array2D<ViewRay> Camera::getAllViewRays() const{
      Array2D<ViewRay> m(getRenderParams().chipSize);
      FixedMatrix<icl32f,3,4> Qi = getInvQMatrix();

      for(int y=0;y<m.getHeight();++y){
        for(int x=0;x<m.getWidth();++x){
          m(x,y) = create_view_ray(Qi,x,y,m_pos);
        }
      }
      return m;
    }


    ViewRay Camera::getViewRay(const Vec &Xw) const{
      return ViewRay(m_pos, Xw-m_pos);
    }

    Vec Camera::getIntersection(const ViewRay &v, const PlaneEquation &plane) throw (ICLException) {
      float denom = sprod_3(v.direction, plane.normal);
      if (fabs(denom) < 1e-6) throw ICLException("no intersection -> plane normal is perdendicular to view-ray direction");
      float lambda = -sprod_3(v.offset-plane.offset,plane.normal) / denom;
      return v(lambda);
    }

    Vec Camera::estimate3DPosition(const Point32f &pixel, const PlaneEquation &plane) const throw (ICLException) {
      return getIntersection(getViewRay(pixel),plane);
    }


    /*
        static Point32f to_normalized_viewport(const Point32f &p, const Camera &cam){
        // {{{ open
        Vec uv = cam.getViewportMatrixGL().inv() * Vec(p.x,p.y,0,1);
        return Point32f(-uv[0],-uv[1]);
        }
        // }}
        }
    */

    static Vec estimate_3D_internal(const std::vector<Camera*> cams,
                                    const std::vector<Point32f> &ps) throw (ICLException){
      // {{{ open
      int K = (int)cams.size();
      ICLASSERT_THROW(K > 1,ICLException("Camera::estimate_3D_internal: 3D point estimation needs at least 2 views"));

      DynMatrix<float> A(3,2*K), B(1,2*K);

      for(int i=0;i<K;++i){
        const float &u = ps[i].x;
        const float &v = ps[i].y;
        FixedMatrix<float,4,3> Q = cams[i]->getQMatrix();
        FixedRowVector<float,3> x = Q.part<0,0,3,1>();
        FixedRowVector<float,3> y = Q.part<0,1,3,1>();
        FixedRowVector<float,3> z = Q.part<0,2,3,1>();
        FixedColVector<float,3> t = Q.part<3,0,1,3>();

        FixedRowVector<float,3> a1 = z*u - x;
        FixedRowVector<float,3> a2 = z*v - y;

        std::copy(a1.begin(),a1.end(),A.row_begin(2*i));
        std::copy(a2.begin(),a2.end(),A.row_begin(2*i+1));

        B[2*i]   = t[0]-u*t[2];
        B[2*i+1] = t[1]-v*t[2];
      }

      try{
        DynMatrix<float> pEst = A.pinv() * B;
        return Vec(pEst[0],pEst[1],pEst[2],1);
      }catch(const ICLException &ex){
        throw ICLException(str("Camera::estimate_3D_internal: unable to solve linear equation (")+ex.what()+")");
      }

      return Vec(0,0,0,1);
    }

    // }}}



    Vec Camera::estimate_3D(const std::vector<Camera*> cams,
                            const std::vector<Point32f> &UVs,
                            bool removeInvalidPoints) throw (ICLException){
      // {{{ open
      ICLASSERT_THROW(cams.size() == UVs.size(),ICLException("Camera::estimate_3D: given camera count and point count differs"));
      if(removeInvalidPoints){
        std::vector<Camera*> camsOk;
        std::vector<Point32f> uvsOk;
        for(unsigned int i=0;i<cams.size();++i){
          const Rect32f &vp = cams[i]->getRenderParams().viewport;
          if(vp.contains(UVs[i].x,UVs[i].y)){
            camsOk.push_back(cams[i]);
            uvsOk.push_back(UVs[i]);
          }
        }
        return estimate_3D_internal(camsOk,uvsOk);
      }else{
        return estimate_3D_internal(cams,UVs);
      }
    }
    // }}}

    Mat Camera::estimatePose(const std::vector<Vec> &objectCoords,
      const std::vector<utils::Point32f> &imageCoords,
                             bool performLMAOptimization){
      Camera cam = calibrate_extrinsic(objectCoords, imageCoords, *this, RenderParams(), performLMAOptimization);
      return cam.getInvCSTransformationMatrix();
    }


    /// returns the tensor obtained by contraction of v with epsilon tensor.
    static inline FixedMatrix<icl32f,3,3> contr_eps(const FixedMatrix<icl32f,1,3> &v){
      return FixedMatrix<icl32f,3,3>(0, v[2], -v[1],
                                     -v[2], 0, v[0],
                                     v[1], -v[0], 0);
    }

    template<class ForwardIterator>
    static bool all_negative(ForwardIterator begin, ForwardIterator end){
      while(begin != end){
        if(*begin++ > 0) return false;
      }
      return true;
    }
    template<class ForwardIterator>
    static bool all_positive(ForwardIterator begin, ForwardIterator end){
      while(begin != end){
        if(*begin++ < 0) return false;
      }
      return true;
    }

    /// multiview 3D point estimation using svd-based linear optimization
    Vec Camera::estimate_3D_svd(const std::vector<Camera*> cams,
                                  const std::vector<Point32f> &UVs){
      int K = (int)cams.size();
      ICLASSERT_THROW(K>1,ICLException("estimate_3D_svd needs at least 2 views"));
      ICLASSERT_THROW((int)UVs.size() == (int)cams.size(),
                      ICLException("estimate_3D_svd got more or less cameras than points"));

      std::vector<FixedMatrix<icl32f,4,3> > P(K);
      std::vector<FixedMatrix<icl32f,1,2> > u(K);

      for(int i=0;i<K;++i) {
        P[i] = cams[i]->getQMatrix();
        u[i] = FixedMatrix<icl32f,1,2>(UVs[i].x,UVs[i].y);
      }

  #if 0
      // this does not work for some reason!, however it works without!
      for(int k=0;k<K;++k){
        Mat33 H( 2.0/imageSizes[k].width,  0, -1,
                 0, 2.0/imageSizes[k].height, -1,
                 0, 0,                         1);
        P[k] = H*P[k];
        u[k] = Mat22(H.part<0,0,2,2>()) * u[k] + Vec2(H.part<2,2,1,2>());
      }
  #endif

      DynMatrix<float> A(4,K*3);
      for(int k=0;k<K;++k){
        FixedMatrix<icl32f,4,3> m = contr_eps(FixedMatrix<icl32f,1,3>(u[k][0],u[k][1],1))*P[k];
        for(unsigned int y=0;y<3;++y){
          for(unsigned int x=0;x<4;++x){
            A(x,k*3+y) = m(x,y);
          }
        }
      }

      DynMatrix<float> _U,_s,V;
      svd_dyn(A,_U,_s,V);

      // search eigenvector to lowest eigenvalue
      DynMatrix<float> X(1,V.rows());
      X = V.col(V.cols()-1);

      // create matrix with all 3rd rows of the camera matrices
      DynMatrix<float> M(4,K);
      for(int k=0;k<K;++k){
        std::copy(P[k].row_begin(2),P[k].row_end(2),M.row_begin(k));
      }

      DynMatrix<float> s = M*X;
      if(all_negative(s.begin(),s.end())){
        return homogenize(-Vec(X.begin()));
      }else if(all_positive(s.begin(),s.end())){
        return homogenize(Vec(X.begin()));
      }else{
        throw ICLException("estimate_3D_svd: Inconsistent orientation of point match");
        return Vec(0,0,0,1);
      }
    }

    void Camera::setResolution(const Size &newScreenSize){
      getRenderParams().chipSize = newScreenSize;
      getRenderParams().viewport = Rect(Point::null,newScreenSize);
      setPrincipalPointOffset(newScreenSize.width/2,newScreenSize.height/2);
    }

    void Camera::setResolution(const Size &newScreenSize, const Point &newPrincipalPointOffset){
      getRenderParams().chipSize = newScreenSize;
      getRenderParams().viewport = Rect(Point::null,newScreenSize);
      setPrincipalPointOffset(newPrincipalPointOffset);
    }

    Camera Camera::create_camera_from_calibration_or_udist_file(const std::string &filename) throw (utils::ICLException){
      SmartPtr<io::ImageUndistortion> udist;
      try{
        udist = new io::ImageUndistortion(filename);
      }catch(...){
        return Camera(filename);
      }
      if(udist->getModel() != "MatlabModel5Params"){
        throw ICLException("unable to parse udist file: wrong Model!");
      }

      const std::vector<double> &params = udist->getParams();
      Camera cam;

      ConfigFile f(filename);
      cam.setResolution(Size(f["config.size.width"], f["config.size.height"]));
      cam.setFocalLength(1);
      cam.setSamplingResolution(params[0], params[1]);
      cam.setPrincipalPointOffset(Point32f(params[2], params[3]));
      return cam;
    }

  } // namespace geom
}
