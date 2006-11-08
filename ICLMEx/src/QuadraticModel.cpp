#include <QuadraticModel.h>

using namespace icl;

template<class T>
Array<T> __solvePQ(T p, T q){
  T rootTerm = p*p/4-q;
  Array<T> a(2);
  if(rootTerm >= 0){ // real solution
    a[0]=(-p*0.5 + sqrt(rootTerm));
    a[1]=(-p*0.5 - sqrt(rootTerm));
  }else{
    a[0]=0;
    a[1]=0;
  }     
  return a;
} 

template<class T>
Array<T> QuadraticModel<T>::x(T y, T *params) const{
  return __solvePQ( px(y,params), qx(y,params) );
}

template<class T>
Array<T> QuadraticModel<T>::y(T x, T *params) const{
  return __solvePQ( py(x,params), qy(x,params) );
}

template<class T>
QuadraticModel<T>::QuadraticModel(int dim):
  GeneralModel<T>(dim){}


#endif
