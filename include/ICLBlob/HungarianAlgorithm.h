/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/HungarianAlgorithm.h                   **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef HUNGARIAN_ALGORITHM_H
#define HUNGARIAN_ALGORITHM_H

#include <ICLUtils/SimpleMatrix.h>
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
