/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/RansacFitter.h                     **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Random.h>
#include <ICLMath/DynVector.h>
#include <ICLMath/FixedVector.h>

namespace icl{
  namespace math{
   
    
    /// Generic RANSAC (RAndom SAmpling Consensus) Implementation
    /** The RansacFitter provides a generic framework, for RANSAC based model fitting.
        
        \section ALG RANSAC Algorithm
        The RANSAC Algorithm is well described on Wikipedia 
        @see http://de.wikipedia.org/wiki/RANSAC-Algorithmus
    
        \section EX Example

        An example is given in the ICL Manual
    
        \section TEM Template Parameters
        The two tempalte parameters are kept very general. Therefore, there
        are just a few restrictions for the DataPoint and Model classes.
        - default constructable
        - copyable
    */
    template<class DataPoint=std::vector<float>,
             class Model=std::vector<float> >
    class RansacFitter{
      public:
      /// DataSet type (just a set of DataPoint instances)
      typedef std::vector<DataPoint> DataSet;
      
      /// Function for the fitting module (gets a dataset and returns the fitted model)
      typedef utils::Function<Model,const DataSet&> ModelFitting;
      
      /// Error function for single points
      typedef utils::Function<icl64f,const Model&,const DataPoint&> PointError;
      
      private:
      /// minimum points that are used to create a coarse model
      int m_minPointsForModel;
      
      /// number of iterations
      int m_iterations;
      
      /// maximum distance of a point to the model to become an inlier
      icl64f m_maxModelDistance;
      
      /// minimum amount of inliers for a 'good' model
      int m_minClosePointsForGoodModel;
      
      /// fitting function
      ModelFitting m_fitting;
      
      /// point-model error function
      PointError m_err;
      
      /// min error criterion for early exit
      icl64f m_minErrorExit;
      
      public:
      /// result structure
      struct Result{
        /// reached error
        icl64f error;
        
        /// model (zero sized if no model was found)
        Model model;
        
        /// consensus set of best match (i.e. inliers)
        /** empty if no model was found */
        DataSet consensusSet;
  
        /// number of iterations needed
        int iterationCount;
        
        /// returns whether any model was found
        bool found() const { return consensusSet.size(); }
      };
      
      private:
      /// internal result buffer
      Result m_result;
      
      /// internal utility method
      static inline bool find_in(const std::vector<int> &v, int i, int n){
        return std::find(v.data(), v.data()+n, i) != v.data()+n;
      }
  
      /// internal utility method
      void find_random_consensus_set(DataSet &currConsensusSet,
                                     const DataSet &allPoints,
                                     std::vector<int> &usedIndices){
        const int n = currConsensusSet.size();
        utils::URandI r(allPoints.size()-1);
        
        for(int i=0;i<n;++i){
          do { usedIndices[i] = r; } while ( find_in(usedIndices, usedIndices[i], i-1) );
          currConsensusSet[i] = allPoints[ usedIndices[i] ];
        }
      }
      
      public:
      /// empty constructor (creates a dummy instance)
      RansacFitter(){}
      
      /// constructor with given parameters
      /** The parameters functionality is documented with the
          analogously named member variables */
      RansacFitter(int minPointsForModel, 
                   int iterations, 
                   ModelFitting fitting,
                   PointError err,
                   icl64f maxModelDistance,
                   int minClosePointsForGoodModel,
                   icl64f minErrorExit=0):
        m_minPointsForModel(minPointsForModel),
        m_iterations(iterations),
        m_maxModelDistance(maxModelDistance),
        m_minClosePointsForGoodModel(minClosePointsForGoodModel),
        m_fitting(fitting),m_err(err),
        m_minErrorExit(minErrorExit){
      }
      
      /// fitting function (actual RANSAC algorithm)
      const Result &fit(const DataSet &allPoints){
        m_result.model = Model();
        m_result.consensusSet.clear();
        m_result.error = utils::Range64f::limits().maxVal;
        m_result.iterationCount = 0;
        std::vector<DataPoint> consensusSet(m_minPointsForModel);
        std::vector<int> usedIndices(m_minPointsForModel);
        
        int i = 0;
        for(i=0;i<m_iterations;++i){
          consensusSet.resize(m_minPointsForModel);
          find_random_consensus_set(consensusSet, allPoints, usedIndices);

          /*          std::cout << "   selected indices: [ " 
                    << usedIndices[0] << ", "
                    << usedIndices[1] << ", "
                    << usedIndices[2] << ", "
                    << usedIndices[3] << "]" << std::endl;
              */
          Model model = m_fitting(consensusSet);
          for(int j=0;j<(int)allPoints.size();++j){
            if(find_in(usedIndices, j, usedIndices.size())) continue;
            if(m_err(model, allPoints[j]) < m_maxModelDistance){
              consensusSet.push_back(allPoints[j]);
            }
          }
          
          
          if((int)consensusSet.size() >= m_minClosePointsForGoodModel){
            model = m_fitting(consensusSet);
            double error = 0;
            for(unsigned int j=0;j<consensusSet.size();++j){
              error += m_err(model, consensusSet[j]);
            }
            error /= consensusSet.size();
            
            if(error < m_result.error){
              m_result.error = error;
              m_result.model = model;
              m_result.consensusSet = consensusSet;
              if(m_result.error < m_minErrorExit){
                m_result.iterationCount = i;
                return m_result;
              }
            }
          }
        }
        m_result.iterationCount = i;
        return m_result;
      }
    };
  } // namespace math
}

