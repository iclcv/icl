/// utility function (works for arbitrary forward iterator types)
template<class ForwardIterator>
void apply_linear_threshold(ForwardIterator begin, 
                            ForwardIterator end, icl8u t){
  do{ 
    *begin = (*begin>t)*255; 
  }while(++begin != end);
}

// Benchmark result: 0.96ms (very fast!)
void threshold(Img8u &image, icl8u t){
  for(int c=0;c<image.getChannels();++c){
    apply_linear_threshold(image.begin(c),image.end(c),t);
  }
}
