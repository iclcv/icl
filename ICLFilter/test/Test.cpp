#include "Img.h"


using namespace icl;


class A{
public:
  static void f(ImgI *x, ImgI* y, float val){
    (void)x;(void)y; (void)val;
    printf("non template \n");
  }
  template<class T>
  static void f(Img<T> *x, Img<T> *y, T val){
    (void)x;(void)y; (void)val;
    printf("template \n");
  }
};

int main(){
  Img8u *i8 = 0;
  Img32f *i32 = 0;
  ImgI *i = 0;
  
  
  A::f(i8,i32,4);
  A::f(i8,i8,4);
  A::f(i32,i32,4);
  A::f(i32,i8,4);
  A::f(i,i8,4);
  A::f(i,i32,4);
  A::f(i8,i,4);
  A::f(i32,i,4);
  A::f(i,i,4);
  
  return 0;
}
