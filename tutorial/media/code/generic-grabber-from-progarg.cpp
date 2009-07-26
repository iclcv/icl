#include <iclGenericGrabber.h>
#include <iclProgArg.h>

using namespace icl;

int main(int n, char **ppc){
  pa_init(n,ppc,"-input(2)");
  
  GenericGrabber g(FROM_PROGARG("-input"));
}
