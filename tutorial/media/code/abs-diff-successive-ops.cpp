#include <iclCommon.h>
#include <iclBinaryArithmeticalOp.h>
#include <iclUnaryArithmeticalOp.h>
#include <iclStackTimer.h>

/// utility function
inline float abs_diff(float a, float b){
  return fabs(a-b);
}

int main(){
  
  /// create source images of size VGA
  ImgQ a = scale(create("parrot"),640,480);
  ImgQ b = scale(create("flowers"),640,480);

  /// create operators
  BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
  UnaryArithmeticalOp abs(UnaryArithmeticalOp::absOp);
  BinaryArithmeticalOp absDiff(BinaryArithmeticalOp::absSubOp);
  
  /// prepare result image
  const ImgBase *res = new ImgQ(a.getParams());

  // apply operation once to allow CPU to pre-cache
  // the images (this is just done to balance the following
  // benchmarks: otherwise the first call will be much slower
  // because the image data must be tranferred to the processor
  // cache)
  res = abs.apply(sub.apply(&a,&b));

  { // first version, here we use binary op and unary op's
    // subsequently. Internally, both of these ops use Intel IPP
    // [The intermediate image (result of 'sub') must be
    // pushed twice through the processor]
    for(int i=0;i<100;++i){
      BENCHMARK_THIS_SECTION(abs(sub)); // average time: 5.9ms*)
      res = abs.apply(sub.apply(&a,&b)); 
    }
  }

  { // 2nd version, here we use the absSubOp directly
    // this is also done by an Intel IPP function
    // [ a,b and dst are just used once ]
    for(int i=0;i<100;++i){
      BENCHMARK_THIS_SECTION(absSubOp); // average time: 3.3ms
      res = absDiff.apply(&a,&b);
    }
  }

  
  { // 3rd version, we apply a functor on the source
    // image (also just one line of code).
    // [ The operation itself is applied in a single 
    // iteration through all image pixels]
    static ImgQ c(a.getParams());
    for(int i=0;i<100;++i){
      BENCHMARK_THIS_SECTION(Img.combine); // average time: 6.0ms
      a.combine(abs_diff,b,c);
    }
    res = &c;
  }

  { // 4th version using the 'incredible' ICLQuick API
    // however, this code very human readable, it's not
    // significantly slower then the other version above.
    // [ internally, this code is similar to the first
    // version + some additional overhead for internal
    // selection of an appropriate temporary image buffer]
    static ImgQ c(a.getParams());
    for(int i=0;i<100;++i){
      BENCHMARK_THIS_SECTION(ImgQ); // average time: 6.2ms
      c = icl::abs(a-b);
    }
    res = &c;
  }
  
  // show result using ICLQuick function
  show(cvt(res));

  // *) benchmark results refer to a 2GHz Core2 Duo CPU 
  //    and gcc 4.4 with optimization '-march=native -O4'
}
