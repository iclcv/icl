// Benchmark result: 6.8ms
void threshold(Img8u &image, icl8u t){
  ICLASSERT_RETURN(image.getChannels() == 3);
  Channel<icl8u> cs[3];
  image.extractChannels(cs);
  for(int x=0;x<image.getWidth();++x){
    for(int y=0;y<image.getHeight();++y){
      for(int c=0;c<3;++c){
        cs[c](x,y) = 255*(cs[c](x,y)>t);
      }
    }
  }
}
