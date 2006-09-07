#include "Img.h"


using namespace icl;


class Aok {
public:
  static void f(const ImgI *x, ImgI* y, float val){
     printf("ImgI\n");
  }
  static void f(const Img<icl8u> *x, Img<icl8u> *y, icl8u val){
     printf("icl8u\n");
  }
  static void f(const Img<icl32f> *x, Img<icl32f> *y, icl32f val){
     printf("icl32f\n");
  }
};

class Awrong {
public:
  template <typename T1, typename T2>
  static void f(const T1 *x, T2 *y, float val){
     printf("ImgI\n");
  }
  template <typename T>
  static void f(const Img<T> *x, Img<T> *y, T val){
     printf("template\n");
  }
};

class B {
public:
   static void f(const void*, const void*, float val) {
      printf ("void*\n");
   }
   static void f(const int*, const int*, float val) {
      printf ("int*\n");
   }
};

class a {
   int val;
};
class b : public a {
};

class C {
public:
   static void f(const struct a*, const struct a*, float val) {
      printf ("struct a*\n");
   }
   static void f(const struct b*, const struct b*, float val) {
      printf ("struct b*\n");
   }
};

int main(){
  Img8u *i8 = 0;
  Img32f *i32 = 0;
  
  Aok::f(i8,i32,4);
  Aok::f(i8,i8,4);
  Aok::f(i32,i32,4);

  Awrong::f(i8,i32,4);
  Awrong::f(i8,i8,4);
  Awrong::f(i32,i32,4);

  int*  pi=0;
  void* pv=pi;
  B::f(pi, pi, 1.0);
  B::f(pv, pv, 1.0);

  class b *pb=0;
  class a *pa=pb;
  C::f(pb, pb, 1.0);
  C::f(pa, pa, 1.0);

  return 0;
}
