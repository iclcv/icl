#include <ICLUtils/DynVector.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Macros.h>

using namespace icl;

typedef DynMatrix<float> Mat;
typedef DynColVector<float> CVec;
typedef DynRowVector<float> RVec;

int main(){
  Mat m(5,5);
  for(int x=0;x<5;++x){
    for(int y=0;y<5;++y){
      m(x,y) = x + 10*y;
    }
  }
  
  CVec v2 = m.col(2);
  RVec r2 = m.row(2);
  CVec v22 = r2.transp();
  RVec r22 = v2.transp();
  
  SHOW((v2,v2));

  SHOW(v2%v2);

  SHOW(v2%r2);
  
  //  SHOW(v2);
  // SHOW(r2);
  // SHOW(v22);
  // SHOW(r22);

}
