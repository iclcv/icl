#include <WeightedSum.h>
#include <Timer.h>
#include <vector>

using namespace icl;

template <typename T>
void print (Img<T> &src, Img<icl32f> &dst, std::vector<float>::iterator wIt) {
   printf ("weighted sum: %s", translateDepth (src.getDepth()).c_str());
   for (int c=0; c < src.getChannels(); c++, wIt++) {
      printf ("\nchannel %d: w=%4.2f  ", c, *wIt);
      for (ImgIterator<T> it = src.getIterator(c);it.inRegion();++it) 
         printf("%4.1f,", (float) *it);
   }
   printf ("\nresult:            ");
   for (ImgIterator<icl32f> it = dst.getIterator(0);it.inRegion();++it) 
      printf("%4.1f,", *it);
   printf ("\n");
}

void performance (depth eDepth) {
   ImgI *pSrc = imgNew (eDepth, Size(1000,1000), 10);
   Img32f dst (Size(1000,1000),1);
   std::vector<float> weights(10);

   Timer t;
   printf("\nWeightedSum Benchmark: %s", translateDepth (eDepth).c_str());

   int N = 10;
   WeightedSum ws; ws.apply (pSrc, &dst, weights);

   t.start();
   for (int i=0;i<N;i++) ws.apply (pSrc, &dst, weights);
   t.stop();

   delete pSrc;
}

int main() {
   Size size (2,2);
   float afSrc[2][4] = {
      {2, 3.5, 7, 2.3},
      {1, 4, 2.5, 1.7}};
   float* apfSrc[2] = {afSrc[0], afSrc[1]};
   Img32f src32 (size, 2, std::vector<float*>(apfSrc, apfSrc+2));
   Img32f dst (size,1);

   std::vector<float> weights;
   weights.push_back(2);
   weights.push_back(0.5);

   WeightedSum ws;
   ws.apply (&src32, &dst, weights);
   print (src32, dst, weights.begin());
  
   icl8u auSrc[2][4] = {
      {1, 2, 3, 4},
      {4, 3, 2, 1}};
   icl8u* apuSrc[2] = {auSrc[0], auSrc[1]};
   Img8u src8u (size, 2, std::vector<icl8u*>(apuSrc, apuSrc+2));

   ws.apply (&src8u, &dst, weights);
   print (src8u, dst, weights.begin());

   performance (depth8u);
   performance (depth32f);
   return 0;
}
