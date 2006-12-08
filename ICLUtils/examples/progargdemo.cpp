#include "ProgArg.h"

using namespace icl;

int main(int n, char **ppc){
  pa_init(n,ppc,"-size(2) -format(1) -channels(1) -fast");

  printf("programs name is %s \n",pa_progname().c_str());
  printf("argcount is %d \n",pa_argcount());

  if(pa_defined("-size")){
    printf("given size was %d %d \n",pa_subarg<int>("-size",0),pa_subarg<int>("-size",1));  
  }
  if(pa_defined("-format")){
    printf("given format was %s \n",pa_subarg<char*>("-format",0));
  }
  if(pa_defined("-fast")){
    printf("enabling fast (whatever this will effect!?) \n");
  }
  
  for(unsigned int i=0;i<pa_argcount();i++){
    printf("arg %d was %s \n",i,pa_arg<char*>(i));
  }
  
  return 0;
}
