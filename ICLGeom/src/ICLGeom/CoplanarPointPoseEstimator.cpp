/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CoplanarPointPoseEstimator.cpp     **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLMath/Homography2D.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/ProgArg.h>
#include <ICLMath/SimplexOptimizer.h>
#include <ICLMath/PolynomialSolver.h>

#include <algorithm>
#include <iostream>

#include <iostream>
#include <fstream>
#include <cctype>
#include <cmath>
#include <cfloat>

using namespace icl::utils;
using namespace icl::math;

using std::max;
using std::min;

namespace icl{
  namespace geom{
    typedef DynMatrix<float> DMat;

    static inline void assign_row(float* p, float a, float b, float c, float d,
                                  float e, float f, float g, float h, float i){
      p[0] = a; p[1] = b; p[2] = c; p[3] = d; p[4] = e;
      p[5] = f; p[6] = g; p[7] = h; p[8] = i;
    }

    static inline FixedMatrix<float,1,3> cross3(const FixedMatrix<float,1,3> &v1,
                                                const FixedMatrix<float,1,3> &v2){
      return FixedMatrix<float,1,3>(v1[1]*v2[2]-v1[2]*v2[1],
                                    v1[2]*v2[0]-v1[0]*v2[2],
                                    v1[0]*v2[1]-v1[1]*v2[0]);
    }

    struct CoplanarPointPoseEstimator::Data{
      DMat A,U,s,V;

      FixedMatrix<float,3,3> H,R;
      FixedMatrix<float,1,3> C;
      FixedMatrix<float,4,4> T;

      CoplanarPointPoseEstimator::ReferenceFrame referenceFrame;
      CoplanarPointPoseEstimator::PoseEstimationAlgorithm algorithm;

      float samplingInterval;
      int samplingSteps;
      int samplingSubSteps;
      float decreaseFactor;
      float positionMultiplier;
      bool timeMonitoring;
      bool poseCorrection;

    };

    static const std::string &get_all_algorithms(){
      static const std::string s = ("HomographyBasedOnly,SamplingCoarse,SamplingMedium,SamplingFine,"
                                    "SamplingCustom,SimplexSampling");
      return s;
    }

    static std::string algorithm_to_string(CoplanarPointPoseEstimator::PoseEstimationAlgorithm a){
      static std::vector<std::string> as = tok(get_all_algorithms(),",");
      if((int)a < 0 || (int)a>=(int)as.size()){
        throw ICLException("CoplanarPointPoseEstimator: wrong PoseEstimationAlgorithm value");
      }
      return as[(int)a];
    }

    CoplanarPointPoseEstimator::PoseEstimationAlgorithm string_to_algorithm(const std::string &value){
      static std::vector<std::string> as = tok(get_all_algorithms(),",");
      std::vector<std::string>::const_iterator it = std::find(as.begin(),as.end(),value);
      if(it == as.end()) throw ICLException("CoplanarPointPoseEstimator: wrong string-value for PoseEstimationAlgorithm");
      return (CoplanarPointPoseEstimator::PoseEstimationAlgorithm)(int)(it - as.begin());
    }


    CoplanarPointPoseEstimator::CoplanarPointPoseEstimator(ReferenceFrame returnedPoseReferenceFrame,
                                                           PoseEstimationAlgorithm a):
      data(new Data){

      data->algorithm = a;
      data->samplingInterval = 0.6;
      data->samplingSteps = 10;
      data->samplingSubSteps = 1;
      data->decreaseFactor = 0.6;
      data->positionMultiplier = 50;
      data->timeMonitoring = false;
      data->poseCorrection = false;
      data->referenceFrame = returnedPoseReferenceFrame;

      addProperty("algorithm","menu",get_all_algorithms(),algorithm_to_string(a),0,
                  "Specifies the used algorithm:\n"
                  "HomographyBasedOnly: straight forward least-square based\n"
                  "SamplingCoarse:  Use exhaustive sampling around the result\n"
                  "                 of the linear result (using parameters for\n"
                  "                 coarse sampling)\n"
                  "SamplingMedium:  As above, but finer sampling (more\n"
                  "                 accurate, but slower\n"
                  "SamplingFine:    As abouve, but even finer (again more\n"
                  "                 accurate and slower\n"
                  "SamplingCustom:  Exhaustive sampling with custom parameters\n"
                  "SimplexSampling: Use the Simplex-Search algorithm for\n"
                  "                 optimization (usually, this provides the best\n"
                  "                 result and is still much faster than exhaustive\n"
                  "                 sampling");
      addProperty("sampling interval","float","[-3.14,3.14]",data->samplingInterval,0,
                  "(only used if the 'algorithm' property is set to 'SamplingCustom'\n"
                  "Defines the angle search range for exhaustive search");
      addProperty("sampling steps","int","[1,100000]:1",data->samplingSteps,0,
                  "(only used if the 'algorithm' property is set to 'SamplingCustom'\n"
                  "Defines the number of coarse steps for exhaustive sampling.");
      addProperty("sampling substeps","int","[1,100]",data->samplingSubSteps,0,
                  "(only used if the 'algorithm' property is set to 'SamplingCustom'\n"
                  "Defines the number of fine steps for exhaustive sampling.");
      addProperty("decrease factor","float","[0,1]",data->decreaseFactor,0,
                  "(only used if the 'algorithm' property is set to 'SamplingCustom'\n"
                  "Defines the factor, which is used to reduce the step-width after\n"
                  "every coarse step");
      addProperty("position multiplier","float","[1,5000]",data->positionMultiplier,0,
                  "(only used if the 'algorithm' property is set to 'SamplingCustom'\n"
                  "Defines the ratio between angle and position values");
      addProperty("time monitoring","flag","",data->timeMonitoring,0,
                  "If set to true, benchmarking is enabled");
      addProperty("pose correction","flag","",data->poseCorrection,0,
                  "If set to true, the pose is corrected using robust pose estimation algorithm");


      registerCallback(function(this,&CoplanarPointPoseEstimator::propertyChangedCallback));
    }

    void CoplanarPointPoseEstimator::propertyChangedCallback(const Property &p){
      if(p.name == "algorithm") data->algorithm = string_to_algorithm(p.value);
      else if(p.name == "sampling interval") data->samplingInterval = parse<float>(p.value);
      else if(p.name == "sampling steps") data->samplingSteps = parse<int>(p.value);
      else if(p.name == "sampling substeps") data->samplingSubSteps = parse<int>(p.value);
      else if(p.name == "decrease factor") data->decreaseFactor = parse<float>(p.value);
      else if(p.name == "position multiplier") data->positionMultiplier = parse<float>(p.value);
      else if(p.name == "time monitoring") data->timeMonitoring = parse<bool>(p.value);
      else if(p.name == "pose correction") data->poseCorrection = parse<bool>(p.value);
      else {
        WARNING_LOG("invalid property name: " << p.name);
      }
    }


    CoplanarPointPoseEstimator::~CoplanarPointPoseEstimator(){
      delete data;
    }

    CoplanarPointPoseEstimator::CoplanarPointPoseEstimator(const CoplanarPointPoseEstimator &other):
      data(new Data){
      *this = other;
    }



    CoplanarPointPoseEstimator &CoplanarPointPoseEstimator::operator=(const CoplanarPointPoseEstimator &other){
      *data = *other.data;
      return *this;
    }


    CoplanarPointPoseEstimator::ReferenceFrame CoplanarPointPoseEstimator::getReferenceFrame() const{
      return data->referenceFrame;;
    }

    void CoplanarPointPoseEstimator::setReferenceFrame(CoplanarPointPoseEstimator::ReferenceFrame f){
      data->referenceFrame = f;
    }

  #if 0
    static float compute_error(const Mat &P, const Mat &T, const Point32f *M, const Point32f *I, int n){
      float error2 = 0;
      for(int i=0;i<n;++i){
        Vec tmp = homogenize( P * T * Vec(M[i].x,M[i].y,0,1) );
        error2 += Point32f(tmp[0],tmp[1]).distanceTo(I[i]);
      }
      return error2;
    }

    static float compute_error(const Mat &P, const FixedMatrix<float,1,6> &p, const Point32f *M, const Point32f *I, int n){
      const Mat T = create_hom_4x4<float>(p[0],p[1],p[2],p[3],p[4],p[5]);
      float error = 0;
      for(int i=0;i<n;++i){
        Vec tmp = homogenize( P * T * Vec(M[i].x,M[i].y,0,1) );
        error += Point32f(tmp[0],tmp[1]).distanceTo(I[i]);
      }
      return error;
    }
  #endif

    static float compute_error_opt(const Mat &P,
                                   const FixedMatrix<float,1,3> &r,
                                   const FixedMatrix<float,1,3> &t,
                                   const Point32f *_M,
                                   const Point32f *_I,
                                   int n){
      const float &A = P(0,0), &B = P(1,0), &C = P(2,0), &D = P(1,1), &E = P(2,1);
      const float &rx = r[0], &ry = r[1], &rz = r[2], &tx = t[0], &ty=t[1], &tz = t[2];
      const float cx = cos(rx), cy = cos(-ry), cz = cos(-rz), sx = sin(rx), sy = sin(-ry), sz = sin(-rz);
      const float Rx0 = cy*cz-sx*sy*sz, Rx1 = cy*sz+cz*sx*sy, Rx2 = -sy*cx;
      const float Ry0 = -sz*cx, Ry1 = cz*cx, Ry2 = sx;
      const float F = Rx0*A + Rx1*B + Rx2*C;
      const float G = Ry0*A + Ry1*B + Ry2*C;

      const float J = Rx1*D + Rx2*E;
      const float K = Ry1*D + Ry2*E;
      const float &N = Rx2;
      const float &O = Ry2;
      const float I = tx*A + ty*B + tz*C;
      const float M = ty*D + tz*E;
      const float &Q = tz;

      float error = 0;
      for(int i=0;i<n;++i){
        const float &Mix = _M[i].x, Miy = _M[i].y, Iix = _I[i].x, Iiy = _I[i].y;
        const float R = F*Mix + G*Miy + I;
        const float S = J*Mix + K*Miy + M;
        const float T_inv = 1.0/(N*Mix + O*Miy + Q);
        error += sqrt( sqr(R*T_inv - Iix) + sqr(S*T_inv - Iiy) );
      }
      return error;
    }

    typedef FixedColVector<float,6> Pose6D;
    typedef FunctionImpl<float,const Pose6D&> ErrorFunction;

    struct SimplexErrorFunction{
      const Mat &P;
      const Point32f *M,*I;
      int n;
      inline SimplexErrorFunction(const Mat &P, const Point32f *M, const Point32f *I, int n):
        P(P),M(M),I(I),n(n){}
      float f(const Pose6D &rt) const {
        return compute_error_opt(P,rt.part<0,0,1,3>(),rt.part<0,3,1,3>(),M,I,n);
      }
    };


#if 0
    COARSE data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                    1.0, 50, 10, 1, 0.6, data->timeMonitoring);

    MEDIUM data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                    1.2, 60, 20, 1, 0.65, data->timeMonitoring);
#endif

    static Mat optimize_error(const Mat &P, const Mat &T_initial, const Point32f *M, const Point32f *I, int n,
                              float interval, const float posFactor, const int steps, const int substeps,
                              const float decreaseFactor, bool timeMonitoring){

      FixedMatrix<float,1,3> r = extract_euler_angles(T_initial);
      FixedMatrix<float,1,3> t = T_initial.part<3,0,1,3>();
      const float E_initial = compute_error_opt(P,r,t,M,I,n);
      FixedMatrix<float,1,3> rBest = r, tBest = t, tInit = t, rInit = r;;
      float E_best = E_initial;

      Time ttt = timeMonitoring ? Time::now() : Time();

      FixedMatrix<float,1,3> rCurr=r, tCurr=t;
      for(int s=0;s<steps;++s){
        for(int rx=-substeps;rx<=substeps;++rx){
          rCurr[0] = r[0]+rx*interval;
          for(int ry=-substeps;ry<=substeps;++ry){
            rCurr[1] = r[1]+ry*interval;
            for(int rz=-substeps;rz<=substeps;++rz){
              rCurr[2] = r[2]+rz*interval;
              float E_curr = compute_error_opt(P,rCurr,t,M,I,n);
              if(E_curr < E_best){
                E_best = E_curr;
                rBest = rCurr;
              }
            }
          }
        }

        //for(int tx=-substeps;tx<=substeps;++tx){
        //  tCurr[0] = t[0] + tx*interval*posFactor;
        //  for(int ty=-substeps;ty<=substeps;++ty){
        //    tCurr[1] = t[1] + ty*interval*posFactor;
        for(int tz=-substeps;tz<=substeps;++tz){
          tCurr[2] = t[2] + tz*interval*posFactor;
          float E_curr = compute_error_opt(P,r,tCurr,M,I,n);
          if(E_curr < E_best){
            E_best = E_curr;
            tBest = tCurr;
          }
        }

        interval *= decreaseFactor;
        r = rBest;
        t = tBest;
      }


      if(timeMonitoring){
          std::cout << "dt: "<<(Time::now()-ttt).toMilliSecondsDouble()
                    << "  ##" <<E_initial << "## --> ####" << E_best << "####"
                    << "  Dt:" << (tBest-tInit).transp()
                    << "  Dr:"  << (rBest-rInit).transp() << std::endl;
      }

      return create_hom_4x4<float>(rBest[0],rBest[1],rBest[2],tBest[0],tBest[1],tBest[2]);
    }


      FixedMatrix<icl32f, 3, 3> getRzyx(icl32f a, icl32f b, icl32f c) {
        FixedMatrix<icl32f, 3, 3> R;

        icl32f sinA = sin(a);
        icl32f cosA = cos(a);
        icl32f sinB = sin(b);
        icl32f cosB = cos(b);
        icl32f sinC = sin(c);
        icl32f cosC = cos(c);

        icl32f sinAsinC = sinA * sinC;
        icl32f sinAcosC = sinA * cosC;
        icl32f cosAsinC = cosA * sinC;
        icl32f cosAcosC = cosA * cosC;

        R(0,0) = cosB * cosC;
        R(0,1) = cosB * sinC;
        R(0,2) = -sinB;
        R(1,0) = sinAcosC * sinB - cosAsinC;
        R(1,1) = sinAsinC * sinB + cosAcosC;
        R(1,2) = sinA * cosB;
        R(2,0) = cosAcosC * sinB + sinAsinC;
        R(2,1) = cosAsinC * sinB - sinAcosC;
        R(2,2) = cosA * cosB;

        return R;
      }

      void RzyxToAngles(FixedMatrix<icl32f, 3, 3> &R, FixedMatrix<icl32f, 1, 3> &res0, FixedMatrix<icl32f, 1, 3> &res1) {
        if (R(0,2) > -0.999999 && R(0,2) < 0.999999) {
          res0(0,1) = -asin(R(0,2));
          res1(0,1) = M_PI - res0(0,1);

          double cosB0 = cos(res0(0,1));
          double cosB1 = cos(res1(0,1));

          res0(0,0) = atan2(R(1,2) / cosB0, R(2,2) / cosB0);
          res1(0,0) = atan2(R(1,2) / cosB1, R(2,2) / cosB1);

          res0(0,2) = atan2(R(0,1) / cosB0, R(0,0) / cosB0);
          res1(0,2) = atan2(R(0,1) / cosB1, R(0,0) / cosB1);

        } else {
          res0(0,0) = res1(0,0) = 0.0;
          if (R(0,2) < 0.0) {
            res0(0,1) = res1(0,1) = M_PI / 2.0;
            res0(0,2) = res1(0,2) = atan2(R(1,0), R(2,0));
          } else {
            res0(0,1) = res1(0,1) = M_PI / -2.0;
            res0(0,2) = res1(0,2) = atan2(-R(1,0), -R(2,0));
          }
        }
      }

      inline void splitMat(FixedMatrix<icl32f, 4, 4> &m, FixedMatrix<icl32f, 3, 3> &R, FixedMatrix<icl32f, 1, 3> &t) {
        R = m.part<0,0,3,3>();
        t(0,0) = m(3,0);
        t(0,1) = m(3,1);
        t(0,2) = m(3,2);
      }

      inline FixedMatrix<icl32f, 4, 4> fuseMat(FixedMatrix<icl32f, 3, 3> &R, FixedMatrix<icl32f, 1, 3> &t) {
        FixedMatrix<icl32f, 4, 4> m = FixedMatrix<icl32f, 4, 4>::id();

        m(0,0) = R(0,0);
        m(1,0) = R(1,0);
        m(2,0) = R(2,0);
        m(0,1) = R(0,1);
        m(1,1) = R(1,1);
        m(2,1) = R(2,1);
        m(0,2) = R(0,2);
        m(1,2) = R(1,2);
        m(2,2) = R(2,2);
        m(3,0) = t(0,0);
        m(3,1) = t(0,1);
        m(3,2) = t(0,2);

        return m;
      }

      void calcRt(const int n, std::vector<FixedMatrix<icl32f, 1, 3> > &v, FixedMatrix<icl32f, 3, 3> &Rt) {
        // 1. norm all columns
        // 2. transpose
        // 3. mean rows
        // 4. transpose
        // 5. normalize
        FixedMatrix<icl32f, 1, 3> center;
        double x = 0.0, y = 0.0, z = 0.0;

        for (int i = 0; i < n; ++i) {
          double len = 1 / sqrt(pow(v[i](0,0), 2) + pow(v[i](0,1), 2) + 1);
          x += v[i](0,0) * len;
          y += v[i](0,1) * len;
          z += len;
        }

        center(0,0) = x / n;
        center(0,1) = y / n;
        center(0,2) = z / n;
        center.normalize();

        // 6. rotation from (0,0,1) to previous solution
        FixedMatrix<icl32f, 1, 3> v_z(0.0f, 0.0f, 1.0f);
        FixedMatrix<icl32f, 1, 3> axis;
        axis(0,0) = center(0,1);
        axis(0,1) = -center(0,0);
        axis(0,2) = 0;
        axis.normalize();

        Rt = create_rot_3D(axis(0,0), axis(0,1), axis(0,2), (float)acos((v_z.dot(center))(0,0)));
      }

      void calculateError(const int n, std::vector< FixedMatrix<icl32f, 1, 3> > &P,
                          std::vector< FixedMatrix<icl32f, 1, 3> > &V,
                          FixedMatrix<icl32f, 3, 3> &R, FixedMatrix<icl32f, 1, 3> &t,
                          float &error) {
        error = 0.0f;

        FixedMatrix<icl32f, 3, 3> I =  FixedMatrix<icl32f, 3, 3>::id();

        FixedMatrix<icl32f, 3, 3> VV[n];
        for (int i = 0; i < n; ++i) {
          VV[i] = (V[i] * V[i].transp()) / (V[i].transp() * V[i])(0,0);
        }

        // calculate the error
        for (int i = 0; i < n; ++i) {
            FixedMatrix<icl32f, 1, 3> tmp = (I - VV[i])*(R*P[i] + t);
            error += pow(tmp(0, 0), 2) + pow(tmp(0, 1), 2) + pow(tmp(0, 2), 2);
        }
      }

      struct MinSol {
        std::vector<double> betas;
        std::vector< FixedMatrix<icl32f, 1, 3> > ts;
        std::vector< FixedMatrix<icl32f, 3, 3> > Rs;
        std::vector<icl32f> errors;
      };

      void calculateMinima(const int n, std::vector< FixedMatrix<icl32f, 1, 3> > &P,
                           std::vector< FixedMatrix<icl32f, 1, 3> > &V,
                           FixedMatrix<icl32f, 3, 3> &Rz, FixedMatrix<icl32f, 1, 3> &t,
                           MinSol &sol) {
        FixedMatrix<icl32f, 3, 3> VV[n], G, Rp;
        FixedMatrix<icl32f, 3, 3> Vsum(0.0f), E(0.0f);
        FixedMatrix<icl32f, 3, 3> I = FixedMatrix<icl32f, 3, 3>::id();
        FixedMatrix<icl32f, 3, 3> t_opt(0.0f);
        icl32f e[5] = {0};
        double coef[5];

        for (int i = 0; i < n; ++i) {
          VV[i] = (V[i] * V[i].transp()) / (V[i].transp() * V[i])(0,0);
          Vsum += VV[i];
        }

        G = (I - (Vsum * (1.0f/n))).inv() * (1.0f/n);

        for (int i = 0; i < n; ++i) {
          FixedMatrix<icl32f, 3, 3> Rp(-P[i](0,0), P[i](0,2)*2.0f, P[i](0,0), P[i](0,1), 0.0f, P[i](0,1), -P[i](0,2), P[i](0,0)*-2.0f, P[i](0,2));
          t_opt += (VV[i] - I) * Rz * Rp;
        }

        t_opt = G * t_opt;

        for (int i = 0; i < n; ++i) {
          Rp = FixedMatrix<icl32f, 3, 3>(-P[i](0,0), P[i](0,2)*2.0f, P[i](0,0), P[i](0,1), 0.0f, P[i](0,1), -P[i](0,2), P[i](0,0)*-2.0f, P[i](0,2));

          E = (I - VV[i]) * (Rz * Rp + t_opt);

          FixedMatrix<icl32f, 1, 3> col0(E.col(0));
          FixedMatrix<icl32f, 1, 3> col1(E.col(1));
          FixedMatrix<icl32f, 1, 3> col2(E.col(2));
          e[0] += (col2.transp() * col2)(0,0);
          e[1] += (col2.transp() * col1)(0,0) * 2.0f;
          e[2] += (col0.transp() * col2)(0,0) * 2.0f + (col1.transp() * col1)(0,0);
          e[3] += (col0.transp() * col1)(0,0) * 2.0f;
          e[4] += (col0.transp() * col0)(0,0);
        }

        coef[0] = -e[3];
        coef[1] = 4.0f*e[4] - 2.0f*e[2];
        coef[2] = -3.0f*e[1] + 3.0f*e[3];
        coef[3] = -4.0f*e[0] + 2.0f*e[2];
        coef[4] = e[1];

        int deg = 4;
        double zeroes[5] = { 0 };
        double real[5];
        double imag[5];
        std::vector<int> mins;
        std::vector<double> betas;

        deg = math::solve_poly(deg, coef, zeroes, real, imag);

        for (int i = 0; i < deg; ++i) {
          double a_tmp = pow(real[i],2);
          double b_tmp = pow(imag[i],2);
          double ans = coef[0]*(a_tmp*a_tmp - 6.0*a_tmp*b_tmp + b_tmp*b_tmp)
                     + coef[1]*real[i]*(a_tmp - 3.0*b_tmp)
                     + coef[2]*(a_tmp - b_tmp)
                     + coef[3]*real[i] + coef[4];

          if (abs(ans) < 0.01) {
            double tmp = 1.0 + pow(real[i],2) - pow(imag[i],2);

            if (tmp > 0.01) {
              mins.push_back(i);
              betas.push_back(atan2(2.0*real[i]/tmp, (1.0-pow(real[i],2)+pow(imag[i],2))/tmp));
            }
          }
        }

        // take only minima
        for(unsigned int i = 0; i < mins.size(); ++i) {
          double a_tmp = pow(real[mins[i]],2);
          double b_tmp = pow(imag[mins[i]],2);
          double ans = 4.0*coef[0]*real[mins[i]]*(a_tmp - 3.0*b_tmp)
                     + 3.0*coef[1]*(a_tmp - b_tmp)
                     + 2.0*coef[2]*real[mins[i]] + coef[3];

          if (ans > 0.0) {
            sol.betas.push_back(betas[mins[i]]);
          }
        }

        for(unsigned int i = 0; i < sol.betas.size(); ++i) {
          Rp = Rz * getRzyx(0.0f, sol.betas[i], 0.0f);
          FixedMatrix<icl32f, 1, 3> t_new = FixedMatrix<icl32f, 1, 3>(0.0f);

          for (int j = 0; j < n; ++j) {
            t_new = t_new + (VV[j]-I) * Rp * P[j];
          }

          t_new = G * t_new;
          sol.ts.push_back(t_new);
        }
      }

    void CoplanarPointPoseEstimator::robustPoseCorrection(int n, const Point32f *modelPoints, std::vector<Point32f> &ips) {
      std::vector< FixedMatrix<icl32f, 1, 3> > V(n), P(n), P_(n);

      for (int i = 0; i < n ; ++i) {
        V[i] = FixedMatrix<icl32f, 1, 3>(ips[i].x, ips[i].y, 1.0f);
        P[i] = FixedMatrix<icl32f, 1, 3>(modelPoints[i].x, modelPoints[i].y, 0.0f);
      }

      FixedMatrix<icl32f, 3, 3> R;
      FixedMatrix<icl32f, 1, 3> t;

      splitMat(data->T, R, t);

      FixedMatrix<icl32f, 3, 3> Rt;
      std::vector< FixedMatrix<icl32f, 1, 3> > V_(n);
      FixedMatrix<icl32f, 3, 3> R_, R__, R2;
      FixedMatrix<icl32f, 3, 3> Rdz, Ry, Rz, Rdzz;
      FixedMatrix<icl32f, 1, 3> t_;
      FixedMatrix<icl32f, 1, 3> angles0, angles1;
      float error;
      MinSol sol;


      // transform everything in a new coordinate system

      calcRt(n, V, Rt);

      for (int i = 0; i < n; ++i) {
        V_[i] = Rt * V[i];
      }

      R_ = Rt * R;
      t_ = Rt * t;


      // find all local minima in the new coordinate system

      Rdz = getRzyx(0.0f, 0.0f, atan2(R_(1,2), R_(0,2)));
      R2 = R_ * Rdz;

      for (int i = 0; i < n; ++i) {
        P_[i] = Rdz.transp() * P[i];
      }

      RzyxToAngles(R2, angles0, angles1);

      if (fabs(angles0[0]) <= M_PI / 2) {
        Ry = getRzyx(0.0f, angles0(0,1), 0.0f);
        Rz = getRzyx(0.0f, 0.0f, angles0(0,2));
      } else {
        Ry = getRzyx(0.0f, angles1(0,1), 0.0f);
        Rz = getRzyx(0.0f, 0.0f, angles1(0,2));
      }

      calculateMinima(n, P_, V_, Rz, t_, sol);


      // calculate the errors of all solutions and 
      // take the solution with the lowest error

      FixedMatrix<icl32f, 3, 3> Rt_inv = Rt.transp();
      sol.errors = std::vector<icl32f>(sol.ts.size());

      for (unsigned int i = 0; i < sol.ts.size(); ++i) {
        Ry = getRzyx(0.0f, sol.betas[i], 0.0f);
        R__ = Rz * Ry * Rdz.transp();

        calculateError(n, P, V_, R__, sol.ts[i], sol.errors[i]);

        sol.Rs.push_back(Rt_inv*R__);
        sol.ts[i] = Rt_inv*sol.ts[i];
      }

      calculateError(n, P, V_, R_, t_, error);

      int ind = -1;

      for (unsigned int i = 0; i < sol.errors.size(); ++i) {
        if (sol.errors[i] < error) {
          error = sol.errors[i];
          ind = i;
        }
      }

      if (ind >= 0) {
        data->T = fuseMat(sol.Rs[ind], sol.ts[ind]);
      }
    }


    //  void simplex_iteration_callback(const SimplexOptimizer<float,Pose6D>::Result &r){
    //    if(r.iterations == 1 || !(r.iterations%100)){
    //    std::cout << "iteration:" << r.iterations << "  error:" << r.fx << std::endl;
    //  }
    //}

    std::vector<Pose6D> create_initial_simplex(const FixedMatrix<float,1,3> &r, const FixedMatrix<float,1,3> &t){
      Pose6D start = r%t;
      std::vector<Pose6D> simplex(7,start);
      for(int i=0;i<6;++i){
        simplex[i][i] *= (i>3 ? 1.2 : 1.5);
      }
      return simplex;
    }

    Mat CoplanarPointPoseEstimator::getPose(int n,
                                            const Point32f *modelPoints,
                                            const Point32f *imagePoints,
                                            const Camera &cam){
      float ifx = 1.0f/(cam.getFocalLength()*cam.getSamplingResolutionX());
      float ify = 1.0f/(cam.getFocalLength()*cam.getSamplingResolutionY());
      float icx = -ifx * cam.getPrincipalPointOffset().x;
      float icy = -ify * cam.getPrincipalPointOffset().y;

      // please note, the old implementation can be found in svn rev. 2753
      std::vector<Point32f> ips(n);//, pbs(n);
      for(int i=0;i<n;++i){
        ips[i] = Point32f(ifx*imagePoints[i].x+icx, ify * imagePoints[i].y+icy);
      }

      typedef float real;
      GenericHomography2D<real> H(ips.data(),modelPoints,n); // tested the homography error which is always almost 0

#ifdef USE_OLD_VERSION

      H *= 1.0/sqrt( pow(H(0,0),2) + pow(H(0,1),2) + pow(H(0,2),2) );

      // if H solves Ax=0 then also -H solves it, therefore, we always
      // take the solution, where z is positive (object is in front of the camera)

      if(H(2,2) < 0){
        H *= -1;
      }

      FixedColVector<real,3> R1 = H.col(0);
      FixedColVector<real,3> R2 = H.col(1);
      R2 -= R1*(R1.transp()*R2)[0];
      R2.normalize();
      FixedColVector<real,3> R3 = cross3(R1,R2);

      data->T.part<0,0,3,3>() = data->R = (R1,R2,R3);

      // -R * t -> provides translation part in 'clear-text'
      data->T.part<3,0,1,3>() = data->R.transp()*FixedColVector<real,3>( -H(2,0),-H(2,1),-H(2,2) );

      // this provides the original camera CS-Transformation Matrix
      data->T.part<3,0,1,3>() = FixedColVector<real,3>( H(2,0),H(2,1),H(2,2) );

      data->T(0,3) = data->T(1,3) = data->T(2,3) = 0;
      data->T(3,3) = 1;

#else

      FixedMatrix<float,4,3> T;

      FixedColVector<float,3> c0 = H.col(0);
      FixedColVector<float,3> c1 = H.col(1);
      FixedColVector<float,3> c2 = H.col(2);

      float norm1 = c0.length();
      float norm2 = c1.length();
      float tnorm = (norm1 + norm2) / 2.0f;

      T.col(0) = c0 * (1/norm1);
      T.col(1) = c1 * (1/norm2);

      FixedColVector<float,3> t0 = T.col(0);
      FixedColVector<float,3> t1 = T.col(1);
      T.col(2) = cross3(t0,t1);
      T.col(3) = c2 * (1/tnorm);

      data->T.part<0,0,4,3>() = T;
      data->T(0,3) = data->T(1,3) = data->T(2,3) = 0;
      data->T(3,3) = 1;


#endif


      if (data->poseCorrection) robustPoseCorrection(n, modelPoints, ips);


      if(data->algorithm != HomographyBasedOnly){
        switch(data->algorithm){
          case SamplingCustom:
            data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                     data->samplingInterval, data->positionMultiplier, data->samplingSteps, data->samplingSubSteps,
                                     data->decreaseFactor,data->timeMonitoring);
            break;
          case SamplingCoarse:
            data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                     1.0, 50, 10, 1, 0.6, data->timeMonitoring);
            break;
          case SamplingMedium:
            data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                     0.5, 50, 50, 1, 0.6, data->timeMonitoring);
            break;
          case SamplingFine:
            data->T = optimize_error(cam.getProjectionMatrix(), data->T, modelPoints, imagePoints, n,
                                     0.3, 50, 100, 2, 0.6, data->timeMonitoring);
            break;
          case SimplexSampling:{
            SimplexErrorFunction err(cam.getProjectionMatrix(),modelPoints,imagePoints,n);
            Function<float,const Pose6D &> ferr = function(err,&SimplexErrorFunction::f);
            SimplexOptimizer<float,Pose6D> opt(ferr,6,400,0.5);
            FixedMatrix<float,1,3> r = extract_euler_angles(data->T);
            FixedMatrix<float,1,3> t = data->T.part<3,0,1,3>();
            SimplexOptimizer<float,Pose6D>::Result res = opt.optimize(create_initial_simplex(r,t));
            const Pose6D &x = res.x;
            data->T = create_hom_4x4(x[0],x[1],x[2],x[3],x[4],x[5]);
            break;
          }
          default:
            throw ICLException("Error in " + str(__FUNCTION__) + ": invalind pose estimation algorithm");
        }
      }


  #if 0
      Mat M = cam.getCSTransformationMatrix().inv()*data->T;
      float error = 0;
      for(int i=0;i<n;++i){
        Vec tmp = M * Vec(modelPoints[i].x,modelPoints[i].y,0,1);
        Point32f p = cam.project(tmp);
        error += p.distanceTo(imagePoints[i]);
      }

      Mat P = cam.getProjectionMatrix();
      float error2 = 0;
      for(int i=0;i<n;++i){
        Vec tmp = homogenize( P * data->T * Vec(modelPoints[i].x,modelPoints[i].y,0,1) );
        error2 += Point32f(tmp[0],tmp[1]).distanceTo(imagePoints[i]);
      }
      std::cout << "error: " << error <<  "  error2:" << error2 << std::endl;

      /* explanation:
                    ||                                     ||  where, I: imagePoints
         E(T) = sum || I[i] - project(C⁻1 * T * vec4(M[i]) ||         M: modelPoints
                 i  ||                                     ||       C⁻1: inverse cam transform

          but since, project(x) p2(hom(P*C*x)), where P: projection matrix and C: camera matrix,
          and p2: extracts x and y from a 4D homogenious vector,

                    ||                                   ||  where,   P: cam projection matrix
         E(T) = sum || I[i] - p2(hom(P * T * vec4(M[i])) ||         hom: homogenization
                 i  ||                                   ||          p2(x,y,z,w) = (x,y)

          The question, that remains is, how this function is minimized and, if we assume a
          closed solution in least sqaure scene, if the initial T does already minimize E(T)
          if T is a homogeneous transform [R|t]

          * first approach: try to use stochastic search (perhaps coarse to fine) to find
            better solutions for T iteratively
          * 2nd approach: try to derivate E(T) wrt. T in order to implement gradient descent
            to optimize T
          * find out whether a closed form solution for E(T) is already found with the method
            above
      */
  #endif
      if(data->referenceFrame == cameraFrame){
        return data->T;
      }else{
        return cam.getCSTransformationMatrix().inv()*data->T;
      }
    }

    REGISTER_CONFIGURABLE_DEFAULT(CoplanarPointPoseEstimator);
  } // namespace geom
}

