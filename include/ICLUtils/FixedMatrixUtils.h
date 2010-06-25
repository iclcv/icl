/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/FixedMatrixUtils.h                    **
** Module : ICLUtils                                               **
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
*********************************************************************/

#ifndef ICL_FIXED_MATRIX_UTILS_H
#define ICL_FIXED_MATRIX_UTILS_H

#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/FixedVector.h>

namespace icl{
  
  /// computes inner product of two matrices (element-wise scalar product) \ingroup LINALG
  template<class T, unsigned int WIDTH, unsigned int HEIGHT, unsigned int WIDTH2>
  inline T inner_vector_product(const FixedMatrix<T,WIDTH,HEIGHT> &a, const FixedMatrix<T,WIDTH2,(WIDTH*HEIGHT)/WIDTH2> &b){
    return std::inner_product(a.begin(),a.end(),b.begin(),T(0));
  }

  /// computes the QR decomposition of a matrix \ingroup LINALG
  /** implements the stabilized Gram-Schmidt orthonormalization. */
  template<class T, unsigned int WIDTH, unsigned int HEIGHT>
  inline void decompose_QR(FixedMatrix<T,WIDTH,HEIGHT> A, FixedMatrix<T,WIDTH,HEIGHT> &Q, FixedMatrix<T,WIDTH,WIDTH> &R){
    DynMatrix<T> D(WIDTH,HEIGHT,const_cast<T*>(A.begin()),false);
    DynMatrix<T> dQ,dR;
    D.decompose_QR(dQ,dR);
    std::copy(dQ.begin(),dQ.end(),Q.begin());
    std::copy(dR.begin(),dR.end(),R.begin());
  }


  /// computes the spare RQ decomposition of a matrix \ingroup LINALG
  /** implements the stabilized Gram-Schmidt orthonormalization. */
  template<class T, unsigned int WIDTH, unsigned int HEIGHT>
  inline void decompose_RQ(FixedMatrix<T,WIDTH,HEIGHT> A, FixedMatrix<T,HEIGHT,HEIGHT> &R, FixedMatrix<T,WIDTH,HEIGHT> &Q){
    DynMatrix<T> D(WIDTH,HEIGHT,const_cast<T*>(A.begin()),false);
    DynMatrix<T> dR,dQ;
    D.decompose_RQ(dR,dQ);
    std::copy(dR.begin(),dR.end(),R.begin());
    std::copy(dQ.begin(),dQ.end(),Q.begin());  
  }
  
#ifdef HAVE_IPP
  template< class T, unsigned int WIDTH, unsigned int HEIGHT>
  inline void svd_fixed(const FixedMatrix<T,WIDTH,HEIGHT> &A, 
                        FixedMatrix<T,WIDTH,HEIGHT> &U, 
                        FixedColVector<T,WIDTH> &s, 
                        FixedMatrix<T,WIDTH,WIDTH> &V) {
    DynMatrix<T> A_dyn(WIDTH,HEIGHT,const_cast<T*>(A.begin()),false);
    DynMatrix<T> U_dyn(WIDTH,HEIGHT,U.begin(),false);
    DynMatrix<T> V_dyn(WIDTH,WIDTH,V.begin(),false);
    DynMatrix<T> s_dyn(1,WIDTH,s.begin(),false);
    svd_dyn(A_dyn,U_dyn,s_dyn,V_dyn);
  }
#endif
  
  /// computes the pseudo-inverse of a matrix (using QR-decomposition based approach) \ingroup LINALG
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT>
  inline FixedMatrix<T,HEIGHT,WIDTH> pinv(const FixedMatrix<T,WIDTH,HEIGHT> &M, bool useSVD=false, float zeroThreshold=0.00000000000000001){
    DynMatrix<T> D(WIDTH,HEIGHT,const_cast<T*>(M.begin()),false);
    DynMatrix<T> pinvD = D.pinv(useSVD,zeroThreshold);
    FixedMatrix<T,HEIGHT,WIDTH> fM;
    std::copy(pinvD.begin(),pinvD.end(),fM.begin());
    return fM;
  }

  /// Vertical Matrix concatenation  \ingroup LINALG
  /** like ICLQuick image concatenation, dont forget the brackets sometimes */
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT, unsigned int HEIGHT2>
  inline FixedMatrix<T,WIDTH,HEIGHT+HEIGHT2> operator%(const FixedMatrix<T,WIDTH,HEIGHT> &a,
                                                       const FixedMatrix<T,WIDTH,HEIGHT2> &b){
    FixedMatrix<T,WIDTH,HEIGHT+HEIGHT2> M;
    for(unsigned int i=0;i<HEIGHT;++i) M.row(i) = a.row(i);
    for(unsigned int i=0;i<HEIGHT2;++i) M.row(i+HEIGHT) = b.row(i);
    return M;
  }

  /// Horizontal Matrix concatenation  \ingroup LINALG
  /** like ICLQuick image concatenation, dont forget the brackets sometimes */
  template<class T,unsigned  int WIDTH,unsigned  int HEIGHT, unsigned int WIDTH2>
  inline FixedMatrix<T,WIDTH+WIDTH2,HEIGHT> operator,(const FixedMatrix<T,WIDTH,HEIGHT> &a,
                                                       const FixedMatrix<T,WIDTH2,HEIGHT> &b){
    FixedMatrix<T,WIDTH+WIDTH2,HEIGHT> M;
    for(unsigned int i=0;i<WIDTH;++i) M.col(i) = a.col(i);
    for(unsigned int i=0;i<WIDTH2;++i) M.col(i+WIDTH) = b.col(i);
    return M;
  }

  
}



#endif
