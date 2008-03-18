#include <iclRegionDetector.h>
#include <iclImgChannel.h>
#include <algorithm>

namespace icl{
  
  namespace{
    template<class T>
    static inline bool eqfunc(const T &a,const T&b){
      return a == b;
    }
  }
  
  RegionDetector::RegionDetector(unsigned int minSize, unsigned int maxSize, icl64f minVal, icl64f maxVal):
    m_uiMinSize(minSize),m_uiMaxSize(maxSize),m_dMinVal(minVal),m_dMaxVal(maxVal){
  }
  void RegionDetector::setRestrictions(unsigned int minSize, unsigned int maxSize, icl64f minVal, icl64f maxVal){
    m_uiMinSize = minSize;
    m_uiMaxSize = maxSize;
    m_dMinVal = minVal;
    m_dMaxVal = maxVal;
  }
  void RegionDetector::setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl64f> &valueRange){
    setRestrictions(sizeRange.minVal,sizeRange.maxVal,valueRange.minVal, valueRange.maxVal);
  }
  
  const std::vector<Region> &RegionDetector::detect(const ImgBase *image){
    m_vecBlobData.clear();
    ICLASSERT_RETURN_VAL(image,m_vecBlobData);
    ICLASSERT_RETURN_VAL(image->getChannels()==1,m_vecBlobData);
    ICLASSERT_RETURN_VAL(image->getROISize().getDim(),m_vecBlobData);
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: detect_intern(*image->asImg<icl##D>(),eqfunc<icl##D>); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    return m_vecBlobData;
  }

  void RegionDetector::adaptLIM(int w, int h){
    if((int)m_vecLIM.size() < w*h){
      m_vecLIM.resize(w*h);
    }
  }
  void RegionDetector::resetPartList(){
    std::for_each(m_vecParts.begin(),m_vecParts.end(),RegionPart::del_func);
    m_vecParts.clear();    
  }
  inline RegionPart *RegionDetector::newPart(){
    RegionPart *p = new RegionPart;
    m_vecParts.push_back(p);
    return p;
  }

  namespace{
    template<class T>
    class ImgChannelROI{
    public:
      inline ImgChannelROI(const Img<T> &image, int channel):
        chan(pickChannel(&image,channel)),xOffs(image.getROI().x),yOffs(image.getROI().y){
      }
      inline const T &operator()(int x, int y) const{
        return chan(x+xOffs,y+yOffs);
      }
      
    private:
      const ImgChannel<T> chan;
      int xOffs;
      int yOffs;
    };
    
    template<class T> struct EQFunctor{
      EQFunctor(const T &ref):ref(ref){}
      T ref;
      inline bool operator()(const T &val) const{ return ref == val; }
    };
  }
  
  template<class T, typename Compare>
  void RegionDetector::detect_intern(const Img<T> &image, Compare cmp){
    const int xOffs = image.getROI().x;
    const int yOffs = image.getROI().y;
    const int w = image.getROI().width;
    const int h = image.getROI().height;

    adaptLIM(w,h);
    resetPartList();
    RegionPart **lim = &m_vecLIM[0];
    ImgChannelROI<T> data(image,0);

    
    // first pixel:
    lim[0] = newPart();
    
    // first line
    register int slX = 0;
    for(int x=1;x<w;++x){
      if( cmp( data(x,0), data(x-1,0))){
        lim[x] = lim[x-1];
      }else{
        lim[x-1]->add(ScanLine(slX+xOffs,yOffs,x-slX));
        slX = x;
        lim[x] = newPart();
      }
    }
    // first line end
    lim[w-1]->add(ScanLine(slX+xOffs,yOffs,w-slX));
    
    // rest pixles
    for(int y=1;y<h;++y){
       
      //1st pix
      if( eqfunc( data(0,y), data(0,y-1)) ){
        lim[w*y] = lim[w*(y-1)];
      }else{
        lim[w*y] = newPart();
      }

      RegionPart **limC = lim+w*y;
      RegionPart **limL = limC-w;
      const T *dataC = &(data(0,y));
      const T *dataL = &(data(0,y-1));
      
      //rest of pixels
      slX = 0;
         
      for(int x=1;x<w;++x){
        if( eqfunc( dataC[x] , dataC[x-1] ) ){
          if( eqfunc( dataC[x] , dataL[x] ) && limC[x-1] != limL[x] ){
            RegionPart *pOld = limL[x];
            RegionPart *pNew = limC[x-1];

            limC[x] = pNew;
            pNew->add(pOld);
        
            std::replace_if(&limL[x+1],&limC[x],EQFunctor<RegionPart*>(pOld),pNew);
        
          }else{
            limC[x] = limC[x-1];
          }   
        }else{
          if( eqfunc( dataC[x] , dataL[x] )){
            limC[x] = limL[x];
          }else{
            limC[x] = newPart();
          }
       
          limC[x-1]->add(ScanLine(slX+xOffs,y+yOffs,x-slX));
          slX = x; 
        }
      }
      limC[w-1]->add(ScanLine(slX+xOffs,y+yOffs,w-slX));
    }

    for(unsigned int i=0;i<m_vecParts.size();i++){
      if(m_vecParts[i]->top){
        const T &val = image(m_vecParts[i]->scanlines[0].x,m_vecParts[i]->scanlines[0].y,0);
        if(val >= m_dMinVal && val <= m_dMaxVal){
          m_vecBlobData.push_back(Region(m_vecParts[i],m_uiMaxSize,val,&image));
          Region &b = m_vecBlobData.back();
          if((unsigned int)b.getSize() > m_uiMaxSize || (unsigned int)b.getSize() < m_uiMinSize){
            m_vecBlobData.pop_back();
          }      
        }
      }
    }
  }
}
