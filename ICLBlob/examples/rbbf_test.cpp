#include <RegionBasedBlobSearcher.h>
#include <ICLQuick.h>
#include <ImgRegionDetector.h>

vector<icl8u> vec3(icl8u r, icl8u g, icl8u b){
  vector<icl8u> v(3);
  v[0] = r;
  v[1] = g;
  v[2] = b;
  return v;
} 

void testA(){
  ImgQ A = scale(create("parrot"),0.3);
  A = gray(A);
  A = levels(A,5);
  A = filter(A,"median");

  ImgRegionDetector RD(100,1000,100,255);
  vector<BlobData> data = RD.detect(&A);
  A = rgb(A);
  for(unsigned int i=0;i<data.size();i++){
    color(255,255,255);
    pix(A,data[i].getCOG());  
       
    color(255,0,0);
    pix(A,data[i].getBoundary());
    
    color(255,0,0,100);
    pix(A,data[i].getPixels());
  }
  A = scale(A,3);
  
  show(A);
}


int main(){
  ImgQ A = scale(create("parrot"),0.3);
  A = gray(A);
  A = levels(A,5);
  A = filter(A,"median");
  
  RegionBasedBlobSearcher R;
  FMCreator *fmc[] = {
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(10,200,10), vec3(180,180,180)),
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(200,10,10), vec3(180,180,180)),
    FMCreator::getDefaultFMCreator(A.getSize(),formatRGB,vec3(10,10,200), vec3(180,180,180))
  };
  
  RegionFilter *rf[] = { new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ),   // size range
                         new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ),   // size range
                         new RegionFilter( new Range<icl8u>(200,255),       // value range
                                           new Range<icl32s>(10,1000000) ) };   // size range
  for(int i=0;i<3;++i){
    R.add(fmc[i],rf[i]);
  }
 
  R.extractRegions(&A);
 
  A =rgb(A);
  color(255,0,0,200);
  pix(A,R.getCOGs());
    
  
  color(0,255,0,100);
  const Array<Array<Point> > &boundaries = R.getBoundaries();
  for(unsigned int i=0;i<boundaries.size();i++){
    pix(A,boundaries[i]);
  }
  
  show(A);
}
