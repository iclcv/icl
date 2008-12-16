#include <iclConvolutionOp.h>
#include <iclImg.h>
#include <limits>

namespace icl{
  namespace{
    template<class SrcType, class DstType, typename ippfunc>
    inline void ipp_call_fixed(const Img<SrcType> &src, Img<DstType> &dst, int channel, ippfunc func){
      func(src.getROIData(channel),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize());
    }
    template<class SrcType, class DstType, typename ippfunc>
    inline void ipp_call_fixed_mask(const Img<SrcType> &src, Img<DstType> &dst, int channel, IppiMaskSize m, ippfunc func){
      func(src.getROIData(channel),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),m);
    }
    template<class ImageType,typename ippfunc>
    inline void ipp_call_filter_int(const Img<ImageType> &src, Img<ImageType> &dst, const int *kernel, int channel, ConvolutionOp &op, ippfunc func){
      func(src.getROIData(channel),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),
              kernel, op.getKernel().getSize(), op.getAnchor(), op.getKernel().getFactor() );
    }
    template<class ImageType,typename ippfunc>
    inline void ipp_call_filter_float(const Img<ImageType> &src, Img<ImageType> &dst, const float *kernel, int channel, ConvolutionOp &op, ippfunc func){
      func(src.getROIData(channel),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),
              dst.getROISize(), kernel, op.getKernel().getSize(), op.getAnchor() );
    }


    template<class KernelType, class SrcType, class DstType>
    void generic_cpp_convolution(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){
      ConstImgIterator<SrcType> s(src.getData(c), src.getWidth(),Rect(op.getROIOffset(), dst.getROISize()));
      ImgIterator<DstType>      d = dst.getROIIterator(c);
      Point an = op.getAnchor();
      Size si = op.getMaskSize();
      int factor = op.getKernel().getFactor();
      for(; s.inRegion(); ++s){
        const KernelType *m = k; 
        KernelType buffer = 0;
        for(ConstImgIterator<SrcType> sR (s,si,an);sR.inRegion(); ++sR, ++m){
          buffer += (*m) * (KernelType)(*sR);
        }
        *d++ = clipped_cast<KernelType, DstType>(buffer / factor);
      }
    }
    
    template<class KernelType, class SrcType, class DstType, ConvolutionKernel::fixedType t>
    inline void convolute(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){
      /// here we call the generic conv method and do not implement the convolution directly to 
      /// get rid of the 4th template parameter 't' which is not regarded in this general case
      DEBUG_LOG("calling generic conv " << c);
        
      generic_cpp_convolution(src,dst,k,op,c);
    }

#ifdef HAVE_IPP
    /// define specializations of convolute-template

    // note: each specialization is doubled because the kernel-type template parameters is handled in the IPP
#define FIXED_SPEC(SD,DD,KT,IPPF)                                                                                               \
    template<> inline void                                                                                                      \
    convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp&, int c){     \
      ipp_call_fixed(src,dst,c,IPPF);                                                                                           \
    }                                                                                                                           \
    template<> inline void                                                                                                      \
    convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp&, int c){ \
      ipp_call_fixed(src,dst,c,IPPF);                                                                                           \
    }    
#define FIXED_SPEC_M(SD,DD,KT,IPPF,MASK)                                                                                        \
    template<> inline void                                                                                                      \
    convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp&, int c){     \
      ipp_call_fixed_mask(src,dst,c,MASK,IPPF);                                                                                 \
    }                                                                                                                           \
    template<> inline void                                                                                                      \
    convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp&, int c){ \
      ipp_call_fixed_mask(src,dst,c,MASK,IPPF);                                                                                 \
    }
#define CONV_SPEC(KD,ID,IPPF)                                                                                                           \
    template<> inline void                                                                                                              \
    convolute<KD,icl##ID,icl##ID,ConvolutionKernel::custom>(const Img##ID &src,Img##ID &dst,const KD* kernel,ConvolutionOp& op, int c){ \
      ipp_call_filter_##KD(src,dst,kernel,c,op,IPPF);                                                                                   \
    }

    // fixed sobel x filters
    FIXED_SPEC(8u,8u,sobelX3x3,ippiFilterSobelHoriz_8u_C1R);
        

    FIXED_SPEC(16s,16s,sobelX3x3,ippiFilterSobelHoriz_16s_C1R);
    FIXED_SPEC(32f,32f,sobelX3x3,ippiFilterSobelHoriz_32f_C1R);
    FIXED_SPEC_M(8u,16s,sobelX3x3,ippiFilterSobelHoriz_8u16s_C1R,ippMskSize3x3);
    FIXED_SPEC_M(8u,16s,sobelX5x5,ippiFilterSobelHoriz_8u16s_C1R,ippMskSize5x5);
    FIXED_SPEC_M(32f,32f,sobelX5x5,ippiFilterSobelHorizMask_32f_C1R,ippMskSize5x5);

    // fixed sobel y filters
    FIXED_SPEC(8u,8u,sobelY3x3,ippiFilterSobelVert_8u_C1R);
    FIXED_SPEC(16s,16s,sobelY3x3,ippiFilterSobelVert_16s_C1R);
    FIXED_SPEC(32f,32f,sobelY3x3,ippiFilterSobelVert_32f_C1R);
    FIXED_SPEC_M(8u,16s,sobelY3x3,ippiFilterSobelVert_8u16s_C1R,ippMskSize3x3);
    FIXED_SPEC_M(8u,16s,sobelY5x5,ippiFilterSobelVert_8u16s_C1R,ippMskSize5x5);
    FIXED_SPEC_M(32f,32f,sobelY5x5,ippiFilterSobelVertMask_32f_C1R,ippMskSize5x5);


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



    CONV_SPEC(int,8u,ippiFilter_8u_C1R);
    CONV_SPEC(int,16s,ippiFilter_16s_C1R);
    CONV_SPEC(float,32f,ippiFilter_32f_C1R);


    CONV_SPEC(float,8u,ippiFilter32f_8u_C1R);
    CONV_SPEC(float,16s,ippiFilter32f_16s_C1R);

#undef FIXED_SPEC
#undef FIXED_SPEC_M
#undef CONV_SPEC

#endif //HAVE_IPP


    template<class KernelType, class SrcType, class DstType, ConvolutionKernel::fixedType t>
    inline void apply_convolution_sdt(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op){
      for(int c=src.getChannels()-1;c>=0;--c){
        convolute<KernelType,SrcType,DstType,t>(src,dst,k,op,c);
      }
    }
    
    template<class KernelType, class SrcType, class DstType>
    inline void apply_convolution_sd(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op){
      switch(op.getKernel().getFixedType()){
#define CASE(X) case ConvolutionKernel::X: apply_convolution_sdt<KernelType,SrcType,DstType,ConvolutionKernel::X>(src,dst,k,op); break
        CASE(gauss3x3);CASE(gauss5x5);CASE(sobelX3x3);CASE(sobelX5x5);
        CASE(sobelY3x3);CASE(sobelY5x5);CASE(laplace3x3);CASE(laplace5x5);
        CASE(custom);
#undef CASE
        default:
          ERROR_LOG("undefined ConvolutionKernel::fixedType here: (int value:" << (int)op.getKernel().getFixedType() << ")");
      }
    } 
    
    
    template<class KernelType, class SrcType>
    inline void apply_convolution_s(const Img<SrcType> &src, ImgBase &dst,const KernelType *k, ConvolutionOp &op){
      switch(dst.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_sd(src,*dst.asImg<icl##D>(),k,op); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_DEPTH;
      }
    } 
    
    template<class KernelType>
    inline void apply_convolution(const ImgBase &src, ImgBase &dst,const KernelType *k, ConvolutionOp &op){
      switch(src.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_convolution_s(*src.asImg<icl##D>(),dst,k,op); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_DEPTH;
      }
    }  
  } // end of anonymous namespace
  
  ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel):
    NeighborhoodOp(kernel.getSize()),m_forceUnsignedOutput(false){
    setKernel(kernel);
  }
  ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel, bool forceUnsignedOutput):
    NeighborhoodOp(kernel.getSize()),m_forceUnsignedOutput(forceUnsignedOutput){
    setKernel(kernel);
  }

  void ConvolutionOp::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(!m_kernel.isNull());
    if(m_forceUnsignedOutput){
      if(!prepare(dst,src)) return;
    }else{
      if(!prepare(dst,src,src->getDepth()==depth8u ? depth16s : src->getDepth())) return;
    }

    if(src->getDepth() >= depth32f){
      m_kernel.toFloat();
    }else{
      m_kernel.toInt(false);
    }

    if(m_kernel.isFloat()){
      apply_convolution<float>(*src,**dst,m_kernel.getFloatData(),*this);
    }else{
      apply_convolution<int>(*src,**dst,m_kernel.getIntData(),*this);
    }
  }
}
