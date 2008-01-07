#include <iclDCGrabber.h>
#include <iclDC.h>
#include <map>
#include <vector>

using namespace std;
using namespace icl;
typedef std::map<int,vector<dc1394camera_t*> > cammap;

int main(int n, char **ppc){
  cammap m;
  
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  printf("found %d cameras \n",devs.size());
  
  
  for(unsigned int i=0;i<devs.size();i++){
    m[devs[i].getUnit()].push_back(devs[i].getCam());
    dc1394_camera_reset(devs[i].getCam());
    printf("calling reset for camera %d [%s] \n",i,devs[i].getModelID().c_str());
  }
  printf("\n");
  for(cammap::iterator it = m.begin();it != m.end();++it){
    dc1394_iso_release_all((it->second)[0]);
    //    dc1394_cleanup_iso_channels_and_bandwidth((it->second)[0]);
    printf("cleaning iso channel for port %d \n",it->first);
  }
  printf("\n");
  for(unsigned int i=0;i<devs.size();i++){
    dc1394_camera_free(devs[i].getCam());
    printf("releasing camera %d [%s] \n",i,devs[i].getModelID().c_str());
  }
  printf("\n");
  printf("iso channels cleared successfully !\n");
  
  
  return 0;
}

