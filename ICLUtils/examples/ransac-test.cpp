/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/ransac-test.cpp                      **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/RansacFitter.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Point32f.h>

using namespace icl;

typedef FixedColVector<float,2> Line;

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

double line_dist(const Line &line, const Point32f &p){
  return sqr(line[0] + line[1]*p.x - p.y);
}

static const Line THE_LINE(1.23456, 7.89);

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

  RansacFitter<Point32f,Line> fitLine(2,1000,fit_line,line_dist,1.5,30);
  std::cout << "original line was " <<   THE_LINE.transp() << std::endl;
  RansacFitter<Point32f,Line>::Result r = fitLine.fit(get_line_points());
  
  std::cout << "fitted result was " << r.model.transp() << std::endl;
  std::cout << "fitting error was " << r.error << std::endl;
}
