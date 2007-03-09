#include <iclWeightedSumOp.h>

namespace icl {
  using namespace std;
   // {{{ C++ fallback
  namespace{
    template <class T, class D>
    void apply_ws(const Img<T> *src, Img<D> *dst, const vector<D>& weights) {
     
      ConstImgIterator<T> itSrc = src->getROIIterator(0);
      ImgIterator<D> itDst = dst->getROIIterator(0);
      
      D w = weights[0];
      for(;itSrc.inRegion(); ++itSrc, ++itDst){
        *itDst = *itSrc * w;
      }
      
      for (int c=src->getChannels()-1; c > 0; c--) {
        w = weights[c];
        itSrc = src->getROIIterator(c);
        itDst = dst->getROIIterator(0);
        for(;itSrc.inRegion(); ++itSrc, ++itDst){
          *itDst += *itSrc * w;
        }
      }
    }
  }

  // }}}
  
  void WeightedSumOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
    ICLASSERT_RETURN(poSrc);
    ICLASSERT_RETURN( (int)m_vecWeights.size() == poSrc->getChannels() );
    
    if(!prepare(ppoDst,
                poSrc->getDepth() == depth64f ? depth64f : depth32f,
                getClipToROI() ? poSrc->getROISize() : poSrc->getSize(),
                formatMatrix,1,getClipToROI() ? Rect(Point::null,poSrc->getROISize()) : poSrc->getROI(),
                poSrc->getTime()) ) return;
    
    if((*ppoDst)->getDepth() == depth64f){
      switch (poSrc->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_ws(poSrc->asImg<icl##D>(),(*ppoDst)->asImg<icl64f>(),m_vecWeights); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }else{
      vector<icl32f> v(m_vecWeights.size());
      for(unsigned int i=0;i<m_vecWeights.size();++i){
        v[i] = (float)m_vecWeights[i];
      }
      switch (poSrc->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_ws(poSrc->asImg<icl##D>(),(*ppoDst)->asImg<icl32f>(),v); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }

}
