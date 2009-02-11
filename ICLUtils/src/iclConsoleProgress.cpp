#include <iclConsoleProgress.h>

using namespace std;
using namespace icl;

namespace{

  char BACK_LINE[] = { char(27), char(91), char(65), char(0)};// "\E[A\0"

  int BAR_LEN = 30;
  
  string PROGRESS_STR;
}
namespace icl{
  void progress_init(const std::string &text){
    // {{{ open

    printf("\n");
    printf("%s%s\n",BACK_LINE,text.c_str());
    PROGRESS_STR = text;
  }

  // }}}
  
  void progress_finish(){
    // {{{ open

    printf("%s%s:100%s[##############################]\n",BACK_LINE,PROGRESS_STR.c_str(),"%");
    
  }

  // }}}
  
  void progress(int curr, int max){
    // {{{ open

    printf("%s%s:",BACK_LINE,PROGRESS_STR.c_str());
    float frac = (float)curr/(float)max;
    printf("%3d%s",(int)(frac*100),"%");
    
    int N1 = (int)(frac*BAR_LEN);
    int N2 = BAR_LEN - N1;
    
    printf("[");
    for(int i=0;i<N1;i++){
      printf("#");
    }
    for(int i=0;i<N2;i++){
      printf("=");
    }
    printf("]\n");
    
  }
}
