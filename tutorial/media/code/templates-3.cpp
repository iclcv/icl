#include <iclImg.h>

// for main() only
#include <iclQuick.h> 

// just temporarily
#include <iclStackTimer.h>

using namespace icl;

// utiliy function 
template<class T>
T mean_val(std::vector<const T*> &srcs, int idx){
  T accu = 0;
  for(unsigned int i=0;i<srcs.size();++i){
    accu += srcs[i][idx];
  }
  return accu / srcs.size();
}

/// demo function computes channel mean value
template<class T>
void mean_val(const Img<T> &src, Img<T> &dst){
  BENCHMARK_THIS_FUNCTION;
  // to avoid erros
  if(!src.getChannels()) return; 
  
  // adapt dst
  dst.setSize(src.getSize());  
  dst.setChannels(1);
  
  // store channel-pointers
  std::vector<const T*> srcs(src.getChannels());
  for(int i=0;i<src.getChannels();++i){
    srcs[i] = src.begin(i);
  }

  // extract destination pointer
  T *d = dst.begin(0);
  const int dim = src.getDim();
  
  // run successively over all pixels
  for(int i=0;i<dim;++i){
    d[i] = mean_val(srcs,i);
  }
}


int main(){
  Img8u a = cvt8u(create("parrot"));
  Img8u dst;
  for(int i=0;i<100;++i){
    mean_val(a,dst);
  }
}
