/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/examples/dyn-matrix-utils-test.cpp            **
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

#include <ICLUtils/DynMatrixUtils.h>

using namespace icl;

/// this is abused here for a similarity check
template<class T>
bool operator&&(const DynMatrix<T> &m, const DynMatrix<T> &x){
  return m.isSimilar(x,0.0000001);
}

template<class T>
struct IncInit{
  mutable T curr;
  IncInit():curr(0){}
  operator T() const { return curr++; }
};

template<class T,int COLS, int ROWS>
struct CheckClass{

  static DynMatrix<T> mat(T val){
    DynMatrix<T> M(COLS,ROWS);
    matrix_init(M,val);
    return M;
  }
  static DynMatrix<unsigned char> ucmat(unsigned char val){
    DynMatrix<unsigned char> M(COLS,ROWS);
    matrix_init(M,val);
    return M;
  }
  static DynMatrix<T> &mat_ref(T val){
    static DynMatrix<T> M(COLS,ROWS);
    matrix_init(M,val);
    return M;
  }

  
  static void check(){
    // abs log exp sqrt sqr sin cos tan arcsin arccos arctan reciprocal
    
    ICLASSERT(matrix_abs ( mat_ref(-1)   ) ==  mat(1)  );
    ICLASSERT(matrix_log ( mat_ref(M_E)  ) &&  mat(1)  );
    ICLASSERT(matrix_sqrt( mat_ref(4)    ) &&  mat(2)  );
    ICLASSERT(matrix_sqr ( mat_ref(3)    ) ==  mat(9)  );

    ICLASSERT(matrix_sin ( mat_ref(M_PI) ) &&  mat(0)  );
    ICLASSERT(matrix_cos ( mat_ref(M_PI) ) ==  mat(-1) );
    ICLASSERT(matrix_tan ( mat_ref(M_PI) ) &&  mat(0)  );

    ICLASSERT(matrix_arcsin ( mat_ref(0) ) ==  mat(0)  );
    ICLASSERT(matrix_arccos ( mat_ref(1) ) ==  mat(0)  );
    ICLASSERT(matrix_arctan ( mat_ref(0) ) ==  mat(0)  );

    ICLASSERT(matrix_reciprocal ( mat_ref(10) ) ==  mat(0.1) );
    ICLASSERT(matrix_reciprocal ( mat_ref(1)  ) ==  mat(1)   );

    // powc addc subc divc mulc
    ICLASSERT(matrix_powc( mat_ref(2) ,T(3) ) == mat(8) );
    ICLASSERT(matrix_addc( mat_ref(2) ,T(3) ) == mat(5) );
    ICLASSERT(matrix_subc( mat_ref(3) ,T(2) ) == mat(1) );
    ICLASSERT(matrix_divc( mat_ref(8) ,T(4) ) == mat(2) );
    ICLASSERT(matrix_mulc( mat_ref(2) ,T(3) ) == mat(6) );
    
    DynMatrix<T> dst;
    ICLASSERT(matrix_add( mat(2), mat(3), dst ) == mat(5));
    ICLASSERT(matrix_sub( mat(3), mat(2), dst ) == mat(1));
    ICLASSERT(matrix_mul( mat(3), mat(2), dst ) == mat(6));
    ICLASSERT(matrix_div( mat(5), mat(2), dst ) == mat(2.5));

    ICLASSERT(matrix_pow( mat(2), mat(4), dst ) == mat(16));
    ICLASSERT(matrix_arctan2( mat(0), mat(1), dst ) == mat(0));

    DynMatrix<T> M(COLS,ROWS);
    matrix_init(M,IncInit<T>());

    int pos[2]={-1,-1};
    ICLASSERT( matrix_min(M,pos,pos+1) == 0);
    ICLASSERT( pos[0] == 0 && pos[1] == 0);
    
    ICLASSERT( matrix_max(M,pos,pos+1) == M[M.dim()-1]);
    ICLASSERT( (pos[0] == (int)M.cols()-1) && (pos[1] == (int)M.rows()-1) );

    int p[4];
    T minMax[2];
    matrix_minmax(M,minMax,p,p+1,p+2,p+3);
    ICLASSERT( minMax[0] == 0 && minMax[1] == M[M.dim()-1] );
    ICLASSERT( p[0] == 0 && p[1] == 0 );
    ICLASSERT( p[2] == (int)M.cols()-1 );
    ICLASSERT( p[3] == (int)M.rows()-1 );

    /// mean
    ICLASSERT( matrix_mean(mat(5)) == 5 );
    ICLASSERT( fabs( matrix_var(mat(5)) ) < 0.0000001);
    ICLASSERT( fabs( matrix_stddev(mat(5)) ) < 0.0000001);

    T mv[2];
    matrix_meanvar( mat(5),mv,mv+1);
    ICLASSERT(mv[0] == 5);
    ICLASSERT(mv[1] == 0);

    ICLASSERT( matrix_muladd( mat(2), T(3), mat(5), T(4), T(4), mat_ref(0) ) == mat(30) );
    ICLASSERT( matrix_muladd( mat(2), T(3), T(4), mat_ref(0) ) == mat(10) );
    
    ICLASSERT( matrix_mask( ucmat(1), mat_ref(7)) == mat(7) );
    ICLASSERT( matrix_mask( ucmat(0), mat_ref(7)) == mat(0) );

    ICLASSERT( matrix_mask( ucmat(1), mat(7), dst) == mat(7) );
    ICLASSERT( matrix_mask( ucmat(0), mat(7), dst) == mat(0) );

    
    
    
  }
};

int main(){
  std::cout << "this is a unit test for the DynMatrixUtils.h functions" << std::endl;
  std::cout << "no error message means that everything is ok!" << std::endl;
  CheckClass<float,2,6>::check();
  CheckClass<double,2,6>::check();
  CheckClass<float,1,1>::check();
  CheckClass<double,1,1>::check();
  CheckClass<float,100,100>::check();
  CheckClass<double,100,100>::check();

}
