#include <iclQuick.h>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <iclRegionBasedBlobSearcher.h>
#include <iclFMCreator.h>
#include <iclRegionFilter.h>
#include <vector>

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
