#include <ICLGeneralModel.h>


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
  m_oConstraintMatrix.resize(m_iDim*m_iDim);
  std::fill(m_oConstraintMatrix.begin(),m_oConstraintMatrix.end(),0);
  for(unsigned int i=0;i<m_oConstraintMatrix.size();i+=(m_iDim+1)){
    m_oConstraintMatrix[i]=1;
  }
}

template<class T> 
GeneralModel<T>::GeneralModel(int dim):m_oCurrParams(dim), m_iDim(dim){
  setIdentityConstraintMatrix();
}


template class GeneralModel<icl32f>;
template class GeneralModel<icl64f>;
