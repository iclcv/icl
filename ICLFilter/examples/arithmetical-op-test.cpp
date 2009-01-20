#include <iclCommon.h>

int main(int nArgs, char **ppcArg){
  
  /// this will implicitly use the arithmetical op
  Img32f a = scale(create("parrot"),640,480);
  Img32f b = scale(create("windows"),640,480);
  
  show(norm(a+b));
  show(norm(a-b));
  
  show(norm(a-100));


}
