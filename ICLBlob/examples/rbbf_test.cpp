#include <RegionBasedBlobSearcher.h>
#include <ICLQuick.h>

vector<icl8u> vec3(icl8u r, icl8u g, icl8u b){
  vector<icl8u> v(3);
  v[0] = r;
  v[1] = g;
  v[2] = b;
  return v;
} 

int main(){
  ImgQ A = create("flowers");
  A = levels(scale(a,0.4),5);
  
  
  RegionBasedBlobSearcher R;
  R.addDefaultFMCreator (a.getSize(), formatRGB, vec3(200,10,10), vec3(30,30,30), 10,1000,true);

  vector<int> ca  = rbbs.getCentersPOD(&a);
  vector<Rect> bb = rbbs.getBoundingBoxes(&a);
  
  color(255,255,255);
  
  for(unsigned int i=0;i<ca.size()/2;i++){
    cross(a,ca[2*i],ca[2*i+1]);
  }
  for(unsigned int i=0;i<cb.size()/2;i++){
    cross(b,cb[2*i],cb[2*i+1]);
  }
  show((a,b));
  
}
