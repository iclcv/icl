#include "iclRegionBasedBlobSearcher.h"
#include "iclConverter.h"
#include "iclImg.h"
#include "iclRegionDetector.h"
#include <math.h>
#include <limits>
#include <iclRegionFilter.h>
#include <iclFMCreator.h>

namespace icl{
  using namespace std;

  namespace{

    /// struct to use a Size struct as std::map - key
   

    std::vector<Point> &cat(const std::vector<std::vector<Point> > &src, std::vector<Point> &dst){
      // {{{ open
      dst.clear();
      for(std::vector<std::vector<Point> >::const_iterator it = src.begin();it!=src.end();++it){
        for(std::vector<Point>::const_iterator jt = it->begin();jt!= it->end();++jt){
          dst.push_back(*jt);
        }
      }
      return dst;
    }
    // }}}

    std::vector<Point32f> &cat(const std::vector<std::vector<Point32f> > &src, std::vector<Point32f> &dst){
      // {{{ open
      dst.clear();
      for(std::vector<std::vector<Point32f> >::const_iterator it = src.begin();it!=src.end();++it){
        for(std::vector<Point32f>::const_iterator jt = it->begin();jt!= it->end();++jt){
          dst.push_back(*jt);
        }
      }
      return dst;
    }
    // }}}

    std::vector<Rect> &cat(const std::vector<std::vector<Rect> > &src, std::vector<Rect> &dst){
      // {{{ open
      dst.clear();
      for(std::vector<std::vector<Rect> >::const_iterator it = src.begin();it!=src.end();++it){
        for(std::vector<Rect>::const_iterator jt = it->begin();jt!= it->end();++jt){
          dst.push_back(*jt);
        }
      }
      return dst;
    }
    // }}}
    std::vector<RegionPCAInfo> &cat(const std::vector<std::vector<RegionPCAInfo> > &src, std::vector<RegionPCAInfo> &dst){
      // {{{ open
      dst.clear();
      for(std::vector<std::vector<RegionPCAInfo> >::const_iterator it = src.begin();it!=src.end();++it){
        for(std::vector<RegionPCAInfo>::const_iterator jt = it->begin();jt!= it->end();++jt){
          dst.push_back(*jt);
        }
      }
      return dst;
    }
    // }}}

    std::vector<int> &toPOD(const std::vector<std::vector<Point> >&src, std::vector<int> &dst){
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
    std::vector<int> &toPOD(const std::vector<Point> &src, std::vector<int> &dst){
      // {{{ open

    dst.clear();
    for(unsigned int i=0;i<src.size();++i){
      dst.push_back(src[i].x);
      dst.push_back(src[i].y);
    }   
    return dst;
  }

    // }}}   

    std::vector<float> &toPOD(const std::vector<Point32f> &src, std::vector<float> &dst){
      // {{{ open
      
      dst.clear();
      for(unsigned int i=0;i<src.size();++i){
        dst.push_back(src[i].x);
        dst.push_back(src[i].y);
      }   
      return dst;
    }
    
    // }}}       

    std::vector<std::vector<int> > &toPOD(const std::vector<std::vector<Point> >&src, std::vector<std::vector<int> > &dst){
      // {{{ open
      dst.clear();
      for(unsigned int i=0;i<src.size();++i){
        dst.push_back(std::vector<int>());
        for(unsigned int j=0;j<src[i].size();++j){
          dst[i].push_back(src[i][j].x);
          dst[i].push_back(src[i][j].y);
        }
      }   
      return dst;
    }

  // }}}

    std::vector<std::vector<float> > &toPOD(const std::vector<std::vector<Point32f> >&src, std::vector<std::vector<float> > &dst){
      // {{{ open
      dst.clear();
      for(unsigned int i=0;i<src.size();++i){
        dst.push_back(std::vector<float>());
        for(unsigned int j=0;j<src[i].size();++j){
          dst[i].push_back(src[i][j].x);
          dst[i].push_back(src[i][j].y);
        }
      }   
      return dst;
    }

  // }}}


    std::vector<int> &toPOD(const std::vector<std::vector<Rect> > &src, std::vector<int> &dst){
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
    std::vector<float> &toPOD(const std::vector<std::vector<RegionPCAInfo> > &src, std::vector<float> &dst){
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

    m_poRD = new RegionDetector(0,0,0,0);
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
 
  const std::vector<const ImgBase*> RegionBasedBlobSearcher::getFeatureMaps() const{
    // {{{ open

    std::vector<const ImgBase *> v(m_oFMRF.size());
    for(unsigned int i=0;i<m_oFMRF.size();i++){
      v[i] = m_oFMRF[i].fmc->getLastFM();
    }
    return v;
  }

  // }}}

  const std::vector<Point> &RegionBasedBlobSearcher::getCOGs(){
    // {{{ open

    m_oCOGsOut.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      FF &fac = m_oScaleFactors[i];
      Point32f p32 = m_oInternalData[i]->getCOG();
      Point p(p32.x,p32.y);
      Point q = p.transform(fac.f1,fac.f2);
      m_oCOGsOut.push_back(q);
      // m_oCOGsOut.push_back(m_oInternalData[i]->getCOG().transform(fac.f1,fac.f2));
    }
    return m_oCOGsOut; 
  }

  // }}}


  const std::vector<Point32f> &RegionBasedBlobSearcher::getCOGsFloat(){
    // {{{ open

    m_oCOGsFloatOut.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      FF &fac = m_oScaleFactors[i];
      Point32f p =  m_oInternalData[i]->getCOG();
      Point32f q = p.transform(fac.f1,fac.f2);
      m_oCOGsFloatOut.push_back(q);
      // m_oCOGsOut.push_back(m_oInternalData[i]->getCOG().transform(fac.f1,fac.f2));
    }
    return m_oCOGsFloatOut; 
  }

  // }}}


  const std::vector<Rect> &RegionBasedBlobSearcher::getBoundingBoxes(){
    // {{{ open

    m_oBBsOut.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      FF &fac = m_oScaleFactors[i];
      m_oBBsOut.push_back(m_oInternalData[i]->getBoundingBox().transform(fac.f1,fac.f2));
    }
    return m_oBBsOut; 
  }

  // }}}
  const std::vector<RegionPCAInfo> &RegionBasedBlobSearcher::getPCAInfo(){
    // {{{ open

    m_oPCAInfosOut.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      FF &fac = m_oScaleFactors[i];
      RegionPCAInfo pcaInfo = m_oInternalData[i]->getPCAInfo();
      //      pcaInfo.len1 = .. TODO adapt to factor
      (void)fac;
      m_oPCAInfosOut.push_back(pcaInfo);
    }
    return m_oPCAInfosOut; 
  }

  // }}}

  
  const std::vector<std::vector<Point> > &RegionBasedBlobSearcher::getBoundaries(){
    // {{{ open

    m_oBoundariesOut.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      RegionBasedBlobSearcher::FF &fac = m_oScaleFactors[i];
      const vector<Point> &b = m_oInternalData[i]->getBoundary();
      m_oBoundariesOut.push_back(std::vector<Point>());
      std::vector<Point> &l = m_oBoundariesOut[i];
      for(unsigned int j=0;j<b.size();++j){
        l.push_back(b[j].transform(fac.f1,fac.f2));
      }
    }
    return m_oBoundariesOut;
    
  }

  // }}}
  
  const std::vector<int> &RegionBasedBlobSearcher::getBoundaryLengthsPOD(){
    // {{{ open

    m_oBoundaryLengthsPOD.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      m_oBoundaryLengthsPOD.push_back(m_oInternalData[i]->getBoundaryLength());
    }
    return m_oBoundaryLengthsPOD; 
  }

  // }}}
  
  const std::vector<float> &RegionBasedBlobSearcher::getFormFactorsPOD(){
    // {{{ open

    m_oFormFactorsPOD.clear();
    for(unsigned int i=0;i<m_oInternalData.size();++i){
      m_oFormFactorsPOD.push_back(m_oInternalData[i]->getFormFactor());
    }
    return m_oFormFactorsPOD; 
  }

  // }}}
  
  const std::vector<Region*> &RegionBasedBlobSearcher::getRegions(){
    // {{{ open

    return m_oInternalData;
  }

  // }}}
  const std::vector<int> &RegionBasedBlobSearcher::getCOGsPOD(){
    // {{{ open
		getCOGs();
    return toPOD(m_oCOGsOut,m_oCOGsOutPOD);
  }

  // }}}

  const std::vector<float> &RegionBasedBlobSearcher::getCOGsFloatPOD(){
    // {{{ open
    getCOGsFloat();
    return toPOD(m_oCOGsFloatOut,m_oCOGsFloatOutPOD);
  }

  // }}}

  const std::vector<int> &RegionBasedBlobSearcher::getBoundingBoxesPOD(){
    // {{{ open 
    getBoundingBoxes();
    return toPOD(m_oBBs,m_oBBsOutPOD);
  }

  // }}}
  const std::vector<float> &RegionBasedBlobSearcher::getPCAInfoPOD(){
    // {{{ open
    getPCAInfo();
    return toPOD(m_oPCAInfos,m_oPCAInfosOutPOD);
  }

  // }}}

  const std::vector<std::vector<int> > &RegionBasedBlobSearcher::getBoundariesPOD(){
    // {{{ open
    getBoundaries();
    return toPOD(getBoundaries(),m_oBoundariesPOD);
  }

  // }}}




  
  Img8u *RegionBasedBlobSearcher::getImage(const Size &size, format fmt, const ImgBase *inputImage){
    // {{{ open

    //map<Size,map<format,Img8u*> > m_mmImages;
    sizemap &m = m_mmImages;
    sizemap::iterator i = m.find(size);
    if(i != m.end()){
      fmtmap::iterator j = (*i).second.find(fmt);
      if(j != (*i).second.end()){
        // this does only work if the images have valid timestanps
        // if(m_poInputImage->getTime() != (*j).second->getTime()){
        m_poConverter->apply(inputImage,(*j).second);
        //}
        return (*j).second;
      }else{
        Img8u *image = new Img8u(size,fmt);
        m_poConverter->apply(inputImage,image);
        ((*i).second)[fmt] = image;
        return image;
      }
    }else{
      Img8u *image = new Img8u(size,fmt); 
      m_poConverter->apply(inputImage,image);
      (m[size])[fmt]=image;
      return image;
    }
    ERROR_LOG("unknown error!");    
  }       

  // }}}

  void RegionBasedBlobSearcher::extractRegions(const ImgBase *image){
    // {{{ open

    m_oInternalData.clear(); // std::vector<BlobData*>
    m_oScaleFactors.clear(); // std::vector<FF> >  
    const Size &ims = image->getSize();
    
    for(unsigned int i=0;i<m_oFMRF.size(); ++i){
      FMCreator &fmc = *(m_oFMRF[i].fmc);
      RegionFilter &rf = *(m_oFMRF[i].rf);
      
      Range<unsigned int> sizeRange(rf.getSizeRange().minVal, rf.getSizeRange().maxVal);
      m_poRD->setRestrictions(sizeRange,rf.getValueRange().castTo<icl64f>());
      FF factor( (float)(ims.width)/fmc.getSize().width,(float)(ims.height)/fmc.getSize().height);
      const vector<Region> &vecBD = m_poRD->detect(fmc.getFM(getImage(fmc.getSize(),fmc.getFormat(),image)));
      for(unsigned int i=0;i<vecBD.size();++i){
        if(rf.validate(vecBD[i])){
          m_oInternalData.push_back(const_cast<Region*>(&(vecBD[i])));        
          m_oScaleFactors.push_back(factor);
        }        
      }
    }
  }

  // }}}

  /******************************************************************************
      void RegionBasedBlobSearcher::extractRegions(){
      m_oCenters.clear();
      m_oBBs.clear();
      m_oRegionPCAInfos.clear();
      
      for(unsigned int i=0;i<m_oFMCreators.size(); ++i){
      FMCreator &fmc = *(m_oFMCreators[i]);
      Img8u *image = getImage(fmc.getSize(),fmc.getFormat());
        Img8u *fm = fmc.getFM(image);
      RegionFilter &rf = *(fmc.getFilter());
      m_poRD->setRestrictions(rf.getMinSize(),rf.getMaxSize(),rf.getMinVal(),rf.getMaxVal());
      float facx =  (float)(m_poInputImage->getSize().width) / (float)(fmc.getSize().width);
      float facy =  (float)(m_poInputImage->getSize().height) / (float)(fmc.getSize().height);
      
      m_oCenters.push_back(std::vector<Point>());
      m_oBBs.push_back(std::vector<Rect>());
      m_oPCAInfos.push_back(std::vector<PCAInfo>());
      
      const vector<BlobData> &vecBD = m_poRD->detect(fm);
      for(vector<BlobData>::const_iterator it = vecBD.begin();it!= vecBD.end();it++){
      const BlobData &bd = *it;
      
      if(!rf.needSpecialFeatures()){
      Point pos = bd.getCOG();
      if(rf.ok(bd.getVal(),pos)){
      m_oCenters[i].push_back(pos.transform(facx,facy));
      }
      }else{
      Point pos = bd.getCOG();
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
  ****************************************************************************/

  void RegionBasedBlobSearcher::add(FMCreator* fmc, RegionFilter *rf){
    // {{{ open

    m_oFMRF.push_back(Plugin(fmc,rf));
  }

  // }}}

  void RegionBasedBlobSearcher::remove(FMCreator *fmc, bool release){
    // {{{ open
    for(unsigned int i=0;i<m_oFMRF.size();++i){
      if(m_oFMRF[i].fmc == fmc){
        if(release){
          delete m_oFMRF[i].fmc;
          delete m_oFMRF[i].rf;
        }
        m_oFMRF.erase(m_oFMRF.begin()+i);
        return;
      }
    }
  }
  // }}}
  
  void RegionBasedBlobSearcher::remove(RegionFilter *rf, bool release){
    // {{{ open
    for(unsigned int i=0;i<m_oFMRF.size();++i){
      if(m_oFMRF[i].rf == rf){
        if(release){
          delete m_oFMRF[i].fmc;
          delete m_oFMRF[i].rf;
        }
        m_oFMRF.erase(m_oFMRF.begin()+i);
        return;
      }
    }
  }
  
  // }}}


  RegionBasedBlobSearcher::Plugin RegionBasedBlobSearcher::getPlugin(FMCreator *fmc){
    // {{{ open

    for(unsigned int i=0;i<m_oFMRF.size();++i){
      if(m_oFMRF[i].fmc == fmc){
        return m_oFMRF[i];
      }
    }
    return Plugin();  
  }

  // }}}
  
  RegionBasedBlobSearcher::Plugin RegionBasedBlobSearcher::getPlugin(RegionFilter *rf){
    // {{{ open

    for(unsigned int i=0;i<m_oFMRF.size();++i){
      if(m_oFMRF[i].rf == rf){
        return m_oFMRF[i];
      }
    }
    return Plugin();  
  }

  // }}}

  void RegionBasedBlobSearcher::removeAll(bool release){
    // {{{ open
    if(release){
      for(unsigned int i=0;i<m_oFMRF.size();++i){
        delete m_oFMRF[i].fmc;
        delete m_oFMRF[i].rf;
      }
    }
    m_oFMRF.clear();
  }
  
  // }}}

}
