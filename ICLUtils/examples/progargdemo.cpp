#include "iclProgArg.h"
#include <string>
#include <cstdio>

using namespace icl;
using std::string;

int main(int n, char **ppc){
  
  pa_explain("-size",
             "image size\n"
             "first param = width (one of 160, 320 or 640)\n"
             "second param = height one of (120, 240 or 480)");
  pa_explain("-format","image format\none of:\n- formatRGB\n- formatGray\n- formatHLS");
  pa_explain("-channels","count of image channels\none of {1,2,3,4}");
  pa_explain("-fast","enables the \"fast\"-mode which does everything\nmuch faster!");
  pa_init(n,ppc,"-size(2) -format(1) -channels(1) -fast",true);

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
