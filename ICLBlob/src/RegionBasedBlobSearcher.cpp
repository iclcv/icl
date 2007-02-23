#include "RegionBasedBlobSearcher.h"
#include "Converter.h"
#include "Img.h"
#include "ImgRegionDetector.h"
#include <math.h>

namespace icl{
  using namespace regiondetector;
  using namespace std;

  RegionBasedBlobSearcher::RegionBasedBlobSearcher(){
    // {{{ open

    m_poRD = new ImgRegionDetector(0,0,0,0);
    m_poInputImage = 0;
    m_poConverter = new Converter;
  }

  // }}}
 
  RegionBasedBlobSearcher::~RegionBasedBlobSearcher(){
    // {{{ open
    removeAll();
    delete m_poRD;

    /// delete all images !!!
    //map<Size,map<format,Img8u*> > m_mmImages;
    /*
        sizemap &m = m_mmImages;
        sizemap::iterator i = m.find(size);
        if(i != m.end()){
        fmtmap::iterator j = (*i).second.find(fmt);
    */

  }

  // }}}
  
  const Array<int> &RegionBasedBlobSearcher::getCenters(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    m_oOutputBuffer.clear();
    for(unsigned int i=0;i<m_oPoints.size();++i){
      for(unsigned int j=0;j<m_oPoints[i].size();++j){
        m_oOutputBuffer.push_back(m_oPoints[i][j].x);
        m_oOutputBuffer.push_back(m_oPoints[i][j].y);
      }
    }
    return m_oOutputBuffer;
  }

  // }}}

  const Array<int> &RegionBasedBlobSearcher::getBoundingBoxes(ImgBase *image){
    // {{{ open
    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    m_oOutputBuffer.clear();
    for(unsigned int i=0;i<m_oRects.size();++i){
      for(unsigned int j=0;j<m_oRects[i].size();++j){
        m_oOutputBuffer.push_back(m_oRects[i][j].x);
        m_oOutputBuffer.push_back(m_oRects[i][j].y);
        m_oOutputBuffer.push_back(m_oRects[i][j].width);
        m_oOutputBuffer.push_back(m_oRects[i][j].height);        
      }
    }
    return m_oOutputBuffer;
  }

  // }}}

  const Array<float> &RegionBasedBlobSearcher::getPCAInfo(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    m_oOutputBufferF.clear();
    for(unsigned int i=0;i<m_oPCA.size();++i){
      for(unsigned int j=0;j<m_oPCA[i].size();++j){
        m_oOutputBufferF.push_back(m_oPCA[i][j]);
      }
    }
    return m_oOutputBufferF;
  }

  // }}}

  void RegionBasedBlobSearcher::detectAll(ImgBase *image, Array<int> &centers, Array<int> &boundingBoxes, Array<float> &pcaInfos){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
   
    centers.clear();
    boundingBoxes.clear();
    pcaInfos.clear();
    
    for(unsigned int i=0;i<m_oPoints.size();++i){
      for(unsigned int j=0;j<m_oPoints[i].size();++j){
        centers.push_back(m_oPoints[i][j].x);
        centers.push_back(m_oPoints[i][j].y);

        boundingBoxes.push_back(m_oRects[i][j].x);
        boundingBoxes.push_back(m_oRects[i][j].y);
        boundingBoxes.push_back(m_oRects[i][j].width);
        boundingBoxes.push_back(m_oRects[i][j].height);   
      }
      for(unsigned int j=0;j<m_oPCA[i].size();++j){
        pcaInfos.push_back(m_oPCA[i][j]);
      }
    }
  }

  // }}}
  
  Img8u *RegionBasedBlobSearcher::getImage(const Size &size, format fmt){
    // {{{ open

    //map<Size,map<format,Img8u*> > m_mmImages;
    sizemap &m = m_mmImages;
    sizemap::iterator i = m.find(size);
    if(i != m.end()){
      fmtmap::iterator j = (*i).second.find(fmt);
      if(j != (*i).second.end()){
        // this does only work if the images have valid timestanps
        // if(m_poInputImage->getTime() != (*j).second->getTime()){
        m_poConverter->apply(m_poInputImage,(*j).second);
        //}
        return (*j).second;
      }else{
        Img8u *image = new Img8u(size,fmt);
        m_poConverter->apply(m_poInputImage,image);
        ((*i).second)[fmt] = image;
        return image;
      }
    }else{
      Img8u *image = new Img8u(size,fmt); 
      m_poConverter->apply(m_poInputImage,image);
      (m[size])[fmt]=image;
      return image;
    }
    ERROR_LOG("unknown error!");    
  }       

  // }}}
  
  void RegionBasedBlobSearcher::extractRegions(){
    // {{{ open

    Array<int> cs, bbs;
    Array<icl8u> vs;
    Array<icl32f> pca;
    
    m_oPoints.clear();
    m_oRects.clear();
    int x,y;
    
    for(unsigned int i=0;i<m_oFMCreators.size(); ++i){
      FMCreator &fmc = *(m_oFMCreators[i]);
      Img8u *image = getImage(fmc.getSize(),fmc.getFormat());
      Img8u *fm = fmc.getFM(image);
      RegionFilter &rf = *(fmc.getFilter());
      m_poRD->setRestrictions(rf.getMinSize(),rf.getMaxSize(),rf.getMinVal(),rf.getMaxVal());
      float facx =  (float)(m_poInputImage->getSize().width) / (float)(fmc.getSize().width);
      float facy =  (float)(m_poInputImage->getSize().height) / (float)(fmc.getSize().height);
      if(rf.needSpecialFeatures()){
        m_poRD->detect(fm,cs,vs,bbs,pca);
        m_oPoints.push_back(Array<Point>(vs.size()));
        m_oRects.push_back(Array<Rect>(vs.size()));
        m_oPCA.push_back(pca);
        for(unsigned int j=0;j<vs.size();++j){
          x = cs[2*j];
          y = cs[2*j+1];
          if(rf.ok(vs[j],x,y,&(bbs[4*j]),&(pca[4*j]))){
            m_oPoints[i][j]=Point((int)(facx*x),(int)(facy*y));
            m_oRects[i][j]=Rect((int)(facx*bbs[4*j]),
                                (int)(facy*bbs[4*j+1]),
                                (int)(facx*bbs[4*j+2]),
                                (int)(facy*bbs[4*j+3]));
          }       
        }
      }else{
        m_poRD->detect(fm,cs,vs);
        m_oPoints.push_back(Array<Point>(vs.size()));
        m_oRects.push_back(Array<Rect>());
        for(unsigned int j=0;j<vs.size();++j){
          x = cs[2*j];
          y = cs[2*j+1];
          if(rf.ok(vs[j],x,y)){
            m_oPoints[i][j]=Point((int)(facx*x),(int)(facy*y));
          }       
        }
      }      
    }
  }

  // }}}
    
  void RegionBasedBlobSearcher::unifyRegions(){
    // {{{ open
    // do nothing
  }

  // }}}
  
  void RegionBasedBlobSearcher::add(FMCreator *fmc){
    // {{{ open
    m_oFMCreators.push_back(fmc);
  }

  // }}}
  void RegionBasedBlobSearcher::remove(FMCreator *fmc){
    // {{{ open
    for(unsigned int i=0;i<m_oFMCreators.size();++i){
      if(m_oFMCreators[i] == fmc){
        m_oFMCreators.erase(m_oFMCreators.begin()+i);
        return;
      }
    }
  }
  
  // }}}
    
  void RegionBasedBlobSearcher::removeAll(){
    // {{{ open
    for(unsigned int i=0;i<m_oFMCreators.size();++i){
      delete m_oFMCreators[i];
    }
    m_oFMCreators.clear();
  }
  
  // }}}


  void RegionBasedBlobSearcher::addDefaultFMCreator(const Size &imageSize,
                                                    // {{{ open

                                                    format imageFormat,
                                                    std::vector<icl8u> refcolor,
                                                    std::vector<icl8u> thresholds,
                                                    unsigned int minBlobSize,
                                                    unsigned int maxBlobSize,
                                                    bool enableSpecialFeatures ){
    
    RegionFilter *rf = RegionFilter::getDefaultRegionFilter(minBlobSize,
                                                            maxBlobSize,
                                                            255,
                                                            255,
                                                            enableSpecialFeatures);
    add(FMCreator::getDefaultFMCreator(imageSize,imageFormat,refcolor,thresholds,rf));                                      
  }  

  // }}}


  
  struct DefaultRegionFilter : public RegionFilter{
    // {{{ open

    DefaultRegionFilter(unsigned int minSize, unsigned int maxSize,
                        icl8u minVal, icl8u maxVal, bool specialFeatures){
      m_uiMinSize = minSize;
      m_uiMaxSize = maxSize;
      m_ucMinVal = minVal;
      m_ucMaxVal = maxVal;
      m_bSF = specialFeatures;
    }
    unsigned int m_uiMinSize;
    unsigned int m_uiMaxSize;
    icl8u m_ucMinVal;
    icl8u m_ucMaxVal;
    bool m_bSF;
    
    virtual ~DefaultRegionFilter(){}
    virtual unsigned int getMinSize(){ return m_uiMinSize; }
    virtual unsigned int getMaxSize(){ return m_uiMaxSize; }
    virtual icl8u getMinVal(){ return m_ucMinVal; }
    virtual icl8u getMaxVal(){ return m_ucMaxVal; }
    virtual bool needSpecialFeatures(){ return m_bSF; }
    virtual bool ok(icl8u value, int x, int y){
      (void)value; (void)x; (void)y; 
      return true;
    }
    virtual bool ok(icl8u value, int x, int y, int *bb, icl32f *pca){
      (void)value; (void)x; (void)y; (void)bb; (void)pca;
      return true;
    }
  };

  // }}}
  
  RegionFilter* RegionFilter::getDefaultRegionFilter(unsigned int minSize, 
                                                     // {{{ open

                                                     unsigned int maxSize,
                                                     icl8u minVal, 
                                                     icl8u maxVal, 
                                                     bool specialFeaturesEnabled){
    return new DefaultRegionFilter(minSize,maxSize,minVal, maxVal, specialFeaturesEnabled);
  }

  // }}}
  
  struct DefaultFMCreator : public FMCreator{
    // {{{ open
    DefaultFMCreator(const Size &size, 
                     format fmt,
                     vector<icl8u> refcolor,
                     vector<icl8u> thresholds,
                     RegionFilter *rf){

      
      m_oSize = size;
      m_eFormat = fmt;
      m_vecRefColor = refcolor;
      m_vecThresholds = thresholds;
      m_poRF = rf;   
      m_poFM = new Img8u(size,formatMatrix);
      ICLASSERT_RETURN(refcolor.size() == thresholds.size());
      ICLASSERT_RETURN(refcolor.size() > 0);
    }
    virtual ~DefaultFMCreator(){
      if(m_poRF) delete m_poRF;
      delete m_poFM;
    }
    
    Size m_oSize;
    format m_eFormat;
    vector<icl8u> m_vecRefColor;
    vector<icl8u> m_vecThresholds;
    RegionFilter *m_poRF;
    Img8u *m_poFM;
    
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
          *pucDst = 255 * (std::abs(*pucSrc0-r0)<t0);
        }
        return m_poFM;
      }
     
      // 2 channels
      int t1 = m_vecThresholds[1];
      int r1 = m_vecRefColor[1];
      icl8u *pucSrc1 = src.getData(1);
      if(nc == 2){
        for(;pucSrc0!=pucSrc0End;++pucSrc0,++pucSrc1,++pucDst){
          *pucDst = 255 * ( (std::abs(*pucSrc0-r0)<t0) & (std::abs(*pucSrc1-r1)<t1) );
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
          *pucDst = 255 * ( (std::abs(*pucSrc0-r0)<t0) & (std::abs(*pucSrc1-r1)<t1) & (std::abs(*pucSrc2-r2)<t2) );
        }
        return m_poFM;
      }


      // n-channel version
      std::vector<icl8u*> vecSrc(nc);
      for(int i=0;i<nc;i++){
        vecSrc[i]=image->getData(i);
      }
      
      for(int c=0;pucSrc0!=pucSrc0End;++pucDst,++pucSrc0){
        *pucDst = 255 * (std::abs(*pucSrc0-r0)<t0);
        for(c=1;c<nc;++c){
          *pucDst &= 255 * ( std::abs(*(vecSrc[c])++ - m_vecRefColor[c]) < m_vecThresholds[c] );
        }
      }
      return m_poFM;
    }
    virtual RegionFilter *getFilter(){
      return m_poRF;
    }
  };

  // }}}
  
  FMCreator *FMCreator::getDefaultFMCreator(const Size &size, 
                                            // {{{ open

                                 format fmt,
                                 vector<icl8u> refcolor,
                                 vector<icl8u> thresholds,
                                 RegionFilter *rf){
    return new DefaultFMCreator(size,fmt,refcolor,thresholds,rf);
    
  }

  // }}}

  
}

