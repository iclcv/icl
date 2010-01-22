#include <ICLBlob/FMCreator.h>
#include <ICLCore/Img.h>
#include <vector>

using namespace std;

namespace icl{
  
  struct DefaultFMCreator : public FMCreator{
    // {{{ open
    DefaultFMCreator(const Size &size, 
                     format fmt,
                     vector<icl8u> refcolor,
                     vector<icl8u> thresholds){
      
      
      m_oSize = size;
      m_eFormat = fmt;
      m_vecRefColor = refcolor;
      m_vecThresholds = thresholds;
      m_poFM = new Img8u(size,formatMatrix);
      ICLASSERT_RETURN(refcolor.size() == thresholds.size());
      ICLASSERT_RETURN(refcolor.size() > 0);
    }
    virtual ~DefaultFMCreator(){
      delete m_poFM;
    }
     
    Size m_oSize;
    format m_eFormat;
    vector<icl8u> m_vecRefColor;
    vector<icl8u> m_vecThresholds;
    Img8u *m_poFM;
    virtual const ImgBase *getLastFM() const{ 
      return m_poFM;
    }
    virtual Size getSize(){ return m_oSize; }
    virtual format getFormat(){ return m_eFormat; }
    virtual Img8u* getFM(Img8u *image){
      ICLASSERT_RETURN_VAL(image && m_poFM , 0);
      ICLASSERT_RETURN_VAL(image->getChannels() == (int)m_vecRefColor.size() , 0);
      ICLASSERT_RETURN_VAL(image->getSize() == m_poFM->getSize(), 0);
      ICLASSERT_RETURN_VAL(image->getSize() == m_oSize, 0);
      Img8u src = *image;
      Img8u dst = *m_poFM;
       
      int nc = image->getChannels();
      icl8u *pucDst = dst.getData(0);
       
      // 1 channel
      int t0 = m_vecThresholds[0];
      int r0 = m_vecRefColor[0];
      icl8u *pucSrc0 = src.getData(0);
      icl8u *pucSrc0End = src.getData(0)+m_oSize.getDim();
      if(nc == 1){
        for(;pucSrc0!=pucSrc0End;++pucSrc0,++pucDst){
          *pucDst = 255 * (abs(*pucSrc0-r0)<t0);
        }
        return m_poFM;
      }
     
      // 2 channels
      int t1 = m_vecThresholds[1];
      int r1 = m_vecRefColor[1];
      icl8u *pucSrc1 = src.getData(1);
      if(nc == 2){
        for(;pucSrc0!=pucSrc0End;++pucSrc0,++pucSrc1,++pucDst){
          *pucDst = 255 * ( (abs(*pucSrc0-r0)<t0) & (abs(*pucSrc1-r1)<t1) );
        }
        return m_poFM;
      }
     
      // 3 channels
      int t2 = m_vecThresholds[2];
      int r2 = m_vecRefColor[2];
      icl8u *pucSrc2 = src.getData(2);
      if(nc == 3){
        //        printf("nd = 3 ref=%d %d %d  thresh=%d %d %d\n",r0,r1,r2,t0,t1,t2);
        for(;pucSrc0!=pucSrc0End;++pucSrc0,++pucSrc1,++pucSrc2,++pucDst){
          *pucDst = 255 * ( (abs(*pucSrc0-r0)<t0) & (abs(*pucSrc1-r1)<t1) & (abs(*pucSrc2-r2)<t2) );
        }
        return m_poFM;
      }


      // n-channel version
      vector<icl8u*> vecSrc(nc);
      for(int i=0;i<nc;i++){
        vecSrc[i]=image->getData(i);
      }
      
      for(int c=0;pucSrc0!=pucSrc0End;++pucDst,++pucSrc0){
        *pucDst = 255 * (abs(*pucSrc0-r0)<t0);
        for(c=1;c<nc;++c){
          *pucDst &= 255 * ( abs(*(vecSrc[c])++ - m_vecRefColor[c]) < m_vecThresholds[c] );
        }
      }
      return m_poFM;
    }
  };

  // }}}


  FMCreator *FMCreator::getDefaultFMCreator(const Size &size, 
                                            format fmt,
                                            vector<icl8u> refcolor,
                                            vector<icl8u> thresholds){
    // {{{ open
    return new DefaultFMCreator(size,fmt,refcolor,thresholds);
  }
  // }}}



}
