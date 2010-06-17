/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/examples/img-iterator-benchmark.cpp            **
** Module : ICLCore                                                **
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

#include <ICLCore/Img.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Random.h>

/**
no opt:
-O4 -march=native -funroll-loops Centrino-Pro 2x2GHz (using single core)
calls[       1]  time[ 73.2 ms]  avg[ 73.2 ms]  min[ 73.2 ms]  max[ 73.2 ms]  {find_min_ipp}
calls[       1]  time[  2.0  s]  avg[  2.0  s]  min[  2.0  s]  max[  2.0  s]  {find_min_iterator_cpp_inRegion}
calls[       1]  time[  1.8  s]  avg[  1.8  s]  min[  1.8  s]  max[  1.8  s]  {find_min_iterator_cpp}
calls[       1]  time[  1.4  s]  avg[  1.4  s]  min[  1.4  s]  max[  1.4  s]  {find_min_pointer_cpp}
calls[       1]  time[  1.4  s]  avg[  1.4  s]  min[  1.4  s]  max[  1.4  s]  {find_min_iterator_stl}
calls[       1]  time[  1.4  s]  avg[  1.4  s]  min[  1.4  s]  max[  1.4  s]  {find_min_pointer_stl}
**/


using namespace icl;
#define M 1000

icl8u find_min_ipp(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u val = 0;
  for(int j=0;j<M;++j){
    val = i.getMin();
  }
  return val;
}

icl8u find_min_pointer_stl(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u min = 0;
  for(int j=0;j<M;++j){
    min = *std::min_element(i.begin(0),i.end(0));
  }
  return min;
}
icl8u find_min_iterator_stl(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u min = 0;
  for(int j=0;j<M;++j){
    min = *std::min_element(i.beginROI(0),i.endROI(0));
  }
  return min;
}

icl8u find_min_pointer_cpp(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u min = 0;
  for(int j=0;j<M;++j){
    const icl8u *p = i.begin(0);
    const icl8u *end = p+i.getDim();
    icl8u minVal = *p++;
    for(;p!=end;++p){
      if(*p < minVal) minVal = *p;
    }
    min = minVal;
  }
  return min;
}
icl8u find_min_iterator_cpp(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u min = 0;
  for(int j=0;j<M;++j){
    Img8u::roi_iterator p = i.beginROI(0);
    Img8u::const_roi_iterator end = i.endROI(0);

    Img8u::roi_iterator minIt = p;
    while(++p != end){
      if(*p < *minIt) minIt = p;
    }
    min = *minIt;
  }
  return min;
}
icl8u find_min_iterator_cpp_inRegion(const Img8u &i){
  BENCHMARK_THIS_FUNCTION;
  icl8u min = 0;
  for(int j=0;j<M;++j){
    Img8u::roi_iterator p = i.beginROI(0);
    icl8u minVal = *p++;
    for(;p.inRegionSubROI();++p){
      if(*p<minVal) minVal = *p;
    }
    min = minVal;
  }
  return min;
}



int main(){
  std::cout << "benchmark: 1000x1000-icl8u gray-image," << std::endl
            << M << " iterations, searching min element" << std::endl
            << "different algorithms. Test showed, that" << std::endl
            << "minVal = std::min(minVal,currVal) is significantly" << std::endl
            << "slower than if(currVal<minVal)minVal=currVal heuristic." << std::endl
            << "If ICL is compiled without IPP support, IPP-results" << std::endl
            << "meets std::min_element performance" << std::endl << std::endl;
  
  randomSeed();

  Img8u a(Size(1000,1000),1);
  std::fill(a.begin(0),const_cast<icl8u*>(a.end(0)),100);
  
  find_min_pointer_stl(a);  
  find_min_iterator_stl(a);  

  find_min_pointer_cpp(a);  
  find_min_iterator_cpp(a);  
  
  find_min_iterator_cpp_inRegion(a);

  find_min_ipp(a);
}
