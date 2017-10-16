/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ICP.h                              **
** Module : ICLGeom                                                **
** Authors: Christian Groszewski, Christof Elbrechter              **
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
    class ICLGeom_API ICP : public utils::Uncopyable{
      public:
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

