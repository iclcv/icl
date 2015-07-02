#include <iostream>

#include <ICLUtils/Random.h>
#include <ICLGeom/ICP3D.h>

using namespace icl::geom;

int main(int argc, char **argv) {

	ICP3D icp;

	std::vector<ICP3D::ICP3DVec> source, target;

	icl::math::Mat4 T = icl::math::create_hom_4x4_trans(5,-5,2);

	for(uint32_t i = 0; i < 100; ++i) {
		ICP3D::ICP3DVec v(icl::utils::random(-50,50),
						  icl::utils::random(-50,50),
						  icl::utils::random(-50,50),
						  1);

		source.push_back(T*v);
		target.push_back(v);
	}

	std::vector<ICP3D::ICP3DVec> result;
	icp.build(target,-100,200);
	icp.setErrorDeltaTh(0.001f);
	icp.setMaxDistance(10.f);
	icp.setMaximumIterations(20);
	ICP3D::Result r = icp.apply(source,result);

	std::cout << "--------------------------\n";
	SHOW(r.error);
	SHOW(r.iterations);
	SHOW(r.transformation);
	std::cout << "--------------------------\n";
	SHOW(T);
}
