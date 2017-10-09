/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/test/converter-test.cpp                        **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCC/Converter.h>
#include <ICLCC/CCFunctions.h>
#include <ICLIO/TestImages.h>
#include <ICLQuick/Common.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLUtils/StackTimer.h>
#include <numeric>

int main(){

  Img8u a(Size(1000,1000),formatRGB);
  Img32f b(Size(1000,1000),formatGray);

  for(int i=0;i<5;++i){
    cc(&a,&b);
  }

  {
    BENCHMARK_THIS_FUNCTION
    for(int i=0;i<1000;++i){
      cc(&a,&b);
    }
  }
}


ImgBase *createDiffImage(ImgBase *a, ImgBase *b){
  ImgBase *r = 0;
  BinaryArithmeticalOp(BinaryArithmeticalOp::absSubOp).apply(a,b,&r);
  return r;
}

icl64f errorSum(Img64f *img){
  icl64f err=0;
  for(int c=0;c<img->getChannels();++c){
    err += std::accumulate(img->begin(c),img->end(c),icl64f(0));
  }
  return err;
}

string getName(int i){
  switch(i){
    case 0: return "orderScaleConvertCC";
    case 1: return "orderScaleCCConvert";
    case 2: return "orderConvertScaleCC";
    case 3: return "orderConvertCCScale";
    case 4: return "orderCCScaleConvert";
    case 5: return "orderCCConvertScale";
  }
  return "wrong mode";
}

int main2(){
  printf("This is the Converter test application: \n"
         "It reads the maccaw2.ppm image in ICLCC/examples, \n"
         "with params depth64f, formatRGB and Size(750,1002).\n"
         "This image is converted to depth8u,formatYUV and \n"
         "Size(222,817) and then converted back. The resulting \n"
         "Image should differ averagely not more than 15 \n"
         "from the original image. \n"
         "The Test application applies this conversion for all \n"
         "6 by the Converter supported operation-orders. \n "
         "The result is \"successful\" if all 6 estimated \n"
         "average pixel errors are < 15. \n\n" );
  /// test for PCA compress
  Img64f image = cvt64f(create("parrot"));

  /// Testing 64f,rgb,750,1002 -> 8u,lab,222x817 ->  and back

  ImgBase *src = image.deepCopy();
  ImgBase *mid = new Img8u(Size(222,817), formatYUV);
  ImgBase *dst = new Img64f(src->getParams());


  Converter::oporder oos[6] = { Converter::orderScaleConvertCC,
                                Converter::orderScaleCCConvert,
                                Converter::orderConvertScaleCC,
                                Converter::orderConvertCCScale,
                                Converter::orderCCScaleConvert,
                                Converter::orderCCConvertScale };
  bool foundErr = false;
  for(int i=0;i<6;i++){
    Converter(oos[i]).apply(src,mid);
    Converter(oos[i]).apply(mid,dst);

    ImgBase *diff = createDiffImage(src,dst);

    icl64f err = errorSum(diff->asImg<icl64f>());

    if(err/(diff->getDim()*diff->getChannels()) > 15.0){
      foundErr = true;
      printf("per pixel error for mode %s is too high: %f \n",getName(i).c_str(),err/(diff->getDim()*diff->getChannels()));
    }
    show( scale((label(cvt(dst),"result"),label(cvt(diff),"diff-map")),0.2) );
    delete diff;

  }
  delete src;
  delete mid;
  delete dst;

  if(foundErr){
    printf("Converter test failed due to above errors! \n");
    return 1;
  }else{
    printf("Converter test successfull \n");
    return 0;
  }

  return 0;
}
