#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <Timer.h>
#include <Img.h>
#include <ImgIterator.h>

using namespace icl;
using namespace std;

typedef icl8u T;
typedef std::vector<ImgIterator<T>*> itvec;

static const int N =3;
static int tH = 5;
static int tS = 5;
static int tI = 5;

static int aiT[3] = {5,5,5};
static int d[N];
static int *pd=0;
static int *pt=0;


int countPix = 0;
// hart: pro pixel : 9 Op 3*"=" 3*"*" und 3*"-"
inline float matchProp(T *h, T *s, T *i){
   int dh = *h-tH;
   int ds = *s-tS;
   int di = *i-tI;
   return (float)(dh+ds+di)/765.0;
}

inline float matchProp(itvec &pixel){
   //loop unfolded 18 Op 9*"*" 3*"+" 3*"-" 3*"=" 
   int dh = *(*(pixel[0])) - tH;
   int ds = *(*(pixel[1])) - tS;
   int di = *(*(pixel[2])) - tI;
   return (float)(dh+ds+di)/765.0;
}

inline float matchPropN(itvec &pixel){
   //loop
   pt = const_cast<int*>(aiT);
   float f=0;
   // N*[4*"*" 1*"=" 1*"-", 1*"if" 3*"++"] == 30 Op
  
   for(itvec::iterator it = pixel.begin(); it!=pixel.end(); ++it,++pt){
      f += (*(*(*it))) - (*pt);
   }
   
   return f/765.0;
}


int main(){
   printf("Start tests:\n");
   int NTIMES = 10;
   int count = 0;
   Img8u im(Size(640,480),3);
   Img32f dst(Size(640,480),1);

   ImgIterator<T> it0 = im.getIterator(0);
   ImgIterator<T> it1 = im.getIterator(1);
   ImgIterator<T> it2 = im.getIterator(2);
   
   Timer oTimer;
   oTimer.startTimer();
   printf("Test1\n");

   for(int i=0;i<NTIMES;i++){
      T *p0 = im.getData(0);
      T *p1 = im.getData(1);
      T *p2 = im.getData(2);

      float *d = dst.getData(0);
      float *dend = d+im.getDim();

      for(;d<dend; ++p0, ++p1, ++p2, ++d){
         *d = matchProp(p0,p1,p2);
      }
   }

   oTimer.stopSubTimer();
   printf("Test2\n");
   for(int i=0;i<NTIMES;i++){
      it0 = im.getIterator(0);
      it1 = im.getIterator(1);
      it2 = im.getIterator(2);
      
      itvec its;
      its.push_back(&it0);
      its.push_back(&it1);
      its.push_back(&it2);
      
      float *d = dst.getData(0);
      float *dend = d+im.getDim();

      for(;it0.inRegion();++it0, ++it1, ++it2,++d){
         *d = matchProp(its);
      }
   }
   
   oTimer.stopSubTimer();

   printf("Test3\n");
   for(int i=0;i<NTIMES;i++){
      it0 = im.getIterator(0);
      it1 = im.getIterator(1);
      it2 = im.getIterator(2);

      itvec its;
      its.push_back(&it0);
      its.push_back(&it1);
      its.push_back(&it2);
      
      float *d = dst.getData(0);
      float *dend = d+im.getDim();

      for(;it0.inRegion();++it0, ++it1, ++it2,++d){
         *d = matchPropN(its);
      }
   }

   oTimer.stopTimer();
}
