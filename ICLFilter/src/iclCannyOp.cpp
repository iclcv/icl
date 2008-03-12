#include "iclCannyOp.h"
#include "iclImg.h"
#include "iclConvolutionOp.h"

namespace icl {

#ifdef WITH_IPP_OPTIMIZATION
  // without ipp non of the function is implemented
  CannyOp::CannyOp(icl32f lowThresh, icl32f highThresh):
    // {{{ open

    m_pucBuffer8u(0),m_fLowThresh(lowThresh), 
    m_fHighThresh(highThresh), m_bDeleteOps(true){
    FUNCTION_LOG("");
    m_apoDXY[0] = NULL;
    m_apoDXY[1] = NULL;
    m_apoDXYOps[0] = new ConvolutionOp(ConvolutionOp::kernelSobelX3x3);
    m_apoDXYOps[1] = new ConvolutionOp(ConvolutionOp::kernelSobelY3x3);
  }

  // }}}
  
  CannyOp::CannyOp(UnaryOp *dxOp, UnaryOp *dyOp,icl32f lowThresh, icl32f highThresh, bool deleteOps):
    // {{{ open

    m_pucBuffer8u(0),m_fLowThresh(lowThresh), 
    m_fHighThresh(highThresh), m_bDeleteOps(deleteOps) {
    FUNCTION_LOG("");
    m_apoDXY[0] = NULL;
    m_apoDXY[1] = NULL;
    m_apoDXYOps[0] = dxOp;
    m_apoDXYOps[1] = dyOp;
  }

  // }}}
  
  CannyOp::~CannyOp(){
    // {{{ open

    FUNCTION_LOG("");
    if(m_pucBuffer8u){
      delete [] m_pucBuffer8u;
    }
    if (m_apoDXY[0]){delete m_apoDXY[0];}
    if (m_apoDXY[1]){delete m_apoDXY[1];}
    if(m_bDeleteOps){
      if(m_apoDXYOps[0]) delete m_apoDXYOps[0];
      if(m_apoDXYOps[1]) delete m_apoDXYOps[1];
    }
  }

  // }}}

  /// no CannyOp::apply without ipp

  void CannyOp::apply (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
  {
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( ppoDst );
    ICLASSERT_RETURN( poSrc != *ppoDst);
    ICLASSERT_RETURN( poSrc->getDepth() == depth32f );
    FUNCTION_LOG("");
    for(int i=0;i<2;i++){
      m_apoDXYOps[i]->setClipToROI (true);
      m_apoDXYOps[i]->apply(poSrc,m_apoDXY+i);
    }
    if (!prepare (ppoDst, m_apoDXY[0], depth8u)) return;

    int bufferSizeNeeded;
    ippiCannyGetSize(m_apoDXY[0]->getSize(), &bufferSizeNeeded);
    if(bufferSizeNeeded != m_iBufferSize){
      if(m_pucBuffer8u) delete m_pucBuffer8u;
      m_pucBuffer8u = new icl8u[bufferSizeNeeded];
      m_iBufferSize = bufferSizeNeeded;
    }
    
    for (int c=m_apoDXY[0]->getChannels()-1; c >= 0; --c) {
      ippiCanny_32f8u_C1R (m_apoDXY[0]->asImg<icl32f>()->getROIData (c), m_apoDXY[0]->getLineStep(),
                           m_apoDXY[1]->asImg<icl32f>()->getROIData (c), m_apoDXY[1]->getLineStep(),
                           (*ppoDst)->asImg<icl8u>()->getROIData (c), (*ppoDst)->getLineStep(),
                           (*ppoDst)->getROISize(),m_fLowThresh,m_fHighThresh,m_pucBuffer8u);
    }
  }
   // }}}
  
  void CannyOp::setThresholds(icl32f lo, icl32f hi){
    // {{{ open

    m_fLowThresh = lo;
    m_fHighThresh = hi;
  }

  // }}}
  
  icl32f CannyOp::getLowThreshold()const {
    // {{{ open

    return m_fLowThresh;
  }

  // }}}
  icl32f CannyOp::getHighThreshold()const {
    // {{{ open

    return m_fHighThresh;
  }

  // }}}
#endif

  
}
