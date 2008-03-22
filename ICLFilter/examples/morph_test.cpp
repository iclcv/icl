#include <iclQuick.h>
#include <iclMorphologicalOp.h>
#include <ippi.h>
int main(){
  Img8u image = cvt8u(thresh(gray(scale(create("parrot"),280,400)),128));
  Img8u imageG = cvt8u(gray(scale(create("parrot"),280,400)));
  Img8u imageC = cvt8u(scale(create("parrot"),280,400));
  
  
  
  MorphologicalOp::optype ts[11]={ 
    MorphologicalOp::dilate,
    MorphologicalOp::erode,
    MorphologicalOp::dilate3x3,
    MorphologicalOp::erode3x3,
    MorphologicalOp::dilateBorderReplicate,
    MorphologicalOp::erodeBorderReplicate,
    MorphologicalOp::openBorder,
    MorphologicalOp::closeBorder,
    MorphologicalOp::tophatBorder,
    MorphologicalOp::blackhatBorder,
    MorphologicalOp::gradientBorder
  };
  std::string ns[11]={ 
    "dilate",
    "erode",
    "dilate3x3",
    "erode3x3",
    "dilateBorderReplicate",
    "erodeBorderReplicate",
    "openBorder",
    "closeBorder",
    "tophatBorder",
    "blackhatBorder",
    "gradientBorder"
  };

  
  static Size dilationKernelSize(3,3);
  static std::vector<char> kernel(dilationKernelSize.getDim(),1);

  ImgQ X = zeros(1,1,1);
  
  ImgQ result1 = label(cvt(image),"orig");
  ImgQ result2 = X;

  ImgQ result1G = label(cvt(imageG),"orig");
  ImgQ result2G = X;

  ImgQ result1C = label(cvt(imageC),"orig");
  ImgQ result2C = X;


  for(int i=0;i<11;++i){
    printf("applying %s \n",ns[i].c_str());
    MorphologicalOp mo(dilationKernelSize,&kernel[0],ts[i]);
    ImgBase *dst = 0;
    ImgBase *dstG = 0;
    ImgBase *dstC = 0;
    
    mo.apply(&image,&dst);
    mo.apply(&imageG,&dstG);
    mo.apply(&imageC,&dstC);
    if(i<5){
      result1 = (result1,X,label(cvt(dst),ns[i]+translateSize(dst->getROISize())));
      result1G = (result1G,X,label(cvt(dstG),ns[i]+translateSize(dst->getROISize())));
      result1C = (result1C,X,label(cvt(dstC),ns[i]+translateSize(dst->getROISize())));
    }else{
      result2 = (result2,X,label(cvt(dst),ns[i]+translateSize(dst->getROISize())));
      result2G = (result2G,X,label(cvt(dstG),ns[i]+translateSize(dst->getROISize())));
      result2C = (result2C,X,label(cvt(dstC),ns[i]+translateSize(dst->getROISize())));
    }
    
    ICL_DELETE( dst );
    ICL_DELETE( dstG );
    ICL_DELETE( dstC );
    

  }
  show( (result1%X%result2) );
  show( (result1G%X%result2G) );
  show( (result1C%X%result2C) );
  
  

}
