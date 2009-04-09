// Benchmark result 1.3ms
void threshold(Img8u &image, icl8u t){
  for(int c=0;c<image.getChannels();++c){
    apply_linear_threshold(image.beginROI(c),image.endROI(c),t);
  }
}
