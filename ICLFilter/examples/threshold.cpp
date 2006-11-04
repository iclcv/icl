#include "Threshold.h"
#include "Img.h"
#include "TestImages.h"
#include "Timer.h"

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

void performance () {
  Img8u src(Size(1000,1000),1),dst(Size(1000,1000),1);
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
    Threshold::lt(&src,&dst,tetta);
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
  int i=0;

  t.setROI(Rect((i++)*s.width,0,100,100));
  im.deepCopyROI(&t);

  t.setROI(Rect((i++)*s.width,0,100,100));
  Threshold::lt(&im, &t, 150); 

  t.setROI(Rect((i++)*s.width,0,100,100));
  Threshold::gt(&im, &t, 150); 

  t.setROI(Rect((i++)*s.width,0,100,100));
  Threshold::ltgt(&im, &t, 100, 200); 

  t.setROI(Rect((i++)*s.width, 0,100,100));
  Threshold::ltVal(&im, &t, 150, 0); 

  t.setROI(Rect((i++)*s.width,0,100,100));
  Threshold::gtVal(&im, &t, 150, 255); 

  t.setROI(Rect((i++)*s.width,0,100,100));
  Threshold::ltgtVal(&im, &t, 150, 0, 150, 255); 

  printf("showing results\n");
  TestImages::xv(&t,"threshold_results.pgm");

  printf("Original colors are [0,100,200,255]\n");
  printf("Image order is: \n");
  printf("orig. - lt(150) - gt(150) - ltgt(100,200) - ltVal(150,0) - gtVal(150,255) - ltgtVal(150,0,150,255)\n");

  performance ();
  return 0;
}
