// Benchmark result 0.16ms (Wow!)
void threshold(Img8u &image, icl8u t){
  for(int c=0;c<image.getChannels();++c){
    ippiCompareC_8u_C1R(image.getROIData(c),image.getLineStep(),t,
                        image.getROIData(c),image.getLineStep(),
                        image.getROISize(), ippCmpGreater);
  }
}
