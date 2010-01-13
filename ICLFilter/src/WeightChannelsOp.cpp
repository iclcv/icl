#include <ICLFilter/WeightChannelsOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>

namespace icl {
  
  using namespace std;
  
  void WeightChannelsOp::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
    ICLASSERT_RETURN(poSrc);
    ICLASSERT_RETURN( (int) m_vecWeights.size() == poSrc->getChannels() );
    
    const ImgBase *oTmpSrcImg;
    ImgBase *oTmpDstImg;

    if(!UnaryOp::prepare(ppoDst,poSrc)) return;
    
    UnaryArithmeticalOp op = UnaryArithmeticalOp(UnaryArithmeticalOp::mulOp);
    
    for (int c=0;c<poSrc->getChannels();c++) {
      oTmpSrcImg = poSrc->selectChannel(c);
      oTmpDstImg = (*ppoDst)->selectChannel(c);
      op.setValue(m_vecWeights[c]);
      op.apply(oTmpSrcImg, &oTmpDstImg);
      delete oTmpSrcImg;
      delete oTmpDstImg;
    }
  }
} // namespace icl
