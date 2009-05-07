#include <iclCommon.h>

int main(int nArgs, char **ppcArg){

  ImgQ a = scale(create("flowers"),320,240);
  ImgQ b = scale(create("windows"),320,240);

  /// this will implicitly use the arithmetical op  
  ImgQ apb = norm(a+b);
  ImgQ amb = norm(a-b);
  ImgQ am100 = norm(a-100);

  ImgQ x = (label(a,"A"), label(b,"B")) %
  (label(apb,"norm(A+B)"), label(amb,"norm(A-B)"), label(am100,"norm(a-100)"));
  
  show(x);
  
}
