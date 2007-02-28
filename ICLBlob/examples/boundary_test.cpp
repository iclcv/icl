#include <ICLQuick.h>
#include <RegionDetector.h>

using namespace regiondetector;

int main(){
  //0  1  2  3  4  5  6  7  8  9
  unsigned char data[] = { 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  
  vector<unsigned char*> vData; vData.push_back(data);
  Img8u IM(Size(10,10),formatMatrix,vData);

  //

      ImgQ A = create("parrot");
   
      A = scale(A,0.8);
      A = gray(A);
      A = levels(A,3);
      IM = cvt8u(A);
 //

  ImgQ P = cvt(IM);
  ImgQ R = P*0;
  
  RegionDetector rd(IM.getWidth(),IM.getHeight(),500,1000000,0,255);
  
  BlobList *l = rd.find_blobs(IM.getData(0));

  for(BlobList::iterator it=l->begin();it!=l->end();++it){
    
    RegionDetectorBlob *b = *it;
    vector<Point> pts;
    tic();
    for(int i=0;i<1000;i++){
      pts = b->getBoundary(IM.getSize());
    }
    toc();
    for(unsigned int i=0;i<pts.size();i++){
      pix(R,pts[i].x,pts[i].y);
    }    
  } 
  
  show((P,R));
  

  
  return 0;
}
