#include "gtest/gtest.h"
#include "ICLGeom/Camera.h"

using namespace icl::geom;
using namespace icl::utils;
using namespace icl::math;

template <typename M1, typename M2>
void expect_equal(const M1& m1, const M2& m2, double eps = std::numeric_limits<double>::epsilon()) {
    ASSERT_EQ(m1.rows(), m2.rows());
    ASSERT_EQ(m1.cols(), m2.cols());
    for (unsigned int r=0; r < m1.rows(); ++r) {
        for (unsigned int c=0; c < m1.cols(); ++c)
            EXPECT_NEAR(m1(r,c), m2(r,c), eps);
    }
}
#define VALIDATE_EQ(...) {\
	SCOPED_TRACE("expect_equal(" #__VA_ARGS__ ")"); \
	expect_equal(__VA_ARGS__); \
}

TEST(Camera, projection)
{
    double eps = std::numeric_limits<float>::epsilon();
    Camera cam(Vec(0,0,0,1), Vec(0,0,-1,1), Vec(1,0,0,1), // pos, normal, up
               1.0, Point32f(320,240)); // f, principal point offset

    VALIDATE_EQ(cam.getQMatrix() * cam.getInvQMatrix(), FixedMatrix<icl::icl32f,3,3>::id(), 1e3 * eps);
    auto id = FixedMatrix<icl::icl32f,4,4>::id(); id(3,3) = 0.0;
    VALIDATE_EQ(cam.getInvQMatrix() * cam.getQMatrix(), id, 1e2 * eps);
}

TEST(Camera, viewray)
{
    double eps = 5*std::numeric_limits<float>::epsilon();
    Camera cam(Vec(0,0,0,1), Vec(0,0,-1,1), Vec(1,0,0,1), // pos, normal, up
               1.0, Point32f(320,240)); // f, principal point offset

    auto dir = Vec(-0.53665620, -0.71554178, -0.44721356, 1);
    ViewRay ray = cam.getViewRay(Point32f(0,0));
    VALIDATE_EQ(ray.offset, Vec(0,0,0,1), eps);
    VALIDATE_EQ(ray.direction, dir, eps);

    ray = cam.getViewRay(Point32f(0,480)); dir[0] *= -1.0;
    VALIDATE_EQ(ray.offset, Vec(0,0,0,1), eps);
    VALIDATE_EQ(ray.direction, dir, eps);

    ray = cam.getViewRay(Point32f(640,480)); dir[1] *= -1.0;
    VALIDATE_EQ(ray.offset, Vec(0,0,0,1), eps);
    VALIDATE_EQ(ray.direction, dir, eps);

    ray = cam.getViewRay(Point32f(640,0)); dir[0] *= -1.0;
    VALIDATE_EQ(ray.offset, Vec(0,0,0,1), eps);
    VALIDATE_EQ(ray.direction, dir, eps);
}
