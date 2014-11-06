#include <ICLPhysics/TriangleIntersectionEstimator.h>
#include<ICLPhysics/SoftObject.h>

namespace icl{

  using namespace utils;
  using namespace geom;
namespace physics{
  static inline float dot(const Vec &a, const Vec &b){
    return a[0]*b[0] +a[1]*b[1] +a[2]*b[2]; 
  }
  
  TriangleIntersectionEstimator::Intersection
  TriangleIntersectionEstimator::find(const TriangleIntersectionEstimator::Triangle &t, const ViewRay &r){
    //Vector    u, v, n;             // triangle vectors
    //Vector    dir, w0, w;          // ray/line vectors
    //float     r, a, b;             // parameters to estimate ray-plane intersect
    static const float EPSILON = 0.00000001;
    // get triangle edge vectors and plane normal
    Vec u = t.b - t.a;
    Vec v = t.c - t.a;
    Vec n = cross(v,u);  // TEST maybe v,u ??
    if (fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON){
      return degenerateTriangle;
    }
      
    const Vec dir = r.direction;  
    Vec w0 =  r.offset - t.a;  

    float a = -dot(n,w0);
    float b = dot(n,dir);
    if (fabs(b) < EPSILON) {     // ray is parallel to triangle plane
      return a<EPSILON ? Intersection(rayIsCollinearWithTriangle) : Intersection(noIntersection);
    }
      
    // get intersect point of ray with triangle plane
    float rr = a / b;
    if (rr < 0) {
      return Intersection(wrongDirection);
    }
      
    Vec intersection = r.offset + dir * rr;

    // is I inside T?
    float uu = dot(u,u);
    float uv = dot(u,v);
    float vv = dot(v,v);
    Vec w = intersection - t.a;
    float wu = dot(w,u);
    float wv = dot(w,v);
    float D = uv * uv - uu * vv;
      
    // get and test parametric coords
    float s = (uv * wv - vv * wu) / D;
    float tt = (uv * wu - uu * wv) / D;
      
    if (s < 0.0 || s > 1.0){
      return Intersection(noIntersection,intersection,Point32f(s,tt));
    }
    if (tt < 0.0 || (s + tt) > 1.0){
      return Intersection(noIntersection,intersection,Point32f(s,tt));
    }
    intersection[3] = 1;
    return Intersection(foundIntersection,intersection,Point32f(s,tt));
  }

}
}
