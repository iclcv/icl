// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <icl/io/IntrinsicCalibrator.h>
#include <fstream>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl::io {
  class IntrinsicCalibrator::Data{
  public:
    int bWidth;
    int bHeight;
    int successes;
    int bSize;
    int nx;
    int ny;

    DynMatrix<icl64f>* intrinsic_matrix;
    DynMatrix<icl64f>* distortion_coeffs;

    Data(unsigned int boardWidth, unsigned int boardHeight, unsigned int boardCount,unsigned int imageWidth ,unsigned int imageHeight):
      bWidth(boardWidth),bHeight(boardHeight), successes(boardCount),
      bSize(bWidth*bHeight),nx(imageWidth),ny(imageHeight){
      intrinsic_matrix = new DynMatrix<icl64f>(3, 3);
      (*intrinsic_matrix)[8] = 1.0;
      distortion_coeffs = new DynMatrix<icl64f>(5, 1);
    }

    ~Data(){
      delete intrinsic_matrix;
      delete distortion_coeffs;
    }
  };

  IntrinsicCalibrator::IntrinsicCalibrator(unsigned int boardWidth, unsigned int boardHeight,
                                           unsigned int boardCount, unsigned int imageWidth, unsigned int imageHeight):
    m_data(new IntrinsicCalibrator::Data(boardWidth, boardHeight, boardCount,imageWidth, imageHeight)){}

  IntrinsicCalibrator::~IntrinsicCalibrator(){
    delete m_data;
  }

  void IntrinsicCalibrator::init_intrinsic_param(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &X,
                                                 DynMatrix<icl64f> &fc, DynMatrix<icl64f> &cc, DynMatrix<icl64f> &kc, double &alpha_c){

    // Initialize the homographies:
    DynMatrix<icl64f> H(3,3);
    DynMatrix<icl64f> HH(9,m_data->successes);
    DynMatrix<icl64f> x1(m_data->bSize,2);
    for(int kk = 0;kk<m_data->successes;++kk){
      for(int i=0;i<m_data->bSize;++i){
        x1.index_yx(0, i) = x.index_yx(2*kk, i);
        x1.index_yx(1, i) = x.index_yx(2*kk+1, i);
      }
      compute_homography(x1,X,H);
      for(int i=0;i<9;++i){
        HH.index_yx(kk, i) = H[i];
      }
    }
    // initial guess for principal point and distortion:

    // initialize at the center of the image
    DynMatrix<icl64f> c_init(1,2);
    c_init[0] = m_data->nx/2.0-0.5;
    c_init[1] = m_data->ny/2.0-0.5;
    //initialize to zero (no distortion)
    DynMatrix<icl64f> k_init(1,5);

    // Compute explicitely the focal length using all the (mutually orthogonal) vanishing points
    // The vanihing points are hidden in the planar collineations H_kk

    // matrix that subtract the principal point:
    double dat[9] = {1, 0, -c_init[0], 0, 1, -c_init[1], 0, 0, 1};
    DynMatrix<icl64f> Sub_cc(3,3,dat);
    DynMatrix<icl64f> Hkk(3,3), V_hori_pix(1,3), V_vert_pix(1,3), V_diag1_pix(1,3), V_diag2_pix(1,3);
    DynMatrix<icl64f> Hkk2(3,3);
    DynMatrix<icl64f> Hkkc1(1,3), Hkkc2(1,3);
    DynMatrix<icl64f> A(2,2*(m_data->successes)), b(1,2*m_data->successes);
    for(int kk=0, mm=0;kk<m_data->successes;++kk,mm+=2){//:n_ima,
      for(int i=0;i<9;++i){
        Hkk[i] = HH.index_yx(kk, i);
      }
      Hkk2 = Sub_cc * Hkk;
      // Extract vanishing points (direct and diagonals):
      V_hori_pix[0] = Hkk2[0];
      V_hori_pix[1] = Hkk2[3];
      V_hori_pix[2] = Hkk2[6];

      V_vert_pix[0] = Hkk2[1];
      V_vert_pix[1] = Hkk2[4];
      V_vert_pix[2] = Hkk2[7];

      Hkkc1[0] = Hkk2[0]; Hkkc1[1] = Hkk2[3]; Hkkc1[2] = Hkk2[6];
      Hkkc2[0] = Hkk2[1]; Hkkc2[1] = Hkk2[4]; Hkkc2[2] = Hkk2[7];

      V_diag1_pix = Hkkc1+Hkkc2;
      V_diag2_pix = Hkkc1-Hkkc2;
      for(int i=0;i<3;++i){
        V_diag1_pix[i] = V_diag1_pix[i]/2.0;
        V_diag2_pix[i] = V_diag2_pix[i]/2.0;
      }

      double n1 = 1.0/V_hori_pix.norm();
      double n2 = 1.0/V_vert_pix.norm();
      double n3 = 1.0/V_diag1_pix.norm();
      double n4 = 1.0/V_diag2_pix.norm();
      for(int i=0;i<3;++i){
        V_hori_pix[i] = V_hori_pix[i]*n1;
        V_vert_pix[i] = V_vert_pix[i]*n2;
        V_diag1_pix[i] = V_diag1_pix[i]*n3;
        V_diag2_pix[i] = V_diag2_pix[i]*n4;
      }

      double a1 = V_hori_pix[0];
      double b1 = V_hori_pix[1];
      double c1 = V_hori_pix[2];

      double a2 = V_vert_pix[0];
      double b2 = V_vert_pix[1];
      double c2 = V_vert_pix[2];

      double a3 = V_diag1_pix[0];
      double b3 = V_diag1_pix[1];
      double c3 = V_diag1_pix[2];

      double a4 = V_diag2_pix[0];
      double b4 = V_diag2_pix[1];
      double c4 = V_diag2_pix[2];

      DynMatrix<icl64f> A_kk(2,2);
      A_kk[0] = a1*a2; A_kk[1] = b1*b2; A_kk[2] = a3*a4; A_kk[3] = b3*b4;

      DynMatrix<icl64f> b_kk(1,2);
      b_kk[0] = -c1*c2; b_kk[1] = -c3*c4;

      A.index_yx(mm, 0) = A_kk[0];
      A.index_yx(mm, 1) = A_kk[1];
      A.index_yx(mm+1, 0) = A_kk[2];
      A.index_yx(mm+1, 1) = A_kk[3];

      b.index_yx(mm, 0) = b_kk[0];
      b.index_yx(mm+1, 0) = b_kk[1];
    }

    // use all the vanishing points to estimate focal length:
    bool two_focals_init = false;
    // Select the model for the focal. (solution to Gerd's problem)
    DynMatrix<icl64f> AT = A.transp();
    DynMatrix<icl64f> ATS(AT.cols(),1);
    for(unsigned int i=0;i<AT.cols();++i){
      for(unsigned int j=0;j<AT.rows();++j)
        ATS[i] = ATS[i] + AT.index_yx(j, i);
    }
    DynMatrix<icl64f> ATST = ATS.transp();
    DynMatrix<icl64f> bt = b.transp();
    DynMatrix<icl64f> q = bt*ATST;

    if(q[0] < 0){
      two_focals_init = true;
    }

    DynMatrix<icl64f> f_init(1,2);
    if (two_focals_init){
      // Use a two focals estimate:
      f_init = (AT*A).inv()*AT*b;
      f_init[0] =std::sqrt(std::abs(1.0/f_init[0])); f_init[1] =std::sqrt(std::abs(1.0/f_init[1]));
    } else {
      // Use a single focal estimate:
      DynMatrix<icl64f> tmp = bt*ATST;
      DynMatrix<icl64f> tmp2 = bt*b;
      f_init[0]=std::sqrt(tmp[0]/tmp2[0]);
      f_init[1]=f_init[0];
    }

    cc = c_init;
    fc = f_init;
    kc = k_init;
    alpha_c = 0.0;
  }

  void IntrinsicCalibrator::comp_ext_calib(const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,
                                           const DynMatrix<icl64f> &cc, const DynMatrix<icl64f> &kc, const double alpha_c, const double thresh_cond,
                                           DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk){

    DynMatrix<icl64f> omckk2(1,3),Tckk2(1,3),Rckk2(3,3),JJ_kk(6,2*m_data->bSize);

    compute_extrinsic_init(x_kk,X_kk,fc,cc,kc,alpha_c,
                           omckk2,Tckk2,Rckk2);

    compute_extrinsic_refine(omckk2,Tckk2,x_kk,X_kk,fc,cc,kc,alpha_c,20,thresh_cond,
                             omckk,Tckk,Rckk,JJ_kk);

    if (JJ_kk.cond()> thresh_cond){
      std::cout << "warning image in comp_ext_calib is bad conditioned\n";
    }
  }

  void IntrinsicCalibrator::rigid_motion(const DynMatrix<icl64f> &X, const DynMatrix<icl64f> &om, const DynMatrix<icl64f> &T,
                                         DynMatrix<icl64f> &Y, DynMatrix<icl64f> &dYdom, DynMatrix<icl64f> &dYdT){

    DynMatrix<icl64f> R(3,3), dRdom(3,9);
    rodrigues(om, R,dRdom);

    int n = X.cols();

    Y = R*X;

    for(unsigned int i=0;i<Y.cols();++i){
      Y.index_yx(0, i) = Y.index_yx(0, i)+T[0];
      Y.index_yx(1, i) = Y.index_yx(1, i)+T[1];
      Y.index_yx(2, i) = Y.index_yx(2, i)+T[2];
    }
    DynMatrix<icl64f> dYdR(9,3*n);
    int j=0;
    for(int i=0;i<n;++i){
      j=3*i;
      dYdR.index_yx(j, 0) = X.index_yx(0, i); dYdR.index_yx(j+1, 1) = X.index_yx(0, i); dYdR.index_yx(j+2, 2) = X.index_yx(0, i);
      dYdR.index_yx(j, 3) = X.index_yx(1, i); dYdR.index_yx(j+1, 4) = X.index_yx(1, i); dYdR.index_yx(j+2, 5) = X.index_yx(1, i);

      dYdT.index_yx(3*i, 0) = 1; dYdT.index_yx(3*i+1, 1) = 1; dYdT.index_yx(3*i+2, 2) = 1;
    }
    dYdom = dYdR * dRdom;
  }

  void IntrinsicCalibrator::project_points2(const DynMatrix<icl64f> &X, const DynMatrix<icl64f> &om, const DynMatrix<icl64f> &T,
                                            const DynMatrix<icl64f> &f, const DynMatrix<icl64f> &c, const DynMatrix<icl64f> &k, const double alpha,
                                            DynMatrix<icl64f> &xp, DynMatrix<icl64f> &dxpdom, DynMatrix<icl64f> &dxpdT, DynMatrix<icl64f> &dxpdf,
                                            DynMatrix<icl64f> &dxpdc, DynMatrix<icl64f> &dxpdk, DynMatrix<icl64f> &dxpdalpha){

    int n = X.cols();
    DynMatrix<icl64f> Y(n,3), dYdom(3,3*n), dYdT(3,3*n);
    rigid_motion(X,om,T, Y,dYdom,dYdT);
    DynMatrix<icl64f> inv_Z(n,1);
    DynMatrix<icl64f> x(n,2);
    for(int i=0;i<n;++i){
      inv_Z[i] = 1.0/Y.index_yx(2, i);
      x.index_yx(0, i) = Y.index_yx(0, i)*inv_Z[i];
      x.index_yx(1, i) = Y.index_yx(1, i)*inv_Z[i];
    }
    DynMatrix<icl64f> cc(3,m_data->bSize);
    DynMatrix<icl64f> bb(3,m_data->bSize);
    for(int i=0;i<n;++i){
      bb.index_yx(i, 0) = -x.index_yx(0, i)*inv_Z[i]; bb.index_yx(i, 1) = bb.index_yx(i, 0); bb.index_yx(i, 2) = bb.index_yx(i, 0);
      cc.index_yx(i, 0) = -x.index_yx(1, i)*inv_Z[i]; cc.index_yx(i, 1) = cc.index_yx(i, 0); cc.index_yx(i, 2) = cc.index_yx(i, 0);
    }

    DynMatrix<icl64f> dxdom(3,2*n);
    for(int i=0;i<n;++i){
      dxdom.index_yx(2*i, 0) = inv_Z[i]*dYdom.index_yx(3*i, 0) + bb[3*i]*dYdom.index_yx(3*i+2, 0);
      dxdom.index_yx(2*i, 1) = inv_Z[i]*dYdom.index_yx(3*i, 1) + bb[3*i+1]*dYdom.index_yx(3*i+2, 1);
      dxdom.index_yx(2*i, 2) = inv_Z[i]*dYdom.index_yx(3*i, 2) + bb[3*i+2]*dYdom.index_yx(3*i+2, 2);

      dxdom.index_yx(2*i+1, 0) = inv_Z[i]*dYdom.index_yx(3*i+1, 0) + cc[3*i]*dYdom.index_yx(3*i+2, 0);
      dxdom.index_yx(2*i+1, 1) = inv_Z[i]*dYdom.index_yx(3*i+1, 1) + cc[3*i+1]*dYdom.index_yx(3*i+2, 1);
      dxdom.index_yx(2*i+1, 2) = inv_Z[i]*dYdom.index_yx(3*i+1, 2) + cc[3*i+2]*dYdom.index_yx(3*i+2, 2);
    }

    DynMatrix<icl64f> dxdT(3,2*n);
    for(int i=0;i<n;++i){
      dxdT.index_yx(2*i, 0) = inv_Z[i]*dYdT.index_yx(3*i, 0) + bb[3*i]*dYdT.index_yx(3*i+2, 0);
      dxdT.index_yx(2*i, 1) = inv_Z[i]*dYdT.index_yx(3*i, 1) + bb[3*i+1]*dYdT.index_yx(3*i+2, 1);
      dxdT.index_yx(2*i, 2) = inv_Z[i]*dYdT.index_yx(3*i, 2) + bb[3*i+2]*dYdT.index_yx(3*i+2, 2);
      dxdT.index_yx(2*i+1, 0) = inv_Z[i]*dYdT.index_yx(3*i+1, 0) + cc[3*i]*dYdT.index_yx(3*i+2, 0);
      dxdT.index_yx(2*i+1, 1) = inv_Z[i]*dYdT.index_yx(3*i+1, 1) + cc[3*i+1]*dYdT.index_yx(3*i+2, 1);
      dxdT.index_yx(2*i+1, 2) = inv_Z[i]*dYdT.index_yx(3*i+1, 2) + cc[3*i+2]*dYdT.index_yx(3*i+2, 2);
    }

    //Add distortion:
    DynMatrix<icl64f> r2(m_data->bSize,1);
    DynMatrix<icl64f> r4(m_data->bSize,1);
    DynMatrix<icl64f> r6(m_data->bSize,1);
    for(int i=0;i<(m_data->bSize);++i){
      r2[i] = x.index_yx(0, i)*x.index_yx(0, i)+x.index_yx(1, i)*x.index_yx(1, i);
      r4[i] = r2[i]*r2[i];
      r6[i] = r2[i]*r2[i]*r2[i];
    }

    DynMatrix<icl64f> dr2dom(3,m_data->bSize), dr2dT(3,m_data->bSize);
    for(int i=0;i<n;++i){
      dr2dom.index_yx(i, 0) = 2*x.index_yx(0, i) * dxdom.index_yx(2*i, 0) + 2*x.index_yx(1, i)*dxdom.index_yx(2*i+1, 0);
      dr2dom.index_yx(i, 1) = 2*x.index_yx(0, i) * dxdom.index_yx(2*i, 1) + 2*x.index_yx(1, i)*dxdom.index_yx(2*i+1, 1);
      dr2dom.index_yx(i, 2) = 2*x.index_yx(0, i) * dxdom.index_yx(2*i, 2) + 2*x.index_yx(1, i)*dxdom.index_yx(2*i+1, 2);

      dr2dT.index_yx(i, 0) = 2*x.index_yx(0, i) * dxdT.index_yx(2*i, 0) + 2*x.index_yx(1, i)*dxdT.index_yx(2*i+1, 0);
      dr2dT.index_yx(i, 1) = 2*x.index_yx(0, i) * dxdT.index_yx(2*i, 1) + 2*x.index_yx(1, i)*dxdT.index_yx(2*i+1, 1);
      dr2dT.index_yx(i, 2) = 2*x.index_yx(0, i) * dxdT.index_yx(2*i, 2) + 2*x.index_yx(1, i)*dxdT.index_yx(2*i+1, 2);
    }

    DynMatrix<icl64f> dr4dom(3,m_data->bSize), dr4dT(3,m_data->bSize);
    for(int i=0;i<n;++i){
      dr4dom.index_yx(i, 0) = 2*r2[i]*dr2dom.index_yx(i, 0);
      dr4dom.index_yx(i, 1) = 2*r2[i]*dr2dom.index_yx(i, 1);
      dr4dom.index_yx(i, 2) = 2*r2[i]*dr2dom.index_yx(i, 2);
      dr4dT.index_yx(i, 0) = 2*r2[i]*dr2dT.index_yx(i, 0);
      dr4dT.index_yx(i, 1) = 2*r2[i]*dr2dT.index_yx(i, 1);
      dr4dT.index_yx(i, 2) = 2*r2[i]*dr2dT.index_yx(i, 2);
    }

    DynMatrix<icl64f> dr6dom(3,m_data->bSize), dr6dT(3,m_data->bSize);
    for(int i=0;i<n;++i){
      dr6dom.index_yx(i, 0) = 3*r2[i]*r2[i]*dr2dom.index_yx(i, 0);
      dr6dom.index_yx(i, 1) = 3*r2[i]*r2[i]*dr2dom.index_yx(i, 1);
      dr6dom.index_yx(i, 2) = 3*r2[i]*r2[i]*dr2dom.index_yx(i, 2);
      dr6dT.index_yx(i, 0) = 3*r2[i]*r2[i]*dr2dT.index_yx(i, 0);
      dr6dT.index_yx(i, 1) = 3*r2[i]*r2[i]*dr2dT.index_yx(i, 1);
      dr6dT.index_yx(i, 2) = 3*r2[i]*r2[i]*dr2dT.index_yx(i, 2);
    }

    //Radial distortion:
    DynMatrix<icl64f> cdist(n,1);
    for(unsigned int i=0;i<r2.dim();++i){
      cdist[i] = 1 +k[0]*r2[i]+k[1]*r4[i]+k[4]*r6[i];
    }

    DynMatrix<icl64f> dcdistdom(3,m_data->bSize);
    DynMatrix<icl64f> dcdistdT(3,m_data->bSize);
    for(int i=0;i<m_data->bSize;++i){
      dcdistdom.index_yx(i, 0) = k[0]*dr2dom.index_yx(i, 0); dcdistdom.index_yx(i, 0) += k[1]*dr4dom.index_yx(i, 0); dcdistdom.index_yx(i, 0) += k[4]*dr6dom.index_yx(i, 0);
      dcdistdom.index_yx(i, 1) = k[0]*dr2dom.index_yx(i, 1); dcdistdom.index_yx(i, 1) += k[1]*dr4dom.index_yx(i, 1); dcdistdom.index_yx(i, 1) += k[4]*dr6dom.index_yx(i, 1);
      dcdistdom.index_yx(i, 2) = k[0]*dr2dom.index_yx(i, 2); dcdistdom.index_yx(i, 2) += k[1]*dr4dom.index_yx(i, 2); dcdistdom.index_yx(i, 2) += k[4]*dr6dom.index_yx(i, 2);

      dcdistdT.index_yx(i, 0) = k[0]*dr2dT.index_yx(i, 0); dcdistdT.index_yx(i, 0) += k[1]*dr4dT.index_yx(i, 0); dcdistdT.index_yx(i, 0) += k[4]*dr6dT.index_yx(i, 0);
      dcdistdT.index_yx(i, 1) = k[0]*dr2dT.index_yx(i, 1); dcdistdT.index_yx(i, 1) += k[1]*dr4dT.index_yx(i, 1); dcdistdT.index_yx(i, 1) += k[4]*dr6dT.index_yx(i, 1);
      dcdistdT.index_yx(i, 2) = k[0]*dr2dT.index_yx(i, 2); dcdistdT.index_yx(i, 2) += k[1]*dr4dT.index_yx(i, 2); dcdistdT.index_yx(i, 2) += k[4]*dr6dT.index_yx(i, 2);
    }

    DynMatrix<icl64f> dcdistdk(5,m_data->bSize);
    for(unsigned int i=0;i<r2.cols();++i){
      dcdistdk.index_yx(i, 0) = r2[i]; dcdistdk.index_yx(i, 1) = r4[i]; dcdistdk.index_yx(i, 4) = r6[i];
    }

    DynMatrix<icl64f> xd1(m_data->bSize,2);
    for(int i=0;i<m_data->bSize;++i){
      xd1.index_yx(0, i) = x.index_yx(0, i)*cdist[i]; xd1.index_yx(1, i) = x.index_yx(1, i)*cdist[i];
    }
    DynMatrix<icl64f> dxd1dom(3,2*n);
    for(int i=0;i<n;++i){
      dxd1dom.index_yx(2*i, 0) = x.index_yx(0, i)*dcdistdom.index_yx(i, 0);
      dxd1dom.index_yx(2*i, 1) = x.index_yx(0, i)*dcdistdom.index_yx(i, 1);
      dxd1dom.index_yx(2*i, 2) = x.index_yx(0, i)*dcdistdom.index_yx(i, 2);

      dxd1dom.index_yx(2*i+1, 0) = x.index_yx(1, i)*dcdistdom.index_yx(i, 0);
      dxd1dom.index_yx(2*i+1, 1) = x.index_yx(1, i)*dcdistdom.index_yx(i, 1);
      dxd1dom.index_yx(2*i+1, 2) = x.index_yx(1, i)*dcdistdom.index_yx(i, 2);
    }

    DynMatrix<icl64f> coeff(3,2*n);
    for(int i=0;i<n;++i){
      coeff.index_yx(2*i, 0) = cdist[i]; coeff.index_yx(2*i, 1) = cdist[i]; coeff.index_yx(2*i, 2) = cdist[i];
      coeff.index_yx(2*i+1, 0) = cdist[i]; coeff.index_yx(2*i+1, 1) = cdist[i]; coeff.index_yx(2*i+1, 2) = cdist[i];
    }

    dxd1dom = dxd1dom + coeff.elementwise_mult(dxdom);

    DynMatrix<icl64f> dxd1dT(3,2*n);
    for(int i=0;i<m_data->bSize;++i){
      dxd1dT.index_yx(2*i, 0) = x.index_yx(0, i)*dcdistdT.index_yx(i, 0);
      dxd1dT.index_yx(2*i, 1) = x.index_yx(0, i)*dcdistdT.index_yx(i, 1);
      dxd1dT.index_yx(2*i, 2) = x.index_yx(0, i)*dcdistdT.index_yx(i, 2);

      dxd1dT.index_yx(2*i+1, 0) = x.index_yx(1, i)*dcdistdT.index_yx(i, 0);
      dxd1dT.index_yx(2*i+1, 1) = x.index_yx(1, i)*dcdistdT.index_yx(i, 1);
      dxd1dT.index_yx(2*i+1, 2) = x.index_yx(1, i)*dcdistdT.index_yx(i, 2);
    }

    dxd1dT = dxd1dT + coeff.elementwise_mult(dxdT);

    DynMatrix<icl64f> dxd1dk(5,2*n);
    for(int i=0;i<n;++i){
      dxd1dk.index_yx(2*i, 0) = x.index_yx(0, i)*dcdistdk.index_yx(i, 0);
      dxd1dk.index_yx(2*i, 1) = x.index_yx(0, i)*dcdistdk.index_yx(i, 1);
      dxd1dk.index_yx(2*i, 2) = x.index_yx(0, i)*dcdistdk.index_yx(i, 2);
      dxd1dk.index_yx(2*i, 3) = x.index_yx(0, i)*dcdistdk.index_yx(i, 3);
      dxd1dk.index_yx(2*i, 4) = x.index_yx(0, i)*dcdistdk.index_yx(i, 4);

      dxd1dk.index_yx(2*i+1, 0) = x.index_yx(1, i)*dcdistdk.index_yx(i, 0);
      dxd1dk.index_yx(2*i+1, 1) = x.index_yx(1, i)*dcdistdk.index_yx(i, 1);
      dxd1dk.index_yx(2*i+1, 2) = x.index_yx(1, i)*dcdistdk.index_yx(i, 2);
      dxd1dk.index_yx(2*i+1, 3) = x.index_yx(1, i)*dcdistdk.index_yx(i, 3);
      dxd1dk.index_yx(2*i+1, 4) = x.index_yx(1, i)*dcdistdk.index_yx(i, 4);
    }

    //tangential distortion:
    DynMatrix<icl64f> a1(r2.cols(),1), a2(r2.cols(),1), a3(r2.cols(),1);
    for(unsigned int i=0;i<r2.cols();++i){
      a1[i] = 2*x.index_yx(0, i)*x.index_yx(1, i);
      a2[i] = r2[i] + 2*x.index_yx(0, i)*x.index_yx(0, i);
      a3[i] = r2[i] + 2*x.index_yx(1, i)*x.index_yx(1, i);
    }
    DynMatrix<icl64f> delta_x(m_data->bSize,2);
    for(int i=0;i<m_data->bSize;++i){
      delta_x.index_yx(0, i) = k[2]*a1[i]+k[3]*a2[i];
      delta_x.index_yx(1, i) = k[2]*a3[i]+k[3]*a1[i];
    }

    DynMatrix<icl64f> ddelta_xdom(3,2*n);
    DynMatrix<icl64f> ddelta_xdT(3,2*n);
    DynMatrix<icl64f> ddelta_xdk(5,2*n);
    {
      DynMatrix<icl64f> aa(3,m_data->bSize), bb(3,m_data->bSize), cc(3,m_data->bSize);
      for(int i=0;i<n;++i){
        aa.index_yx(i, 0) = 2*k[2]*x.index_yx(1, i) + 6*k[3]*x.index_yx(0, i); aa.index_yx(i, 1) = aa.index_yx(i, 0); aa.index_yx(i, 2) = aa.index_yx(i, 0);
        bb.index_yx(i, 0) = 2*k[2]*x.index_yx(0, i) + 2*k[3]*x.index_yx(1, i); bb.index_yx(i, 1) = bb.index_yx(i, 0); bb.index_yx(i, 2) = bb.index_yx(i, 0);
        cc.index_yx(i, 0) = 6*k[2]*x.index_yx(1, i) + 2*k[3]*x.index_yx(0, i); cc.index_yx(i, 1) = cc.index_yx(i, 0); cc.index_yx(i, 2) = cc.index_yx(i, 0);
      }

      for(int i=0;i<n;++i){
        ddelta_xdom.index_yx(2*i, 0) = aa.index_yx(i, 0)*dxdom.index_yx(2*i, 0) + bb.index_yx(i, 0)*dxdom.index_yx(2*i+1, 0);
        ddelta_xdom.index_yx(2*i, 1) = aa.index_yx(i, 1)*dxdom.index_yx(2*i, 1) + bb.index_yx(i, 1)*dxdom.index_yx(2*i+1, 1);
        ddelta_xdom.index_yx(2*i, 2) = aa.index_yx(i, 2)*dxdom.index_yx(2*i, 2) + bb.index_yx(i, 2)*dxdom.index_yx(2*i+1, 2);

        ddelta_xdom.index_yx(2*i+1, 0) = bb.index_yx(i, 0)*dxdom.index_yx(2*i, 0) + cc.index_yx(i, 0)*dxdom.index_yx(2*i+1, 0);
        ddelta_xdom.index_yx(2*i+1, 1) = bb.index_yx(i, 1)*dxdom.index_yx(2*i, 1) + cc.index_yx(i, 1)*dxdom.index_yx(2*i+1, 1);
        ddelta_xdom.index_yx(2*i+1, 2) = bb.index_yx(i, 2)*dxdom.index_yx(2*i, 2) + cc.index_yx(i, 2)*dxdom.index_yx(2*i+1, 2);
      }

      for(int i=0;i<n;++i){
        ddelta_xdT.index_yx(2*i, 0) = aa.index_yx(i, 0)*dxdT.index_yx(2*i, 0) + bb.index_yx(i, 0)*dxdT.index_yx(2*i+1, 0);
        ddelta_xdT.index_yx(2*i, 1) = aa.index_yx(i, 1)*dxdT.index_yx(2*i, 1) + bb.index_yx(i, 1)*dxdT.index_yx(2*i+1, 1);
        ddelta_xdT.index_yx(2*i, 2) = aa.index_yx(i, 2)*dxdT.index_yx(2*i, 2) + bb.index_yx(i, 2)*dxdT.index_yx(2*i+1, 2);

        ddelta_xdT.index_yx(2*i+1, 0) = bb.index_yx(i, 0)*dxdT.index_yx(2*i, 0) + cc.index_yx(i, 0)*dxdT.index_yx(2*i+1, 0);
        ddelta_xdT.index_yx(2*i+1, 1) = bb.index_yx(i, 1)*dxdT.index_yx(2*i, 1) + cc.index_yx(i, 1)*dxdT.index_yx(2*i+1, 1);
        ddelta_xdT.index_yx(2*i+1, 2) = bb.index_yx(i, 2)*dxdT.index_yx(2*i, 2) + cc.index_yx(i, 2)*dxdT.index_yx(2*i+1, 2);
      }

      for(int i=0;i<n;++i){
        ddelta_xdk.index_yx(2*i, 2) = a1[i];
        ddelta_xdk.index_yx(2*i, 3) = a2[i];
        ddelta_xdk.index_yx(2*i+1, 2) = a3[i];
        ddelta_xdk.index_yx(2*i+1, 3) = a1[i];
      }
    }

    DynMatrix<icl64f> xd2 = xd1 + delta_x;

    DynMatrix<icl64f> dxd2dom = dxd1dom + ddelta_xdom ;
    DynMatrix<icl64f> dxd2dT = dxd1dT + ddelta_xdT;

    DynMatrix<icl64f> dxd2dk = dxd1dk + ddelta_xdk ;

    //Add Skew:
    DynMatrix<icl64f> xd3(m_data->bSize,2);
    for(int i=0;i<m_data->bSize;++i){
      xd3.index_yx(0, i) = xd2.index_yx(0, i)+alpha*xd2.index_yx(1, i); xd3.index_yx(1, i)=xd2.index_yx(1, i);
    }
    // Compute: dxd3dom, dxd3dT, dxd3dk, dxd3dalpha
    DynMatrix<icl64f> dxd3dom(3,2*n);
    for(int i=0;i<n;++i){
      dxd3dom.index_yx(2*i, 0) = dxd2dom.index_yx(2*i, 0) +alpha*dxd2dom.index_yx(2*i+1, 0);
      dxd3dom.index_yx(2*i, 1) = dxd2dom.index_yx(2*i, 1) +alpha*dxd2dom.index_yx(2*i+1, 1);
      dxd3dom.index_yx(2*i, 2) = dxd2dom.index_yx(2*i, 2) +alpha*dxd2dom.index_yx(2*i+1, 2);

      dxd3dom.index_yx(2*i+1, 0) = dxd2dom.index_yx(2*i+1, 0);
      dxd3dom.index_yx(2*i+1, 1) = dxd2dom.index_yx(2*i+1, 1);;
      dxd3dom.index_yx(2*i+1, 2) = dxd2dom.index_yx(2*i+1, 2);;
    }
    DynMatrix<icl64f> dxd3dT(3,2*n);
    for(int i=0;i<n;++i){
      dxd3dT.index_yx(2*i, 0) = dxd2dT.index_yx(2*i, 0) +alpha*dxd2dT.index_yx(2*i+1, 0);
      dxd3dT.index_yx(2*i, 1) = dxd2dT.index_yx(2*i, 1) +alpha*dxd2dT.index_yx(2*i+1, 1);
      dxd3dT.index_yx(2*i, 2) = dxd2dT.index_yx(2*i, 2) +alpha*dxd2dT.index_yx(2*i+1, 2);

      dxd3dT.index_yx(2*i+1, 0) = dxd2dT.index_yx(2*i+1, 0);
      dxd3dT.index_yx(2*i+1, 1) = dxd2dT.index_yx(2*i+1, 1);;
      dxd3dT.index_yx(2*i+1, 2) = dxd2dT.index_yx(2*i+1, 2);;
    }

    DynMatrix<icl64f> dxd3dk(5,2*n);
    for(int i=0;i<n;++i){
      dxd3dk.index_yx(2*i, 0) = dxd2dk.index_yx(2*i, 0) +alpha*dxd2dk.index_yx(2*i+1, 0);
      dxd3dk.index_yx(2*i, 1) = dxd2dk.index_yx(2*i, 1) +alpha*dxd2dk.index_yx(2*i+1, 1);
      dxd3dk.index_yx(2*i, 2) = dxd2dk.index_yx(2*i, 2) +alpha*dxd2dk.index_yx(2*i+1, 2);
      dxd3dk.index_yx(2*i, 3) = dxd2dk.index_yx(2*i, 3) +alpha*dxd2dk.index_yx(2*i+1, 3);
      dxd3dk.index_yx(2*i, 4) = dxd2dk.index_yx(2*i, 4) +alpha*dxd2dk.index_yx(2*i+1, 4);

      dxd3dk.index_yx(2*i+1, 0) = dxd2dk.index_yx(2*i+1, 0);
      dxd3dk.index_yx(2*i+1, 1) = dxd2dk.index_yx(2*i+1, 1);
      dxd3dk.index_yx(2*i+1, 2) = dxd2dk.index_yx(2*i+1, 2);
      dxd3dk.index_yx(2*i+1, 3) = dxd2dk.index_yx(2*i+1, 3);
      dxd3dk.index_yx(2*i+1, 4) = dxd2dk.index_yx(2*i+1, 4);
    }
    DynMatrix<icl64f> dxd3dalpha(1,2*n);
    for(int i=0;i<n;++i){
      dxd3dalpha.index_yx(2*i, 0) = xd2.index_yx(1, i);
    }

    //Pixel coordinates:
    if (f.dim()>1){
      DynMatrix<icl64f> o(n,1,1);
      xp = xd3.elementwise_mult(f*o)+c*o;

      DynMatrix<icl64f> coeff(1,2*n);
      for(int i=0;i<n;++i){
        coeff[2*i] = f[0];
        coeff[2*i+1] = f[1];
      }
      DynMatrix<icl64f> o3(3,1,1),o5(5,1,1);
      dxpdom = (coeff*o3).elementwise_mult(dxd3dom);
      dxpdT = (coeff*o3).elementwise_mult(dxd3dT);
      dxpdk = (coeff*o5).elementwise_mult(dxd3dk);
      dxpdalpha = coeff.elementwise_mult(dxd3dalpha);
      for(int i=0;i<n;++i){
        dxpdf.index_yx(2*i, 0) = xd3.index_yx(0, i); dxpdf.index_yx(2*i+1, 0) = 0.0;
        dxpdf.index_yx(2*i+1, 1) = xd3.index_yx(1, i); dxpdf.index_yx(2*i, 1) = 0.0;
      }
    } else {
      DynMatrix<icl64f> o(n,1,1);
      xp = f * xd3 + c*o;

      dxpdom = f  * dxd3dom;
      dxpdT = f * dxd3dT;
      dxpdk = f  * dxd3dk;
      dxpdalpha = f.elementwise_mult(dxd3dalpha);
      for(int i=0;m_data->bSize;++i){
        dxpdf[2*i] = xd3.index_yx(0, i);
        dxpdf[2*i+1] = xd3.index_yx(1, i);
      }
    }

    for(int i=0;i<n;++i){
      dxpdc.index_yx(2*i, 0) = 1; dxpdc.index_yx(2*i+1, 1) = 1;
    }
  }

  void IntrinsicCalibrator::compute_extrinsic_refine(const DynMatrix<icl64f> &omc_init, const DynMatrix<icl64f> &Tc_init,
                                                     const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,const DynMatrix<icl64f> &cc,
                                                     const DynMatrix<icl64f> &kc,const double alpha_c, const int MaxIter, double thresh_cond,
                                                     DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk, DynMatrix<icl64f> &JJ){

    // Initialization:
    omckk[0] = omc_init[0]; omckk[1] = omc_init[1]; omckk[2] = omc_init[2];
    Tckk[0] = Tc_init[0]; Tckk[1] = Tc_init[1]; Tckk[2] = Tc_init[2];

    // Final optimization (minimize the reprojection error in pixel): through Gradient Descent:
    DynMatrix<icl64f> param(1,6);
    param[0] = omckk[0]; param[1] = omckk[1]; param[2] = omckk[2];
    param[3] = Tckk[0]; param[4] = Tckk[1]; param[5] = Tckk[2];
    double change = 1;

    int iter = 0;
    DynMatrix<icl64f> x(m_data->bSize,2), dxdom(3,2*m_data->bSize), dxdT(3,2*m_data->bSize),
    dxdf(2,2*m_data->bSize), dxdc(2,2*m_data->bSize), dxdk(5,2*m_data->bSize), dxdalpha(1,2*m_data->bSize);
    DynMatrix<icl64f> ex(m_data->bSize,2);
    while ((change > 1e-10)&&(iter < MaxIter)){
      project_points2(X_kk,omckk,Tckk,fc,cc,kc,alpha_c, x,dxdom,dxdT,  dxdf,dxdc,dxdk,dxdalpha);
      for(int i=0;i<(2*m_data->bSize);++i)
        ex[i] = x_kk[i] - x[i];

      DynMatrix<icl64f> JJ(6,2*m_data->bSize);
      for(int i=0;i<(2*m_data->bSize);++i){
        JJ.index_yx(i, 0) = dxdom.index_yx(i, 0); JJ.index_yx(i, 1) = dxdom.index_yx(i, 1); JJ.index_yx(i, 2) = dxdom.index_yx(i, 2);
        JJ.index_yx(i, 3) = dxdT.index_yx(i, 0); JJ.index_yx(i, 4) = dxdT.index_yx(i, 1); JJ.index_yx(i, 5) = dxdT.index_yx(i, 2);
      }
      if (JJ.cond() > thresh_cond){
        change = 0;
      } else {
        DynMatrix<icl64f> JJT = JJ.transp();
        DynMatrix<icl64f> JJ2 = JJT*JJ;

        DynMatrix<icl64f> param_innov(1,6), param_up;
        DynMatrix<icl64f> temp = JJ2.inv()*JJT;
        DynMatrix<icl64f> ex2(1,2*ex.cols());
        for(unsigned int i=0;i<ex.cols();++i){
          ex2[2*i] = ex[i];
          ex2[2*i+1] = ex[ex.cols()+i];
        }
        param_innov = temp*ex2;

        param_up = param + param_innov;
        change = param_innov.norm()/param_up.norm();
        param = param_up;
        iter = iter + 1;

        omckk[0] = param[0]; omckk[1] = param[1]; omckk[2] = param[2];
        Tckk[0] = param[3]; Tckk[1] = param[4]; Tckk[2] = param[5];
      }
    }

    DynMatrix<icl64f> dummy(3,9);
    rodrigues(omckk,Rckk,dummy);
  }

  void IntrinsicCalibrator::rodrigues(const DynMatrix<icl64f> &in,DynMatrix<icl64f> &out, DynMatrix<icl64f> &dout){

    int m=in.rows();
    int n=in.cols();
    double eps = 2.2204460492503e-16;
    double bigeps = 10e+20*eps;
    DynMatrix<icl64f> R(3,3), dRdin(3,9);
    DynMatrix<icl64f> eye3(3,3); eye3[0]=1;eye3[4]=1;eye3[8]=1;

    if (((m==1) && (n==3)) || ((m==3) && (n==1))){ //it is a rotation vector
      double theta = in.norm();
      if (theta < eps){
        R[0]=1.0;
        R[4]=1.0;
        R[8]=1.0;

        dRdin[5] = 1.0; dRdin[7] = -1.0; dRdin[11] = -1.0;
        dRdin[15] = 1.0; dRdin[19] = 1.0; dRdin[21] = -1.0;

      } else {
        DynMatrix<icl64f> dm3din(3,4);
        dm3din[0] = 1.0; dm3din[4] = 1.0; dm3din[8] = 1.0;
        dm3din[9] = in[0]/theta; dm3din[10] = in[1]/theta; dm3din[11] = in[2]/theta;

        DynMatrix<icl64f> omega(1,3);
        omega[0]= in[0]/theta; omega[1]= in[1]/theta; omega[2]= in[2]/theta;

        DynMatrix<icl64f> dm2dm3(4,4);
        dm2dm3[0] = 1.0/theta; dm2dm3[5] = 1.0/theta; dm2dm3[10] = 1.0/theta; dm2dm3[15] = 1;
        dm2dm3[3] = -in[0]/(theta*theta); dm2dm3[7] = -in[1]/(theta*theta); dm2dm3[11] = -in[2]/(theta*theta);

        double alpha = cos(theta);
        double beta = sin(theta);
        double gamma = 1-alpha;
        DynMatrix<icl64f> omegav(3,3);
        omegav[0] = 0.0; omegav[1] = -omega[2]; omegav[2] = omega[1];
        omegav[3] = omega[2]; omegav[4] = 0.0; omegav[5] = -omega[0];
        omegav[6] = -omega[1]; omegav[7] = omega[0]; omegav[8] = 0.0;
        DynMatrix<icl64f> A = omega*omega.transp();

        DynMatrix<icl64f> dm1dm2(4,21);
        dm1dm2.index_yx(0, 3) = -sin(theta);
        dm1dm2.index_yx(1, 3) = cos(theta);
        dm1dm2.index_yx(2, 3) = -dm1dm2.index_yx(0, 3);

        dm1dm2[18] = 1.0; dm1dm2[21] = -1.0; dm1dm2[26] = -1.0;
        dm1dm2[32] = 1.0; dm1dm2[37] = 1.0; dm1dm2[40] = -1.0;
        double w1 = omega[0];
        double w2 = omega[1];
        double w3 = omega[2];

        dm1dm2.index_yx(12, 0) = 2*w1; dm1dm2.index_yx(13, 0) = w2; dm1dm2.index_yx(14, 0) = w3; dm1dm2.index_yx(15, 0) = w2; dm1dm2.index_yx(18, 0) = w3;
        dm1dm2.index_yx(13, 1) = w1; dm1dm2.index_yx(15, 1) = w1; dm1dm2.index_yx(16, 1) = 2*w2; dm1dm2.index_yx(17, 1) = w3; dm1dm2.index_yx(19, 1) = w3;
        dm1dm2.index_yx(14, 2) = w1; dm1dm2.index_yx(17, 2) = w2; dm1dm2.index_yx(18, 2) = w1; dm1dm2.index_yx(19, 2) = w2; dm1dm2.index_yx(20, 2) = 2*w3;

        DynMatrix<icl64f> temp1;
        eye3.mult(alpha,temp1);
        DynMatrix<icl64f> temp2;
        omegav.mult(beta,temp2);
        DynMatrix<icl64f> temp3;
        A.mult(gamma,temp3);
        R=temp1+temp2+temp3;
        DynMatrix<icl64f> dRdm1(21,9);

        dRdm1.index_yx(0, 0) = 1; dRdm1.index_yx(4, 0) = 1; dRdm1.index_yx(8, 0) = 1;
        DynMatrix<icl64f> omegav_T = omegav.transp();
        for(int i=0;i<9;++i){
          dRdm1.index_yx(i, 1) = omegav_T[i];
        }
        dRdm1.index_yx(0, 3) = beta; dRdm1.index_yx(1, 4) = beta; dRdm1.index_yx(2, 5) = beta;
        dRdm1.index_yx(3, 6) = beta; dRdm1.index_yx(4, 7) = beta; dRdm1.index_yx(5, 8) = beta;
        dRdm1.index_yx(6, 9) = beta; dRdm1.index_yx(7, 10) = beta; dRdm1.index_yx(8, 11) = beta;
        for(int i=0;i<9;++i){
          dRdm1.index_yx(i, 2) = A[i];
        }
        dRdm1.index_yx(0, 12) = gamma; dRdm1.index_yx(1, 13) = gamma; dRdm1.index_yx(2, 14) = gamma;
        dRdm1.index_yx(3, 15) = gamma; dRdm1.index_yx(4, 16) = gamma; dRdm1.index_yx(5, 17) = gamma;
        dRdm1.index_yx(6, 18) = gamma; dRdm1.index_yx(7, 19) = gamma; dRdm1.index_yx(8, 20) = gamma;

        dRdin = dRdm1 * dm1dm2 * dm2dm3 * dm3din;

      }
      out = R;
      dout = dRdin;
    } else if ((m==n) && (m==3) && ( (in.transp() * in - eye3).norm() < bigeps) && (abs(in.det()-1) < bigeps)){
      R = in;
      //project the rotation matrix to SO(3);
      DynMatrix<icl64f> U,S,V;
      R.svd(U,S,V);
      R = U*V.transp();
      double tr = (R.trace()-1)/2;
      DynMatrix<icl64f> dtrdR(9,1);
      dtrdR[0] = 0.5;dtrdR[4] = 0.5;dtrdR[8] = 0.5;
      double theta = std::acos(tr);
      if (sin(theta) >= 1e-4){
        double dthetadtr = -1/sqrt(1-tr*tr);

        DynMatrix<icl64f> dthetadR;
        dtrdR.mult(dthetadtr,dthetadR);
        double vth = 1/(2*sin(theta));
        double dvthdtheta = -vth*cos(theta)/sin(theta);
        DynMatrix<icl64f> dvar1dtheta(1,2);
        dvar1dtheta[0] = dvthdtheta; dvar1dtheta[0] = 1;
        DynMatrix<icl64f> dvar1dR =  dvar1dtheta * dthetadR;


        DynMatrix<icl64f> om1(1,3);
        om1[0] = R.index_yx(2, 1)-R.index_yx(1, 2); om1[1] = R.index_yx(0, 2)-R.index_yx(2, 0); om1[2] = R.index_yx(1, 0)-R.index_yx(0, 1);


        DynMatrix<icl64f> dvardR(9,5);
        dvardR[5] = 1; dvardR[7] = -1; dvardR[11] = -1; dvardR[15] = 1; dvardR[19] = 1; dvardR[21] = -1;
        for(int i=0;i<18;++i){
          dvardR[27+i] = dvar1dR[i];
        }

        DynMatrix<icl64f> om;
        om1.mult(vth,om);
        DynMatrix<icl64f> dvar2dvar(5,4);
        dvar2dvar[0] = vth; dvar2dvar[6] = vth; dvar2dvar[11] = vth; dvar2dvar[19] = 1;
        dvar2dvar[3] = om1[0]; dvar2dvar[8] = om1[1]; dvar2dvar[13] = om1[2];
        out = om*theta;
        DynMatrix<icl64f> domegadvar2(4,3);
        domegadvar2[0] = 1; domegadvar2[5] = 1; domegadvar2[10] = 1;
        domegadvar2[3] = om[0]; domegadvar2[7] = om[1];domegadvar2[11] = om[2];
        dout = domegadvar2 * dvar2dvar * dvardR;

      } else {
        std::cout << "ohoh\n";
        if (tr > 0){
          //case norm(om)=0;
          out[0]=0;out[1]=0;out[2]=0;
          dout[5] = 0.5; dout[7] = -0.5; dout[11] = -0.5; dout[15] = 0.5; dout[19] = 0.5; dout[21] = -0.5;
        } else{
          //case norm(om)=pi;

          // Define hashvec and Smat
          DynMatrix<icl64f> hashvec(1,11);// = [0; -1; -3; -9; 9; 3; 1; 13; 5; -7; -11];
          hashvec[0] =0; hashvec[1] =-1; hashvec[2] =-3; hashvec[3] =-9; hashvec[4] =9;
          hashvec[5] =3; hashvec[6] =1; hashvec[7] =13; hashvec[8] =5; hashvec[9] =-7; hashvec[10] =-11;
          double dat[33] = {1,1,1, 1,0,-1, 0,1,-1, 1,-1,0, 1,1,0, 0,1,1, 1,0,1, 1,1,1, 1,1,-1,1,-1,-1, 1,-1,1};
          DynMatrix<icl64f> Smat(3,11,dat);

          DynMatrix<icl64f> M = (R+eye3)/2.0;
          double uabs = sqrt(M.index_yx(0, 0));
          double vabs = sqrt(M.index_yx(1, 1));
          double wabs = sqrt(M.index_yx(2, 2));

          DynMatrix<icl64f> mvec(3,1);// = [M(1,2), M(2,3), M(1,3)];
          mvec[0] = M.index_yx(0, 1); mvec[1] = M.index_yx(1, 2); mvec[2] = M.index_yx(0, 2);
          DynMatrix<icl64f> syn(3,1);//  = ((mvec > 1e-4) - (mvec < -1e-4)); //robust sign() function

          for(int i=0;i<3;++i){
            if(mvec[i]>1e-4){
              syn[i] = 1.0;
            } else {
              syn[i] = 0.0;
            }
            if(mvec[i]<1e-4){
              syn[i] = syn[i]-1.0;
            }
          }

          DynMatrix<icl64f> hmm(1,3); hmm[0] = 9; hmm[1] = 3; hmm[2] = 1;
          DynMatrix<icl64f> hash = syn *hmm;

          unsigned int idx = 0.0;
          for(unsigned int i=0;i<hashvec.dim();++i){
            if(hash[0] == hashvec[i]){
              idx = i;
            }
          }
          DynMatrix<icl64f> svec(1,3);
          for(unsigned int i=0;i<3;++i){
            svec[i] = hashvec.index_yx(idx, i);
          }
          out[0] = theta*uabs*svec[0];
          out[1] = theta*vabs*svec[1];
          out[2] = theta*wabs*svec[2];
        }
      }
    } else {
      std::cout << "Warning: Not a rotation matrix" << std::endl;
    }
  }

  void IntrinsicCalibrator::comp_distortion_oulu(const DynMatrix<icl64f> xd, const DynMatrix<icl64f> k, DynMatrix<icl64f> &x){

    double k1 = k[0];
    double k2 = k[1];
    double k3 = k[4];
    double p1 = k[2];
    double p2 = k[3];
    //initial guess
    x = xd;
    DynMatrix<icl64f> ones(1,2,1);
    DynMatrix<icl64f> ones1(m_data->bSize,1,1);
    DynMatrix<icl64f> k_radial(m_data->bSize,1);
    DynMatrix<icl64f> delta_x;

    for (int kk=0;kk<20;++kk){
      DynMatrix<icl64f> r_2(m_data->bSize,1);
      DynMatrix<icl64f> r_21(m_data->bSize,1);
      DynMatrix<icl64f> r_22(m_data->bSize,1);
      DynMatrix<icl64f> r_23(m_data->bSize,1);
      for(int i=0;i<m_data->bSize;++i){
        r_2.index_yx(0, i) = x.index_yx(0, i)*x.index_yx(0, i)+x.index_yx(1, i)*x.index_yx(1, i);
        r_21.index_yx(0, i) = r_2.index_yx(0, i);
        r_22.index_yx(0, i) = r_2.index_yx(0, i)*r_2.index_yx(0, i);
        r_23.index_yx(0, i) = r_22.index_yx(0, i)*r_2.index_yx(0, i);
      }

      r_21 *= k1;
      r_22 *= k2;
      r_23 *= k3;
      k_radial = ones1+r_21+r_22+r_23;

      DynMatrix<icl64f> delta_x(m_data->bSize,2);

      for(int i=0;i<m_data->bSize;++i){
        delta_x.index_yx(0, i) = 2*p1*x.index_yx(0, i)*x.index_yx(1, i) + p2*(r_2[i] + 2*x.index_yx(0, i)*x.index_yx(0, i));
        delta_x.index_yx(1, i) = p1 * (r_2[i] + 2*x.index_yx(1, i)*x.index_yx(1, i))+2*p2*x.index_yx(0, i)*x.index_yx(1, i);
      }

      for(unsigned int i=0;i<x.dim();++i){
        x[i] = xd[i] - delta_x[i];
      }
      DynMatrix<icl64f> k_rad2 = ones*k_radial;
      for(unsigned int i=0;i<x.dim();++i){
        x[i] = x[i]/k_rad2[i];
      }
    }
  }

  void IntrinsicCalibrator::normalize_pixel(const DynMatrix<icl64f> x_kk, const DynMatrix<icl64f> fc, const DynMatrix<icl64f> cc,
                                            const DynMatrix<icl64f> kc,const double alpha_c, DynMatrix<icl64f> &xn){

    //First: Subtract principal point, and divide by the focal length:
    DynMatrix<icl64f> x_distort(m_data->bSize,2);
    for(int i=0;i<m_data->bSize;++i){
      x_distort.index_yx(0, i) = (x_kk.index_yx(0, i)-cc[0])/fc[0];
      x_distort.index_yx(1, i) = (x_kk.index_yx(1, i)-cc[1])/fc[1];
    }
    //Second: undo skew
    for(int i=0;i<m_data->bSize;++i){
      x_distort.index_yx(0, i) = x_distort.index_yx(0, i) -alpha_c*x_distort.index_yx(1, i);
    }
    if (kc.norm() != 0){
      //Third: Compensate for lens distortion:
      comp_distortion_oulu(x_distort,kc, xn);
    }else{
      xn = x_distort;
    }
  }

  void IntrinsicCalibrator::mean(const DynMatrix<icl64f> &x_k, DynMatrix<icl64f> &res){
    double mean = 0.0;
    if(x_k.rows()>1){
      res.setBounds(x_k.cols(),1);
      for(unsigned int j=0;j<x_k.cols();++j)
        for(unsigned int i=0;i<x_k.rows();++i){
          res[j] += x_k.index_yx(i, j);
        }

      for(unsigned int i=0;i<res.cols();++i){
        res[i] = res[i]/x_k.rows();
      }
    } else {
      res.setBounds(1,1);
      for(unsigned int i=0;i<x_k.cols();++i){
        mean += x_k[i];
      }
      res[0]=mean/x_k.cols();
    }
  }

  void IntrinsicCalibrator::compute_homography(const DynMatrix<icl64f> &m, const DynMatrix<icl64f> &M, DynMatrix<icl64f> &H){

    int Np = m_data->bSize;
    DynMatrix<icl64f> mm(m_data->bSize,3);
    if(m.rows()<3){
      for(int i=0;i<m_data->bSize;++i){
        mm.index_yx(0, i) = m.index_yx(0, i);
        mm.index_yx(1, i) = m.index_yx(1, i);
        mm.index_yx(2, i) = 1;
      }
    } else {
      for(int i=0;i<m_data->bSize;++i){
        mm.index_yx(0, i) = m.index_yx(0, i);
        mm.index_yx(1, i) = m.index_yx(1, i);
        mm.index_yx(2, i) = m.index_yx(2, i);
      }
    }
    DynMatrix<icl64f> MM(m_data->bSize,3);
    if(M.rows()<3){
      for(int i=0;i<m_data->bSize;++i){
        MM.index_yx(0, i) = M.index_yx(0, i);
        MM.index_yx(1, i) = M.index_yx(1, i);
        MM.index_yx(2, i) = 1;
      }
    } else {
      for(int i=0;i<m_data->bSize;++i){
        MM.index_yx(0, i) = M.index_yx(0, i);
        MM.index_yx(1, i) = M.index_yx(1, i);
        if(M.index_yx(2, i) == 0){
          MM.index_yx(2, i) = 1;
        } else {
          MM.index_yx(2, i) = M.index_yx(2, i);
        }
      }
    }
    for(int i=0;i<m_data->bSize;++i){
      mm.index_yx(0, i) = mm.index_yx(0, i) * 1.0/mm.index_yx(2, i);
      mm.index_yx(1, i) = mm.index_yx(1, i) * 1.0/mm.index_yx(2, i);

      MM.index_yx(0, i) = MM.index_yx(0, i) * 1.0/MM.index_yx(2, i);
      MM.index_yx(1, i) = MM.index_yx(1, i) * 1.0/MM.index_yx(2, i);
    }
    // Prenormalization of point coordinates (very important):
    // (Affine normalization)
    DynMatrix<icl64f> ax(m_data->bSize,1);
    DynMatrix<icl64f> ay(m_data->bSize,1);
    for(int i=0;i<m_data->bSize;++i){
      ax[i] = mm.index_yx(0, i);
      ay[i] = mm.index_yx(1, i);
    }
    DynMatrix<icl64f> mxx(1,1);
    mean(ax,mxx);
    DynMatrix<icl64f> myy(1,1);
    mean(ay,myy);
    for(int i=0;i<m_data->bSize;++i){
      ax[i] = ax[i] - mxx[0];
      if(ax[i]<0){
        ax[i] = -ax[i];
      }
      ay[i] = ay[i] - myy[0];
      if(ay[i]<0){
        ay[i] = -ay[i];
      }
    }
    DynMatrix<icl64f> scxx(1,1);
    mean(ax,scxx);
    DynMatrix<icl64f> scyy(1,1);
    mean(ay,scyy);

    DynMatrix<icl64f> Hnorm(3,3);
    Hnorm[0] = 1.0/scxx[0]; Hnorm[2] = -mxx[0]/scxx[0];
    Hnorm[4] = 1.0/scyy[0]; Hnorm[5] = -myy[0]/scyy[0];
    Hnorm[8] = 1.0;
    DynMatrix<icl64f> inv_Hnorm(3,3);
    inv_Hnorm[0] = scxx[0]; inv_Hnorm[2] = mxx[0];
    inv_Hnorm[4] = scyy[0]; inv_Hnorm[5] = myy[0];
    inv_Hnorm[8] = 1.0;
    DynMatrix<icl64f> mn = Hnorm*mm;
    //Compute the homography between m and mn:
    //Build the matrix:

    DynMatrix<icl64f> L(9,2*m_data->bSize);
    DynMatrix<icl64f> ones(1,3,1);
    for(int i=0;i<Np;++i){
      L.index_yx(2*i, 0) = MM.index_yx(0, i);
      L.index_yx(2*i, 1) = MM.index_yx(1, i);
      L.index_yx(2*i, 2) = MM.index_yx(2, i);
      L.index_yx(2*i+1, 3) = MM.index_yx(0, i);
      L.index_yx(2*i+1, 4) = MM.index_yx(1, i);
      L.index_yx(2*i+1, 5) = MM.index_yx(2, i);
      L.index_yx(2*i, 6) = -mn.index_yx(0, i)*MM.index_yx(0, i);
      L.index_yx(2*i, 7) = -mn.index_yx(0, i)*MM.index_yx(1, i);
      L.index_yx(2*i, 8) = -mn.index_yx(0, i)*MM.index_yx(2, i);
      L.index_yx(2*i+1, 6) = -mn.index_yx(1, i)*MM.index_yx(0, i);
      L.index_yx(2*i+1, 7) = -mn.index_yx(1, i)*MM.index_yx(1, i);
      L.index_yx(2*i+1, 8) = -mn.index_yx(1, i)*MM.index_yx(2, i);
    }

    if (Np > 4){
      L = L.transp()*L;
    }

    DynMatrix<icl64f> U,S,V;
    L.svd(U,S,V);
    DynMatrix<icl64f> hh(1,9);

    for(int i=0;i<9;++i){
      hh[i] = V.index_yx(i, 8)/V.index_yx(8, 8);
    }
    DynMatrix<icl64f> hhh(3,3);
    hhh[0] = hh[0]; hhh[1] = hh[1]; hhh[2] = hh[2];
    hhh[3] = hh[3]; hhh[4] = hh[4]; hhh[5] = hh[5];
    hhh[6] = hh[6]; hhh[7] = hh[7]; hhh[8] = hh[8];

    DynMatrix<icl64f> Hrem = hhh;
    //Final homography:
    H = inv_Hnorm*Hrem;

    //Homography refinement if there are more than 4 points:
    if (Np > 4){
      //Final refinement:
      DynMatrix<icl64f> hhv(1,8);
      hhv[0] = H[0]; hhv[1] = H[1]; hhv[2] = H[2]; hhv[3] = H[3];
      hhv[4] = H[4]; hhv[5] = H[5]; hhv[6] = H[6]; hhv[7] = H[7];
      DynMatrix<icl64f> mrep(m_data->bSize,3), MMM(m_data->bSize,3);
      DynMatrix<icl64f> J(8,2*Np);
      DynMatrix<icl64f> ones(1,3,1);
      DynMatrix<icl64f> m_err(1,2*Np);
      for(int iter=0;iter<10;++iter){
        MMM = DynMatrix<icl64f>(m_data->bSize,3);
        mrep = H * MM;
        for(int i=0;i<m_data->bSize;++i){
          MMM.index_yx(0, i)=MM.index_yx(0, i)/mrep.index_yx(2, i);
          MMM.index_yx(1, i)=MM.index_yx(1, i)/mrep.index_yx(2, i);
          MMM.index_yx(2, i)=MM.index_yx(2, i)/mrep.index_yx(2, i);
        }
        for(int i=0;i<Np;++i){
          J.index_yx(2*i, 0) = MMM.index_yx(0, i);
          J.index_yx(2*i, 1) = MMM.index_yx(1, i);
          J.index_yx(2*i, 2) = MMM.index_yx(2, i);
          J.index_yx(2*i+1, 3) = MMM.index_yx(0, i);
          J.index_yx(2*i+1, 4) = MMM.index_yx(1, i);
          J.index_yx(2*i+1, 5) = MMM.index_yx(2, i);
        }
        J *= -1;
        for(unsigned int i=0;i<mrep.cols();++i){
          mrep.index_yx(0, i) = mrep.index_yx(0, i)/mrep.index_yx(2, i);
          mrep.index_yx(1, i) = mrep.index_yx(1, i)/mrep.index_yx(2, i);
          mrep.index_yx(2, i) = mrep.index_yx(2, i)/mrep.index_yx(2, i);
        }
        for(int i=0;i<Np;++i){
          m_err.index_yx(2*i, 0) = mm.index_yx(0, i)-mrep.index_yx(0, i);
          m_err.index_yx(2*i+1, 0) = mm.index_yx(1, i)-mrep.index_yx(1, i);
        }

        DynMatrix<icl64f> mrep_t = ones*mrep.row(0);
        DynMatrix<icl64f> MMM2 = mrep_t.elementwise_mult(MMM);

        mrep_t = ones*mrep.row(1);
        DynMatrix<icl64f> MMM3 = mrep_t.elementwise_mult(MMM);

        for(int i=0;i<Np;++i){
          J.index_yx(2*i, 6) = MMM2.index_yx(0, i);
          J.index_yx(2*i, 7) = MMM2.index_yx(1, i);
          J.index_yx(2*i+1, 6) = MMM3.index_yx(0, i);
          J.index_yx(2*i+1, 7) = MMM3.index_yx(1, i);
        }

        for(int i=0;i<m_data->bSize;++i){
          MMM.index_yx(0, i) = MM.index_yx(0, i)/mrep.index_yx(2, i);
          MMM.index_yx(1, i) = MM.index_yx(1, i)/mrep.index_yx(2, i);
          MMM.index_yx(2, i) = MM.index_yx(2, i)/mrep.index_yx(2, i);
        }
        MMM = MMM.transp();
        DynMatrix<icl64f> JT = J.transp();
        DynMatrix<icl64f> hh_innov  = (JT*J).inv()*JT*m_err;
        DynMatrix<icl64f> hhv_up = hhv - hh_innov;
        hhv = hhv_up;

        for(int i=0;i<8;++i){
          H[i] = hhv_up[i];
        }
      }
    }
  }

  void IntrinsicCalibrator::compute_extrinsic_init(const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,
                                                   const DynMatrix<icl64f> &cc, const DynMatrix<icl64f> &kc, const double &alpha_c,
                                                   DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk){

    // Compute the normalized coordinates:
    DynMatrix<icl64f> xn(m_data->bSize,2);
    normalize_pixel(x_kk,fc,cc,kc,alpha_c, xn);

    // Check for planarity of the structure:
    DynMatrix<icl64f> X_mean(1,3);
    mean(X_kk.transp(),X_mean);
    DynMatrix<icl64f> ones(m_data->bSize,1,1);
    DynMatrix<icl64f> Y(m_data->bSize,3);
    DynMatrix<icl64f> temp = X_mean.transp()*ones;
    for(unsigned int i=0;i<Y.dim();++i){
      Y[i]= X_kk[i]-temp[i];
    }
    DynMatrix<icl64f> YYT = Y*Y.transp();

    DynMatrix<icl64f> U,S,V;
    YYT.svd(U,S,V);
    double r = S[2]/S[1];

    if ((r < 1e-3) || (m_data->bSize < 5)){
      // Transform the plane to bring it in the Z=0 plane:
      DynMatrix<icl64f> R_transform = V;
      if (std::sqrt(R_transform[2]*R_transform[2]+R_transform[5]*R_transform[5]) < 1e-6){
        R_transform = DynMatrix<icl64f>(3,3);
        R_transform[0] = 1;
        R_transform[4] = 1;
        R_transform[8] = 1;
      }

      if (R_transform.det() < 0){
        R_transform *= -1;
      }
      DynMatrix<icl64f> T_transform = (R_transform)*X_mean.transp();
      T_transform *= -1;
      DynMatrix<icl64f> X_new = R_transform*X_kk + T_transform*ones;

      // Compute the planar homography:
      DynMatrix<icl64f> H(3,3), X_new2(X_new.cols(),2);
      for(unsigned int i=0;i<X_new.cols();++i){
        X_new2.index_yx(0, i) = X_new.index_yx(0, i);
        X_new2.index_yx(1, i) = X_new.index_yx(1, i);
      }

      compute_homography(xn,X_new2, H);
      // De-embed the motion parameters from the homography:

      double sc = (std::sqrt(H[0]*H[0]+H[3]*H[3]+H[6]*H[6]) + std::sqrt(H[1]*H[1]+H[4]*H[4]+H[7]*H[7]))/2.0;
      H *= 1.0/sc;

      // Extra normalization for some reasons...
      DynMatrix<icl64f> dummy(3,9);
      DynMatrix<icl64f> u1(1,3);
      u1[0] = H[0]; u1[1] = H[3]; u1[2] = H[6];
      u1 *= (1.0/u1.norm());
      DynMatrix<icl64f> u2(1,3);
      u2[0] = H[1]; u2[1] = H[4]; u2[2] = H[7];
      double u22 = (u1.transp()*u2)[0];
      u2[0] = u2[0]-u22*u1[0];
      u2[1] = u2[1]-u22*u1[1];
      u2[2] = u2[2]-u22*u1[2];
      double n2 =1.0/u2.norm();
      u2[0] = u2[0]*n2; u2[1] = u2[1]*n2; u2[2] = u2[2]*n2;

      DynMatrix<icl64f> u3 = DynMatrix<icl64f>::cross(u1,u2);
      DynMatrix<icl64f> RRR(3,3);
      RRR[0] = u1[0]; RRR[1] = u2[0]; RRR[2] = u3[0];
      RRR[3] = u1[1]; RRR[4] = u2[1]; RRR[5] = u3[1];
      RRR[6] = u1[2]; RRR[7] = u2[2]; RRR[8] = u3[2];
      rodrigues(RRR, omckk,dummy);
      rodrigues(omckk,Rckk,dummy);
      Tckk[0] = H[2]; Tckk[1] = H[5]; Tckk[2] = H[8];

      Tckk = Tckk + Rckk* T_transform;
      Rckk = Rckk * R_transform;
      rodrigues(Rckk,omckk,dummy);
      rodrigues(omckk,Rckk,dummy);
    }
  }

  IntrinsicCalibrator::Result IntrinsicCalibrator::calibrate(const DynMatrix<icl64f> &impoints, const DynMatrix<icl64f> &worldpoints){

    DynMatrix<icl64f> fc(1,2),cc(1,2),kc(5,1);
    double alpha_c = 0;
    init_intrinsic_param(impoints,worldpoints,fc,cc,kc,alpha_c);

    DynMatrix<icl64f> omckk(1,3),Tckk(1,3),Rckk(3,3);
    DynMatrix<icl64f> x(m_data->bSize,2);
    int offset = 10;
    double *params = new double[offset+m_data->successes*6];
    params[0] = fc[0]; params[1] = fc[1];
    params[2] = cc[0]; params[3] = cc[1];
    params[4] = alpha_c;
    params[5] = params[6] = params[7] = params[8] = params[9] = 0.0;
    //compute extrinsic params
    for(int i=0;i<m_data->successes;++i){
      for(int j=0;j<m_data->bSize;++j){
        x.index_yx(0, j) = impoints.index_yx(2*i, j);
        x.index_yx(1, j) = impoints.index_yx(2*i+1, j);
      }
      comp_ext_calib(x,worldpoints,fc,cc,kc,alpha_c,1e6,omckk,Tckk,Rckk);
      for(int j=0;j<3;++j){
        params[offset+i*6+j] = omckk[j];
        params[offset+i*6+3+j] = Tckk[j];
      }
    }
    optimize(impoints,worldpoints,params);
    return m_calres;
  }

  void IntrinsicCalibrator::optimize(const DynMatrix<icl64f> &impoints, const DynMatrix<icl64f> &X_kk,double *pp){

    int offset = 5+(*m_data->distortion_coeffs).cols();
    int paramcount = offset+6*m_data->successes;

    double *params = new double[paramcount];
    //order of params: au, av, u0, v0, sk, wx, wy, wz, tx, ty, tz

    params = pp;
    bool recompute_extrinsic = true;
    int check_cond = 1;
    int MaxIter = 130;
    int iter = 0;
    double change = 1.0;
    DynMatrix<icl64f> param(1,paramcount,params);
    DynMatrix<icl64f> param_up(1,paramcount);
    DynMatrix<icl64f> f(1,2);
    DynMatrix<icl64f> c(1,2);
    DynMatrix<icl64f> k(1,5);
    DynMatrix<icl64f> kc_current(1,5);
    double alpha_current = 0.0;
    double alpha = 0.0;
    double alpha_smooth = 0.1;
    double thresh_cond = 1e6;

    while ( (change > 1e-9) && (iter < MaxIter) ){

      f[0] = param[0];
      f[1] = param[1];
      c[0] = param[2];
      c[1] = param[3];
      alpha = param[4];

      k[0] = param[5];
      k[1] = param[6];
      k[2] = param[7];
      k[3] = param[8];
      k[4] = param[9];

      DynMatrix<icl64f> JJ3(offset+6*m_data->successes,offset+6*m_data->successes);

      DynMatrix<icl64f> ex3(1,offset+6*m_data->successes);

      DynMatrix<icl64f> omckk(1,3);
      DynMatrix<icl64f> Tckk(1,3);
      DynMatrix<icl64f> exkk;
      for(int kk=0;kk<m_data->successes;++kk){
        omckk[0] = param[offset+6*kk];
        omckk[1] = param[offset+6*kk+1];
        omckk[2] = param[offset+6*kk+2];

        Tckk[0] = param[offset+6*kk+3];
        Tckk[1] = param[offset+6*kk+4];
        Tckk[2] = param[offset+6*kk+5];

        //image coords
        DynMatrix<icl64f> x_kk(m_data->bSize,2);
        for(int i=0;i<m_data->bSize;++i){
          x_kk.index_yx(0, i) = impoints.index_yx(2*kk, i);
          x_kk.index_yx(1, i) = impoints.index_yx(2*kk+1, i);
        }

        DynMatrix<icl64f> x(m_data->bSize,2), dxdom(3,2*m_data->bSize), dxdT(3,2*m_data->bSize),
        dxdf(2,2*m_data->bSize), dxdc(2,2*m_data->bSize), dxdk(5,2*m_data->bSize), dxdalpha(1,2*m_data->bSize);

        project_points2(X_kk,omckk,Tckk,f,c,k,alpha,   x,dxdom,dxdT,dxdf,dxdc,dxdk,dxdalpha);

        exkk = x_kk - x;

        DynMatrix<icl64f> A(2*m_data->bSize,10);
        for(int i=0;i<(2*m_data->bSize);++i){
          A.index_yx(0, i) = dxdf.index_yx(i, 0);
          A.index_yx(1, i) = dxdf.index_yx(i, 1);
          A.index_yx(2, i) = dxdc.index_yx(i, 0);
          A.index_yx(3, i) = dxdc.index_yx(i, 1);
          A.index_yx(4, i) = dxdalpha.index_yx(i, 0);
          A.index_yx(5, i) = dxdk.index_yx(i, 0);
          A.index_yx(6, i) = dxdk.index_yx(i, 1);
          A.index_yx(7, i) = dxdk.index_yx(i, 2);
          A.index_yx(8, i) = dxdk.index_yx(i, 3);
          A.index_yx(9, i) = dxdk.index_yx(i, 4);
        }

        DynMatrix<icl64f> B(2*m_data->bSize,6);
        for(int i=0;i<(2*m_data->bSize);++i){
          B.index_yx(0, i) = dxdom.index_yx(i, 0);
          B.index_yx(1, i) = dxdom.index_yx(i, 1);
          B.index_yx(2, i) = dxdom.index_yx(i, 2);
          B.index_yx(3, i) = dxdT.index_yx(i, 0);
          B.index_yx(4, i) = dxdT.index_yx(i, 1);
          B.index_yx(5, i) = dxdT.index_yx(i, 2);
        }

        DynMatrix<icl64f> AAT = A*A.transp();
        for(int i=0;i<10;++i){
          for(int j=0;j<10;++j){
            JJ3.index_yx(j, i) = JJ3.index_yx(j, i) + AAT.index_yx(j, i);
          }
        }

        DynMatrix<icl64f> BBT = B*B.transp();
        for(int i=0;i<6;++i){
          for(int j=0;j<6;++j){
            JJ3.index_yx(offset+6*kk +j, offset+6*kk + i) = BBT.index_yx(j, i);
          }
        }

        DynMatrix<icl64f> AB = A*B.transp();
        for(int i=0;i<6;++i){
          for(int j=0;j<10;++j){
            JJ3.index_yx(j, offset+6*kk + i) = AB.index_yx(j, i);
          }
        }

        DynMatrix<icl64f> ABT = (AB).transp();
        for(int i=0;i<10;++i){
          for(int j=0;j<6;++j){
            JJ3.index_yx(offset+6*kk +j, i) = ABT.index_yx(j, i);
          }
        }

        DynMatrix<icl64f> exkk2(1,2*exkk.cols());
        for(unsigned int i=0;i<exkk.cols();++i){
          exkk2[2*i] = exkk.index_yx(0, i);
          exkk2[2*i+1] = exkk.index_yx(1, i);
        }

        DynMatrix<icl64f> exkk3 = A*exkk2;
        for(int i=0;i<10;++i){
          ex3[i] = ex3[i] + exkk3[i];
        }
        DynMatrix<icl64f> exkk4 = B*exkk2;

        for(int i=0;i<6;++i){
          ex3[offset+6*kk+i] = exkk4[i];
        }
        // Check if this view is ill-conditioned:
        if (check_cond){
          DynMatrix<icl64f> JJ_kk = B.transp(); //%[dxdom dxdT];
          if (JJ_kk.cond()> thresh_cond){
            std::cout << "Warning: View is ill-conditioned" << std::endl;
          }
        }
      }


      //Smoothing coefficient:
      double alpha_smooth2 = 1-pow((1-alpha_smooth),(iter+1));
      //DynMatrix<icl64f> JJ2_inv = JJ3.pinv(true);
      DynMatrix<icl64f> param_innov = JJ3.solve(ex3);//JJ2_inv*ex3;
      param_innov.mult(alpha_smooth2,param_innov);

      param_up = param + param_innov;

      for(int i=0;i<paramcount;++i)
        param[i] = param_up[i];

      //New intrinsic parameters:
      DynMatrix<icl64f> fc_current(1,2);
      fc_current[0] = param[0];
      fc_current[1] = param[1];
      DynMatrix<icl64f> cc_current(1,2);
      cc_current[0] = param[2];
      cc_current[1] = param[3];

      alpha_current = param[4];
      kc_current[0] = param[5];
      kc_current[1] = param[6];
      kc_current[2] = param[7];
      kc_current[3] = param[8];
      kc_current[4] = param[9];

      //Change on the intrinsic parameters:
      DynMatrix<icl64f> mat(1,4);
      mat[0] = fc_current[0];
      mat[1] = fc_current[1];
      mat[2] = cc_current[0];
      mat[3] = cc_current[1];

      DynMatrix<icl64f> mat2(1,4);
      mat2[0] = f[0];
      mat2[1] = f[1];
      mat2[2] = c[0];
      mat2[3] = c[1];

      DynMatrix<icl64f> mat3 = mat-mat2;
      change=mat3.norm()/mat.norm();

      //Second step: (optional) - It makes convergence faster, and the region of convergence LARGER!!!
      //Recompute the extrinsic parameters only using compute_extrinsic.m (this may be useful sometimes)
      //The complete gradient descent method is useful to precisely update the intrinsic parameters.
      if (recompute_extrinsic){
        int MaxIter2 = 20;
        for (int kk=0;kk<m_data->successes;++kk){

          DynMatrix<icl64f> omc_current(1,3);
          omc_current[0] = param[offset+6*kk];
          omc_current[1] = param[offset+6*kk+1];
          omc_current[2] = param[offset+6*kk+2];

          DynMatrix<icl64f> Tc_current(1,3);
          Tc_current[0] = param[offset+6*kk+3];
          Tc_current[1] = param[offset+6*kk+4];
          Tc_current[2] = param[offset+6*kk+5];

          DynMatrix<icl64f> Rckk(3,3);
          DynMatrix<icl64f> x_kk(m_data->bSize,2);
          for(int i=0;i<m_data->bSize;++i){
            x_kk.index_yx(0, i) = impoints.index_yx(2*kk, i);
            x_kk.index_yx(1, i) = impoints.index_yx(2*kk+1, i);
          }
          DynMatrix<icl64f> omckk(1,3),Tckk(1,3),JJ_kk(6,2*m_data->bSize);
          compute_extrinsic_init(x_kk,X_kk,fc_current,cc_current,kc_current,alpha_current,
                                 omc_current,Tc_current,Rckk);

          compute_extrinsic_refine(omc_current,Tc_current,x_kk,X_kk,fc_current,cc_current,
                                   kc_current,alpha_current,MaxIter2,thresh_cond,
                                   omckk,Tckk,Rckk,JJ_kk);
          if (check_cond){
            if (JJ_kk.cond()> thresh_cond){
              std::cout << "Warning: View is ill-conditioned" << std::endl;
            }
          }
          param[offset+6*kk] = omckk[0];
          param[offset+6*kk+1] = omckk[1];
          param[offset+6*kk+2] = omckk[2];
          param[offset+6*kk+3] = Tckk[0];
          param[offset+6*kk+4] = Tckk[1];
          param[offset+6*kk+5] = Tckk[2];
        }
      }
      iter = iter + 1;

    }

    (m_data->intrinsic_matrix)->index_yx(0, 0) = param[0];
    (m_data->intrinsic_matrix)->index_yx(1, 1) = param[1];
    (m_data->intrinsic_matrix)->index_yx(0, 2) = param[2];
    (m_data->intrinsic_matrix)->index_yx(1, 2) = param[3];
    (m_data->intrinsic_matrix)->index_yx(0, 1) = param[4];
    (m_data->distortion_coeffs)->index_yx(0, 0) = param[5];
    (m_data->distortion_coeffs)->index_yx(0, 1) = param[6];
    (m_data->distortion_coeffs)->index_yx(0, 2) = param[7];
    (m_data->distortion_coeffs)->index_yx(0, 3) = param[8];
    (m_data->distortion_coeffs)->index_yx(0, 4) = param[9];

    std::vector<double> paramsVec(param.begin(),param.end());
    m_calres = Result(paramsVec,Size(m_data->nx,m_data->ny));
  }

  void IntrinsicCalibrator::saveIntrinsics(const std::string &filename){
    std::ofstream s(filename.c_str());
    s << m_calres;
  }


  void IntrinsicCalibrator::loadIntrinsics(const std::string &filename){
    static_cast<ImageUndistortion&>(m_calres) = ImageUndistortion(filename);

    (m_data->intrinsic_matrix)->index_yx(0, 0) = m_calres.getParams()[0];
    (m_data->intrinsic_matrix)->index_yx(1, 1) = m_calres.getParams()[1];
    (m_data->intrinsic_matrix)->index_yx(0, 2) = m_calres.getParams()[2];
    (m_data->intrinsic_matrix)->index_yx(1, 2) = m_calres.getParams()[3];
    (m_data->intrinsic_matrix)->index_yx(0, 1) = m_calres.getParams()[4];
    (m_data->distortion_coeffs)->index_yx(0, 0) = m_calres.getParams()[5];
    (m_data->distortion_coeffs)->index_yx(0, 1) = m_calres.getParams()[6];
    (m_data->distortion_coeffs)->index_yx(0, 2) = m_calres.getParams()[7];
    (m_data->distortion_coeffs)->index_yx(0, 3) = m_calres.getParams()[8];
    (m_data->distortion_coeffs)->index_yx(0, 4) = m_calres.getParams()[9];
  }

  void IntrinsicCalibrator::resetData(unsigned int boardWidth, unsigned int boardHeight,
                                      unsigned int boardCount,unsigned int imageWidth ,unsigned int imageHeight){
    m_data->bWidth = boardWidth;
    m_data->bHeight = boardHeight;
    m_data->bSize = m_data->bWidth * m_data->bHeight;
    delete m_data->intrinsic_matrix;
    m_data->intrinsic_matrix = new DynMatrix<icl64f>(3, 3);
    delete m_data->distortion_coeffs;
    m_data->distortion_coeffs = new DynMatrix<icl64f>(1, 5);
    m_data->successes = boardCount;
    m_data->nx = imageWidth;
    m_data->ny = imageHeight;
  }

  IntrinsicCalibrator::Result IntrinsicCalibrator::optimize(const CalibrationData &data){
    ICLASSERT_THROW(data.data.size() > 3, ICLException("IntrinsicCalibrator::optimize: not enough entries ..."));

    int w = data.data[0].getWidth();
    int h = data.data[0].getHeight();
    int n = data.data.size();
    int d = data.data[0].getDim();
    DynMatrix<icl64f> I(d,n*2), W(d,3);

    IntrinsicCalibrator calib(w,h,n, data.imageSize.width, data.imageSize.height);
    //for all grids
    for(int i=0 ;i<n;++i){
      ICLASSERT_THROW(data.data[i].getWidth() == w, ICLException("IntrinsicCalibrator::optimize: size-width differs"));
      ICLASSERT_THROW(data.data[i].getHeight() == h, ICLException("IntrinsicCalibrator::optimize: size-height differs"));

      for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
          I.index_yx(2*i, x+y*w) = data.data[i](x,y)[0];
          I.index_yx(2*i+1, x+y*w) = data.data[i](x,y)[1];
        }
      }
    }
    for(int i=0; i<d; ++i){
      W[i] = i/w;
      W[i+d] = i%w;
      W[i+2*d] = 0.0;
    }
    return calib.calibrate(I,W);
  }
  } // namespace icl::io