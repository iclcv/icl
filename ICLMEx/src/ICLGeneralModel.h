#include <ICLArray.h>
#include <ICLImg.h>
#ifndef GENERAL_MODEL_H
#define GENERAL_MODEL_H


namespace icl{

  /// GeneralModel class 
  /** A general Model is a formalisation of a model that should be
  

  The template is pre-build for types icl32f and icl64f
  */
  template<class T>
  class GeneralModel{
    public:
    virtual ~GeneralModel(){}
    virtual void features(T x,T y, T *dst) const =0;
    virtual Array<T> x(T y) const = 0;
    virtual Array<T> y(T x) const = 0;

    int dim()const;
    const T *constraints()const;
    void setIdentityConstraintMatrix();
    
    T *params(){ return *m_oCurrParams; }
    const T *params() const{ return *m_oCurrParams; }
    T &operator[](int i){ return m_oCurrParams[i]; }
    const T &operator[](int i) const { return m_oCurrParams[i]; }

    protected:
    GeneralModel(int dim);
    
    Array<T> m_oConstraintMatrix;
    Array<T> m_oCurrParams;
    int m_iDim;
  };
   

    
}
#endif
