/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
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

#include <ICLCore/Img.h>

using namespace icl;

void rgb2gray(const icl8u src[3], icl8u dst[1]) {
  *dst = (src[0]+src[1]+src[2])/3;
}

int main(){
  Img8u src(Size::VGA,formatRGB);  
  Img8u dst(Size::VGA,formatGray);

  src.reduce_channels<icl8u,3,1,void(*)(const icl8u src[3], icl8u dst[1])>(dst,rgb2gray);
  

  
  
}
