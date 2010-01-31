#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Size.h>
#include <string>
#include <cstdio>

using namespace icl;

int main(int n, char **ppc){
  
  paex("-s","defines image size")
      ("-f","defines image foramt (RGB,Gray or HLS)")
      ("-c","defines channel count to use")
      ("-fast","enables a 'fast'-mode");

  painit(n,ppc,"-size|-s(Size=VGA) -format|-f(format=RGB) -channels|-c(int) -fast",true);

  // program name
  SHOW(paprogname());

  // number of all arguments
  SHOW(pacount(false));
  
  SHOW(!!pa("-c"));

  SHOW(!!pa("-f"));
  // number of dangling arguments

  Size s = pa("-s");
  std::cout << "argument '-size' was " << (pa("-s")?"":"not ") 
            << "given: " << s << std::endl;
  
  if(pa("-format")){
    SHOW(pa("-format"));
  }
  if(pa("-fast")){
    std::cout << "argument -fast was given:" << std::endl;
  }
  SHOW(pa("-fast"));
    
  
  for(unsigned int i=0;i<pacount();i++){
    std::cout << "dangling argument " << i << " is:'" 
              << pa(i) << "'" << std::endl;
  }
}
