#ifndef ICL_FIXED_VECTOR_H
#define ICL_FIXED_VECTOR_H

#include <iclFixedMatrix.h>

namespace icl{
  
  template<class T, int DIM>
  struct FixedColVector : public FixedMatrix<T,1,DIM>{
    FixedColVector(){}
    FixedColVector(const T &init):FixedMatrix<T,1,DIM>(init){}
    FixedColVector(const T *srcData):FixedMatrix<T,1,DIM>(srcData){}
    FixedColVector(const FixedColVector &other):FixedMatrix<T,1,DIM>(other){}
  };

  
  template<class T, int DIM>
  struct FixedRowVector : public FixedMatrix<T,DIM,1>{
    FixedRowVector(){}
    FixedRowVector(const T &init):FixedMatrix<T,DIM,1>(init){}
    FixedRowVector(const T *srcData):FixedMatrix<T,DIM,1>(srcData){}
    FixedRowVector(const FixedRowVector &other):FixedMatrix<T,DIM,1>(other){}
  };
  
  
  
}

#endif
