#include "WeightedSum.h"
namespace icl {

#ifdef WITH_IPP_OPTIMIZATIONP

  // {{{ ippi-function call templates

  template <typename T> 
  Img32f* getImg32fCopy(Img<T> *src,Img32f &buf)
  {
    return 0;
  }
  template <>
  Img32f* getImg32fCopy<icl32f>(Img32f *src,Img32f &buf)
  {
    return src;
  }
  template <>
  Img32f* getImg32fCopy<icl8u>(Img8u *src,Img32f &buf)
  {
    src->deepCopy(&buf);
    return &buf;
  }





  template <typename T>
  inline void ippiwsCall(Img<T> *src, Img32f *dst,  const std::vector<float>& weights,Img32f &m_oDepthBuf,Img32f &m_oAccuBuf)
  {
    // {{{ open
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels()>=2);
    ICLASSERT_RETURN( src->getChannels() == (int)weights.size() );
    ICLASSERT_RETURN( dst->getChannels() == 1);
    
    Img32f* src32 = getImg32fCopy (src,m_oDepthBuf);
    m_oAccuBuf.resize(dst->getROISize());
    ippiMulC_32f_C1R(src32->getROIData (0), src32->getLineStep(),weights[0],dst->getROIData (0), dst->getLineStep(),dst->getROISize());
    for (int c=1; c <src32->getChannels(); c++) {
      ippiMulC_32f_C1R(src32->getROIData (c), src32->getLineStep(),weights[c],m_oAccuBuf.getROIData (0), m_oAccuBuf.getLineStep(),m_oAccuBuf.getROISize());
      ippiAdd_32f_C1IR(m_oAccuBuf.getROIData (0), m_oAccuBuf.getLineStep(),dst->getROIData (0), dst->getLineStep(),dst->getROISize());
    }
  }
    // }}}

  // }}}
  // {{{ function specializations

  void WeightedSum::ws (Img8u *src, Img32f *dst,  const std::vector<float>& weights)
  {
    ippiwsCall<icl8u>(src,dst,weights,m_oDepthBuf,m_oAccuBuf);
  }
  void WeightedSum::ws (Img32f *src, Img32f *dst,  const std::vector<float>& weights)
  {
    ippiwsCall<icl32f>(src,dst,weights,m_oDepthBuf,m_oAccuBuf);
  }

  // }}}

#else
  // {{{ C++ fallback CompareOp classes

   template <typename T>
   void fallbackWs(const Img<T> *src, Img32f *dst, const std::vector<float>& weights) {
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() >=2);
      ICLASSERT_RETURN( dst->getChannels() ==1);


      ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(0);
      ImgIterator<icl32f> itDst = dst->getROIIterator(0);
      
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
          *itDst = *itSrc * weights[0];
      }

      for (int c=1;c<src->getChannels();c++){
         ImgIterator<T> itSrc = const_cast<Img<T>*>(src)->getROIIterator(c);
         ImgIterator<icl32f> itDst = dst->getROIIterator(0);
         for(;itSrc.inRegion(); ++itSrc, ++itDst){
            *itDst += *itSrc * weights[c];
         }
      }
   }

  void WeightedSum::ws (Img8u *src, Img32f *dst,  const std::vector<float>& weights)
  {
    fallbackWs<icl8u>(src,dst,weights);
  }
  void WeightedSum::ws (Img32f *src, Img32f *dst,  const std::vector<float>& weights)
  {
    fallbackWs<icl32f>(src,dst,weights);
  }




#endif

  // {{{ ImgI* versions

  void WeightedSum::ws (ImgI *poSrc, ImgI **ppoDst, const std::vector<float> weights)
  {

    // {{{ open
    if (!Filter::prepare (ppoDst, poSrc)) return;
    if (poSrc->getDepth () == depth8u)
      ws(poSrc->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>(),weights);
    else
      ws(poSrc->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(),weights);
  }
  // }}}

// }}}
}
