/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQuick/examples/quick-test.cpp                       **
** Module : ICLQuick                                               **
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

#include <ICLQuick/Quick.h>
#include <ICLBlob/RegionBasedBlobSearcher.h>
#include <ICLBlob/FMCreator.h>
#include <ICLBlob/RegionFilter.h>

#include <vector>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>

int main(int nargs, char **ppc){

  ImgQ A = scale(create("parrot"),0.5);
  
  
  RegionBasedBlobSearcher rbbs;
  
  icl8u rc[] = {255,0,0};
  icl8u tr[] = {55,55,55};
  vector<icl8u> refColor;
  vector<icl8u> thresh;
  for(int i=0;i<3;i++){
    refColor.push_back(rc[i]);
    thresh.push_back(tr[i]);
  }
  
  FMCreator *fmc = FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,refColor, thresh);
  Img8u A8u= cvt8u(A);
  Img8u fm = *(fmc->getFM(&A8u));

  ImgQ fm2 = cvt(fm);
  fm2.setFormat(formatGray);

  RegionFilter *rf = new RegionFilter(new Range<icl8u>(200,255),      // val 
                                      new Range<icl32s>( 5,200000),   // size
                                      0,
                                      new Range<icl32f>(10,1000)    );   // formfactor 
                                      
  rbbs.add(fmc,rf);
  rbbs.extractRegions(&A);
  
  const std::vector<Point> &centers = rbbs.getCOGs();

  const std::vector<std::vector<Point> > &boundaries = rbbs.getBoundaries();
  ImgQ B = rgb(fm2);

  for(unsigned int i=0;i<centers.size();i++){
    color(255,0,0);
    cross(B,centers[i]);
    color(0,100,255);
    pix(B,boundaries[i]);
  }

  color(0,255,0,200);
//  fill(255,0,0,200);
  for(int o=0;o<5;o++){
    circle(B,3*o+100,20*o+10,50);
  }
  show((A,B));
  
  /*
      
      ImgQ im = scale(create("parrot"),0.4);  
      
      for(int i=0;i<10;i++){
      for(int j=0;j<10;j++){
      fontsize(3*i+j);
      color(255-20*i,10+20*i,255-20*j,100+5*i+5*j);
      char buf[100];
      sprintf(buf,"[%d]",i+j);
      text(im,20*i,20*j,buf);
      }
      }
      show(im);
      */
      
  //ImgQ i = scale(create("parrot"),0.4);
  //ImgQ j = copy(i);


  // show(filter(i,"laplace"));
  // show(filter(i,"gauss"));

  // show(i+100);
  // show(i*i/255);

  // show(cc(i,formatHLS));

  // show(levels(cc(i,formatGray),4));
  
  /**
      ImgQ a,b;
      for(int i=0;i<10;i++){
      a = ImgQ();
      for(int j=0;j<10;j++){
      a = (a,scale(pwc(),0.1));
      }
      b=(b%a);
      }

      show(b);
  */

  //ImgQ a = create("parrot");
  //a = scale(a,0.2);
  //
  //show(norm(exp(a/100)));
  //print(norm(exp(a/100)));

  
  /*
      for(int i=0;i<100;i++){
      show(pwc());
      usleep(1000*100);
      system("killall xv");
      }
  */
  
  return 0;
}
