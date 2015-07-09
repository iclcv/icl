/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ICP3D.h                            **
** Module : ICLGeom                                                **
** Authors: Tobias Roehlig                                         **
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

#include <ICLUtils/Uncopyable.h>
#include <ICLMath/Octree.h>
#include <ICLMath/HomogeneousMath.h>

namespace icl {
	namespace geom {

		/**
		 * @brief The ICP3D class is a special ICP-case for homogeneous 3D-vectors
		 */
		class ICLGeom_API ICP3D : public utils::Uncopyable {

		public:

			/**
			 * @brief The Result struct contains results of an ICP-loop
			 */
			struct Result {
				Result();
				/// @brief transformation Final transformation of ICP
				math::Mat4 transformation;
				/// @brief error The final error
				icl64f error;
				/// @brief iterations The number of iterations needed
				uint32_t iterations;
			};

			typedef icl::math::FixedColVector<icl32f,4> ICP3DVec;

			/**
			 * @brief ICP3D Standard constructor
			 */
			ICP3D(uint32_t const iterations = 10,
				  icl32f const max_distance = 1.0f,
				  icl64f const errorDelat = 0.01f);

			/**
			 * @brief build creates the octree and saves the target
			 * @param target Target points for the ICP steps
			 * @param min Octree minimum for all axis
			 * @param len Octree length for all axis
			 */
			void build(std::vector<ICP3DVec> const &target,
					   icl32f const &min, icl32f const &len);

			/**
			 * @brief build creates the octree and saves the target
			 * @param target Target points for the ICP steps
			 * @param minX Octree minimum for the x-axis
			 * @param minY Octree minimum for the y-axis
			 * @param minZ Octree minimum for the z-axis
			 * @param width Octree length for the x-axis
			 * @param height Octree length for the y-axis
			 * @param depth Octree length for the z-axis
			 */
			void build(std::vector<ICP3DVec> const &target,
					   icl32f const &minX, icl32f const &minY, icl32f const &minZ,
					   icl32f const &width, icl32f const &height, icl32f const &depth);

			/**
			 * @brief apply Performs the ICP loop
			 * @param source The points to be aligned to the target
			 * @param out The result of the alignment (transformed source)
			 * @return The result for this ICP run (final transformation,error,iterations)
			 */
			Result apply(std::vector<ICP3DVec> const &source,
						 std::vector<ICP3DVec> &out);

			/**
			 * @brief apply Performs the ICP loop
			 * @param target Target points for the ICP steps
			 * @param source The points to be aligned to the target
			 * @param out The result of the alignment (transformed source)
			 * @param min Octree minimum for all axis
			 * @param len Octree length for all axis
			 * @return The result for this ICP run (final transformation,error,iterations)
			 */
			Result apply(std::vector<ICP3DVec> const &target,
						 std::vector<ICP3DVec> const &source,
						 std::vector<ICP3DVec> &out,
						 icl32f const &min, icl32f const &len);

			/**
			 * @brief apply Performs the ICP loop
			 * @param target Target points for the ICP steps
			 * @param source The points to be aligned to the target
			 * @param out The result of the alignment (transformed source)
			 * @param minX Octree minimum for the x-axis
			 * @param minY Octree minimum for the y-axis
			 * @param minZ Octree minimum for the z-axis
			 * @param width Octree length for the x-axis
			 * @param height Octree length for the y-axis
			 * @param depth Octree length for the z-axis
			 * @return The result for this ICP run (final transformation,error,iterations)
			 */
			Result apply(std::vector<ICP3DVec> const &target,
						 std::vector<ICP3DVec> const &source,
						 std::vector<ICP3DVec> &out,
						 icl32f const &minX, icl32f const &minY, icl32f const &minZ,
						 icl32f const &width, icl32f const &height, icl32f const &depth);

			virtual
			~ICP3D();

			void setMaxDistance(icl32f const dist) { maxDist = dist; }
			icl32f getMaxDistance() { return maxDist; }

			void setErrorDeltaTh(icl64f const th) { errorDeltaTh = th; }
			icl64f getErrorDeltaTh() { return errorDeltaTh; }

			void setMaximumIterations(uint32_t const iterations) { maxIterations = iterations; }
			uint32_t getMaximumIterations() { return maxIterations; }

			std::vector<ICP3DVec> const &getTarget() { return m_target; }

			/// @brief maxIterations Maximum number of iterations
			uint32_t maxIterations;
			/// @brief maxDist Maximal distance allowed between two corresponding points
			icl32f maxDist;
			/**
			 * @brief errorDeltaTh Threshold for the change of the error between to
			 * ICP-steps. If the error changed less than this threshold the ICP-loop
			 * converged and the accumulated transformation is returned.
			 */
			icl64f errorDeltaTh;

			/// @brief octree Underlying octree
			math::Octree<icl32f,16,1,ICP3DVec> *octree;
			/// @brief m_target Target for the ICP steps
			std::vector<ICP3DVec> m_target;

		}; // class ICP3D
	} // namespace geom
} // namespace icl

