/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ConvolutionOp.cpp              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLFilter/ConvolutionOp.h>
#include <ICLCore/Img.h>
#include <limits>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
    namespace{
  #ifdef ICL_HAVE_IPP

  inline IppStatus borderInit_16s(const Ipp16s* pKernel, IppiSize kernelSize, int divisor, IppDataType dataType, int numChannels, IppRoundMode roundMode, IppiFilterBorderSpec* pSpec){
    return ippiFilterBorderInit_16s(pKernel, kernelSize, divisor, dataType, numChannels, roundMode, pSpec);//devisor only in 16s
  }
  inline IppStatus borderInit_32f(const Ipp32f* pKernel, IppiSize kernelSize, int divisor, IppDataType dataType, int numChannels, IppRoundMode roundMode, IppiFilterBorderSpec* pSpec){
    return ippiFilterBorderInit_32f(pKernel, kernelSize, dataType, numChannels, roundMode, pSpec);
  }

/*      template<class SrcType, class DstType, typename ippfunc>
      inline void ipp_call_fixed(const Img<SrcType> &src, Img<DstType> &dst, int channel, ippfunc func, ConvolutionOp &op){
        func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize());
      }
      template<class SrcType, class DstType, typename ippfunc>
      inline void ipp_call_fixed_mask(const Img<SrcType> &src, Img<DstType> &dst, int channel, IppiMaskSize m, ippfunc func, ConvolutionOp &op){
        func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),m);
      }
      template<class ImageType,typename ippfunc>
      inline void ipp_call_filter_int(const Img<ImageType> &src, Img<ImageType> &dst, const int *kernel, int channel, ConvolutionOp &op, ippfunc func){
        func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),dst.getROISize(),
                kernel, op.getKernel().getSize(), op.getAnchor(), op.getKernel().getFactor() );
      }
      template<class ImageType,typename ippfunc>
      inline void ipp_call_filter_float(const Img<ImageType> &src, Img<ImageType> &dst, const float *kernel, int channel, ConvolutionOp &op, ippfunc func){
        func(src.getROIData(channel,op.getROIOffset()),src.getLineStep(),dst.getROIData(channel),dst.getLineStep(),
                dst.getROISize(), kernel, op.getKernel().getSize(), op.getAnchor() );
      }*/
  #endif

      template<class KernelType, class SrcType, class DstType>
      void generic_cpp_convolution(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){
        const ImgIterator<SrcType> s(const_cast<SrcType*>(src.getData(c)), src.getWidth(),Rect(op.getROIOffset(), dst.getROISize()));
        const ImgIterator<SrcType> sEnd = ImgIterator<SrcType>::create_end_roi_iterator(src.getData(c),src.getWidth(), Rect(op.getROIOffset(), dst.getROISize()));
        ImgIterator<DstType>      d = dst.beginROI(c);
        Point an = op.getAnchor();
        Size si = op.getMaskSize();
        int factor = op.getKernel().getFactor();
        for(; s != sEnd; ++s){
          const KernelType *m = k;
          KernelType buffer = 0;
          for(const ImgIterator<SrcType> sR (s,si,an);sR.inRegionSubROI(); ++sR, ++m){
            buffer += (*m) * (KernelType)(*sR);
          }
          *d++ = clipped_cast<KernelType, DstType>(buffer / factor);
        }
      }

      template<class KernelType, class SrcType, class DstType>
      void generic_cpp_convolution_3x3(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){

        register const SrcType *s = src.getROIData(c,op.getROIOffset());
        register DstType *d = dst.getROIData(c);
        register int factor = op.getKernel().getFactor();

        register const int xEnd = dst.getROISize().width;
        register const int yEnd = dst.getROISize().height;
        register const KernelType m[9] = {k[0],k[1],k[2],k[3],k[4],k[5],k[6],k[7],k[8]};
        register const int dy = src.getWidth();

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


      template<class KernelType, class SrcType, class DstType, ConvolutionKernel::fixedType t>
      inline void convolute(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){
        /// here we call the generic conv method and do not implement the convolution directly to
        /// get rid of the 4th template parameter 't' which is not regarded in this general case
        if(op.getAnchor() == Point(1,1) && op.getMaskSize() == Size(3,3)){
          generic_cpp_convolution_3x3(src,dst,k,op,c);
        }else{
          generic_cpp_convolution(src,dst,k,op,c);
        }
      }



  #ifdef ICL_HAVE_IPP
      /// define specializations of convolute-template

      // note: each specialization is doubled because the kernel-type template parameters is handled in the IPP
  #define FIXED_SPEC_M_GAUSSIAN(SD,DD,KT,IPPF,IPPT,MASK)                                                                                                   \
      template<> inline void                                                                                                          \
      convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp &op, int c){      \
        /*ipp_call_fixed(src,dst,c,IPPF,op);*/                                                                                            \
        IppStatus status = ippStsNoErr; \
        Ipp32u kernelSize = MASK; \
        Ipp32f sigma = 0.35f; \
        Ipp8u *pBuffer = NULL; \
        IppFilter##IPPF##Spec* pSpec = NULL; \
        int iTmpBufSize = 0, iSpecSize = 0;  \
        IppiBorderType borderType = ippBorderRepl; \
        Ipp8u borderValue = 0; \
        int numChannels = 1; \
        status = ippiFilter##IPPF##GetBufferSize(dst.getROISize(), kernelSize, ipp32f, \
          numChannels, &iSpecSize, &iTmpBufSize); \
        pSpec = (IppFilter##IPPF##Spec *)ippsMalloc_8u(iSpecSize); \
        pBuffer = ippsMalloc_8u(iTmpBufSize); \
        status = ippiFilter##IPPF##Init(dst.getROISize(), kernelSize, sigma, \
          borderType, ipp32f, numChannels, pSpec, pBuffer); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), borderValue, pSpec, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer); \
        ippsFree(pSpec); \
      }                                                                                                                               \
      template<> inline void                                                                                                          \
      convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp &op, int c){  \
        /*ipp_call_fixed(src,dst,c,IPPF,op);*/                                                                                            \
        IppStatus status = ippStsNoErr; \
        Ipp32u kernelSize = MASK; \
        Ipp32f sigma = 0.35f; \
        Ipp8u *pBuffer = NULL; \
        IppFilter##IPPF##Spec* pSpec = NULL; \
        int iTmpBufSize = 0, iSpecSize = 0;  \
        IppiBorderType borderType = ippBorderRepl; \
        Ipp8u borderValue = 0; \
        int numChannels = 1; \
        status = ippiFilter##IPPF##GetBufferSize(dst.getROISize(), kernelSize, ipp32f, \
          numChannels, &iSpecSize, &iTmpBufSize); \
        pSpec = (IppFilter##IPPF##Spec *)ippsMalloc_8u(iSpecSize); \
        pBuffer = ippsMalloc_8u(iTmpBufSize); \
        status = ippiFilter##IPPF##Init(dst.getROISize(), kernelSize, sigma, \
          borderType, ipp32f, numChannels, pSpec, pBuffer); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), borderValue, pSpec, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer); \
        ippsFree(pSpec); \
      }
  #define FIXED_SPEC_SOBEL(SD,DD,KT,IPPF,IPPT)                                                                                                   \
      template<> inline void                                                                                                          \
      convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp &op, int c){      \
        /*ipp_call_fixed(src,dst,c,IPPF,op);*/                                                                                            \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; /*| ippBorderInMemTop | ippBorderInMemRight;*/ \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), ippMskSize3x3, ipp##SD, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()) + src.getLineStep(), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), ippMskSize3x3, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer); \
      }                                                                                                                                \
      template<> inline void                                                                                                          \
      convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp &op, int c){  \
        /*ipp_call_fixed(src,dst,c,IPPF,op);*/                                                                                            \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; /* | ippBorderInMemTop | ippBorderInMemRight;*/ \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), ippMskSize3x3, ipp##SD, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()) + src.getLineStep(), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), ippMskSize3x3, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer);                                                                                                                               \
      }
  #define FIXED_SPEC_M(SD,DD,KT,IPPF,IPPT,MASK)                                                                                                   \
      template<> inline void                                                                                                          \
      convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp &op, int c){      \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), MASK, ipp##SD, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()) + src.getLineStep(), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), MASK, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer); \
      }                                                                                                                                \
      template<> inline void                                                                                                          \
      convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp &op, int c){  \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), MASK, ipp##SD, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        status = ippiFilter##IPPF##Border_##IPPT(src.getROIData(c,op.getROIOffset()) + src.getLineStep(), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), MASK, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer);                                                                                                                               \
      }
  #define FIXED_SPEC_M_PRECONV(SD,DD,KT,IPPF,IPPT,MASK)                                                                                    \
      template<> inline void                                                                                                          \
      convolute<int,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const int*,ConvolutionOp &op, int c){      \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), MASK, ipp16s, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        Img16s src2(src.getSize(), src.getChannels(), src.getFormat()); \
        ippiConvert_8u16s_C1R(src.getROIData(c), src.getLineStep(), src2.getROIData(c), src2.getLineStep(), src2.getROISize()); \
        status = ippiFilter##IPPF##Border_##IPPT(src2.getROIData(c,op.getROIOffset()) + src2.getLineStep(), src2.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), MASK, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer); \
      }                                                                                                                                \
      template<> inline void                                                                                                          \
      convolute<float,icl##SD,icl##DD,ConvolutionKernel::KT>(const Img##SD &src,Img##DD &dst,const float*,ConvolutionOp &op, int c){  \
        Ipp8u *pBuffer=NULL; \
        IppiBorderType borderType = ippBorderRepl; \
        int bufferSize=0; \
        IppStatus status = ippStsNoErr; \
        ippiFilter##IPPF##BorderGetBufferSize(dst.getROISize(), MASK, ipp16s, ipp##DD, 1, &bufferSize); \
        pBuffer = ippsMalloc_8u(bufferSize); \
        Img16s src2(src.getSize(), src.getChannels(), src.getFormat()); \
        ippiConvert_8u16s_C1R(src.getROIData(c), src.getLineStep(), src2.getROIData(c), src2.getLineStep(), src2.getROISize()); \
        status = ippiFilter##IPPF##Border_##IPPT(src2.getROIData(c,op.getROIOffset()) + src2.getLineStep(), src2.getLineStep(), dst.getROIData(c), dst.getLineStep(), \
          dst.getROISize(), MASK, borderType, 0, pBuffer); \
        if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
        ippsFree(pBuffer);                                                                                                                               \
      }
  #define CONV_SPEC(KD,ID,KDD)                                                                                                     \
      template<> inline void                                                                                                              \
      convolute<KD,icl##ID,icl##ID,ConvolutionKernel::custom>(const Img##ID &src,Img##ID &dst,const KD* kernel,ConvolutionOp& op, int c){ \
        IppStatus status = ippStsNoErr; \
	      Ipp8u *pBuffer = NULL; \
	      IppiFilterBorderSpec* pSpec = NULL; \
	      int iTmpBufSize = 0, iSpecSize = 0; \
	      IppiBorderType borderType = ippBorderRepl; \
	      Ipp##ID borderValue = 0; \
	      int numChannels = 1; \
	      status = ippiFilterBorderGetSize(op.getKernel().getSize(), src.getROISize(), ipp##ID, ipp##KDD/*KERNELTYPE*/, numChannels, &iSpecSize, &iTmpBufSize); \
	      pSpec = (IppiFilterBorderSpec *)ippsMalloc_8u(iSpecSize); \
	      pBuffer = ippsMalloc_8u(iTmpBufSize); \
        status = borderInit_##KDD((const Ipp##KDD *)kernel, op.getKernel().getSize()/*KERNELSIZE*/, 4, ipp##ID, numChannels, ippRndNear, pSpec); \
	      status = ippiFilterBorder_##ID##_C1R(src.getROIData(c,op.getROIOffset()), src.getLineStep(), dst.getROIData(c), dst.getLineStep(), dst.getROISize(), borderType, &borderValue, pSpec, pBuffer); \
	      if(status!=ippStsNoErr){ \
          WARNING_LOG("IPP Error"); \
        } \
	      ippsFree(pBuffer); \
        ippsFree(pSpec); \
        /*ipp_call_filter_##KD(src,dst,kernel,c,op,IPPF);*/                                                                                   \
      }
      // fixed sobel y filters
      //FIXED_SPEC_SOBEL(8u,16s,sobelY3x3,SobelHoriz,8u16s_C1R);//ippiFilterSobelHorizBorder_8u16s_C1R); //No 8u to 8u exist
      FIXED_SPEC_SOBEL(16s,16s,sobelY3x3,SobelHoriz,16s_C1R);//ippiFilterSobelHorizBorder_16s_C1R);
      FIXED_SPEC_SOBEL(32f,32f,sobelY3x3,SobelHoriz,32f_C1R);//ippiFilterSobelHorizBorder_32f_C1R);
      FIXED_SPEC_M(8u,16s,sobelY3x3,SobelHoriz,8u16s_C1R,ippMskSize3x3);
      FIXED_SPEC_M(8u,16s,sobelY5x5,SobelHoriz,8u16s_C1R,ippMskSize5x5);
      FIXED_SPEC_M(32f,32f,sobelY5x5,SobelHoriz,32f_C1R,ippMskSize5x5);

      // fixed sobel x filters
      //FIXED_SPEC_SOBEL(8u,16s,sobelX3x3,SobelVert,8u16s_C1R);//ippiFilterSobelVertBorder_8u16s_C1R);
      FIXED_SPEC_SOBEL(16s,16s,sobelX3x3,SobelVert,16s_C1R);//ippiFilterSobelVertBorder_16s_C1R);
      FIXED_SPEC_SOBEL(32f,32f,sobelX3x3,SobelVert,32f_C1R);//ippiFilterSobelVertBorder_32f_C1R);
      FIXED_SPEC_M(8u,16s,sobelX3x3,SobelVert,8u16s_C1R,ippMskSize3x3);
      FIXED_SPEC_M(8u,16s,sobelX5x5,SobelVert,8u16s_C1R,ippMskSize5x5);
      FIXED_SPEC_M(32f,32f,sobelX5x5,SobelVert,32f_C1R,ippMskSize5x5);


      // fixed laplace filters
      FIXED_SPEC_M(8u,8u,laplace3x3,Laplace,8u_C1R,ippMskSize3x3);
      FIXED_SPEC_M(16s,16s,laplace3x3,Laplace,16s_C1R,ippMskSize3x3);//ippiFilterLaplaceBorder_16s_C1R
      FIXED_SPEC_M(32f,32f,laplace3x3,Laplace,32f_C1R,ippMskSize3x3);
      FIXED_SPEC_M_PRECONV(8u,16s,laplace3x3,Laplace,16s_C1R,ippMskSize3x3);//8u16s no longer exist
      FIXED_SPEC_M(8u,8u,laplace5x5,Laplace,8u_C1R,ippMskSize5x5);
      FIXED_SPEC_M(16s,16s,laplace5x5,Laplace,16s_C1R,ippMskSize5x5);
      FIXED_SPEC_M(32f,32f,laplace5x5,Laplace,32f_C1R,ippMskSize5x5);
      FIXED_SPEC_M_PRECONV(8u,16s,laplace5x5,Laplace,16s_C1R,ippMskSize5x5);


      // fixed gaussian filters
      FIXED_SPEC_M_GAUSSIAN(8u,8u,gauss3x3,Gaussian,8u_C1R,3);//ippMskSize3x3);
      FIXED_SPEC_M_GAUSSIAN(16s,16s,gauss3x3,Gaussian,16s_C1R,3);//ippMskSize3x3);
      FIXED_SPEC_M_GAUSSIAN(32f,32f,gauss3x3,Gaussian,32f_C1R,3);//ippMskSize3x3);
      FIXED_SPEC_M_GAUSSIAN(8u,8u,gauss5x5,Gaussian,8u_C1R,5);//ippMskSize5x5);
      FIXED_SPEC_M_GAUSSIAN(16s,16s,gauss5x5,Gaussian,16s_C1R,5);//ippMskSize5x5);
      FIXED_SPEC_M_GAUSSIAN(32f,32f,gauss5x5,Gaussian,32f_C1R,5);//ippMskSize5x5);



      CONV_SPEC(int,8u,16s);
      CONV_SPEC(int,16s,16s);
      CONV_SPEC(float,32f,32f);

      CONV_SPEC(float,8u,32f);
      CONV_SPEC(float,16s,32f);

  #undef FIXED_SPEC_SOBEL
  #undef FIXED_SPEC_M_GAUSSIAN
  #undef FIXED_SPEC_M
  #undef FIXED_SPEC_M_PRECONV
  #undef CONV_SPEC

  #endif //ICL_HAVE_IPP


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
      }else if(m_kernel.isFloat()){
        WARNING_LOG("convolution of non-float images with float kernels is not supported\n"
                    "use an int-kernel instead. For now, the kernel is casted to int-type");
        m_kernel.toInt(true);
      }

      if(m_kernel.isFloat()){
        apply_convolution<float>(*src,**dst,m_kernel.getFloatData(),*this);
      }else{
        apply_convolution<int>(*src,**dst,m_kernel.getIntData(),*this);
      }
    }
  } // namespace filter
}
