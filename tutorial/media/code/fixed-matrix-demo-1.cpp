#include <iclFixedMatrix.h>
#include <iclFixedVector.h>

int main(){
  /// create an empty matrix filled with 0.0
  icl::FixedMatrix<float,4,4> T(0.0);

  /// set up the upper left 3x3 matrix to a rotation-matrix
  T.part<0,0,3,3>() = icl::create_rot_3D<float>(M_PI,M_PI/2,0);

  /// set up the lower row to [0,0,0,1]
  T.row(3) = icl::FixedRowVector<float,4>(0,0,0,1);

  /// set up the rightmost column to [1,2,3,1]
  T.col(3) = icl::FixedColVector<float,4>(1,2,3,1);
  
  /// show the inverse matrix
  std::cout << "inverse transformation matrix:" 
            << std::endl << T.inv() << std::endl;
}
