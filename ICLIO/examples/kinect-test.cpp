#include <ICLQuick/Quick.h>
#include <ICLIO/GenericGrabber.h>

int main(int argc, char *argv[])
{
  icl::GenericGrabber("kinectc",0);
  return 0;
}
