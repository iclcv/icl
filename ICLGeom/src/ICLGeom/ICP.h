// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLMath/DynMatrix.h>
#include <ICLMath/KDTree.h>
#include <ICLGeom/PoseEstimator.h>

namespace icl{
  namespace geom{

    /// Implementaiton of the Iterator Closest utils::Point (ICP) Algorithm
    /** TODO: Add Documentation
        What about a fixed 3D-Version that uses 3D-Fixed Matrix data?
    */
    class ICLGeom_API ICP {
      public:
      ICP(const ICP&) = delete;
      ICP& operator=(const ICP&) = delete;

      /// Simple result structure
      struct ICLGeom_API Result{
        Result();
        math::DynMatrix<icl64f> rotation;    //!< Model rotation matrix
        math::DynMatrix<icl64f> translation; //!< Model translation matrix
        double error;                  //!< Error value
      };

      private:
      /// rotation, translation and error value    Result m_result;
      Result m_result;

      /// internal data structure for efficient search
      math::KDTree kdt;

      /// internally used utility function
      math::DynMatrix<icl64f> *compute(const std::vector<math::DynMatrix<icl64f>* > &data,
                                 const std::vector<math::DynMatrix<icl64f>* > &model);
      public:



      /// constructor with given model data
      /** TODO is the data passed shallowly or deeply */
      ICP(std::vector<math::DynMatrix<icl64f> > &model);

      /// constructor with given model data
      /** TODO is the data passed shallowly or deeply */
      ICP(std::vector<math::DynMatrix<icl64f>* > &model);

      /// Empty constructor without given model data
      ICP();

      /// Destructor
      ~ICP();


      /// applies the ICP on given point cloude
      const Result &apply(const std::vector<math::DynMatrix<icl64f>* > &pointlist);

      /// computes th error between given model and data cloude
      static double error(const std::vector<math::DynMatrix<icl64f>* > &dat,
                          const std::vector<math::DynMatrix<icl64f>* > &mod);

  #if 0
      // hope we dont need that anymore ...
      /// Returns the error from last icp(..) call
      inline double getError(){
        return m_result.error;
      }

      /// returns the resulting rotation matrix from the last icp(..) call
      inline const math::DynMatrix<icl64f>& getRotation() const{
        return m_result.rotation;
      }

      /// returns the resulting translation matrix from the last icp(..) call
      inline const math::DynMatrix<icl64f>& getTranslation() const{
        return m_result.translation;
      }
  #endif
    };
  } // namespace geom
}
