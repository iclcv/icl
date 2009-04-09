// Benchmark result: ~ 15ms
void thresh(Img8u &image, icl8u t){
  for(int x=0;x<image.getWidth();++x){
    for(int y=0;y<image.getHeight();++y){
      for(int c=0;c<image.getChannels();++c){
        image(x,y,c) = image(x,y,c) > t ? 255 : 0;
      }
    }
  }
} 
