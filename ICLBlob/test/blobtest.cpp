#include "DefaultColorBlobSearcher.h"
#include <Img.h>
#include <vector>

using namespace icl;
using namespace std;



int main(){
  printf("-------preparing images -------------- \n");
  Img8u image(Size(320,240),3);
  Img8u mask(Size(320,240),1);

  mask.clear(0,1);
  

  // image is red !!
  image.clear(0,255);
  image.clear(1,0);
  image.clear(2,2);
  

  printf("------- preparing blobmatcher -------------- \n");
  DefaultColorBlobSearcher blob(image.getSize());
  
  vector<icl8u> rs,gs,bs;
  rs.push_back(255);
  gs.push_back(10);
  bs.push_back(10);
  
  icl8u thresh[3] = {50,20,20}; 
  
  printf("------- adding new blobs -------------- \n");
  blob.addSubSearcher(rs,gs,bs,thresh);

  rs.push_back(0);
  gs.push_back(100);
  bs.push_back(100);

  printf("------- adding another blobs -------------- \n");
  blob.addSubSearcher(rs,gs,bs,thresh);

  printf("------- searching -------------- \n");

  vector< FoundBlob<float> > result = blob.search(&image,&mask);
  
  printf("------- showing results -------------- \n");
  for(uint i=0;i<result.size();i++){
    printf(" found a blob at %d %d  id=%d rating = %f \n",result[i].x(),result[i].y(),result[i].id(),result[i].rating());
  }
  
  printf("-------done -------------- \n");
  
  return 0;
}
