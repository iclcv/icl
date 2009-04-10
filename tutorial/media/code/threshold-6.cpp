// Benchmark result: 0.58ms (wow!)
void threshold(Img8u &image, icl8u t){
  for(int c=0;c<image.getChannels();++c){
    register icl8u *p = image.getData(c);
    register icl8u *end = p+image.getDim();
    register int steps = (end-p)/64;
    // programmer likes loop unrolling ...
    for(int i=0;i<steps;++i){
#define X tt(*p++,t);
      X X X X X X X X X X X X X X X X
      X X X X X X X X X X X X X X X X
      X X X X X X X X X X X X X X X X
      X X X X X X X X X X X X X X X X
#undef X
    }
    while(p<end){
      tt(*p++,t);
    }
  }
}
