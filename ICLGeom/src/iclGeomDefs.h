#ifndef ICL_GEOM_DEFS_H
#define ICL_GEOM_DEFS_H

#include <iclTypes.h>
#include <iclFixedMatrix.h>
#include <iclFixedVector.h>
#include <vector>
#include <iclColor.h>

namespace icl{
  /// color for geometry primitives
  typedef Color4D32f GeomColor;

  /// Matrix Typedef of float matrices
  typedef FixedMatrix<icl32f,4,4> Mat4D32f;

  /// Matrix Typedef of double matrices
  typedef FixedMatrix<icl64f,4,4> Mat4D64f;

  /// Vector typedef of float vectors
  typedef FixedColVector<icl32f,4> Vec4D32f;

  /// Vector typedef of double vectors
  typedef FixedColVector<icl64f,4> Vec4D64f;

  /// Short typedef for 4D float vectors
  typedef Vec4D32f Vec;

  /// Short typedef for 4D float matrices
  typedef Mat4D32f Mat;

  
  /// normalize a vector to length 1
  template<class T>
  inline FixedColVector<T,4> normalize(const FixedMatrix<T,1,4> &v) { 
    double l = v.length();
    ICLASSERT_RETURN_VAL(l,v);
    return v/l;
  }
  /// normalize a vector to length 1
  template<class T>
  inline FixedColVector<T,4> normalize3(const FixedMatrix<T,1,4> &v,const double& h=1) { 
    double l = ::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    ICLASSERT_RETURN_VAL(l,v);
    Vec n = v/l;
    // XXX 
    n[3]=h;
    return n;
  }

  /// homogenize a vector be normalizing 4th component to 1
  template<class T>
  inline FixedColVector<T,4> homogenize(const FixedMatrix<T,1,4> &v){
    ICLASSERT_RETURN_VAL(v[3],v); return v/v[3];
  }

  /// perform perspective projection
  template<class T>
  inline FixedColVector<T,4> project(FixedMatrix<T,1,4> v, T z){
    T zz = z*v[2];
    v[0]/=zz;
    v[1]/=zz;
    v[2]=0;
    v[3]=1;
    return v;
  }
  
  /// homogeneous 3D cross-product
  template<class T>
  inline FixedColVector<T,4> cross(const FixedMatrix<T,1,4> &v1, const FixedMatrix<T,1,4> &v2){
    return FixedColVector<T,4>(v1[1]*v2[2]-v1[2]*v2[1],
                               v1[2]*v2[0]-v1[0]*v2[2],
                               v1[0]*v2[1]-v1[1]*v2[0],
                               1 );
  }

  typedef std::vector<Vec> VecArray;
}


#endif
