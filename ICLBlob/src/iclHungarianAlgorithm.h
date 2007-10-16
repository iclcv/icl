#ifndef HUNGARIAN_ALGORITHM_H
#define HUNGARIAN_ALGORITHM_H

#include <iclSimpleMatrix.h>
#include <vector>

namespace icl{
  
  /// Implementation of the Hungarian Algorithm to solve Linear Assignment problems \ingroup G_PT \ingroup G_UTILS
  /** @see PositionTracker
      
      \section Linear Assignment Problems (LAP)
      A LAP is defined as follows:
      You have \f$N\f$ workers \f$w_i\f$ and  \f$N\f$ tasks \f$t_j\f$. Assigning a certain worker 
      \f$w_i\f$ to a certain task \f$t_j\f$ produces costs \f$d_{ij}\f$ which are defined by a 
      square cost matrix \f$D=\{d_{ij}\}\f$. Each worker can only be assigned to perform 
      a single task, and each task has to be processed. The problem is to find the
      optimal assignment to minimize the arising costs.
      
      \section BENCH Benchmark (Pentium-M 1.6GHz)
      
      - 41ms for a 100² matrix
      - 0.0375ms for 10² matrix
      - 12s for  a 500² matrix
      */
  template<class real>
  class HungarianAlgorithm {
    /// Internal used cost matrix type
    typedef SimpleMatrix<real> mat;
    public:
    
    /// calculate best assignment given cost matrix m
    /** if isCostMatrix is false, its elements are internally multiplied by -1 */
    static std::vector<int> apply(const SimpleMatrix<real> &m, bool isCostMatrix=true);

    /// visualized the assignment with given cost matrix and assignment vector
    static void visualizeAssignment(const SimpleMatrix<real> &cost, const std::vector<int> &assignment);

    /// calculates the error made by a given const matrix and assignment vector
    static real calculateError(const SimpleMatrix<real> &cost, const std::vector<int> &assignement);
  };

} //namespace

#endif
