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

// hart: pro pixel : 6 Op 3*"=" und 3*"-"
inline float matchProp(T h, T s, T i){
   int dh = h-tH;
   int ds = s-tS;
   int di = i-tI;
   return (float)(dh+ds+di)/765.0;
}

inline float matchPropA(const ImgIterator<T> pixel[3]){
   //loop unfolded 18 Op 9*"*" 3*"+" 3*"-" 3*"=" 
   int dh = *(pixel[0]) - tH;
   int ds = *(pixel[1]) - tS;
   int di = *(pixel[2]) - tI;
   return (float)(dh+ds+di)/765.0;
}

inline float matchPropV(const itvec &pixel){
   //loop unfolded 18 Op 9*"*" 3*"+" 3*"-" 3*"=" 
   int dh = *(*(pixel[0])) - tH;
   int ds = *(*(pixel[1])) - tS;
   int di = *(*(pixel[2])) - tI;
   return (float)(dh+ds+di)/765.0;
}

inline float matchPropVL(itvec::const_iterator it, const itvec::const_iterator& end){

   //loop
   int *pt=aiT;
   float sum=0;
   // N*[4*"*" 1*"=" 1*"-", 1*"if" 3*"++"] == 30 Op
#if 1
   for(;it!=end; ++it, ++pt) {
      sum += (*(*(*it))) - (*pt);
   }
#else
   sum = (*(*(*it))) - (*pt); ++it; ++pt;
   sum += (*(*(*it))) - (*pt); ++it; ++pt;
   sum += (*(*(*it))) - (*pt);
#endif
   return (float)(sum)/765.0;
}


int main(){
   printf("Start tests:\n");
   int NTIMES = 10;
   Img8u im(Size(640,480),3);
   Img32f dst(Size(640,480),1);

   ImgIterator<T> it0;
   ImgIterator<T> it1;
   ImgIterator<T> it2;
   itvec its(3);

   float *d, *dend;
   Timer oTimer;

   printf("transfer of pixel values  ");
   oTimer.startTimer();
   for(int i=0;i<NTIMES;i++){
      it0 = im.getIterator(0);
      it1 = im.getIterator(1);
      it2 = im.getIterator(2);

      d = dst.getData(0);
      dend = d+im.getDim();

      for(;d<dend; ++it0, ++it1, ++it2, ++d){
         *d = matchProp(*it0,*it1,*it2);
      }
   }
   oTimer.stopSubTimer("transfer of pixel values");

   printf("fixed sized array         ");
   for(int i=0;i<NTIMES;i++){
      ImgIterator<T> ait[3] = {it0=im.getIterator(0),
                               it1=im.getIterator(1),
                               it2=im.getIterator(2)};
      
      d = dst.getData(0);
      dend = d+im.getDim();

      for(;d<dend; ++it0, ++it1, ++it2, ++d){
         *d = matchPropA(ait);
      }
   }
   oTimer.stopSubTimer("fixed sized array");

   printf("fixed sized vector        ");
   for(int i=0;i<NTIMES;i++){
      it0 = im.getIterator(0);
      it1 = im.getIterator(1);
      it2 = im.getIterator(2);
      
      its.clear();
      its.push_back(&it0);
      its.push_back(&it1);
      its.push_back(&it2);
      
      d = dst.getData(0);
      dend = d+im.getDim();

      for(;d<dend; ++it0, ++it1, ++it2, ++d){
         *d = matchPropV(its);
      }
   }
   oTimer.stopSubTimer("fixed sized vector");

   printf("variable sized vector     ");
   for(int i=0;i<NTIMES;i++){
      it0 = im.getIterator(0);
      it1 = im.getIterator(1);
      it2 = im.getIterator(2);

      its.clear();
      its.push_back(&it0);
      its.push_back(&it1);
      its.push_back(&it2);
      itvec::const_iterator start=its.begin();
      itvec::const_iterator end=its.end();

      d = dst.getData(0);
      dend = d+im.getDim();

      for(;d<dend; ++it0, ++it1, ++it2, ++d){
         *d = matchPropVL(start, end);
      }
   }
   oTimer.stopTimer("variable sized vector");
}
