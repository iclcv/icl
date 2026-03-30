#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLFilter/ConvolutionOp.h>

// Many fixed-kernel IPP functions (ippiFilterSobelHoriz_*, ippiFilterLaplace_*,
// ippiFilterGauss_*, ippiFilter_*) were renamed/removed in modern IPP (oneAPI 2022+).
// The new APIs use ippiFilterSobelBorder_*, ippiFilterGaussBorder_*, etc.
// TODO: update all 34 specializations to modern Border-based IPP APIs.
#if 0 // was: ICL_HAVE_IPP — fixed-kernel filter APIs removed from modern IPP

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // ================================================================
  // IPP helper functions
  // ================================================================

  template<class SrcType, class DstType, typename ippfunc>
  inline void ipp_call_fixed(const Img<SrcType> &src, Img<DstType> &dst, int channel, ippfunc func, filter::ConvolutionOp &op){
    func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize());
  }
  template<class SrcType, class DstType, typename ippfunc>
  inline void ipp_call_fixed_mask(const Img<SrcType> &src, Img<DstType> &dst, int channel, IppiMaskSize m, ippfunc func, filter::ConvolutionOp &op){
    func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),m);
  }
  template<class ImageType,typename ippfunc>
  inline void ipp_call_filter_int(const Img<ImageType> &src, Img<ImageType> &dst, const int *kernel, int channel, filter::ConvolutionOp &op, ippfunc func){
    func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),
            kernel, op.getKernel().getSize(), op.getAnchor(), op.getKernel().getFactor() );
  }
  template<class ImageType,typename ippfunc>
  inline void ipp_call_filter_float(const Img<ImageType> &src, Img<ImageType> &dst, const float *kernel, int channel, filter::ConvolutionOp &op, ippfunc func){
    func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),
            dst.getROISize(), kernel, op.getKernel().getSize(), op.getAnchor() );
  }

  // ================================================================
  // C++ generic convolution (fallback for unsupported IPP combos)
  // ================================================================

  template<class KernelType, class SrcType, class DstType>
  void generic_cpp_convolution(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, filter::ConvolutionOp &op, int c){
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
  void generic_cpp_convolution_3x3(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, filter::ConvolutionOp &op, int c){
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
  // Generic convolute template + IPP specializations
  // ================================================================

  template<class KernelType, class SrcType, class DstType, filter::ConvolutionKernel::fixedType t>
  inline void convolute(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, filter::ConvolutionOp &op, int c){
    if(op.getAnchor() == Point(1,1) && op.getMaskSize() == Size(3,3)){
      generic_cpp_convolution_3x3(src,dst,k,op,c);
    }else{
      generic_cpp_convolution(src,dst,k,op,c);
    }
  }

  // IPP specializations via macros (same as original, with filter:: namespace)

#define FIXED_SPEC(SD,DD,KT,IPPF)                                                                                                          \
  template<> inline void                                                                                                                    \
  convolute<int,icl##SD,icl##DD,filter::ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,filter::ConvolutionOp &op, int c){\
    ipp_call_fixed(src,dst,c,IPPF,op);                                                                                                      \
  }                                                                                                                                         \
  template<> inline void                                                                                                                    \
  convolute<float,icl##SD,icl##DD,filter::ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,filter::ConvolutionOp &op, int c){\
    ipp_call_fixed(src,dst,c,IPPF,op);                                                                                                      \
  }
#define FIXED_SPEC_M(SD,DD,KT,IPPF,MASK)                                                                                                    \
  template<> inline void                                                                                                                    \
  convolute<int,icl##SD,icl##DD,filter::ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,filter::ConvolutionOp &op, int c){\
    ipp_call_fixed_mask(src,dst,c,MASK,IPPF,op);                                                                                            \
  }                                                                                                                                         \
  template<> inline void                                                                                                                    \
  convolute<float,icl##SD,icl##DD,filter::ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,filter::ConvolutionOp &op, int c){\
    ipp_call_fixed_mask(src,dst,c,MASK,IPPF,op);                                                                                            \
  }
#define CONV_SPEC(KD,ID,IPPF)                                                                                                                    \
  template<> inline void                                                                                                                         \
  convolute<KD,icl##ID,icl##ID,filter::ConvolutionKernel::custom>(const Img##ID &src,Img##ID &dst,const KD* kernel,filter::ConvolutionOp& op, int c){\
    ipp_call_filter_##KD(src,dst,kernel,c,op,IPPF);                                                                                              \
  }

  // fixed sobel y filters
  FIXED_SPEC(8u,8u,sobelY3x3,ippiFilterSobelHoriz_8u_C1R);
  FIXED_SPEC(16s,16s,sobelY3x3,ippiFilterSobelHoriz_16s_C1R);
  FIXED_SPEC(32f,32f,sobelY3x3,ippiFilterSobelHoriz_32f_C1R);
  FIXED_SPEC_M(8u,16s,sobelY3x3,ippiFilterSobelHoriz_8u16s_C1R,ippMskSize3x3);
  FIXED_SPEC_M(8u,16s,sobelY5x5,ippiFilterSobelHoriz_8u16s_C1R,ippMskSize5x5);
  FIXED_SPEC_M(32f,32f,sobelY5x5,ippiFilterSobelHorizMask_32f_C1R,ippMskSize5x5);

  // fixed sobel x filters
  FIXED_SPEC(8u,8u,sobelX3x3,ippiFilterSobelVert_8u_C1R);
  FIXED_SPEC(16s,16s,sobelX3x3,ippiFilterSobelVert_16s_C1R);
  FIXED_SPEC(32f,32f,sobelX3x3,ippiFilterSobelVert_32f_C1R);
  FIXED_SPEC_M(8u,16s,sobelX3x3,ippiFilterSobelVert_8u16s_C1R,ippMskSize3x3);
  FIXED_SPEC_M(8u,16s,sobelX5x5,ippiFilterSobelVert_8u16s_C1R,ippMskSize5x5);
  FIXED_SPEC_M(32f,32f,sobelX5x5,ippiFilterSobelVertMask_32f_C1R,ippMskSize5x5);

  // fixed laplace filters
  FIXED_SPEC_M(8u,8u,laplace3x3,ippiFilterLaplace_8u_C1R,ippMskSize3x3);
  FIXED_SPEC_M(16s,16s,laplace3x3,ippiFilterLaplace_16s_C1R,ippMskSize3x3);
  FIXED_SPEC_M(32f,32f,laplace3x3,ippiFilterLaplace_32f_C1R,ippMskSize3x3);
  FIXED_SPEC_M(8u,16s,laplace3x3,ippiFilterLaplace_8u16s_C1R,ippMskSize3x3);
  FIXED_SPEC_M(8u,8u,laplace5x5,ippiFilterLaplace_8u_C1R,ippMskSize5x5);
  FIXED_SPEC_M(16s,16s,laplace5x5,ippiFilterLaplace_16s_C1R,ippMskSize5x5);
  FIXED_SPEC_M(32f,32f,laplace5x5,ippiFilterLaplace_32f_C1R,ippMskSize5x5);
  FIXED_SPEC_M(8u,16s,laplace5x5,ippiFilterLaplace_8u16s_C1R,ippMskSize5x5);

  // fixed gaussian filters
  FIXED_SPEC_M(8u,8u,gauss3x3,ippiFilterGauss_8u_C1R,ippMskSize3x3);
  FIXED_SPEC_M(16s,16s,gauss3x3,ippiFilterGauss_16s_C1R,ippMskSize3x3);
  FIXED_SPEC_M(32f,32f,gauss3x3,ippiFilterGauss_32f_C1R,ippMskSize3x3);
  FIXED_SPEC_M(8u,8u,gauss5x5,ippiFilterGauss_8u_C1R,ippMskSize5x5);
  FIXED_SPEC_M(16s,16s,gauss5x5,ippiFilterGauss_16s_C1R,ippMskSize5x5);
  FIXED_SPEC_M(32f,32f,gauss5x5,ippiFilterGauss_32f_C1R,ippMskSize5x5);

  // custom convolution
  CONV_SPEC(int,8u,ippiFilter_8u_C1R);
  CONV_SPEC(int,16s,ippiFilter_16s_C1R);
  CONV_SPEC(float,32f,ippiFilter_32f_C1R);
  CONV_SPEC(float,8u,ippiFilter32f_8u_C1R);
  CONV_SPEC(float,16s,ippiFilter32f_16s_C1R);

#undef FIXED_SPEC
#undef FIXED_SPEC_M
#undef CONV_SPEC

  // ================================================================
  // Dispatch chain (same as in ConvolutionOp.cpp)
  // ================================================================

  template<class KernelType, class SrcType, class DstType, filter::ConvolutionKernel::fixedType t>
  inline void apply_convolution_sdt(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, filter::ConvolutionOp &op){
    for(int c=src.getChannels()-1;c>=0;--c){
      convolute<KernelType,SrcType,DstType,t>(src,dst,k,op,c);
    }
  }

  template<class KernelType, class SrcType, class DstType>
  inline void apply_convolution_sd(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, filter::ConvolutionOp &op){
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
  inline void apply_convolution_s(const Img<SrcType> &src, ImgBase &dst,const KernelType *k, filter::ConvolutionOp &op){
    switch(dst.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_sd(src,*dst.asImg<icl##D>(),k,op); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }

  template<class KernelType>
  inline void apply_convolution(const ImgBase &src, ImgBase &dst,const KernelType *k, filter::ConvolutionOp &op){
    switch(src.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_s(*src.asImg<icl##D>(),dst,k,op); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }

  // ================================================================
  // IPP backend entry point
  // ================================================================

  void ipp_convolution(const Image &src, Image &dst, filter::ConvolutionOp &op) {
    auto &kernel = op.getKernel();
    if(kernel.isFloat()){
      apply_convolution<float>(*src.ptr(), *dst.ptr(), kernel.getFloatData(), op);
    }else{
      apply_convolution<int>(*src.ptr(), *dst.ptr(), kernel.getIntData(), op);
    }
  }

  using COp = filter::ConvolutionOp;

  static int _reg = [] {
    using Op = COp::Op;
    auto& proto = COp::prototype();
    proto.addBackend<COp::ConvSig>(Op::apply, Backend::Ipp, ipp_convolution,
      applicableTo<icl8u, icl16s, icl32f>, "IPP convolution (fixed+custom kernels)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_IPP
