// Benchmark result: ~ 29ms
void thresh(Img8u &image, icl8u t){
  for(int x=0;x<image.getWidth();++x){
    for(int y=0;y<image.getHeight();++y){
      PixelRef<icl8u> p = image(x,y);
      for(int c=0;c<p.getChannels();++c){
        p[c] = 255*(p[c]>t);
      }
    }
  }
}
