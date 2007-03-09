#include "iclVQ.h"
#include "iclInterleaved.h"
#include "iclImg.h"
#include "iclMathematics.h"
#include "iclFileReader.h"
#include "iclFileWriter.h"

using namespace icl;
using namespace std;

#define TRAINSTEPS 3000
#define CLUSTER 10

int main() {
  // Initialization
  randomSeed();
  vector<icl32f*> dataPtr;
  vqinitmode vqInitMode=initRndFromData;
  
  // Real image
  vector<icl32f*> vecResultData;
  FileReader reader("demoImages/testImg.ppm");
  const ImgBase *tImgIn = reader.grab();
  ImgBase *tImg = tImgIn->convert<icl32f>();
  ImgBase *tResultImg = imgNew(depth32f, tImg->getSize(), formatGray);
  vecResultData.resize(tImg->getChannels());
  
  for (int i=0;i<tResultImg->getChannels();i++) {
    vecResultData[i] = (icl32f*) tResultImg->getDataPtr(i);
  }
  
  // create cluster with 10 nodes
  VQ<icl32f, Interleaved> vq(tImg);
  vq.createCluster(CLUSTER);

  // initialize cluster
  vq.initClusterFromSrc(vqInitMode);
  
  //---- Pixel coloring interval  ----
  unsigned int uiColoring = 255 / ( CLUSTER - 1);
  for(unsigned int i=0;i<TRAINSTEPS;i++) {
    vq.vq(i);
  }
  
  float fMinDist;
  for(unsigned int i=0;i<(unsigned int)tResultImg->getDim();i++) {
    unsigned int uiWinner = vq.nn(i,fMinDist);
    vecResultData[0][i] = uiWinner * uiColoring;  
  }
  
  //write result image
  FileWriter("resultImg.pgm").write(tResultImg);
}


