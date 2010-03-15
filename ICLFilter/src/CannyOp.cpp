/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/CannyOp.cpp                              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLFilter/CannyOp.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ConvolutionOp.h>

namespace icl {

#ifdef HAVE_IPP
  // without ipp non of the function is implemented
  CannyOp::CannyOp(icl32f lowThresh, icl32f highThresh,bool preBlur):
    // {{{ open
    m_lowT(lowThresh),m_highT(highThresh),m_ownOps(true),m_preBlur(preBlur),m_preBlurBuffer(0){
    FUNCTION_LOG("");
    m_ops[0] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3));
    m_ops[1] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3));
    m_derivatives[0]=m_derivatives[1]=0;
  }

  // }}}
  
  CannyOp::CannyOp(UnaryOp *dxOp, UnaryOp *dyOp,icl32f lowThresh, icl32f highThresh, bool deleteOps, bool preBlur):
    // {{{ open
    m_lowT(lowThresh),m_highT(highThresh),m_ownOps(deleteOps),m_preBlur(preBlur),m_preBlurBuffer(0){
    FUNCTION_LOG("");
    m_ops[0] = dxOp;
    m_ops[1] = dyOp;

    m_derivatives[0]=m_derivatives[1]=0;
  }

  // }}}
  
  CannyOp::~CannyOp(){
    // {{{ open

    FUNCTION_LOG("");

    for(int i=0;i<2;++i){
      if(m_ownOps){
        ICL_DELETE(m_ops[i]);
      }
      ICL_DELETE(m_derivatives[i]);
    }
    ICL_DELETE(m_preBlurBuffer);
  }
  
  // }}}

  /// no CannyOp::apply without ipp

  void CannyOp::apply (const ImgBase *poSrc, ImgBase **ppoDst)
      // {{{ open
  {

    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( ppoDst );
    ICLASSERT_RETURN( poSrc != *ppoDst);

    if(m_preBlur){
      ConvolutionOp con(ConvolutionKernel(ConvolutionKernel::gauss3x3));
      con.setClipToROI(false);
      con.apply(poSrc,&m_preBlurBuffer);
      poSrc = m_preBlurBuffer;
    }


    for(int i=0;i<2;i++){
      m_ops[i]->setClipToROI (true);
      m_ops[i]->apply(poSrc,&m_derivatives[i]);
    }
    
    if (!prepare (ppoDst, m_derivatives[0], depth8u)) return;

    int minSize=0;
    ippiCannyGetSize(m_derivatives[0]->getSize(), &minSize);
    m_cannyBuf.resize(minSize);
    
    for (int c=m_derivatives[0]->getChannels()-1; c >= 0; --c) {
      switch(m_derivatives[0]->getDepth()){
        case depth32f:
          ippiCanny_32f8u_C1R (m_derivatives[0]->asImg<icl32f>()->getROIData(c), m_derivatives[0]->getLineStep(),
                               m_derivatives[1]->asImg<icl32f>()->getROIData(c), m_derivatives[1]->getLineStep(),
                               (*ppoDst)->asImg<icl8u>()->getROIData(c), (*ppoDst)->getLineStep(),
                               (*ppoDst)->getROISize(),m_lowT,m_highT,m_cannyBuf.data());
          break;
        case depth16s:
          ippiCanny_16s8u_C1R (m_derivatives[0]->asImg<icl16s>()->getROIData(c), m_derivatives[0]->getLineStep(),
                               m_derivatives[1]->asImg<icl16s>()->getROIData(c), m_derivatives[1]->getLineStep(),
                               (*ppoDst)->asImg<icl8u>()->getROIData(c), (*ppoDst)->getLineStep(),
                               (*ppoDst)->getROISize(),m_lowT,m_highT,m_cannyBuf.data());
          break;
        default:
          ICL_INVALID_DEPTH;
      }
    }    
  }
   // }}}
  
  void CannyOp::setThresholds(icl32f lo, icl32f hi){
    // {{{ open

    m_lowT= lo;
    m_highT = hi;
  }

  // }}}
  
  icl32f CannyOp::getLowThreshold()const {
    // {{{ open

    return m_lowT;
  }

  // }}}
  icl32f CannyOp::getHighThreshold()const {
    // {{{ open

    return m_highT;
  }

  // }}}
#endif

  
}
