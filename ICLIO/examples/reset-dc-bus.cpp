#include <ICLIO/DCDevice.h>
#include <ICLUtils/ProgArg.h>

int main(int n, char **ppc){
  icl::painit(n,ppc,"-quiet|-q");
  icl::DCDevice::dc1394_reset_bus(!icl::pa("-q"));
}
