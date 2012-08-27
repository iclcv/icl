/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLFilter/CannyOp.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ConvolutionOp.h>

namespace icl {
  namespace filter{
  
  #ifdef HAVE_IPP
    // without ipp non of the function is implemented
    CannyOp::CannyOp(icl32f lowThresh, icl32f highThresh,int preBlurRadius):
      // {{{ open
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(true),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3));
      m_ops[1] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3));
      m_derivatives[0]=m_derivatives[1]=0;
      m_preBlurOp = 0;
      setUpPreBlurOp();
    }
  
    // }}}
    
    CannyOp::CannyOp(UnaryOp *dxOp, UnaryOp *dyOp,icl32f lowThresh, icl32f highThresh, bool deleteOps, int preBlurRadius):
      // {{{ open
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(deleteOps),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = dxOp;
      m_ops[1] = dyOp;
  
      m_derivatives[0]=m_derivatives[1]=0;
  
      m_preBlurOp = 0;
      setUpPreBlurOp();
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
      ICL_DELETE(m_preBlurOp);
    }
    
    // }}}
  
    void CannyOp::setUpPreBlurOp(){
      int r = m_preBlurRadius;
      if(r <=0) return;
      switch(r){
        case 1: m_preBlurOp = new ConvolutionOp(ConvolutionKernel::gauss3x3); break;
        case 2: m_preBlurOp = new ConvolutionOp(ConvolutionKernel::gauss5x5); break;
        default:
          WARNING_LOG("higher pre blur radii than 2 are not yet supported correctly ...");
          Size s(2*r+1,2*r+1);
          Img32s k(s,1);
          Channel32s kc = k[0];
          int sum;
          for(int y=-r;y<=r;++y){
            for(int x=-r;x<=r;++x){
              kc(x+r,y+r) = 10000/(2*M_PI*r) * exp (float(x*x+y*y)/float(2*r*r));
              sum += kc(x+r,y+r);
            }
          }
          m_preBlurOp = new ConvolutionOp(ConvolutionKernel(k.begin(0),s,10000));
      }
    }
  
    /// no CannyOp::apply without ipp
  
    void CannyOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
        // {{{ open
      FUNCTION_LOG("");
      ICLASSERT_RETURN( poSrc );
      ICLASSERT_RETURN( ppoDst );
      ICLASSERT_RETURN( poSrc != *ppoDst);
  
      if(m_preBlurRadius>0){
        poSrc = m_preBlurOp->apply(poSrc);
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
  
    
  } // namespace filter
}
