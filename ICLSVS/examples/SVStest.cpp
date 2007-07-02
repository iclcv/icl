#include <iclSVS.h>
#include <iclImg.h>
#include <iclFileReader.h>
#include <iclFileWriter.h>
#include <iclTimer.h>

using namespace std;
using namespace icl;

int main(){

  const ImgBase *imgL, *imgR;
  ImgBase *dst =new Img32f(Size(320,240),formatGray);
  ImgBase *outImg=0;
  FileReader img_l("demoImages/face320-cal-L.pgm");
  FileReader img_r("demoImages/face320-cal-R.pgm");
  FileWriter w("face320-disp.pgm");

  imgL = img_l.grab();
  imgR = img_r.grab();
  SVS t;
  Timer timer;
  printf("\nBenchmarking SVS\n");
  timer.start();
  int N=5;
  printf("Doing %d iterations\n",N);
  for(int i=0;i<N;i++){
    t.Load(imgL->asImg<icl8u>(),imgR->asImg<icl8u>());
    t.load_calibration("/vol/vision/SVS/4.2/data/wallcal.ini");
//      t.load_calibration("/vol/pgacvis/sonyfw.ini");

    timer.stopSubTimer("Iterations of Load done");
    t.do_stereo();
    outImg=t.get_disparity();
  }
  timer.stop("Iterations of get_disparity done");
  Converter().apply(outImg,dst);
//  w.write(outImg);
  w.write(dst);
}
