// Benchmark result: 1.4ms
void threshold(Img8u &image, icl8u t){
  ICLASSERT_RETURN(image.getChannels() == 3);
  Channel<icl8u> cs[3];
  image.extractChannels(cs);
  for(int c=0;c<3;++c){
    for(int i=0, dim=image.getDim(); i<dim; ++i){
      cs[c][i] = (cs[c][i]>t)*255;
    }
  }
}
