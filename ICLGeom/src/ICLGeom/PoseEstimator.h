/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PoseEstimator.h                    **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Robert Haschke                    **
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
#include <ICLGeom/GeomDefs.h>

namespace icl{
  namespace geom{


    /// Utility class for 6D PoseEstimation
    /** Given N points in one coordinate frame and corresponding points in another coordinate frame,
        the relative transformation between these two frames can be computed (the points must not be
        collinear). The PoseEstimator class provides functions to compute the relative mapping between
        coordinate frames. Given a set of points in frame A (called Xs, each point is one column of the
        matrix Xs), and a set of the same size of points in frame B (called Ys, each point is one column
        of the matrix Ys, the map function will return the homogeneous 4x4-transform matrix that
        transforms points givne w.r.t. frame A into points in frame B.
        If MapMode is 'Affine' map needs at least 4 points to solve the problem.

        Internally, a MapMode variable is used to determine the algorithm to compute the mapping.

        The internal implementation is based on an implementation found in VTK 5.6.0, that was also
        re-implemented with libeigen2 from Robert Haschke.
    */
    class PoseEstimator{

      /// Private constructor -> no instances of PoseEstimator possible
      PoseEstimator(){}

      public:
      /// The MapMode determines, how the point-to-point transformation is computed
      enum MapMode {
        Translation,  //!< returns a transformation for the mean translation between the point-sets
        RigidBody,    //!< <B>most common</B> RigidBody mapping (using eigenvector analysis and quaternion representation)
        Similarity,   //!< as RigidBody, but with additional scaling
        Affine        //!< affine mapping: (XX^-1 * XY)^T (here, at least 4 points are needed)
      };

      /// main mapping function, that is called by all other convenience wrappers (instantiated for T=icl32f and T=icl64f)
      /** Xs and Ys must
          - have same column-count
          - same row-count
          - row-cout 3 or 4 (if it is 4, the last row is not used at all)
          - at least one column (however you'll need 3 columns for 6D mapping */
      template<class T> ICLGeom_API
      static math::FixedMatrix<T,4,4> map(const math::DynMatrix<T> &Xs, const math::DynMatrix<T> &Ys, MapMode mode=RigidBody)
      throw (math::IncompatibleMatrixDimensionException,math::SingularMatrixException);

      /// Convenienc template that uses FixedMatrix inputs (available for T=icl32f and T=icl64f)
      /** The inputs's data points are passes to the main map-function using a shallow DynMatrix<T> wrappter*/
      template<class T, unsigned int NUM_POINTS>
      static math::FixedMatrix<T,4,4> map(const math::FixedMatrix<T,NUM_POINTS,3> &Xs,
                                          const math::FixedMatrix<T,NUM_POINTS,3> &Ys, MapMode mode=RigidBody)
        throw (math::IncompatibleMatrixDimensionException,math::SingularMatrixException){
        return map(Xs.dyn(),Ys.dyn(), mode);
      }

      /// Convenienc template that uses FixedMatrix inputs (available for T=icl32f and T=icl64f)
      /** The inputs's data points are passes to the main map-function using a shallow DynMatrix<T> wrappter*/
      template<class T, unsigned int NUM_POINTS>
      static math::FixedMatrix<T,4,4> map(const math::FixedMatrix<T,NUM_POINTS,4> &Xs,
                                    const math::FixedMatrix<T,NUM_POINTS,4> &Ys, MapMode mode=RigidBody)
        throw (math::IncompatibleMatrixDimensionException,math::SingularMatrixException){
        return map(Xs.dyn(),Ys.dyn(), mode);
      }

      /// Convenience function that passes std::vector<Vec> data as DynMatrix<T> to other map function
      static Mat map(const std::vector<Vec> &Xs, const std::vector<Vec> &Ys, MapMode mode=RigidBody)
      throw (utils::ICLException,math::SingularMatrixException,math::IncompatibleMatrixDimensionException){
        ICLASSERT_THROW(Xs.size() == Ys.size(), utils::ICLException("PoseEstimator::map: need same number of input- and output-points"));
        math::DynMatrix<double> XsD(Xs.size(),3),YsD(Ys.size(),3);
        for(unsigned int i=0;i<Xs.size();++i){
          std::copy(Xs[i].begin(),Xs[i].begin()+3, XsD.col_begin(i));
          std::copy(Ys[i].begin(),Ys[i].begin()+3, YsD.col_begin(i));
        }
        return map(XsD,YsD,mode);
      }

      /// utility function (instantiated for depth32f and depth64f)
      template<class T> ICLGeom_API
      static math::FixedMatrix<T,3,3> quaternion_to_rotation_matrix(T w, T x, T y, T z);
    };

  } // namespace geom
}
