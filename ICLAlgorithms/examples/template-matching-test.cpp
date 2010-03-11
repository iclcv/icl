/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLAlgorithms module of ICL                   **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLQuick/Quick.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>

int main(int n, char **ppc){
  //  640x480@(5,10)
  paex
  ("-roi|-r","specify template roi like (X,Y)WIDTHxHEIGHT")
  ("-significance-level|-s","specify significance level range [0,1]");
  painit(n,ppc,"-roi(Rect=(200,400)100x120) -s(float=0.9)");
  
  Img8u image = cvt8u(create("parrot"));
  image.scale(Size(640,480));
  
  Rect roi = pa("-roi");
  roi &= image.getImageRect();
  
  image.setROI(roi);
  Img8u templ = cvt8u(copyroi(cvt(image)));
  image.setFullROI();
  

  float s = pa("-s");
  printf("using significance: %f \n",s);
  Img8u *buffer = new Img8u;
  RegionDetector rd;
  vector<Rect> rs = iclMatchTemplate(image,templ,s,buffer,false,&rd);
  std::cout << "Estimating Time for 100 Iterations ..." << std::endl;
  tic();
  for(int i=0;i<100;i++){
    rs = iclMatchTemplate(image,templ,s,buffer,false,&rd);
  }
  toc();
  
  
  ImgQ resultImage = cvt(image);
  
  color(255,255,255);
  for(unsigned int i=0;i<rs.size();++i){
    rect(resultImage,rs[i]);
  }
  
  show(label(resultImage,"results"));

  return 0;
}



