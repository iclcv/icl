/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/HomogeneousMath.h                  **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
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

#pragma once

#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>

namespace icl{
  namespace math{

    /// another shortcut for 3D vectors
    typedef FixedColVector<icl32f,3> Vec3;

    /// another shortcut for 3D vectors
    typedef FixedColVector<icl32f,4> Vec4;

    /// typedef for 3x3 fixed matrices
    typedef FixedMatrix<icl32f, 3,3> Mat3;

    /// typedef for 4x4 fixed matrices
    typedef FixedMatrix<icl32f, 4,4> Mat4;

    /// linearly interpolates between a, b (x must be in range [0,1])
    /** if x is 0, a is returned and if x is 1, b is returned */
    template<class T>
    T linear_interpolate(const T &a, const T &b, float x){
      return a * (1-x) + b*x;
    }

    /// distance from point p to line segment (lineStart -> lineEnd)
    /** The implementation is based on the code of http://www.geometrictools.com which is
        povided under the terms of the "Boost Software License 1.0"
        (http://www.boost.org/LICENSE_1_0.txt) */
    ICLMath_API float dist_point_linesegment(const Vec4 &p,
                                             const Vec4 &lineStart,
                                             const Vec4 &lineEnd,
                                             Vec4 *nearestPoint=0);


    /// distance from point p to triangle (a - b - c)
    /** The implementation is based on the code of http://www.geometrictools.com which is
        povided under the terms of the "Boost Software License 1.0"
        (http://www.boost.org/LICENSE_1_0.txt) */
    ICLMath_API float dist_point_triangle(const Vec4 &p,
                                          const Vec4 &a,
                                          const Vec4 &b,
                                          const Vec4 &c,
                                          Vec4 *nearestPoint=0);


    /// bilinear vector interpolation
    /** corner order ul, ur, ll, lr
           0 ----- 1
        |  |       |   --> x
        |  2 ------3
        \/
        y
    */
    inline Vec4 bilinear_interpolate(const Vec4 corners[4], float x, float y){
      const Vec4 a = linear_interpolate(corners[0],corners[1],x);
      const Vec4 b = linear_interpolate(corners[2],corners[3],x);
      Vec4 c = linear_interpolate(a,b,y);
      c[3] = 1;
      return c;
    }

    /// normalize a vector to length 1
    template<class T>
    inline math::FixedColVector<T,4> normalize(const math::FixedMatrix<T,1,4> &v) {
      double l = v.length();
      ICLASSERT_RETURN_VAL(l,v);
      return v/l;
    }
    /// normalize a vector to length 1
    template<class T>
    inline math::FixedColVector<T,4> normalize3(const math::FixedMatrix<T,1,4> &v,const double& h=1) {
      double l = ::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
      ICLASSERT_RETURN_VAL(l,v);
      Vec4 n = v/l;
      // XXX
      n[3]=h;
      return n;
    }

    /// 3D scalar (aka dot-) product for 4D homogeneous vectors (ignoring the homegeneous component)
    inline float sprod3(const Vec3 &a, const Vec3 &b){
      return a[0]*b[0] + a[1]*b[1]+ a[2]*b[2];
    }

    template<class T>
    inline FixedMatrix<T,3,3> gramSchmidtOrtho(FixedMatrix<T,3,3> const &mat) {
        FixedMatrix<T,3,3> result(T(0));
        Vec3 w1 = mat.col(0);
        Vec3 w2 = mat.col(1);
        Vec3 w3 = mat.col(2);
        Vec3 v1 = w1.normalized();
        Vec3 v2 = (w2 - (v1*sprod3(v1,w2))).normalized();
        Vec3 v3 = (w3 - v1*sprod3(v1,w3) - v2*sprod3(v2,w3)).normalized();
        result.col(0) = v1;
        result.col(1) = v2;
        result.col(2) = v3;
        return result;
    }

    /*template<class T, unsigned int DIM>
    inline float sprod(FixedColVector<T,DIM> const &a, FixedColVector<T,DIM> const &b) {
        float result = 0;
        for(int i = 0; i < DIM; ++i) {
            result += a[i]*b[i];
        }
        return result;
    }

    template<class T, unsigned int COLS, unsigned int ROWS>
    inline FixedMatrix<T,COLS,ROWS> gramSchmidtOrtho(FixedMatrix<T,COLS,ROWS> const &mat) {
        FixedMatrix<T,COLS,ROWS> r(T(0.f));

        FixedColVector<T,ROWS> w = mat.col(0);
        r.col(0) = w.normalized();

        for(unsigned int i = 1; i < COLS; ++i) {
            FixedColVector<T,ROWS> w_i = mat.col(i);
            FixedColVector<T,ROWS> tmp(0.f);
            for(unsigned int k = 0; k < i; ++k) {
                FixedColVector<T,ROWS> v_k = mat.col(k);
                tmp += v_k*sprod(v_k,w_i);
            }
            r.col(i) = (w_i-tmp).normalized();
        }

        return r;
    }*/

    /// 3D scalar (aka dot-) product for 4D homogeneous vectors (ignoring the homegeneous component)
    inline float sprod3(const Vec4 &a, const Vec4 &b){
      return a[0]*b[0] + a[1]*b[1]+ a[2]*b[2];
    }


    /// sqared norm for 4D homogeneous vectors (ignoring the homegeneous component)
    inline float sqrnorm3(const Vec4 &a){
      return sprod3(a,a);
    }

    /// 3D- euclidian norm for 4D homogeneous vectors (ignoring the homegeneous component)
    inline float norm3(const Vec4 &a){
      return ::sqrt(sqrnorm3(a));
    }

    /// 3d euclidian distance
    inline float dist3(const Vec4 &a, const Vec4 &b){
      return norm3(a-b);
    }


    /// homogenize a vector by normalizing 4th component to 1
    template<class T>
    inline math::FixedColVector<T,4> homogenize(const math::FixedMatrix<T,1,4> &v){
      ICLASSERT_RETURN_VAL(v[3],v); return v/v[3];
    }

    /// perform perspective projection
    template<class T>
    inline math::FixedColVector<T,4> project(math::FixedMatrix<T,1,4> v, T z){
      T zz = z*v[2];
      v[0]/=zz;
      v[1]/=zz;
      v[2]=0;
      v[3]=1;
      return v;
    }

    /// homogeneous 3D cross-product
    template<class T>
    inline math::FixedColVector<T,4> cross(const math::FixedMatrix<T,1,4> &v1, const math::FixedMatrix<T,1,4> &v2){
      return math::FixedColVector<T,4>(v1[1]*v2[2]-v1[2]*v2[1],
                                 v1[2]*v2[0]-v1[0]*v2[2],
                                 v1[0]*v2[1]-v1[1]*v2[0],
                                 1 );
    }

		template<class T>
		inline math::FixedColVector<T,3> cross(const math::FixedMatrix<T,1,3> &v1, const math::FixedMatrix<T,1,3> &v2){
			return math::FixedColVector<T,3>(v1[1]*v2[2]-v1[2]*v2[1],
																 v1[2]*v2[0]-v1[0]*v2[2],
																 v1[0]*v2[1]-v1[1]*v2[0]);
		}

    /// rotates a vector around a given axis
    inline Vec4 rotate_vector(const Vec4 &axis, float angle, const Vec4 &vec){
      return create_rot_4x4(axis[0],axis[1],axis[2],angle)*vec;
    }

  }
}
