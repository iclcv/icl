#include <ICLSimpleMatrix.h>
#include <vector>
#ifndef HUNGARIAN_ALGORITHM_H
#define HUNGARIAN_ALGORITHM_H


namespace icl{
/**
Benchmark: 41ms for a 100² matrix
           0.0375ms for 10² matrix
           12s for  a 500² matrix
*/
  template<class real>
  class HungarianAlgorithm {
    typedef SimpleMatrix<real> mat;
    public:
    static std::vector<int> apply(const SimpleMatrix<real> &m, bool isCostMatrix=true);
    static void visualizeAssignment(const SimpleMatrix<real> &cost, const std::vector<int> &assignment);
    static real calculateError(const SimpleMatrix<real> &cost, const std::vector<int> &assignement);
  };

} //namespace

#endif
