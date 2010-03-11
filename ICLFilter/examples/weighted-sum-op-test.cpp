/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLFilter/WeightedSumOp.h>
#include <ICLIO/TestImages.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Mathematics.h>
#include <vector>

using namespace icl;
using namespace std;


void apply_weighted_sum(WeightedSumOp &wo, ImgBase *&src, ImgBase **&dst){
  BENCHMARK_THIS_FUNCTION;
  wo.apply(src,dst);
}

int main() {
  ImgBase *src = new Img64f(Size(1000,1000),3);
  //  ImgBase *src = TestImages::create("parrot",formatRGB,depth8u);
  ImgBase *dst = 0;
  ImgBase **ppoDst = &dst;

  vector<icl64f> w(src->getChannels());
  for(int i=0;i<src->getChannels();i++){
    w[i] = random(double(1));
  }
  
  WeightedSumOp wo(w);
  for(int i=0;i<100;i++){
    apply_weighted_sum(wo,src,ppoDst);
  }
  return 0;
}
