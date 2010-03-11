/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLFilter/LUTOp3Channel.h>
#include <ICLUtils/Exception.h>
#include <math.h>

namespace icl{  

  template<class T> 
  LUTOp3Channel<T>::LUTOp3Channel(Plugin *p,icl8u shift):m_poPlugin(0),m_ucShift(shift){
    ICLASSERT_THROW(shift<8,ICLException("invalid shift value ( must be < 8 )"));
    int dim = (int)pow(256>>m_ucShift,3);
    m_oLUT = Img<T>(Size(dim,1),1);
    setPlugin(p);
  }
  
  template<class T>
  LUTOp3Channel<T>::~LUTOp3Channel(){
    setPlugin(0);
  }

  namespace{
    
  
    
#define CAST(x) clipped_cast<srcT,icl8u>(x)
    template<class dstT, class srcT>
    void apply_lut_op_3(const Img<srcT> &src, Img<dstT> &dst, dstT *lut, icl8u shift){
      const unsigned int fac1 = 256>>shift;
      const unsigned int fac2 = fac1*fac1;
      if(src.hasFullROI() && dst.hasFullROI()){
        const srcT * pSrcR = src.getData(0); 
        const srcT * pSrcG = src.getData(1); 
        const srcT * pSrcB = src.getData(2);
        dstT *pDst = dst.getData(0);
        dstT *pDstEnd = pDst+dst.getDim();

        
        if(shift){
          while(pDst < pDstEnd){
            *pDst++  = lut[ (CAST(*pSrcR++)>>shift) + (fac1*(CAST(*pSrcG++)>>shift)) + (fac2*(CAST(*pSrcB++)>>shift)) ];
          }
        }else{
          while(pDst < pDstEnd){
            *pDst++  = lut[ CAST(*pSrcR++) + 256*CAST(*pSrcG++) + 65536*CAST(*pSrcB++) ];
          }
        }

      }else{
        const ImgIterator<srcT> itSrcR= src.beginROI(0);        
        const ImgIterator<srcT> itSrcG= src.beginROI(1);        
        const ImgIterator<srcT> itSrcB= src.beginROI(2);        
        
        ImgIterator<dstT> itDst = dst.beginROI(0);
        const ImgIterator<dstT> itDstEnd = dst.endROI(0);
        if(shift){
          while(itDst != itDstEnd){
            *itDst++  = lut[ (CAST(*itSrcR++)>>shift) + (fac1*(CAST(*itSrcG++)>>shift)) + (fac2*(CAST(*itSrcB++)>>shift)) ];
            
          }        
        }else{
          while(itDst != itDstEnd){
            *itDst++  = lut[ CAST(*itSrcR++) + 256*CAST(*itSrcG++) + 65536*CAST(*itSrcB++) ];
          }        
        }
      }
    }
#undef CAST
  }

  template<class T>
  void LUTOp3Channel<T>::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    ICLASSERT_RETURN(src != *dst);
    ICLASSERT_RETURN(src->getChannels() == 3);
    if(!prepare(dst,getDepth<T>(), src->getSize(),formatMatrix,1, src->getROI(),src->getTime())){
      ERROR_LOG("unable to prepare output image");
    }
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_lut_op_3(*(src->asImg<icl##D>()), *((*dst)->asImg<T>()), m_oLUT.getData(0), m_ucShift); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }
  
  template<class T>
  void LUTOp3Channel<T>::setPlugin(Plugin *p){
    if(p){
      if(m_poPlugin) delete m_poPlugin;
      m_poPlugin = p;
      T *lut = m_oLUT.getData(0);

      if(m_ucShift){
        const unsigned int fac1 = 256>>m_ucShift;
        const unsigned int fac2 = fac1*fac1;
        const unsigned int inc = pow(2,m_ucShift);// ? 2<<(m_ucShift-1) : 1;
        for(int r = 0; r < 256; r+=inc){
          for(int g = 0; g < 256; g+=inc){
            for(int b = 0; b < 256; b+=inc){
              lut[(r>>m_ucShift) + (fac1*(g>>m_ucShift)) + (fac2*(b>>m_ucShift)) ] = p->transform( r,g,b );
            }
          }
        } 
      }else{
        for(int r = 0; r < 256; ++r){
          for(int g = 0; g < 256; ++g){
            for(int b = 0; b < 256; ++b){
              lut[r+ 256*g + 65536*b] = p->transform( r,g,b );
            }
          }
        } 
      }
    }else{
      if(m_poPlugin){
        delete m_poPlugin;
        m_poPlugin = 0;
      }        
    }
  }

#define ICL_INSTANTIATE_DEPTH(D) template class LUTOp3Channel<icl##D>;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
} // namespace icl

