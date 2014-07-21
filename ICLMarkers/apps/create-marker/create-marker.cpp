/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/create-marker/create-marker.cpp        **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
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

#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLMarkers/MarkerCodeICL1.h>

int main(int n, char **ppc){
  pa_explain
  ("-i","the first sub-argument defines the marker type (one of bch, icl1 and art). The 2nd sub-argument "
   "defines which marker to create (this is the marker ID in case of type bch and icl1 and "
   "in case of makrer type art, an image filename is expected")
  ("-b","border width, which is only relevant for bch markers (the detectors default value is 2)")
  ("-r","border ratio, which is only relevant for art markers (the detectors default is value is 0.4)")
  ("-o","optionally given output filename. If this is not given, the marker image is shown instead")
  ("-s","output size of the marker image");
  

  pa_init(n,ppc,"-id|-i(type,int) -border-size|-b(int=2) "
          "-border-ratio|-r(float=0.4) -output|-o(filename) "
          "-size|-s(size=300x300) "
          "-show-valid-icl1-codes");

  if(pa("-show-valid-icl1-codes")){
    std::vector<MarkerCodeICL1> cs = MarkerCodeICL1::generate();
    std::sort(cs.begin(),cs.end());
    std::cout << "icl1 marker codes with minimal hamming distance > 1" << std::endl;
    for(size_t i=0;i<cs.size();++i){
      if(i<10) std::cout << " ";
      std::cout << i << ": " << cs[i] << std::endl;
    }
    return 0;
  }else{
    if(!pa("-i")){
      pa_show_usage("please define marker type and ID using '-i type id'");
      return -1;
    }
  }

  
  FiducialDetector d(*pa("-i"));
  ParamList params;
  if(*pa("-i") == "art"){
    params = ParamList("border ratio",*pa("-r"));
  }else if(*pa("-i") == "bch"){
    params = ParamList("border width",*pa("-b"));
  }
  Img8u image = d.createMarker(*pa("-i",1), pa("-s"), params);

  if(pa("-o")){
    save(image,*pa("-o"));
  }else{
    show(image);
  }

}


