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

#include <ICLFilter/WeightChannelsOp.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Mathematics.h>
#include <vector>

void apply_weighted_channel(WeightChannelsOp &wc, 
                            ImgBase &src, 
                            ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  wc.apply(&src,dst);
}

int main() {
  ImgQ src = create("parrot");
  ImgBase *dst = 0;

  vector<icl64f> w(3);
  for(int i=0;i<3;i++){
    w[i] = random(double(0.1), double(1));
  }
  
  WeightChannelsOp wc(w);
  for(int i=0;i<100;i++){
    apply_weighted_channel(wc,src,&dst);
  }
  
  show(scale(cvt(dst),0.4));
}
