#include <iclQuick.h>

int main(){
  ImgQ a = create("parrot");
  
  // cut outer border
  a.setROI(a.getImageRect().enlarge(-100));
  ImgQ b(a.getROISize(),a.getChannels());
  roi(b) = roi(a);
  
  show(b);
  
}
