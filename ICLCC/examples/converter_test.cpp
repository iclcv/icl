#include "ICLConverter.h"
#include "ICLCC.h"
#include "ICLImg.h"
#include "ICLTestImages.h"

using namespace icl;
using namespace std;

template<class T>
Img64f *createDiffImageTemplate(Img<T> *src, Img<T> *dst){
  Img64f *res = new Img64f(src->getParams());

  for(int c=0;c<src->getChannels();++c){
    Img64f::iterator r = res->getIterator(c);
    for(ImgIterator<T> s=src->getIterator(c),d=dst->getIterator(c);s.inRegion();++s,++d,++r){
      icl64f buf = icl64f((*s)-(*d));
      *r = buf > 0 ? buf : -buf;
    }
  }
  return res;
}

ImgBase *createDiffImage(ImgBase *src, ImgBase *dst){
  ICLASSERT_RETURN_VAL(src && dst && src->getDepth() == dst->getDepth() && src->getParams() == dst->getParams() ,0);
  switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return createDiffImageTemplate(src->asImg<icl##D>(),dst->asImg<icl##D>());
    ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
  }
  return 0;
}

icl64f errorSum(Img64f *img){
  icl64f err=0;
  for(int c=0;c<img->getChannels();++c){
    for(Img64f::iterator r = img->getIterator(c);r.inRegion();++r){
      err += *r;
    }
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

int main(){
  printf("This is the Converter test application: \n"
         "It reads the maccaw2.ppm image in ICLCC/examples, \n"
         "with params depth64f, formatRGB and Size(750,1002).\n"
         "This image is converted to depth8u,formatLAB and \n"
         "Size(222,817) and then converted back. The resulting \n"
         "Image should differ averagely not more than 15 \n"
         "from the original image. \n"
         "The Test application applies this conversion for all \n"
         "6 by the Converter supported operation-orders. \n "
         "The result is \"successful\" if all 6 estimated \n"
         "average pixel errors are < 15. \n\n" );
  /// test for PCA compress
  Img64f *image = TestImages::create("parrot",formatRGB,depth64f)->asImg<icl64f>();
  
  /// Testing 64f,rgb,750,1002 -> 8u,lab,222x817 ->  and back
  
  ImgBase *src = image->deepCopy();
  ImgBase *mid = new Img8u(Size(222,817), formatLAB);
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
