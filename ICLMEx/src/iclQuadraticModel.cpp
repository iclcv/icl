#include <iclQuadraticModel.h>

using namespace icl;

template<class T>
inline Array<T> __solvePQ(T p, T q){
  T rootTerm = p*p/4-q;
  Array<T> a(2);
  if(rootTerm >= 0){ // real solution
    a[0]=(-p*0.5 + sqrt(rootTerm));
    a[1]=(-p*0.5 - sqrt(rootTerm));
  }else{
    a.clear();
  }     
  return a;
} 

template<class T>
Array<T> QuadraticModel<T>::x(T y) const{
  return __solvePQ( px(y), qx(y) );
}

template<class T>
Array<T> QuadraticModel<T>::y(T x) const{
  return __solvePQ( py(x), qy(x) );
}

template<class T>
QuadraticModel<T>::QuadraticModel(int dim):
  GeneralModel<T>(dim){}


template class QuadraticModel<icl32f>;
template class QuadraticModel<icl64f>;
