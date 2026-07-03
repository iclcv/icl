// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Random.h>
#include <icl/math/la/DynVector.h>
#include <icl/math/la/FixedVector.h>
#include <functional>
#include <cmath>

namespace icl::math {
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
    using DataSet = std::vector<DataPoint>;

    /// Function for the fitting module (gets a dataset and returns the fitted model)
    using ModelFitting = std::function<Model(const DataSet&)>;

    /// Error function for single points
    using PointError = std::function<icl64f(const Model&, const DataPoint&)>;

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
      utils::get_random_subset(allPoints, static_cast<int>(currConsensusSet.size()),
                               currConsensusSet, usedIndices);
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

      // Adaptive termination: once a model with inlier ratio w is found, the
      // number of samples needed to hit the desired confidence P is
      // N = log(1-P) / log(1 - w^s). We shrink the iteration budget towards N
      // as better models appear, never running more than the requested cap.
      const double P_CONF = 0.99;
      const int S = m_minPointsForModel;
      int nRequired = m_iterations;

      int i = 0;
      for(i=0;i<m_iterations && i<nRequired;++i){
        consensusSet.resize(m_minPointsForModel);
        find_random_consensus_set(consensusSet, allPoints, usedIndices);

        Model model = m_fitting(consensusSet);
        for(int j=0;j<static_cast<int>(allPoints.size());++j){
          if(find_in(usedIndices, j, usedIndices.size())) continue;
          if(m_err(model, allPoints[j]) < m_maxModelDistance){
            consensusSet.push_back(allPoints[j]);
          }
        }


        if(static_cast<int>(consensusSet.size()) >= m_minClosePointsForGoodModel){
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

          // update the adaptive iteration budget from this model's inlier ratio
          const double w = double(consensusSet.size()) / allPoints.size();
          if(w >= 1.0){
            nRequired = i + 1;   // all inliers — no point sampling further
          }else if(w > 0.0){
            const double denom = std::log(1.0 - std::pow(w, S));
            if(denom < 0.0){
              const int est = static_cast<int>(std::log(1.0 - P_CONF) / denom) + 1;
              if(est < nRequired) nRequired = est;
            }
          }
        }
      }
      m_result.iterationCount = i;
      return m_result;
    }
  };
  } // namespace icl::math