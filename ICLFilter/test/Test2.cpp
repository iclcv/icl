#include "Img.h"
#include "Timer.h"

using namespace icl;

struct Base{ 
  virtual ~Base() {}
  virtual inline int f(int c) const{    return c > 5 ? c : 5; } 
};

struct Direct{
  inline int f(int c) const{ return c > 5 ? c : 5; }
};

void eval(const char *text,char *p, int N, Timer &t, const Base &a){
  printf("%s \n",text);
 
  t.startTimer();
  for(int x=0;x<5;x++){
    char *pp = p;
    for(int i=0;i<N;++i,++pp){
      *pp = a.f(*pp);
    }
  }
  t.stopTimer("Part 1");
}
void eval(const char *text,char *p, int N, Timer &t, const Direct &a){
  printf("%s \n",text);

  t.startTimer();
  for(int x=0;x<5;x++){  
    char *pp = p;
    for(int i=0;i<N;++i,++pp){
      *pp = a.f(*pp);
    }
  }
  t.stopTimer("Part 2");
}
struct ExtBase : public Base{
  virtual inline int f(int c) const{ return c > 6 ? c : 6; }
};

struct ExtBaseNoInline : public Base{ virtual int f(int c) const; };
int ExtBaseNoInline::f(int c) const{ return c > 8 ? c : 8;}


int main(){
  Timer t;
  int N = 1000*1000*100;
  char *p = new char[N];

  for(int i=0;i<N;i++){
    p[i]=i;
  }

  Base base;
  ExtBase extbase;
  ExtBaseNoInline extbasenoinline;
  
  Base extbase2 = ExtBase();
  Direct direct;


  eval("base",p,N,t,base);  
  eval("extbase",p,N,t,extbase);
  eval("extbase-noinline",p,N,t,extbasenoinline);
  eval("extbase2 given as base",p,N,t,extbase2);
  eval("direct",p,N,t,direct);    

  return 0;
}
