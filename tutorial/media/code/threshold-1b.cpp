// Benchmark result: ~ 7.5ms
void thresh(Img8u &image, icl8u t){
  for(int c=0;c<image.getChannels();++c){
    for(int x=0;x<image.getWidth();++x){
      for(int y=0;y<image.getHeight();++y){
        icl8u &pix = image(x,y,c);
        pix = 255*(pix>t);
      }
    }
  }
}
