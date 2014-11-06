#include <ICLPhysics/GeometricTools.h>

namespace icl{
  namespace physics{
    using namespace utils;
    using namespace geom;


    bool line_segment_intersect(const Point32f &a, const Point32f &b, const Point32f &c,
                                const Point32f &d, Point32f *dst,
                                float *dstr, float *dsts){
      const float x1 = (a.y-c.y)*(d.x-c.x)-(a.x-c.x)*(d.y-c.y);
      const float x2 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
      if(x1 == 0 && x2 == 0) return false; // lines are collinear
      if(x2 == 0) return false; // lines are parallel

      const float x3 = (a.y-c.y)*(b.x-a.x)-(a.x-c.x)*(b.y-a.y);
      const float x4 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
      if(x4 == 0) return false;
      const float r = x1 / x2;
      const float s = x3 / x4;

      if(dstr) *dstr = 1.0f-r;
      if(dsts) *dsts = 1.0f-s;

      if(r >= 0 && r <= 1 && s >= 0 && s <= 1){
        if(dst) *dst = a + (b-a)*r;
        return true;
      }
      return false;
    }

    inline float point_in_triangle_sign(const Point32f &p1, const Point32f &p2, const Point32f &p3){
      return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    bool point_in_triangle(const Point32f &p, const Point32f &v1, const Point32f &v2, const Point32f &v3){
      bool b1 = point_in_triangle_sign(p, v1, v2) <= 0;
      bool b2 = point_in_triangle_sign(p, v2, v3) <= 0;
      bool b3 = point_in_triangle_sign(p, v3, v1) <= 0;
      return (b1 == b2) && (b2 == b3);
    }


  }
}
