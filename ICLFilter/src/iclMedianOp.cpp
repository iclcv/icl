#include <iclMedianOp.h>
#include <iclImg.h>
#include <vector>
#include <algorithm>

namespace icl {
  
  namespace{
    template<typename T>
    void apply_median (const Img<T> *src, Img<T> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) {
      // {{{ open
      std::vector<T> oList(oMaskSize.getDim());
      typename std::vector<T>::iterator itList = oList.begin();
      typename std::vector<T>::iterator itMedian = oList.begin()+((oMaskSize.width * oMaskSize.height)/2);
      
      for (int c=0;c<src->getChannels();c++){
        ConstImgIterator<T> s(src->getData(c),src->getWidth(), Rect(roiOffset, dst->getROISize()));
        ImgIterator<T> d= dst->getROIIterator(c); 
        for(;s.inRegion();++s,++d){
          for(ConstImgIterator<T> sR(s,oMaskSize,oAnchor); sR.inRegion(); ++sR, ++itList){
            *itList = *sR;
          }
          std::sort(oList.begin(),oList.end());
          *d = *itMedian;
          itList = oList.begin();
        }
      }
    }

    // }}}
    
#ifdef HAVE_IPP

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint)>
    void ippMedian(const Img<T>* src, Img<T> *dst, const Size& maskSize,const Point &roiOffset, const Point &oAnchor) {
      // {{{ open

    for(int c=0; c < src->getChannels(); c++) {
      ippiFunc(src->getROIData (c,roiOffset), src->getLineStep(),
               dst->getROIData (c), dst->getLineStep(), 
               dst->getROISize(), maskSize, oAnchor);
     }
  }

    // }}}
    
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiMaskSize)>
    void ippMedianFixed(const Img<T>*src, Img<T> *dst,const Point &roiOffset, int maskSize) {
      // {{{ open

      for(int c=0; c < src->getChannels(); c++) {
      ippiFunc(src->getROIData(c,roiOffset), src->getLineStep(),
               dst->getROIData(c), dst->getLineStep(), 
               dst->getROISize(), maskSize == 3 ? ippMskSize3x3 : ippMskSize5x5);
    }
  }

    // }}}

    template<> 
    void apply_median<icl8u>(const Img8u *src, Img8u *dst, const Size &maskSize,const Point &roiOffset, const Point &anchor){
      // {{{ open

      if(maskSize == Size(3,3)){
        ippMedianFixed<icl8u,ippiFilterMedianCross_8u_C1R>(src,dst,roiOffset,3);
      }else if(maskSize == Size(5,5)){
        ippMedianFixed<icl8u,ippiFilterMedianCross_8u_C1R>(src,dst,roiOffset,5);
      }else{
        ippMedian<icl8u,ippiFilterMedian_8u_C1R>(src,dst,maskSize,roiOffset,anchor);
      }
    } 

    // }}}
    
    template<> 
    void apply_median<icl16s>(const Img16s *src, Img16s *dst, const Size &maskSize,const Point &roiOffset, const Point &anchor){
      // {{{ open

      if(maskSize == Size(3,3)){
        ippMedianFixed<icl16s,ippiFilterMedianCross_16s_C1R>(src,dst,roiOffset,3);
      }else if(maskSize == Size(5,5)){
        ippMedianFixed<icl16s,ippiFilterMedianCross_16s_C1R>(src,dst,roiOffset,5);
      }else{
        ippMedian<icl16s,ippiFilterMedian_16s_C1R>(src,dst,maskSize,roiOffset,anchor);
      }
    }

    // }}}
#endif
 
  } // end of anonymous namespace
  
  void MedianOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open

    FUNCTION_LOG("");
    
    if (!prepare (ppoDst, poSrc)) return;

    switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                        \
      case depth##D:                                    \
        apply_median(poSrc->asImg<icl##D>(),            \
                     (*ppoDst)->asImg<icl##D>(),        \
                     getMaskSize(),                     \
                     getROIOffset(),                    \
                     getAnchor());                      \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  // }}}
  
}
