#include "RegionBasedBlobSearcher.h"
#include "Converter.h"
#include "Img.h"
#include "ImgRegionDetector.h"
#include <math.h>

namespace icl{
  using namespace regiondetector;
  using namespace std;

  namespace{
    Array<Point> &cat(const Array<Array<Point> > &src, Array<Point> &dst){
      // {{{ open
      dst.clear();
      for(Array<Array<Point> >::const_iterator it = src.begin();it!=src.end();++it){
        dst.assign(it->begin(),it->end());
      }
      return dst;
    }
    // }}}
    Array<Rect> &cat(const Array<Array<Rect> > &src, Array<Rect> &dst){
      // {{{ open
      dst.clear();
      for(Array<Array<Rect> >::const_iterator it = src.begin();it!=src.end();++it){
        dst.assign(it->begin(),it->end());
      }
      return dst;
    }
    // }}}
    Array<PCAInfo> &cat(const Array<Array<PCAInfo> > &src, Array<PCAInfo> &dst){
      // {{{ open
      dst.clear();
      for(Array<Array<PCAInfo> >::const_iterator it = src.begin();it!=src.end();++it){
        dst.assign(it->begin(),it->end());
      }
      return dst;
    }
    // }}}

    Array<int> &toPOD(const Array<Array<Point> >&src, Array<int> &dst){
      // {{{ open

    dst.clear();
    for(unsigned int i=0;i<src.size();++i){
      for(unsigned int j=0;j<src[i].size();++j){
        dst.push_back(src[i][j].x);
        dst.push_back(src[i][j].y);
      }
    }   
    return dst;
  }

  // }}}
    
    Array<int> &toPOD(const Array<Array<Rect> > &src, Array<int> &dst){
      // {{{ open

    dst.clear();
    for(unsigned int i=0;i<src.size();++i){
      for(unsigned int j=0;j<src[i].size();++j){
        dst.push_back(src[i][j].x);
        dst.push_back(src[i][j].y);
        dst.push_back(src[i][j].width);
        dst.push_back(src[i][j].height);
      }
    }
    return dst;
  }

  // }}}
    Array<float> &toPOD(const Array<Array<PCAInfo> > &src, Array<float> &dst){
      // {{{ open
      
    dst.clear();
    for(unsigned int i=0;i<src.size();++i){
      for(unsigned int j=0;j<src[i].size();++j){    
        dst.push_back(src[i][j].len1);
        dst.push_back(src[i][j].len2);
        dst.push_back(src[i][j].arc1);
        dst.push_back(src[i][j].arc2);
      }
    }
    return dst;
  }

  // }}}
  }


  
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

  }

  // }}}
  
  const Array<Point> &RegionBasedBlobSearcher::getCenters(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return cat(m_oCenters,m_oCentersOut);
  }

  // }}}

  const Array<Rect> &RegionBasedBlobSearcher::getBoundingBoxes(ImgBase *image){
    // {{{ open
    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return cat(m_oBBs,m_oBBsOut);
  }

  // }}}

  const Array<PCAInfo> &RegionBasedBlobSearcher::getPCAInfo(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return cat(m_oPCAInfos,m_oPCAInfosOut);
  }

  // }}}

  const Array<int> &RegionBasedBlobSearcher::getCentersPOD(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return toPOD(m_oCenters,m_oCentersOutPOD);
  }

  // }}}

  const Array<int> &RegionBasedBlobSearcher::getBoundingBoxesPOD(ImgBase *image){
    // {{{ open 

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return toPOD(m_oBBs,m_oBBsOutPOD);
  }

  // }}}

  const Array<float> &RegionBasedBlobSearcher::getPCAInfoPOD(ImgBase *image){
    // {{{ open

    m_poInputImage = image;
    extractRegions();
    unifyRegions();
    return toPOD(m_oPCAInfos,m_oPCAInfosOutPOD);
  }

  // }}}


  void RegionBasedBlobSearcher::detectAll(ImgBase *image, Array<Point> &centers, Array<Rect> &boundingBoxes, Array<PCAInfo> &pcaInfos){
    // {{{ open
    
    m_poInputImage = image;
    extractRegions();
    unifyRegions();
   
    cat(m_oCenters,centers);
    cat(m_oBBs,boundingBoxes);
    cat(m_oPCAInfos,pcaInfos);
  }
  // }}}
  
  void RegionBasedBlobSearcher::detectAllPOD(ImgBase *image, Array<int> &centers, Array<int> &boundingBoxes, Array<float> &pcaInfos){
    // {{{ open
    m_poInputImage = image;
    extractRegions();
    unifyRegions();
   
    toPOD(m_oCenters,centers);
    toPOD(m_oBBs,boundingBoxes);
    toPOD(m_oPCAInfos,pcaInfos);
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

    m_oCenters.clear();
    m_oBBs.clear();
    m_oPCAInfos.clear();
    
    for(unsigned int i=0;i<m_oFMCreators.size(); ++i){
      FMCreator &fmc = *(m_oFMCreators[i]);
      Img8u *image = getImage(fmc.getSize(),fmc.getFormat());
      Img8u *fm = fmc.getFM(image);
      RegionFilter &rf = *(fmc.getFilter());
      m_poRD->setRestrictions(rf.getMinSize(),rf.getMaxSize(),rf.getMinVal(),rf.getMaxVal());
      float facx =  (float)(m_poInputImage->getSize().width) / (float)(fmc.getSize().width);
      float facy =  (float)(m_poInputImage->getSize().height) / (float)(fmc.getSize().height);
      
      m_oCenters.push_back(Array<Point>());
      m_oBBs.push_back(Array<Rect>());
      m_oPCAInfos.push_back(Array<PCAInfo>());

      const vector<BlobData> &vecBD = m_poRD->detect(fm);
      for(vector<BlobData>::const_iterator it = vecBD.begin();it!= vecBD.end();it++){
        const BlobData &bd = *it;

        if(rf.needSpecialFeatures()){
          Point pos = bd.getCenter();
          if(rf.ok(bd.getVal(),pos)){
            m_oCenters[i].push_back(pos.transform(facx,facy));
          }
        }else{
          Point pos = bd.getCenter();
          Rect bb = bd.getBoundingBox();
          PCAInfo pca = bd.getPCAInfo();
          if(rf.ok(bd.getVal(),pos,bb,pca)){
            m_oCenters[i].push_back(pos.transform(facx,facy));
            m_oBBs[i].push_back(bb.transform(facx,facy));
            m_oPCAInfos[i].push_back(pca);
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
                                                    vector<icl8u> refcolor,
                                                    vector<icl8u> thresholds,
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
    virtual bool ok(icl8u value, const Point &p){
      (void)value; (void)p;
      return true;
    }
    virtual bool ok(icl8u value, const Point &p, const Rect &bb, const PCAInfo &pca){
      (void)value; (void)p; (void)bb; (void)pca;
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
