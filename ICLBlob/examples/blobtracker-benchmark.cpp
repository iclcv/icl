#include <ICLUtils/Timer.h>
#include <ICLCore/Img.h>
#include <ICLCore/Mathematics.h>
#include <ICLBlob/Extrapolator.h>
#include <ICLBlob/PositionTracker.h>
#include <ICLBlob/HungarianAlgorithm.h>

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
using namespace icl;

typedef std::vector<int> vec;

int main(int n, char  **ppc){
  PositionTracker<int> pt;
  randomSeed();
  
  printf("Benchmarking and Testing the BlobTracker class: \n");
  int ns[] = {2,3,4,5,10,20,50,100,200};
  
  for(int iN = 2;iN<9;iN++){
    int N = ns[iN];
    printf("testing algorithm with N=%d positions \n",N);
    const int K = 100;
    const int D = 10;
    int  *pi = new int[N*K];
    for(int i=0;i<N*K;i++){
      pi[i] = random(static_cast<unsigned int>(10));
    }
    pt.pushData(vec(pi,pi+N),vec(pi,pi+N));    
    Timer t(1); t.start();
    for(int i=0;i<D;i++){
      int *ppp = pi+N*i;
      pt.pushData(vec(ppp,ppp+N),vec(ppp,ppp+N));    
    }
    printf("average time of 10 = %fms \n",float(t.stopSilent())/(10*1000.0));
    delete pi;
  }
  
  return 0;
}
