/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
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

#include <ICLBlob/RegionDetector.h>
#include <algorithm>
namespace icl{

  struct RegionImage{
    Region **rs; 
    int w;
    RegionImage(Region **rs, int w):rs(rs),w(w){}
    
    Region *operator()(int x, int y){
      return rs[x+w*y];
    }
  };

  
  struct RegionDetector::Tree{
    std::vector<Region*> region_image;
  };
  
  void RegionDetector::createTree(const ImgBase *image){
    const Rect &roi = image->getROI();
    const int w = roi.width;
    const int h = roi.height;
    const int rx = roi.x;
    const int ry = roi.y;
    
    if(w<2 || h<2) return;
    // create region-image 
    m_tree->region_image.resize(w*h);
    Region **rim = m_tree->region_image.data();
    for(unsigned int i=0;i<m_vecAllBlobData.size();++i){
      Region *r = &m_vecAllBlobData[i];
      const std::vector<ScanLine> &sls = r->getScanLines();
      for(unsigned int j=0;j<sls.size();++j){
        const ScanLine &sl = sls[j];
        int offs = sl.x-rx +w*(sl.y-ry);
        std::fill(rim+offs,rim+offs+sl.len,r);
      }
    }


    /// create neighbourhood tree
    RegionImage r(rim,w);

    // upper left
    r(0,0)->addNeighbour(0);


    // upper right // needed for right column
    r(w-1,0)->addNeighbour(0);
    
    // lower left // needed for bottom row
    r(0,h-1)->addNeighbour(0);
    
    // top & bottom row
    for(int x=1;x<w;++x){
      if(r(x-1,0) != r(x,0)){
        r(x,0)->addNeighbour(0);
      }
      if(r(x-1,h-1) != r(x,h-1)){
        r(x,h-1)->addNeighbour(0);
      }
    }
    // left & right columns
    for(int y=1;y<h;++y){
      if(r(0,y-1) != r(0,y)){
        r(0,y)->addNeighbour(0);
      }
      if(r(w-1,y-1) != r(w-1,y)){
        r(w-1,y)->addNeighbour(0);
      }
    }
    // remaining
    for(int y=1;y<h;++y){ // force row-major chaching
      for(int x=1;x<w;++x){
        Region *c = r(x,y);
        Region *l = r(x-1,y);
        Region *u = r(x,y-1);
        if(c != l){
          c->addNeighbour(l);
          l->addNeighbour(c);
        }
        if(c != u){
          c->addNeighbour(u);
          u->addNeighbour(c);
        }
      }
    }
  }
  
  template<class T>
  static inline bool eqfunc(const T &a,const T&b){
    return a == b;
  }
  
  
  RegionDetector::RegionDetector(unsigned int minSize, unsigned int maxSize, icl64f minVal, icl64f maxVal, bool createTree):
    m_uiMinSize(minSize),m_uiMaxSize(maxSize),m_dMinVal(minVal),m_dMaxVal(maxVal),m_createTree(createTree),
    m_tree(createTree?new Tree:0){
  }
  RegionDetector::~RegionDetector(){
    ICL_DELETE(m_tree);
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
    m_vecAllBlobData.clear();
    ICLASSERT_RETURN_VAL(image,m_vecBlobData);
    ICLASSERT_RETURN_VAL(image->getChannels()==1,m_vecBlobData);
    ICLASSERT_RETURN_VAL(image->getROISize().getDim(),m_vecBlobData);
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: detect_intern(*image->asImg<icl##D>(),eqfunc<icl##D>); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    
    if(m_createTree){
      if(!m_tree) m_tree = new Tree;
      createTree(image);
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
        chan(image[channel]),xOffs(image.getROI().x),yOffs(image.getROI().y){
      }
      inline const T &operator()(int x, int y) const{
        return chan(x+xOffs,y+yOffs);
      }
      
    private:
      const Channel<T> chan;
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

    if(m_createTree){
      for(unsigned int i=0;i<m_vecParts.size();i++){
        if(m_vecParts[i]->top){
          const T &val = image(m_vecParts[i]->scanlines[0].x,m_vecParts[i]->scanlines[0].y,0);
          m_vecAllBlobData.push_back(Region(m_vecParts[i],image.getDim(),val,&image,&m_vecAllBlobData));
          unsigned int size = (unsigned int)m_vecAllBlobData.back().getSize();
          if(val >= m_dMinVal && val <= m_dMaxVal && size <= m_uiMaxSize && size >= m_uiMinSize){
            m_vecBlobData.push_back(m_vecAllBlobData.back());
          }
        }
      }
    }else{
      for(unsigned int i=0;i<m_vecParts.size();i++){
        if(m_vecParts[i]->top){
          const T &val = image(m_vecParts[i]->scanlines[0].x,m_vecParts[i]->scanlines[0].y,0);
          if(val >= m_dMinVal && val <= m_dMaxVal){
            m_vecBlobData.push_back(Region(m_vecParts[i],m_uiMaxSize,val,&image,&m_vecBlobData));
            Region &b = m_vecBlobData.back();
            if((unsigned int)b.getSize() > m_uiMaxSize || (unsigned int)b.getSize() < m_uiMinSize){
              m_vecBlobData.pop_back();
            }      
          }
        }
      }
    }
  }
  
}
