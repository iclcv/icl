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

#include <ICLFilter/MedianOp.h>
#include <ICLQuick/Common.h>

int main(){
  Img8u image = cvt8u(scale(create("parrot"),0.4));
  
  Size s[3] = { Size(3,3),Size(4,4),Size(10,10) };

  for(int i=0;i<3;++i){
    ImgBase *dst = 0;
    MedianOp(s[i]).apply(&image,&dst);
    
    show(label(cvt(dst),std::string("mask-size:")+str(s[i])));
    delete dst;
  }
}
