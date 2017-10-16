/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/GraphCutter.h                      **
** Module : ICLMath                                                **
** Authors: Andre Ueckermann                                       **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLMath/DynMatrix.h>
#include <ICLUtils/Point.h>

namespace icl{
  namespace math{

    /// class for graph cut algorithms on undirected graphs (a graph is represented by an adjacency matrix).
    /** The GraphCutter class implements the CONTRACT/CAP algorithm from H. Nagamochi, T. Ono, T. Ibaraki, "Implementing an efficient
        minimum capacity cut algorithm", Mathematical Programming 67 (1994). */
	  class ICLMath_API GraphCutter{

  	  public:

        typedef struct{
          std::vector<int> subset;
          int parent;
          std::vector<int> children;
          float cost;
        }CutNode;

        /// Applies a single minimum cut on a connected undirected graph
        /** @param adjacencyMatrix the input symmetric adjacency matrix (0 for "no edge", >0 for edge capacity)
            @param subset1 pointer to the first subset result
            @param subset2 pointer to the second subset result
            @return the cut cost.
        */
        static float minCut(DynMatrix<float> &adjacencyMatrix, std::vector<int> &subset1, std::vector<int> &subset2);

        /// Applies minimum cut as long as a cut with costs smaller a given threshold exists and returns a list of subsets (for weighted graphs).
        /** @param adjacencyMatrix the input symmetric adjacency matrix (0 for "no edge", >0 for edge capacity)
            @param threshold the maximum cut cost
            @return a list of subsets.
        */
        static std::vector<std::vector<int> > thresholdCut(DynMatrix<float> &adjacencyMatrix, float threshold);

        /// Applies minimum cut as long as a cut with costs smaller a given threshold exists and returns a list of subsets (unweighted graphs will be weighted).
        /** @param adjacencyMatrix the input symmetric adjacency matrix (0 for "no edge", 1 for "edge")
            @param threshold the maximum cut cost
            @return a list of subsets.
        */
        static std::vector<std::vector<int> > thresholdCut(DynMatrix<bool> &adjacencyMatrix, float threshold);

        /// Applies hierarchical minimum cut and returns a list of nodes including subset, parent, children and weight representing a cut tree (for weighted graphs).
        /** @param adjacencyMatrix the input symmetric adjacency matrix (0 for "no edge", >0 for edge weights)
            @return a list of tree-nodes.
        */
        static std::vector<CutNode> hierarchicalCut(DynMatrix<float> &adjacencyMatrix);

        /// Applies hierarchical minimum cut and returns a list of nodes including subset, parent, children and weight representing a cut tree (unweighted graphs will be weighted).
        /** @param adjacencyMatrix the input symmetric adjacency matrix (0 for "no edge", 1 for "edge")
            @return a list of tree-nodes.
        */
        static std::vector<CutNode> hierarchicalCut(DynMatrix<bool> &adjacencyMatrix);

        /// Creates a weighted matrix from an unweighted boolean matrix.
        /** @param initialMatrix the unweighted boolean matrix (0 for "no edge", 1 for "edge")
            @return a weighted matrix.
        */
        static math::DynMatrix<float> calculateProbabilityMatrix(math::DynMatrix<bool> &initialMatrix, bool symmetry=true);

        /// Find all unconnected subgraphs and return a list of the subsets
        /** @param adjacencyMatrix the input matrix (0 for "no edge", >0 for edge weights)
            @return a list of subsets.
        */
        static std::vector<std::vector<int> > findUnconnectedSubgraphs(DynMatrix<float> &adjacencyMatrix);

	      /// Creates a submatrix with a given subset.
        /** @param adjacencyMatrix the input matrix
            @param subgraph the subset of nodes in the matrix
            @return a matrix from the subset.
        */
	      static DynMatrix<float> createSubMatrix(DynMatrix<float> &adjacencyMatrix, std::vector<int> &subgraph);

	      /// Merges the src matrix into the dst matrix (dst is the maximum of both matrices).
        /** @param dst the destination matrix
            @param src the source matrix
        */
	      static void mergeMatrix(DynMatrix<bool> &dst, DynMatrix<bool> &src);

	      /// Weights the dst matrix (value*=weight) for all true cells in the featureMatrix.
        /** @param dst the destination matrix
            @param featureMatrix the boolean feature matrix (indicates where to weight)
            @param weight the weight
        */
	      static void weightMatrix(DynMatrix<float> &dst, DynMatrix<bool> &featureMatrix, float weight);

      private:
        static std::vector<float> capforest(std::vector<utils::Point> &edgeList, std::vector<float> &edgeCosts, int subsetsSize);

        static float initialLambda(DynMatrix<float> &adjacencyMatrix, int &lambda_id);

        static void createEdgeList(DynMatrix<float> &adjacencyMatrix, std::vector<utils::Point> &edgeList, std::vector<float> &edgeCosts);

        static std::vector<std::vector<int> > createInitialNodes(DynMatrix<float> &adjacencyMatrix);

        static float merge(std::vector<utils::Point> &edgeList, std::vector<float> &edgeCosts, std::vector<float> &q,
	                                   std::vector<std::vector<int> > &subsets, float lambda_score, int j, int &lambda_id);
    };

  } // namespace math
}
