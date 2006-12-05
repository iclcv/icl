#ifndef ICLARRAY_H
#define ICLARRAY_H

#include <vector>

namespace icl{

  template <class T>
  class Array : public std::vector<T>{
    public:
    Array(unsigned int dim=0) : std::vector<T>(dim){}
    Array(const Array &a){   *this = a; }    
    
    const T* operator*() const{
      if(!this->size()) return 0;
      return &((*this)[0]);
    }
    
    T* operator*(){
      if(!this->size()) return 0;
      return &((*this)[0]);
    }
  };

} // namespace icl
#endif
