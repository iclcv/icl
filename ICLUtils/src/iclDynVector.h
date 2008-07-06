#ifndef ICL_DYN_VECTOR_H
#define ICL_DYN_VECTOR_H

#include <iclTypes.h>
#include <valarray>

namespace icl{
  template<class T>
  struct DynVector : public std::valarray<T>{
    DynVector():std::valarray<T>(){}
    explicit DynVector(size_t dim):std::valarray<T>(dim){};
    DynVector(const T &val, int dim):std::valarray<T>(val,dim){}
    DynVector(const T *data, int dim):std::valarray<T>(data,dim){}
    DynVector(const DynVector &v):std::valarray<T>(v){}
    
    
    DynVector(const std::slice_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::gslice_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::mask_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::indirect_array< T> &a):std::valarray<T>(a){}
    
    void ensureSize(size_t dim){
      if(std::valarray<T>::size() != dim) std::valarray<T>::resize(dim);    
    }
    
    DynVector<T> &operator=(const DynVector<T> &v){
      ensureSize(v.size());
      return std::valarray<T>::operator=(v);
    }
    DynVector<T> &operator=(const T &t){
      return std::valarray<T>::operator=(t); 
    }
    DynVector<T> &operator=(const std::slice_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a);
    }
    DynVector<T> &operator=(const std::gslice_array<T> &a){ 
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
    DynVector<T> &operator=(const std::mask_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
    DynVector<T> &operator=(const std::indirect_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
  };
  
}

#endif
