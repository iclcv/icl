#include <iclHoughLine.h>
#include <iclPoint32f.h>
#include <iclRange.h>
#include <iclFixedMatrix.h>
#include <cmath>

namespace icl{

  /// create dedicated file for this class
  HoughLine::HoughLine():m_distance(0),m_angle(0){}
  
  HoughLine::HoughLine(const Point32f &offs, const Point32f &dir):
    m_offset(offs),m_direction(dir){
    
    m_angle=::atan2(m_direction.y,m_direction.x);
    m_distance=m_offset.norm();
    
  }
  HoughLine::HoughLine(icl32f distance, icl32f angle):
    m_distance(distance),m_angle(angle){
    
    float c = ::cos(angle);
    float s = ::sin(angle);
    m_offset = Point32f(c,s)*distance;
    m_direction = Point32f(-s,c);
  }
  
  Point32f HoughLine::getIntersection(const HoughLine &a, const HoughLine &b){
    
    float sa = ::sin(a.m_angle);
    float sb = ::sin(b.m_angle);
    float ca = ::cos(a.m_angle);
    float cb = ::cos(b.m_angle);
    const float &r = a.m_distance;
    const float &s = b.m_distance;
    
    FixedMatrix<float,2,2> M(-sa, sb,
                             ca, -cb);
    FixedMatrix<float,1,2> B(-r*ca + s*cb,
                             -r*sa + s*sb);
    
    try{ // Maybe M cannot be inverted if lines are colinear
      FixedMatrix<float,1,2> x = M.inv()*B; 
      // insert x[0] = lamda1 into line-equation a
      return a.m_offset + (a.m_direction*x[0]);
    }catch(SingularMatrixException&){}
    return NO_INTERSECTION;
  }

  
  std::vector<Point32f> HoughLine::getPairwiseIntersections(const std::vector<HoughLine> &lines){
    std::vector<Point32f> v;
    for(unsigned int i=0;i<lines.size();++i){
      for(unsigned int j=i+1;j<lines.size();++j){
        Point32f p = getIntersection(lines[i],lines[j]);
        if(p != NO_INTERSECTION){
          v.push_back(p);
        }
      }
    }
    return v;

  }
  
  const Point32f HoughLine::NO_INTERSECTION = Point32f(Range32f::limits().maxVal,Range32f::limits().maxVal);
  
}
