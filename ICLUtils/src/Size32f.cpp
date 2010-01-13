#include <ICLUtils/Size32f.h>

const icl::Size32f icl::Size32f::null(0,0);

namespace icl{
  std::ostream &operator<<(std::ostream &s, const Size32f &size){
    return s << size.width << 'x' << size.height;
  }
  
  std::istream &operator>>(std::istream &s, Size32f &size){
    char c;
    return s >> size.width >> c >> size.height;
  }
  
}
