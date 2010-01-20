#include <ICLUtils/FixedMatrix.h>
#include <iostream>
#include <sstream>

typedef unsigned char real;
int main(){
  icl::FixedMatrix<real,2,3> m( 2, 4,
                            6, 7,
                            0, -1);
  
  std:: cout << m << std::endl;
  
  std::ostringstream os;
  os << m << m << m;
  
  std::istringstream is(os.str());
  icl::FixedMatrix<real,2,3> ms[3]={
    icl::FixedMatrix<real,2,3>(0.0),
    icl::FixedMatrix<real,2,3>(1.0),
    icl::FixedMatrix<real,2,3>(2.0)
  };
  is >> ms[0];
  is >> ms[1];
  is >> ms[2];
  
  std::cout << "ms[0]:\n" << ms[0] << std::endl;
  std::cout << "ms[1]:\n" << ms[1] << std::endl;
  std::cout << "ms[2]:\n" << ms[2] << std::endl;
  
}
