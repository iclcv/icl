#include <iclDCGrabber.h>


using namespace std;
using namespace icl;


int main(int n, char **ppc){
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  printf("found %d cameras \n",devs.size());

  
  for(unsigned int i=0;i<devs.size();i++){
    DCDevice &d = devs[i];
    char acBuf[100];
    sprintf(acBuf,"%2d",i);
    d.show(acBuf);
  }
  
  return 0;
}

