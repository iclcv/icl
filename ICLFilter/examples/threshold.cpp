#include <ICLThresholdOp.h>
#include <ICLImg.h>
#include <ICLTestImages.h>
#include <ICLTimer.h>

using namespace icl;

static char *apc[38]={ // 40 x 38
  ".................................###....",
  ".................................#####..",
  "........##..#####........##......######.",
  "...#######.............######.....######",
  "...#####.............#########.....###..",
  "...####........#.....##########.........",
  "............#####...############........",
  "...........#######.######aaa#####.......",
  "..........########.###aaaaaaa#####......",
  "..####....#########aaa.bbaaaab####......",
  ".######..#######aaaaaa.bbaaaaab###......",
  "..#####..####aaaaaaaaa.bbaaaaaab##......",
  "#.##########a.aaaaaaaa.baaaaaaaab.......",
  "#.#########aa.aaaaaaaaaaaaaaaaaaab..####",
  "#.########a.a.aaaaaaaaaaaaaaaaaaaab.####",
  "...######aa.a.a.aaaaaaaaaaaaaaaaabbb####",
  ".#######a.a.a.a.a.aaaaaaaaaaaabbb#######",
  "#######a..a.a.a.a.aaaaaaaaabbbbbb#######",
  "######aaaaaaaaa.a.a.aaaabbbbbbbbb#######",
  "#####a#a....aaaaaaabbbbbbbbbbbbbb..#####",
  "#######a...........bbbbbbbbbbbbbb.......",
  "#######a...........bbbbbbbbaabbbb.......",
  "#######a......aaa..bbbbbaabaabbbb.......",
  "#######a......a.a..bbaabaabaabbbbaaa....",
  "####aaaa......aaa..bbaabaaaaabbbb.aaaaaa",
  "aaaa...a...........bbaaaaabbbbbbb.....aa",
  "aa.....a.aaa.......bbaabbbbbbbbbb......a",
  ".......a.a...a.....bbbbbbbbbbbbbbaaa....",
  ".......a.a..#a.....bbbbbbbbbbbbbbaaaaa..",
  ".......a.a..#a.....bbbbbbbbbbbbbbaaaaaa.",
  ".......a.a.##a.....bbbbbbbbbbbbbaaaaaaa.",
  ".......a.a..#a.....bbbbbbbbbbaaaaaaaaa..",
  ".......a.a..#a.....bbbbbbbaaaaaaaaaa....",
  ".......aaaaa.a.....bbbbaaaaaaaaaaaa.....",
  "............aaaaaaabaaaaaaaaaaaa........",
  "........................................",
  "........................................",
  "........................................"};
ThresholdOp * pThresh = new ThresholdOp(ThresholdOp::lt,77,77,0,255);
void performance () {
  Img8u src(Size(1000,1000),1),dst(Size(1000,1000),1);
  ImgBase *dst2=0;
  Timer t;
  printf("\nThreshold Benchmark!");
  int N = 100;
  int tetta = 77;

  t.start();
  for(int i=0;i<N;i++){
    for(Img8u::iterator s=src.getIterator(0),d=dst.getIterator(0);
        s.inRegion();++s,++d){
      (*d) = (*s) > tetta ? 255 : 0;
    }
  }
  t.stopSubTimer("C++ fallback");

  for(int i=0;i<N;i++){
    pThresh->apply(&src,&dst2);
  }
  t.stop("IPP");
}

int main(){
  Img8u im(Size(40,38),1);
  for(ImgIterator<icl8u> it = im.getIterator(0);it.inRegion();++it){
    int x = it.x();
    int y = it.y();
    switch(apc[y][x]){
      case '.': *it = 255; break;
      case 'a': *it = 100; break;
      case 'b': *it = 0; break;
      default:  *it = 200; break;
    }  
  }

  im.scale(Size(100,100));
  
  Size s = im.getSize();
  Img8u t(Size(s.width*7,s.height),1);
  ImgBase *t2=0;
  int i=0;

  t.setROI(Rect((i++)*s.width,0,100,100));
  im.deepCopyROI(&t);

  t.setROI(Rect((i++)*s.width,0,100,100));
  pThresh->setType(ThresholdOp::lt);
  pThresh->setLowThreshold(150);
  pThresh->apply(&im, &t2); 
  t2->convertROI(&t);
  
  t.setROI(Rect((i++)*s.width,0,100,100));
  pThresh->setType(ThresholdOp::gt);
  pThresh->setHighThreshold(150);
  pThresh->apply(&im, &t2);
  t2->convertROI(&t);
  t.setROI(Rect((i++)*s.width,0,100,100));
  //ThresholdOp::ltgt(&im, &t, 100, 200); 
  
  pThresh->setType(ThresholdOp::ltgt);
  pThresh->setLowThreshold(100);
  pThresh->setHighThreshold(200);
  pThresh->apply(&im, &t2);
  t2->convertROI(&t);
  
  t.setROI(Rect((i++)*s.width, 0,100,100));
//  ThresholdOp::ltVal(&im, &t, 150, 0);
  pThresh->setType(ThresholdOp::ltVal);
  pThresh->setLowThreshold(150);
  pThresh->setLowVal(0);
  pThresh->apply(&im, &t2);
  t2->convertROI(&t);

  t.setROI(Rect((i++)*s.width,0,100,100));
//  ThresholdOp::gtVal(&im, &t, 150, 255); 
  pThresh->setType(ThresholdOp::gtVal);
  pThresh->setHighThreshold(150);
  pThresh->setHighVal(255);
  pThresh->apply(&im, &t2);
  t2->convertROI(&t);

  t.setROI(Rect((i++)*s.width,0,100,100));
//  ThresholdOp::ltgtVal(&im, &t, 150, 0, 150, 255); 
  pThresh->setType(ThresholdOp::ltgtVal);
  pThresh->setLowThreshold(150);
  pThresh->setLowVal(0);
  pThresh->setHighThreshold(150);
  pThresh->setHighVal(255);
  pThresh->apply(&im, &t2);
  t2->convertROI(&t);

  printf("showing results\n");
  TestImages::xv(&t,"threshold_results.pgm");

  printf("Original colors are [0,100,200,255]\n");
  printf("Image order is: \n");
  printf("orig. - lt(150) - gt(150) - ltgt(100,200) - ltVal(150,0) - gtVal(150,255) - ltgtVal(150,0,150,255)\n");

  performance ();
  return 0;
}
