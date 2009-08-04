#include <iclCommon.h>
#include <iclBinaryArithmeticalOp.h>
#include <iclBinaryCompareOp.h>

void usage_exit(){
  std::cout << "usage:\n\ticl-compare-images <1st image> <2nd image> [out-file-name]" << std::endl;
  exit(-1);
}

int main(int n, char **ppc){
  pa_init(n,ppc,"-a(2) -b(2) -o(1) -ca(1) -cb(1)");
  
  GenericGrabber ga(FROM_PROGARG("-a"));
  GenericGrabber gb(FROM_PROGARG("-b"));

  ga.setIgnoreDesiredParams(true);
  gb.setIgnoreDesiredParams(true);

  const ImgBase *a = ga.grab();
  const ImgBase *b = gb.grab();
  
  if(pa_defined("-ca")){
    a = a->selectChannel(pa_subarg<int>("-ca",0,0));
    std::cout << "using channel " << pa_subarg<int>("-ca",0,0) << " from image A" << std::endl;
  }
  if(pa_defined("-cb")){
    b = b->selectChannel(pa_subarg<int>("-cb",0,0));
    std::cout << "using channel " << pa_subarg<int>("-cb",0,0) << " from image B" << std::endl;
  }
  

  bool canBeCompared = true;
  
#define CHECK(X,MANDATORY)                                              \
  if(a->get##X() != b->get##X()){                                       \
    std::cout << "Images differ in " << #X << ":   A:" << a->get##X() << "    B:" << b->get##X() << std::endl; \
    if(MANDATORY) canBeCompared = false;                                \
  }else{                                                                \
    std::cout << #X << " is equal:             " << a->get##X() << std::endl; \
  }

  CHECK(Size,true);
  CHECK(Depth,true);
  CHECK(Channels,true);
  CHECK(Format,false);
  CHECK(ROI,false);
#undef CHECK

  if(!canBeCompared){
    std::cout << "images cannot be compared pixel-wise due to incompatibilities" << std::endl;
    return 0;
  }

  ImgBase *subImage = 0, *cmpImage=0;
  BinaryArithmeticalOp(BinaryArithmeticalOp::subOp).apply(a,b,&subImage);
  BinaryCompareOp(BinaryCompareOp::eq).apply(a,b,&cmpImage);


  QApplication app(n,ppc);
  GUI gui("hbox");
  gui << "image[@handle=sub@label='A-B']" << "image[@handle=eq@label='A==B']";
  gui.show();
  
  gui["sub"] = subImage;
  gui["eq"] = cmpImage;

  return app.exec();

}
