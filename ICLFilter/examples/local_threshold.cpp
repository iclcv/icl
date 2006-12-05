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

/*************************************
............
............
..xxxxxxxx..
..xxxxxxxx..
..xxxxxxxx..
**************************************/
int *createROISizeImage(const Size &s, int r){
  int *p = new int[s.getDim()];
  memset(p,0,s.getDim()*sizeof(int));
  
  int w = s.width;
  int h = s.height;
  int r1 = r+1;
  int rr1 = r+r+1;
  int dim = rr1*rr1;
  
  // corners:
  // top left / right
  for(int y=0;y<r;y++){ 
    for(int x=0;x<r;x++){
      p[x+w*y] = (r1+x)*(r1+y); //>>>>>>>> left
    }
    for(int x=w-r;x<w;x++){
      p[x+w*y] = (r1+(w-x-1))*(r1+y); //>> right
    }
    // center
    for(int x=r,xEnd=w-r;x<xEnd;++x){
      p[x+w*y] = rr1*(r1+y);
    }    
  }
 
  // bottom left / right
  for(int y=h-r;y<h;++y){ 
    for(int x=0;x<r;++x){
      p[x+w*y] = (r1+x)*(r1+(h-y-1)); //>>>>>>>> left
    }
    for(int x=w-r;x<w;x++){
      p[x+w*y] = (r1+(w-x-1))*(r1+(h-y-1)); //>> right
    }
    // center
    for(int x=r,xEnd=w-r;x<xEnd;++x){
      p[x+w*y] = rr1*(r1+(h-y-1));
    }  
  }

  // left and right
  for(int y=r,yEnd=h-r;y<yEnd;++y){ 
    for(int x=0;x<r;x++){
      p[x+w*y] = (r1+x)*rr1; //>>>>>>>> left
    }
    for(int x=w-r;x<w;++x){
      p[x+w*y] = (r1+(w-x-1))*rr1; //>> right
    }
  }
  for(int y=r,yEnd=h-r; y<yEnd;++y){
    for(int x=r,xEnd=w-r; x<xEnd; ++x){
      p[x+w*y]=dim;
    }
  }
  // looks correctly !!
  return p;
}

void write(int *p,const Size &s, std::string name){
  Img32f image(s,1);
  copy<int,icl32f>(p,p+s.getDim(),image.getData(0));
  image.print(name);  
  image.scaleRange();
  FileWriter(name).write(&image);
  
}

void localThreshold5(Img8u &src, Img8u &dst){
  Size s = src.getSize();
  const int w = s.width;
  const int h = s.height;
  const int r = 10;
  const int r1 = r+1;
  const int r_1 = -r-1;

  const int add_factor = -1; // its not a factor
  
  timer.start();
  int *I;
  for(int i=0;i<100;i++){
    I = (IntegralImg::create<icl8u,int>(&src,r1))[0];
  }
  timer.stop();

  icl8u *S = src.getData(0);
  icl8u *D = dst.getData(0);
  
  int thresh, yu, yl, xr, xl;
  int roisize = (2*r+1)*(2*r+1);            // todo roisize depends on the current position
                                            // speed up by creating a roisize image as LUT
  int *roiSizeImage = createROISizeImage(s,r);

  int iw =w+2*(r1);
  int ih =h+2*(r1);

  write(I,Size(iw,ih),"integral_image.pgm");
 
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
  
  I+=(r1+r1*iw);
  for(int y=0;y<h;y++){
    yu = (y-r1)*iw;  // (y-(r+1))*iw
    yl = (y+r)*iw;   // (y+r)*iw 
    xr = r;          // r
    xl = r_1;        // -r-1
    for(int idx=w*y,idxEnd=w*(y+1); idx<idxEnd ; ++idx,++xr,++xl){
      thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roiSizeImage[idx]; //roisize;
      D[idx] = S[idx] < (thresh+add_factor) ? 0 : 255;
    }
  }
  
  I-=(r1+r1*iw);
  delete I;

}

int main (int argc, char **argv) {
  if(argc != 2){
    printf("usage %s filename\n",*argv);
    exit(0);
  }
  
  Img8u  I(s,formatGray), R(s,formatGray);
  
  FileReader(argv[1]).grab(&I);
 
  localThreshold5(I,R);
  
  FileWriter("out.jpg").write(&R);}
