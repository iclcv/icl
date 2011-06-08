#include <ICLQuick/Quick.h>
#include <ICLIO/GenericGrabber.h>

#include <sstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
  bool c = true;
  int n = 0;
  
  while(c) {
    try {
      std::stringstream pss;
      pss << "kinectc=" << n;
      
      icl::GenericGrabber("kinectc",pss.str());

      n++;
    } catch(ICLException e) {
      c = false;
    }
  }

  std::cout << "Found " << n << " Kinect Devices." << std::endl;
  
  return !n;
}
