/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLCore/examples/sampled-line-test.cpp                 **
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
*********************************************************************/

#include <ICLCore/SampledLine.h>
#include <ICLCore/Random.h>

int main(){
  icl::randomSeed();
  icl::URand r(-1000,1000);
  for(int i=0;i<10;++i){
    int x=r,y=r,x2=r,y2=r;
    icl::SampledLine s(x,y,x2,x2);
    std::cout << "Line " << icl::Point(x,y) << " --> " << icl::Point(x2,y2) << std::endl;
    while(s){
      //icl::SampledLine tmp = s; works, but SLOW (of course)
      //s = tmp;
      icl::Point p = *s;
      ++s;
    }
  }
  for(int i=0;i<10;++i){
    int x=r,y=r,x2=r,y2=r;
    icl::SampledLine s(x,y,x2,x2,-100,-100,100,100);
    std::cout << "Line " << icl::Point(x,y) << " --> " << icl::Point(x2,y2) << std::endl;
    while(s){
      //icl::SampledLine tmp = s; works, but SLOW (of course)
      //s = tmp;
      icl::Point p = *s;
      ++s;
    }
  }

}
