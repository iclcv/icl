#include "GeneralModel.h"


using namespace icl;

template<class T> 
int GeneralModel<T>::dim() const{
  return m_iDim;  
}

template<class T> 
const T *GeneralModel<T>::constraints() const{
  return *m_oConstraintMatrix;
}

template<class T> 
void GeneralModel<T>::setIdentityConstraintMatrix(){
  m_oConstraintMatrix.resize(dim()*dim());
  std::fill(constraints(),constraints()+m_iDim*m_iDim,0);
  for(int i=0;i<m_iDim;i++){
    m_oConstraintMatrix[i*m_iDim*i]=1;
  }
}

template<class T> 
GeneralModel<T>::GeneralModel(int dim):m_iDim(dim){
  setIdentityConstraintMatrix();
}
    
