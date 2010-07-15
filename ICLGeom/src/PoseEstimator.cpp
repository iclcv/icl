/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PoseEstimator.cpp                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

/********************************************************************
** Further Copyright Note: Parts of this code were taken, from     **
** VTK 5.6.0 vtkLandmarkTransform.cxx                              **
** Copyright (c) 1993-2008 Ken Martin, Will Schroeder,             **
** Bill Lorensen   -- All  rights reserved.                        **
*********************************************************************/
#include <ICLGeom/PoseEstimator.h>
#include <ICLCore/CoreFunctions.h>

namespace icl{


  template<typename T>
  static void perpendiculars(const T x[3], T y[3], T z[3], T theta){
    int dx,dy,dz;

    double x2 = x[0]*x[0];
    double y2 = x[1]*x[1];
    double z2 = x[2]*x[2];
    double r = sqrt(x2 + y2 + z2);

    // transpose the vector to avoid divide-by-zero error
    if(x2 > y2 && x2 > z2){ dx = 0; dy = 1; dz = 2; }
    else if(y2 > z2) { dx = 1; dy = 2; dz = 0; }
    else{ dx = 2; dy = 0; dz = 1; }

    double a = x[dx]/r, b = x[dy]/r, c = x[dz]/r;
    double tmp = sqrt(a*a+c*c);

    if(theta){
      double sintheta = sin(theta);
      double costheta = cos(theta);
      if(y){
        y[dx] = (c*costheta - a*b*sintheta)/tmp;
        y[dy] = sintheta*tmp;
        y[dz] = (-a*costheta - b*c*sintheta)/tmp;
      }
      if(z){
        z[dx] = (-c*sintheta - a*b*costheta)/tmp;
        z[dy] = costheta*tmp;
        z[dz] = (a*sintheta - b*c*costheta)/tmp;
      }
    }else{
      if (y) {
        y[dx] = c/tmp;
        y[dy] = 0;
        y[dz] = -a/tmp;
      }
      if (z){
        z[dx] = -a*b/tmp;
        z[dy] = tmp;
        z[dz] = -b*c/tmp;
      }
    }      
  }

  template <class T>
  static inline FixedColVector<T,3> get_mean_col_vec(const DynMatrix<T> &m){
    return FixedColVector<T,3> ( std::accumulate(m.row_begin(0),m.row_end(0),T(0)),
                                 std::accumulate(m.row_begin(1),m.row_end(1),T(0)),
                                 std::accumulate(m.row_begin(2),m.row_end(2),T(0))) * (1.0/m.cols());
  }

  template<class T>
  static inline FixedColVector<T,3> cross_prod(const FixedMatrix<T,1,3> &v1, const FixedMatrix<T,1,3> &v2){
    return FixedColVector<T,3>(v1[1]*v2[2]-v1[2]*v2[1],
                               v1[2]*v2[0]-v1[0]*v2[2],
                               v1[0]*v2[1]-v1[1]*v2[0]);
  }
  
  template<class T>
  FixedMatrix<T,3,3> PoseEstimator::quaternion_to_rotation_matrix(T w, T x, T y, T z){
    T ww = w*w;
    T wx = w*x;
    T wy = w*y;
    T wz = w*z;
  
    T xx = x*x;
    T yy = y*y;
    T zz = z*z;
  
    T xy = x*y;
    T xz = x*z;
    T yz = y*z;
  
    FixedMatrix<T,3,3> M;
    M(0,0) = ww + xx - yy - zz; 
    M(0,1) = 2.0*(wz + xy);
    M(0,2) = 2.0*(-wy + xz);

    M(1,0) = 2.0*(-wz + xy);  
    M(1,1) = ww - xx + yy - zz;
    M(1,2) = 2.0*(wx + yz);

    M(2,0) = 2.0*(wy + xz);
    M(2,1) = 2.0*(-wx + yz);
    M(2,2) = ww - xx - yy + zz;
    
    return M;
  }


  template<class T>
  FixedMatrix<T,4,4> PoseEstimator::map(const DynMatrix<T> &Xs, const DynMatrix<T> &Ys, PoseEstimator::MapMode mode) throw (IncompatibleMatrixDimensionException){
    ICLASSERT_THROW(Xs.rows() == 3 || Xs.rows() == 4, IncompatibleMatrixDimensionException("PoseEstimator::map: Xs.rows must be 3 or 4 (for homogeneous coordinates)"));
    ICLASSERT_THROW(Ys.rows() == 3 || Ys.rows() == 4, IncompatibleMatrixDimensionException("PoseEstimator::map: Ys.rows must be 3 or 4 (for homogeneous coordinates)"));
    ICLASSERT_THROW(Xs.cols() == Ys.cols(), IncompatibleMatrixDimensionException("compute_relative_transformation: Point count in Xs and Ys must be equal"));
    ICLASSERT_THROW(Xs.cols() > 0, IncompatibleMatrixDimensionException("compute_relative_transformation: At least 1 point is needed for relative pose estimation"));
  
    static const T eps = getDepth<T>() == getDepth<float>() ? -23 : -52; 
  
    typedef FixedMatrix<T,4,4> M4;
    typedef FixedMatrix<T,3,3> M3;
    typedef FixedColVector<T,3> V3;
    typedef FixedColVector<T,4> V4;

    const unsigned int N_PTS = Xs.cols();
    V3 cx = get_mean_col_vec(Xs);
    V3 cy = get_mean_col_vec(Ys);

    // -- if only one point, stop right here
    if (N_PTS == 1 || mode==Translation) {
      return M4(1,0,0,cy[0]-cx[0],
                0,1,0,cy[1]-cx[1],
                0,0,1,cy[2]-cx[2],
                0,0,0,1);
    }

    M4 TM = M4::id();

    // -- build the 3x3 matrix M --
    M3 M(T(0)),XXt(T(0));
    T sx=0,sy=0;
    for(unsigned i=0; i < N_PTS; ++i) {
      // get origin centered point
      V3 x=V3(Xs(i,0),Xs(i,1),Xs(i,2))-cx;
      V3 y=V3(Ys(i,0),Ys(i,1),Ys(i,2))-cy;
    
      // accumulate the products a*b' into the matrix M
      M += x*y.transp();
    
      // for the affine transform, compute ((x*x')^-1 * x*b')'.
      // x*y' is already in M. Here we put x*x' in XXt.
      if (mode == Affine){
        XXt += x * x.transp();
      }
      // accumulate scale factors
      sx += x[0]*x[0]+x[1]*x[1]+x[2]*x[2];
      sy += y[0]*y[0]+y[1]*y[1]+y[2]*y[2];
    }  

    if (mode == Affine) {
      M3 R =  XXt.inv() * M;
    
      // copy transposed
      for(int y=0;y<3;++y){
        for(int x=0;x<3;++x){
          TM(x,y) = R(y,x);
        }
      }
    
    }else{
      // compute required scaling factor (if desired)
      const double scale = (double)sqrt(sx/sy);

      // -- build the 4x4 matrix N --
      M4 N;

      // on-diagonal elements
      N(0,0) = M(0,0)+M(1,1)+M(2,2);
      N(1,1) = M(0,0)-M(1,1)-M(2,2);
      N(2,2) = -M(0,0)+M(1,1)-M(2,2);
      N(3,3) = -M(0,0)-M(1,1)+M(2,2);
      // off-diagonal elements
      N(1,0) = N(0,1) = M(2,1)-M(1,2);
      N(2,0) = N(0,2) = M(0,2)-M(2,0);
      N(3,0) = N(0,3) = M(1,0)-M(0,1);
    
      N(1,2) = N(2,1) = M(1,0)+M(0,1);
      N(1,3) = N(3,1) = M(0,2)+M(2,0);
      N(2,3) = N(3,2) = M(2,1)+M(1,2);

      // the eigenvector with the largest eigenvalue is the quaternion we want
      T w; V3 v;
      M4 evecs;
      V4 evals;
      N.eigen(evecs,evals);
    
      if(fabs(evals[0]-evals[1]) < 0.00001 || N_PTS == 2){
        // if points are collinear, choose the quaternion that 
        // results in the smallest rotation.

        V3 dx( Xs(1,0)-Xs(0,0), Xs(1,1)-Xs(0,1), Xs(1,2)-Xs(0,2));
        V3 dy( Ys(1,0)-Ys(0,0), Ys(1,1)-Ys(0,1), Ys(1,2)-Ys(0,2));
        dx.normalize();
        dy.normalize();
      
        w = dx.dot(dy)[0];
        v = cross_prod(dx,dy);
        T r = v.length();
        T theta  = atan2(r,w);
      
        // construct quaternion
        w = cos(theta/2);
        if (r > eps) v *= sin(theta/2)/r;
        else { // rotation by 180 degrees: special case
          // rotate around a vector perpendicular to dx
          perpendiculars<T>(dx.data(),v.data(),NULL,0);
          v *= sin(theta/2);
        }
      } else {
        // points are not collinear
        w = evecs(0,0); // 
        // goes not! -> g++ seems not to be in the mood to compile this :-)
        // v = evecs.part<3,1,1,3>();

        for(int y=0;y<3;++y){
          v[y] = evecs(0,y+1);
        }
      
      }

      M3 R = quaternion_to_rotation_matrix<T>(w,v[0],v[1],v[2]);
      // here, also part<..> was supposed to work
      for(int i=0;i<3;++i){
        for(int j=0;j<3;++j){
          TM(i,j) = R(i,j);
        }
      }
      

      if (mode != RigidBody) {
        TM = M4(scale,0,0,0,
                0,scale,0,0,
                0,0,scale,0,
                0,0,0,1) * TM;
        //      TM.scale(scale);
      }
    }
  
    // the final translation is given by the difference in the transformed 
    // source centroid and the target centroid
  
    M3 R; // = TM.part<0,0,3,3>(1); this can also not be compiled! -> g++ bug
    for(int y=0;y<3;++y) {
      std::copy(TM.row_begin(y),TM.row_begin(y)+3,R.row_begin(y));
    }
  
    V3 trans = cy - R * cx;
    std::copy(trans.begin(),trans.end(), TM.col_begin(3));
  
    return TM;

    
  }
  
  template FixedMatrix<icl32f,4,4> PoseEstimator::map(const DynMatrix<icl32f>&,const DynMatrix<icl32f>&,PoseEstimator::MapMode mode) throw (IncompatibleMatrixDimensionException);
  template FixedMatrix<icl64f,4,4> PoseEstimator::map(const DynMatrix<icl64f>&,const DynMatrix<icl64f>&,PoseEstimator::MapMode mode) throw (IncompatibleMatrixDimensionException);
}
