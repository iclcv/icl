/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLCC/examples/converter-test.cpp                      **
** Module : ICLCC                                                  **
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
