#include "iclProgArg.h"
#include <string>

using namespace icl;
using std::string;

int main(int n, char **ppc){
  pa_init(n,ppc,"-size(2) -format(1) -channels(1) -fast");

  printf("programs name is %s \n",pa_progname().c_str());
  printf("argcount is %d \n",pa_argcount());

  if(pa_defined("-size")){
    printf("given size was %d %d \n",pa_subarg<int>("-size",0,0),pa_subarg<int>("-size",1,0));  
  }
  if(pa_defined("-format")){
    printf("given format was %s \n",pa_subarg<string>("-format",0,"").c_str());
  }
  if(pa_defined("-fast")){
    printf("enabling fast (whatever this will effect!?) \n");
  }
  
  for(unsigned int i=0;i<pa_argcount();i++){
    printf("arg %d was %s \n",i,pa_arg<string>(i).c_str());
  }
  
  return 0;
}
