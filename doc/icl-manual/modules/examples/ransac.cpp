#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Point32f.h>

#include <ICLMath/FixedVector.h>
#include <ICLMath/RansacFitter.h>

using namespace icl::utils;
using namespace icl::math;

// the line model (y=mx+b) is defined by two values
typedef FixedColVector<float,2> Line;

// the fitting is done using standard least squares approach
Line fit_line(const std::vector<Point32f> &pts){
  int n = pts.size();
  DynMatrix<float> xs(n,2), ys(n,1);
  for(int i=0;i<n;++i){
    xs(i,0) = 1;
    xs(i,1) = pts[i].x;
    ys(i,0) = pts[i].y;
  }
  DynMatrix<float> fit = ys * xs.pinv(true);
  return Line(fit[0],fit[1]);
}

// distance function for single points (y-distance here)
double line_dist(const Line &line, const Point32f &p){
  return sqr(line[0] + line[1]*p.x - p.y);
}

// the original line
static const Line THE_LINE(1.23456, 7.89);

// create test data:
// 50% noisy point on the line
// 50% random outliers
const std::vector<Point32f> get_line_points(){
  Line l = THE_LINE;
  std::vector<Point32f> pts(100);
  URand r(-100,100);
  GRand gr(0,1);
  for(int i=0;i<50;++i){
    pts[i].x = r;
    pts[i].y = l[0] + l[1]* pts[i].x + gr;
  }
  for(int i=0;i<50;++i){
    pts[i+50] = Point32f(r,r);
  }
  return pts;
}

int main(int n, char **ppc){
  randomSeed();

  // create the fitter
  RansacFitter<Point32f,Line> fitLine(2,1000,fit_line,line_dist,1.5,30);

  // fit ...
  RansacFitter<Point32f,Line>::Result r = fitLine.fit(get_line_points());

  // show results
  std::cout << "original line was " << THE_LINE.transp() << std::endl;
  std::cout << "fitted result was " << r.model.transp() << std::endl;
  std::cout << "fitting error was " << r.error << std::endl;
}


