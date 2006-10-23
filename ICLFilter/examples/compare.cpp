#include "Compare.h"
#include "Img.h"
#include "FileWrite.h"
#include "TestImages.h"

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


int main(){
  Img8u im(Size(40,38),1);
  Img8u im2(Size(40,38),1);
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
  for(ImgIterator<icl8u> it2 = im2.getIterator(0);it2.inRegion();++it2){
    int x = it2.x();
    int y = it2.y();
    switch(apc[y][x]){
      case '.': *it2 = 254; break;
      case 'a': *it2 = 150; break;
      case 'b': *it2 = 50; break;
      default:  *it2 = 200; break;
    }  
  }

  printf("-------------->scaling images \n");
  im.scale(Size(100,100));
  printf("-------------->first images scaled \n");
  im2.scale(Size(100,100));
  printf("-------------->second images scaled \n");
  
  Size s = im.getSize();
  Img8u t(Size(s.width*8,s.height),1);
  int i=0;
  
  printf("started operations \n");
  t.setROI(Rect((i++)*s.width,0,100,100));
  im.deepCopyROI(&t);
  t.setROI(Rect((i++)*s.width,0,100,100));
  im2.deepCopyROI(&t);
  t.setROI(Rect((i++)*s.width,0,100,100));
  Compare::compare(&im,&im2,&t,Compare::compareEq);
  t.setROI(Rect((i++)*s.width,0,100,100));
  Compare::compareC(&im,100,&t,Compare::compareEq);
  t.setROI(Rect((i++)*s.width,0,100,100));
  Compare::compareC(&im2,50,&t,Compare::compareLessEq);
  t.setROI(Rect((i++)*s.width, 0,100,100));
  Compare::equalEps(&im,&im2,&t,2);
  t.setROI(Rect((i++)*s.width,0,100,100));
  Compare::equalEpsC(&im,228,&t,30);
  t.setROI(Rect((i++)*s.width,0,100,100));
  Compare::equalEpsC(&im2,175,&t,25);

  printf("showing results\n");
  TestImages::xv(&t,"result_image.pgm",500);

  printf("Original colors are im1:[0,100,200,255] and im2:[50,150,200,254] \n");
  printf(": Image order is: \n");
  printf("ori. im1 - ori. im2  - comp(im1,im2,eq)"
         " - compC(im1,100,eq) - compC(im2,50,lesseq)"
         "- compEqualEps(im1,im2,2) -"
         "compEqualEpsC(im1,228,30) -"
         " compEqualEpsC(im2,175,,25)\n");
  
  return 0;
}
