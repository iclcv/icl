#include "Proximity.h"
#include "FileReader.h"
#include "TestImages.h"
#include "Threshold.h" //test
#include "Arithmetic.h"
#include "Median.h"
#include <Timer.h>

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
#ifdef WITH_IPP_OPTIMIZATION
  const ImgBase *src;
  ImgBase *dst=0;
  string srcName("src.ppm");
  string dstName("wiener.ppm");
  Timer t;
  if (nArgs > 2) dstName = ppcArg[2];
  if (nArgs > 1) {
    // read image from file
    FileReader reader(ppcArg[1]);
    src = reader.grab();
  } else src = TestImages::create("women",Size(640,480),formatRGB,depth32f);
  Median* pMedian = new Median(Size(3,3));
  t.start();
  for (int i=1;i<10;i++){
    pMedian->applyColor (src, &dst);
  }
  t.stopSubTimer("MedianColor");
  for (int i=1;i<10;i++){
    pMedian->apply (src, &dst);
  }
  t.stop("Median");
  // write and display the image
    //src->print("src");
    //dst->print("dst");
    //TestImages::xv (src, string("src.ppm"));
    //TestImages::xv (dst, string("dst.ppm"));
#else
  printf("Canny only implemented with IPP\n");
#endif

   return 0;
}
// [640x480] rgb 10 x Filteranwendung, IPP aktiviert
// --  [Median] -> Time: 8435 ms
// --  [MedianColor] -> Time: 902 ms

// --  [Median] -> Time: 8631 ms
// --  [MedianColor] -> Time: 903 ms

// --  [MedianColor] -> Time: 929 ms
// --  [Median] -> Time: 9331 ms

// ohne IPP:

// --  [Median] -> Time: 8382 ms
// wiederholung
// --  [Median] -> Time: 8340 ms
// ohne IPP ist mediancolor nicht implementiert

// Werte basierend auf Centrino 1,6

