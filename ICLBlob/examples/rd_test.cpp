#include <iclQuick.h>
#include <iclQuickRegions.h>
#include <iclRegionDetector.h>


int main(){

  ImgQ a = create("parrot");

  a = scale(a,0.5);

  a = gray(a);

  a = thresh(a,128);

  a = filter(a,"median");

  int s = 1478;

  ERROR_LOG("centers");
  vector<Point> cs = centers(a,s,s);
  ERROR_LOG("bbs");
  vector<Rect> bs = boundingboxes(a,s,s);
  ERROR_LOG("pcas");
  vector<RegionPCAInfo> pcas = pca(a,s,s);
  ERROR_LOG("boundaries");


  vector<vector<Point> > bounds = boundaries(a,s,s);

  a = rgb(a);
  color(255,0,0); pix(a,cs);
  for(unsigned int i=0;i<bs.size();++i){
    color(255,255,0); rect(a,bs[i]);
  }
  color(0,0,255); draw(a,pcas);

  for(unsigned int i=0;i<bounds.size();++i){
    color(0,255,255); pix(a,bounds[i]);
  }
  
  
  
  show(a);
  

  return 0;
}
