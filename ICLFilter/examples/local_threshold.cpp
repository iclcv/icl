#include <Img.h>
#include <Compare.h>
#include <FileReader.h>
#include <FileWriter.h>
#include <Converter.h>
#include <Convolution.h>
#include <IntegralImg.h>

#include <stdio.h>
#include <Timer.h>

using namespace icl;
using namespace std;


typedef ImgIterator<icl8u> it8u;
Size s(640,480);
int fac = 4;
int diffThresh = 35;
int minSize = 20;
Timer timer;



/*
.C....A...
..+++++...
..+++++...
..++X++...
..+++++...    for(int y=
.B++++D...
..........
|+| = D - A - B + C
*/




void localThreshold4(Img8u &src, Img8u &dst){
  Size s = src.getSize();
  const int w = s.width;
  const int h = s.height;

  const int r = 10;

  const int add_factor = 0; // its not a factor
  
  // integral image

  int *I = (IntegralImg::create<icl8u,int>(&src))[0];

  icl8u *S = src.getData(0);
  icl8u *D = dst.getData(0);
  // local pointers
  
  /* r=2
  .C....A...
  ..+++++...
  ..+++++...
  ..++X++...
  ..+++++...    for(int y=
  .B++++D...
  ..........
  |+| = D - A - B + C
  */
  


  ///! start time mesure here (image 640,480,gray)
  int idx, thresh, yu, yl, xr, xl;
  int roisize = (2*r+1)*(2*r+1);
  for(int y=r;y<h-r;y++){
    yu = (y-r-1)*w;
    yl = (y+r)*w;
    idx = w*y+r;
    xr = 2*r;
    xl = -1;
    for(int x=r;x<w-r; ++x, ++idx,++xr,++xl){      
      thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roisize;
      D[idx] = S[idx] < (thresh+add_factor) ? 0 : 255;
    }
  }
  ///! end of time mesure ca 3.4ms (1.4MHz Centrino)
  
  delete I;
}



void localThreshold5(Img8u &src, Img8u &dst){
  Size s = src.getSize();
  const int w = s.width;
  const int h = s.height;
  const int r = 10;
  const int add_factor = -20; // its not a factor
  
  printf("1 \n");
  // integral image
  int *I = (IntegralImg::create<icl8u,int>(&src,r+1))[0];

  printf("2 \n");

  icl8u *S = src.getData(0);
  icl8u *D = dst.getData(0);
  
  int idx, thresh, yu, yl, xr, xl;
  int roisize = (2*r+1)*(2*r+1);            // todo roisize depends on the current position
                                            // speed up by creating a roisize image as LUT
  
  int iw =w+2*(r+1);
  
  /* r=2
  .C....A...
  ..+++++...
  ..+++++...
  ..++X++...
  ..+++++...    for(int y=
  .B++++D...
  ..........
  |+| = D - A - B + C
  */
  
  I+=((r+1)+(r+1)*iw);
  for(int y=0;y<h;y++){
    yu = (y-r-1)*iw;
    yl = (y+r)*iw;
    xr = r;
    xl = -r-1;
    idx = w*y;
    for(int x=0;x<w; ++x, ++idx,++xr,++xl){     //optimize: use xr or index as loop index idx<idxEnd
      thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roisize;
      D[idx] = S[idx] < (thresh+add_factor) ? 0 : 255;
    }
  }
  
  I-=((r+1)+(r+1)*iw);
  delete I;

}

int main (int argc, char **argv) {
  if(argc != 2){
    printf("usage %s filename\n",*argv);
    exit(0);
  }
  
  Img8u  I(s,formatGray), R(s,formatGray);
  
  FileReader read(argv[1]);
  read.grab(&I);
  
  FileWriter("this.jpg").write(&I);
  
  //  localThreshold4(I,R);
  localThreshold5(I,R);
  
  FileWriter("out.jpg").write(&R);
  
  return 1;
}
