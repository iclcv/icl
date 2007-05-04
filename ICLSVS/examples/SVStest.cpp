#include <iclSVS.h>
#include <iclImg.h>
#include <iclFileReader.h>
#include <iclFileWriter.h>
#include <iclTimer.h>

using namespace std;
using namespace icl;

int main(){

  const ImgBase *imgL, *imgR;
  Img16s *outImg;
  FileReader img_l("demoImages/face320-cal-L.pgm");
  FileReader img_r("demoImages/face320-cal-R.pgm");
  FileWriter w("face320-disp.pgm");

  imgL = img_l.grab();
  imgR = img_r.grab();
  SVS t;
  Timer timer;
  printf("\nBenchmarking SVS\n");
  timer.start();
  int N=1;
  printf("Doing %d iterations\n",N);
  for(int i=0;i<N;i++){
    t.Load(imgL->asImg<icl8u>(),imgR->asImg<icl8u>());
//    t.Load_cut(imgL->asImg<icl8u>(),imgR->asImg<icl8u>(),Point(30,30),Size(200,100));
  }
  t.load_calibration("/vol/vision/SVS/4.2/data/wallcal.ini");
  t.printvars();
  timer.stopSubTimer("Iterations of Load done");
  for(int i=0;i<N;i++){
    t.do_stereo();
  }
  timer.stopSubTimer("Iterations of do_stereo done");
  for(int i=0;i<N;i++){
    outImg=t.get_disparity();
  }
  timer.stop("Iterations of get_disparity done");
  w.write(outImg);
}
