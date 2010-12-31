/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/GeomDefs.h                             **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_GEOM_DEFS_H
#define ICL_GEOM_DEFS_H

#include <ICLCore/Types.h>
#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/FixedVector.h>
#include <vector>
#include <ICLCC/Color.h>

namespace icl{
  /// color for geometry primitives
  typedef Color4D32f GeomColor;

  /// inline utililty function to create a white color instance
  inline GeomColor geom_white(float alpha=255) { return GeomColor(255,255,255,alpha); }

  /// inline utililty function to create a red color instance
  inline GeomColor geom_red(float alpha=255) { return GeomColor(255,0,0,alpha); }

  /// inline utililty function to create a blue color instance
  inline GeomColor geom_blue(float alpha=255) { return GeomColor(0,100,255,alpha); }

  /// inline utililty function to create a green color instance
  inline GeomColor geom_green(float alpha=255) { return GeomColor(0,255,0,alpha); }

  /// inline utililty function to create a yellow color instance
  inline GeomColor geom_yellow(float alpha=255) { return GeomColor(255,255,0,alpha); }

  /// inline utililty function to create a magenta color instance
  inline GeomColor geom_magenta(float alpha=255) { return GeomColor(255,0,255,alpha); }

  /// inline utililty function to create a cyan color instance
  inline GeomColor geom_cyan(float alpha=255) { return GeomColor(0,255,255,alpha); }

  /// inline utililty function to create a cyan color instance
  inline GeomColor geom_black(float alpha=255) { return GeomColor(0,0,0,alpha); }

  /// inline utililty function to create an invisible color instance (alpha is 0.0f)
  inline GeomColor geom_invisible() { return GeomColor(0,0,0,0); }

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

  inline Vec rotate_vector(const Vec &axis, float angle, const Vec &vec){
    return create_rot_4x4(axis[0],axis[1],axis[2],angle)*vec;
    /*
        angle /= 2;
        float a = cos(angle);
        float sa = sin(angle);
        float b = axis[0] * sa;
        float c = axis[1] * sa;
        float d = axis[2] * sa;
        
        float a2=a*a, b2=b*b, c2=c*c, d2=d*d;
        float ab=a*b, ac=a*c, ad=a*d, bc=b*c, bd=b*d, cd=c*d;
        
        Mat X(a2+b2-c2-d2,  2*bc-2*ad,   2*ac+2*bd,   0,
        2*ad+2*bd,    a2-b2+c2-d2, 2*cd-2*ab,   0,
        2*bd-2*ac,    2*ab+2*cd,   a2-b2-c2+d2, 0,
        0,            0,           0,           1);
        
        return X * vec;
    */
  }

}


#endif
