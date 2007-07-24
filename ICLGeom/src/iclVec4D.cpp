#include "iclVec4D.h"

namespace icl{
  using std::string;
 
  template<class T>
  void Vec4D<T>::show(const string &title) const{
    printf("%s: \n",title.c_str());
    printf("%4.4f\n",(*this)[0]);
    printf("%4.4f\n",(*this)[1]);    
    printf("%4.4f\n",(*this)[2]);
    printf("%4.4f\n",(*this)[3]);
  }

  template<>
  void Vec4D<icl32s>::show(const string &title) const{
    printf("%s: \n",title.c_str());
    printf("%8d\n",(*this)[0]);
    printf("%8d\n",(*this)[1]);    
    printf("%8d\n",(*this)[2]);
    printf("%8d\n",(*this)[3]);
  }

  
  template class Vec4D<icl32f>;
  template class Vec4D<icl64f>;
  template class Vec4D<icl32s>;

}
