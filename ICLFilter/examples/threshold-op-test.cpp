/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/threshold-op-test.cpp               **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLFilter/ThresholdOp.h>
#include <ICLUtils/Timer.h>
#include <ICLQuick/Common.h>

static const char *apc[38]={ // 40 x 38
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
    Img8u::const_iterator s=src.begin(0);
    Img8u::const_iterator sEnd=src.end(0);

    Img8u::iterator d=dst.begin(0);
    for(;s!=sEnd;++s,++d){
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
  for(Img8u::iterator it = im.begin(0);it != im.end(0) ;++it){
    Point p = im.getLocation(it,0);
    int x = p.x;
    int y = p.y;
    DEBUG_LOG("X:" << x << " Y:" << y);
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
  show(cvt(t));

  printf("Original colors are [0,100,200,255]\n");
  printf("Image order is: \n");
  printf("orig. - lt(150) - gt(150) - ltgt(100,200) - ltVal(150,0) - gtVal(150,255) - ltgtVal(150,0,150,255)\n");

  performance ();
  return 0;
}
