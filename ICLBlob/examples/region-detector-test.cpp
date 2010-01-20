#include <ICLQuick/Quick.h>
#include <ICLQuick/QuickRegions.h>
#include <ICLBlob/RegionDetector.h>


int main(){

  ImgQ a = create("parrot");

  a = scale(a,0.5);

  a = gray(a);

  a = thresh(a,128);

  a = filter(a,"median");

  a.setROI(Rect(20,20,100,300));
  vector<vector<Point> > px = pixels(a,0,2<<20,255,255);
  vector<RegionPCAInfo> pcas = pca(a,0,2<<20,255,255);
  
  ImgQ b = a*0;
  a = a|b|b;
  for(unsigned int i=0;i<px.size();++i){
    color(0,255,0);
    pix(a,px[i]);
    
    color(0,0,255);
    draw(a,pcas[i]);
  }
  show(a);
  
  /** TEST 2
  a.setROI(Rect(20,20,100,300));

  ERROR_LOG("centers");
  vector<Point> cs = centers(a);
  ERROR_LOG("bbs");
  vector<Rect> bs = boundingboxes(a);
  ERROR_LOG("pcas");
  vector<RegionPCAInfo> pcas = pca(a);
  ERROR_LOG("boundaries");

  
  

  ImgQ xxx = rgb(a);
  xxx = xxx*0.2;
  ImgQ xxx2 = copy(xxx);
  color(255,0,0);
  pix(xxx,118-a.getROI().x,57-a.getROI().y);
  show((xxx,zeros(1,1,1),xxx2));
  

  vector<vector<Point> > bounds = boundaries(a);

  a.setFullROI();
 
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
  */
  
  

  

  return 0;
}
