#include <ICLFilter/ConvolutionOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using COp = filter::ConvolutionOp;

  // ================================================================
  // C++ generic convolution implementations
  // ================================================================

  template<class KernelType, class SrcType, class DstType>
  void generic_cpp_convolution(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, COp &op, int c){
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    const int maskW = op.getMaskSize().width, maskH = op.getMaskSize().height;
    const Point anchor = op.getAnchor();
    const Point roiOff = op.getROIOffset();
    const int factor = op.getKernel().getFactor();
    const SrcType *srcData = src.getData(c);
    DstType *dstROI = dst.getROIData(c);

    for(int y = 0; y < roiH; ++y){
      DstType *dstRow = dstROI + y * dstW;
      for(int x = 0; x < roiW; ++x){
        const KernelType *m = k;
        KernelType buffer = 0;
        for(int my = 0; my < maskH; ++my){
          const SrcType *row = srcData + (roiOff.y - anchor.y + y + my) * srcW
                               + (roiOff.x - anchor.x + x);
          for(int mx = 0; mx < maskW; ++mx){
            buffer += (*m++) * (KernelType)row[mx];
          }
        }
        dstRow[x] = clipped_cast<KernelType, DstType>(buffer / factor);
      }
    }
  }

  template<class KernelType, class SrcType, class DstType>
  void generic_cpp_convolution_3x3(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, COp &op, int c){
    const SrcType *s = src.getROIData(c,op.getROIOffset());
    DstType *d = dst.getROIData(c);
    int factor = op.getKernel().getFactor();

    const int xEnd = dst.getROISize().width;
    const int yEnd = dst.getROISize().height;
    const KernelType m[9] = {k[0],k[1],k[2],k[3],k[4],k[5],k[6],k[7],k[8]};
    const int dy = src.getWidth();

    if(factor != 1){
      for(int y=0;y<yEnd;++y){
        for(int x=0;x<xEnd;++x){
          d[x] = ( s[x-dy-1]*m[0] + s[x-dy]*m[1] + s[x-dy+1]*m[2] +
                   s[x-1]*m[3] + s[x]*m[4] + s[x+1]*m[5] +
                   s[x+dy-1]*m[6] + s[x+dy]*m[7] + s[x+dy+1]*m[8] ) / factor;
        }
        s+=src.getWidth();
        d+=dst.getWidth();
      }
    }else{
      for(int y=0;y<yEnd;++y){
        for(int x=0;x<xEnd;++x){
          d[x] = ( s[x-dy-1]*m[0] + s[x-dy]*m[1] + s[x-dy+1]*m[2] +
                   s[x-1]*m[3] + s[x]*m[4] + s[x+1]*m[5] +
                   s[x+dy-1]*m[6] + s[x+dy]*m[7] + s[x+dy+1]*m[8] );
        }
        s+=src.getWidth();
        d+=dst.getWidth();
      }
    }
  }

  // ================================================================
  // Generic convolute template — C++ fallback for all combos
  // ================================================================

  template<class KernelType, class SrcType, class DstType, filter::ConvolutionKernel::fixedType t>
  inline void convolute(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, COp &op, int c){
    if(op.getAnchor() == Point(1,1) && op.getMaskSize() == Size(3,3)){
      generic_cpp_convolution_3x3(src,dst,k,op,c);
    }else{
      generic_cpp_convolution(src,dst,k,op,c);
    }
  }

  // ================================================================
  // Dispatch chain: KernelType -> SrcDepth -> DstDepth -> FixedType
  // ================================================================

  template<class KernelType, class SrcType, class DstType, filter::ConvolutionKernel::fixedType t>
  inline void apply_convolution_sdt(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, COp &op){
    for(int c=src.getChannels()-1;c>=0;--c){
      convolute<KernelType,SrcType,DstType,t>(src,dst,k,op,c);
    }
  }

  template<class KernelType, class SrcType, class DstType>
  inline void apply_convolution_sd(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, COp &op){
    switch(op.getKernel().getFixedType()){
#define CASE(X) case filter::ConvolutionKernel::X: apply_convolution_sdt<KernelType,SrcType,DstType,filter::ConvolutionKernel::X>(src,dst,k,op); break
      CASE(gauss3x3);CASE(gauss5x5);CASE(sobelX3x3);CASE(sobelX5x5);
      CASE(sobelY3x3);CASE(sobelY5x5);CASE(laplace3x3);CASE(laplace5x5);
      CASE(custom);
#undef CASE
      default:
        ERROR_LOG("undefined ConvolutionKernel::fixedType here: (int value:" << static_cast<int>(op.getKernel().getFixedType()) << ")");
    }
  }

  template<class KernelType, class SrcType>
  inline void apply_convolution_s(const Img<SrcType> &src, ImgBase &dst,const KernelType *k, COp &op){
    switch(dst.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_sd(src,*dst.asImg<icl##D>(),k,op); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }

  template<class KernelType>
  inline void apply_convolution(const ImgBase &src, ImgBase &dst,const KernelType *k, COp &op){
    switch(src.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_s(*src.asImg<icl##D>(),dst,k,op); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }

  // ================================================================
  // C++ backend entry point
  // ================================================================

  void cpp_convolution(const Image &src, Image &dst, COp &op) {
    auto &kernel = op.getKernel();
    if(kernel.isFloat()){
      apply_convolution<float>(*src.ptr(), *dst.ptr(), kernel.getFloatData(), op);
    }else{
      apply_convolution<int>(*src.ptr(), *dst.ptr(), kernel.getIntData(), op);
    }
  }

  static int _reg = [] {
    using Op = COp::Op;
    auto& proto = COp::prototype();
    proto.addBackend<COp::ConvSig>(Op::apply, Backend::Cpp, cpp_convolution, "C++ convolution");
    return 0;
  }();

} // anonymous namespace
