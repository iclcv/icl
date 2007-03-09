#include <vector>
#ifndef ICLARRAY_H
#define ICLARRAY_H


namespace icl{

  template <class T>
  class Array : public std::vector<T>{
    public:
    Array(unsigned int dim=0) : std::vector<T>(dim){}
    Array(const Array &a):std::vector<T>(a){}
    Array(T* src, int dim):std::vector<T>(dim){
      for(int i=0;i<dim;++i){
        (*this)[i]=src[i];
      }
    }
    
    // deprecated
    const T* operator*() const{
      if(!this->size()) return 0;
      return &((*this)[0]);
    }
  
    // deprecated operator
    T* operator*(){
      if(!this->size()) return 0;
      return &((*this)[0]);
    }

    const T* ptr() const {
      if(!this->size()) return 0;
      return &((*this)[0]);
    }
    
    T* ptr(){
      if(!this->size()) return 0;
      return &((*this)[0]);
    } 
  };

} // namespace icl
#endif
