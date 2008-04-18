#include <iclDCGrabber.h>
#include <iclDCDeviceFeatures.h>

using namespace icl;
using namespace std;


int main(){
  vector<DCDevice> devs = DCGrabber::getDeviceList();
  if(devs.size()){
    DCDeviceFeatures f(devs[0]);
    
    printf("---------- features --------------- \n");
    f.show();
    printf("---------- features end--------------- \n");
  }else{
    printf("no devices found ! \n");
  }
  

  return 0;
}

