#include <iclImg.h>

// for main() only
#include <iclQuick.h> 

// just temporarily
#include <iclStackTimer.h>

using namespace icl;

// utiliy function 
template<class T, unsigned int CHANNELS>
T inline mean_val_t(const T* srcs[CHANNELS], int idx){
  T accu = 0;
  for(unsigned int i=0;i<CHANNELS;++i){
    accu += srcs[i][idx];
  }
  return accu / CHANNELS;
}
/*
template<> inline icl8u mean_val_t<icl8u,3>(const icl8u* srcs[3], int idx){
  return ((int)srcs[0][idx]+srcs[1][idx]+srcs[2][idx])/3;
}
template<> inline icl32f mean_val_t<icl32f,3>(const icl32f* srcs[3], int idx){
  return ((int)srcs[0][idx]+srcs[1][idx]+srcs[2][idx])/3;
}
    */

/// demo function computes channel mean value
template<class T, int CHANNELS>
void inline mean_val_t(const Img<T> &src, Img<T> &dst){
  BENCHMARK_THIS_FUNCTION;

  // now: fix-sized array
  const T* srcs[CHANNELS];
  for(int i=0;i<src.getChannels();++i){
    srcs[i] = src.begin(i);
  }

  // extract destination pointer
  T *d = dst.begin(0);
  const int dim = src.getDim();
  
  // run successively over all pixels
  for(int i=0;i<dim;++i){
    d[i] = mean_val_t<T,CHANNELS>(srcs,i);
  }
}

template<class T>
void mean_val(const Img<T> &src, Img<T> &dst){
  // adapt dst
  dst.setSize(src.getSize());  
  dst.setChannels(1);

  switch(src.getChannels()){
    case 0: return;
    case 1: mean_val_t<T,1>(src,dst); break;
    case 2: mean_val_t<T,2>(src,dst); break;
    case 3: mean_val_t<T,3>(src,dst); break;
    default: ERROR_LOG("unsupported!");
  }
}

int main(){
  Img8u a = cvt8u(create("parrot"));
  Img8u dst;
  for(int i=0;i<100;++i){
    mean_val(a,dst);
  }
}
