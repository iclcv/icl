#include <iclQuick.h>
#include <iclChamferOp.h>


int main(int n, char **ppc){
  
  ImgQ image = create("parrot");
  image = gray(image);
  image = scale(image,320,240);
  image = filter(image,"laplace");
  image = thresh(image,250);

  
  
  ImgBase *res=0;
  Img8u im = cvt8u(image);
  for(int i=0;i<10;i++){
    ChamferOp(ChamferOp::metric_real_euclidian).apply(&im,&res);
  }
  
  ImgQ result;
  if(res->getDepth() == depth32s){
    result = cvt( *(res->asImg<icl32s>()));
  }else{
    result = cvt( *(res->asImg<icl32f>()));
  }
  
  print(result);
  result = norm(result);
  
  
  show((image,result));

  return 0;
}
