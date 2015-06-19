/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ICP3D.cpp                          **
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

#include <ICLGeom/ICP3D.h>
#include <ICLGeom/PoseEstimator.h>

namespace icl {
	namespace geom {

		ICP3D::Result::Result()
			: transformation(math::Mat4::id()),
			  error(0.0f), iterations(0) {}

		//======================================================================

		ICP3D::ICP3D(const uint iterations, const icl32f max_distance, const icl64f errorDelat)
			: maxIterations(iterations),
			  maxDist(max_distance),
			  errorDeltaTh(errorDelat),
			  octree(0) {}

		//======================================================================

		void ICP3D::build(std::vector<ICP3DVec> const &target,
						  icl32f const &min, icl32f const &len) {
			m_target.clear();
			m_target = target;
			if (octree)
				delete octree;
			octree = 0;
			octree = new icl::math::Octree<icl32f,16,1,ICP3DVec>(min,len);
			octree->assign(target.begin(),target.end());
		}

		//======================================================================

		void ICP3D::build(std::vector<ICP3DVec> const &target,
						  icl32f const &minX, icl32f const &minY, icl32f const &minZ,
						  icl32f const &width, icl32f const &height, icl32f const &depth) {
			m_target.clear();
			m_target = target;
			if (octree)
				delete octree;
			octree = 0;
			octree = new icl::math::Octree<icl32f,16,1,ICP3DVec>(minX,minY,minZ,
																 width,height,depth);
			octree->assign(target.begin(),target.end());
		}

		//======================================================================

		ICP3D::Result ICP3D::apply(std::vector<ICP3DVec> const &source,
								   std::vector<ICP3DVec> &out) {
			if (!source.size() || !octree) {
				return Result();
			}

			icl64f e_sum = std::numeric_limits<double>::max()-1;
			icl64f e_delta = std::numeric_limits<double>::max();
			math::Mat4 complete_transform = math::Mat4::id();

			// initialize output
			out.clear();
			out.insert(out.begin(),source.begin(),source.end());

			int iterations = maxIterations;
			try {
				while (true) {

					icl64f cur_err = 0;
					std::vector<ICP3DVec> model_matches;
					std::vector<ICP3DVec> in_matches;

					icl64f max_dist = 0;
					for (uint i = 0; i < out.size(); ++i) {
						ICP3DVec &v1 = out[i];
						ICP3DVec v2;
						v2 = octree->nn(v1);
						icl64f error = icl::math::dist3(v2,v1);
						if (maxDist >= error) {
							model_matches.push_back(v2);
							in_matches.push_back(v1);
							max_dist = std::max(max_dist,error);
							cur_err += error*error;
						}
					}
					cur_err = std::sqrt(cur_err / out.size());

					e_delta = e_sum - cur_err;
					if (e_delta <= 0.f) {
						break;
					}

					e_sum = cur_err;

					if (std::fabs(e_delta) <= errorDeltaTh) {
						break;
					}

					math::Mat4 transform;
					transform = icl::geom::PoseEstimator::map(in_matches,model_matches,
																  icl::geom::PoseEstimator::RigidBody);

					math::Mat3 rot = transform.part<0,0,3,3>();
					icl::icl32f det = rot.det();

					if (std::fabs(det - 1.0f) > 0.01) {
						transform.part<0,0,3,3>() = math::gramSchmidtOrtho(rot);
						INFO_LOG("ICP3D::apply(): Orthogonalization of rotation matrix!");
					}

					complete_transform = transform * complete_transform;
					// do transformation
					for (uint i = 0; i < out.size(); ++i) {
						out[i] = transform * out[i];
					}

					--iterations;
					if (iterations <= 0) {
						iterations = 0;
						break;
					}
				}

			} catch(icl::utils::ICLException const &e) {
				WARNING_LOG(e.what());
				Result r;
				r.transformation = math::Mat4::id();
				r.error = std::numeric_limits<icl32f>::max();
				r.iterations = maxIterations;
				return r;
			}

			Result r;
			r.transformation = complete_transform;
			r.error = e_sum;
			r.iterations = maxIterations-iterations;
			return r;
		}

		//======================================================================

		ICP3D::Result ICP3D::apply(std::vector<ICP3DVec> const &target,
								   std::vector<ICP3DVec> const &source,
								   std::vector<ICP3DVec> &out,
								   icl32f const &min, icl32f const &len) {
			this->build(target,min,len);
			return this->apply(source,out);
		}

		//======================================================================

		ICP3D::Result ICP3D::apply(std::vector<ICP3DVec> const &target,
								   std::vector<ICP3DVec> const &source,
								   std::vector<ICP3DVec> &out,
								   icl32f const &minX, icl32f const &minY, icl32f const &minZ,
								   icl32f const &width, icl32f const &height, icl32f const &depth) {

			this->build(target,minX,minY,minZ,width,height,depth);
			return this->apply(source,out);
		}

		//======================================================================

		ICP3D::~ICP3D() {
			if (octree)
				delete octree;
			octree = 0;
		}

	} // namespace geom
} // namespace icl
