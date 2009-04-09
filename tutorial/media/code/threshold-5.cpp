/// Fuctor class
struct PixThreshFunctor{
  icl8u m_thresh;
  PixThreshFunctor(icl8u &thresh):
    m_thresh(thresh){}
  inline void operator()(icl8u &p){
    p = (p>m_thresh)*255;
  }
};

// Benchmark result: 0.75ms
void thresh(Img8u &image, icl8u t){
  image.forEach(PixThreshFunctor(t));
}
