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
  ImgQ A = create("parrot");
  A = levels(A,5);
  
  
  RegionBasedBlobSearcher R;
  R.addDefaultFMCreator (A.getSize(), formatRGB, vec3(10,200,10), vec3(80,80,80), 10,1000,true);
  R.addDefaultFMCreator (A.getSize(), formatRGB, vec3(200,10,10), vec3(80,80,80), 10,1000,true);
  R.addDefaultFMCreator (A.getSize(), formatRGB, vec3(10,10,200), vec3(80,80,80), 10,1000,true);

  vector<Point> ca  = R.getCenters(&A);
  vector<Rect> bb = R.getBoundingBoxes(&A);
  
  color(0,0,0);
  
  for(vector<Point>::iterator it = ca.begin();it!= ca.end();++it){
    cross(A,*it);
  }
  for(vector<Rect>::iterator it = bb.begin();it!= bb.end();++it){
    rect(A,*it);
  }

  // todo !!
  /****
  color(255,255,255,100);
  pix(A,R.getPixels());
  
  color(255,255,255,255);
  pix(A,R.getBoundary());
  ****/


  show(A);
  
}
