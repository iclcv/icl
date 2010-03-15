/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/examples/testimages-example.cpp                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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
*********************************************************************/

#include <ICLIO/TestImages.h>
#include <ICLCore/Img.h>
#include <ICLQuick/Quick.h>

int main(){ 
  
  std::string names[]={"women","tree","house","windows","flowers","parrot"};
  bool horz[] = {1,1,1,0,0,1};
  ImgQ r(Size(1,1),formatRGB);
  for(int i=0;i<6;++i){
    ImgQ im = create(names[i]);
    if(horz[i]){
      r = (r,im);
    }else{
      r = (r%im);
    }
  }
  show(r);

  /** older (deprecated) version of this file
      ImgBase *a = TestImages::create("women");
      ImgBase *b = TestImages::create("tree");
      ImgBase *c = TestImages::create("house");
      
      ImgBase *d = TestImages::create("windows");
      ImgBase *e = TestImages::create("parrot");
      ImgBase *f = TestImages::create("flowers");
      
      
      TestImages::xv(a,"tmp1.ppm",500);
      TestImages::xv(b,"tmp2.ppm",500);
      TestImages::xv(c,"tmp3.ppm",500);
      TestImages::xv(d,"tmp4.ppm",500);
      TestImages::xv(e,"tmp5.ppm",500);
      TestImages::xv(f,"tmp6.ppm",500);
  */
}
