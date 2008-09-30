#include <iclDynMatrix.h>
using namespace icl;

int main(int n, char **ppc){
  DynMatrix<float> v(3,3);
  std::cout << "bitt 9 zahlen eingeben " << std::flush;
  std::copy(std::istream_iterator<float>(std::cin),
            std::istream_iterator<float>(),
            v.begin());
  std::cout << "Die matrix sieht dann wohl so aus: " << std::endl;
  std::cout << v << std::endl;
  
  std::cout << "die inverse matrix ist:" << std::endl;
  try{
    std::cout << "invertiere ... " << std::endl;
    DynMatrix<float> i = v.inv();
    std::cout << i << std::endl;
    
    std::cout << "m*m^1-test" << std::endl;
    std::cout << (v*i) << std::endl;
  }catch(const ICLException &e){
    std::cout << "uups das war wohl nichts:" << e.what() << std::endl;
  }

}
