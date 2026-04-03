// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLMath/Octree.h>
#include <ICLMath/HomogeneousMath.h>

namespace icl::geom {
		/**
		 * @brief The ICP3D class is a special ICP-case for homogeneous 3D-vectors
		 */
		class ICLGeom_API ICP3D {

		public:
			ICP3D(const ICP3D&) = delete;
			ICP3D& operator=(const ICP3D&) = delete;


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

			using ICP3DVec = icl::math::FixedColVector<icl32f,4>;

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
	} // namespace icl::geom