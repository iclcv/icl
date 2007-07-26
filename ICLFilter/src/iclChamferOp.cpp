#include "iclChamferOp.h"
#include <iclImg.h>
#include <iclPoint.h>

#include <iclImgChannel.h>
#include <limits>
#include <math.h>
#include <vector>
using std::vector;

namespace icl{
  /**
      metric_1_1
      metric_1_2
      metric_2_3
      metric_7071_10000
      metric_real_euclidian
  **/

  namespace{
    inline icl32s min3(icl32s a, icl32s b, icl32s c){
      // {{{ open

      return std::min(std::min(a,b),c);
    }

    // }}}
    inline icl32s min5(icl32s a, icl32s b, icl32s c, icl32s d, icl32s e){
      // {{{ open

      return min3(min3(a,b,c),d,e);
    }

    // }}}
    template<class T>
    void prepare_chamfer_image(const Img<T> *poSrc, Img32s *poDst, int channel, int maxVal){
      // {{{ open

      if(poSrc->hasFullROI() && poDst->hasFullROI()){
        
        ImgChannel<T> src = pickChannel(poSrc,channel);
        ImgChannel32s dst = pickChannel(poDst,channel);
        
        /// initialization
        for(int i=0;i<src.getDim();i++){
          dst[i] = src[i] ? 0 : maxVal;
        }

      }else{
        ConstImgIterator<T> src = poSrc->getROIIterator(channel);
        ImgIterator<icl32s> dst = poDst->getROIIterator(channel);
        
        for(;src.inRegion();++src,++dst){
          *dst = *src ? 0 : maxVal;
        }
      }
    }

    // }}}
    void apply_chamfer_op_generic(Img32s *poDst, int channel, int  d1, int d2){
      // {{{ open

      if(poDst->hasFullROI()){
        ImgChannel32s dst = pickChannel(poDst,channel);

        // forward loop
        for(int x=1;x<dst.getWidth()-1;++x){
          for(int y=1;y<dst.getHeight();++y){
            dst(x,y) = min5( dst(x,y),dst(x-1,y)+d1,dst(x-1,y-1)+d2,dst(x,y-1)+d1,dst(x+1,y-1)+d2 );
          }
        }
        
        // backward loop
        for(int x=dst.getWidth()-2;x>=1; --x){
          for(int y=dst.getHeight()-2;y>=0; --y){
            dst(x,y) = min5( dst(x,y),dst(x+1,y)+d1,dst(x+1,y+1)+d2,dst(x,y+1)+d1,dst(x-1,y+1)+d2 );
          }
        }

        // first  and last column / first and last row
        int h1 = dst.getHeight()-1;
        int h2 = h1-1;
        int w1 = dst.getWidth()-1;
        int w2 = w1-1;
        
        for(int y=0;y<dst.getHeight();++y){
          dst(0,y) = dst(1,y);
          dst(w1,y) = dst(w2,y) ;
        }
        for(int x=0;x<dst.getWidth();++x){
          dst(x,0) = dst(x,1);
          dst(x,h1) = dst(x,h2) ;
        }
      }else{
        ERROR_LOG("Chamfering is not yet implemnted with ROI support!");
      }
    }    
  // }}}
    
    template<class T>
    vector<Point> find_pixels(const Img<T> *image, int channel){
      // {{{ open

      vector<Point> v;

      ConstImgIterator<T> it = image->getROIIterator(channel);
      for(;it.inRegion();++it){
        if(*it){
          v.push_back(Point(it.x(),it.y())) ;
        }
      }
      return v;
    }

    // }}}
    
    float min_point_distance(const vector<Point> & pix, const Point &p){
      // {{{ open

      float minDist = pix[0].distanceTo(p);
      for(unsigned int i=1;i<pix.size();++i){
        minDist = std::min(minDist,pix[i].distanceTo(p));
      }
      return minDist;
    }

    // }}}

    void create_distance_map(Img32f *image, const vector<Point> &pix, int channel){
      // {{{ open
      
      ImgIterator<icl32f> it = image->getROIIterator(channel);
      for(;it.inRegion();++it){
        *it = min_point_distance(pix,Point(it.x(),it.y()));
      }      
    }
    // }}}
  }
  
  void ChamferOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open
    ICLASSERT_RETURN(poSrc);
    ICLASSERT_RETURN(ppoDst);
    if(!prepare (ppoDst, m_eMetric == metric_real_euclidian ? depth32f : depth32s, 
                 poSrc->getSize(),poSrc->getFormat(),
                 poSrc->getChannels(),poSrc->getROI(),poSrc->getTime())){
      ERROR_LOG("unable to prepare image \n");
      return;
    }      
    int C = (*ppoDst)->getChannels();
    int WH = (*ppoDst)->getWidth()+(*ppoDst)->getHeight();
    
    if(m_eMetric != metric_real_euclidian){    
      Img32s *dst = (*ppoDst)->asImg<icl32s>();
      
      int d1,d2;
      switch(m_eMetric){
        case metric_1_1: d1=d2=1; break;
        case metric_1_2: d1=1; d2=2; break;
        case metric_2_3: d1=2; d2=3; break;
        case metric_7071_10000: d1=7071; d2=10000; break;
        default: d1=d2=-1;
      }
      
      switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: for(int c=0;c<C;++c){ prepare_chamfer_image(poSrc->asImg<icl##D>(),dst,c,d2*WH); } break;
        ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_DEPTH;
      }
      for(int c=0;c<C;++c){
        apply_chamfer_op_generic(dst,c,d1,d2);
      }
      
    }else{

      Img32f *dst = (*ppoDst)->asImg<icl32f>();
      vector<vector<Point> > pix(C);
      switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                              \
        case depth##D:                                        \
          for(int c=0;c<C;++c){                               \
            pix[c] = find_pixels(poSrc->asImg<icl##D>(),c);   \
          }                                                   \
          break;                                            
        ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_DEPTH;
      }

      for(int c=0;c<C;++c){  
        if(!(pix[c].size())){
          dst->clear(c);
        }else{
          create_distance_map(dst,pix[c],c);
        }
      }
    }
  }

  // }}}
  
  double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage, const std::vector<Point> &model, ChamferOp::hausdorffMetric m){
    // {{{ open

    ICLASSERT_RETURN_VAL(chamferImage,-1);
    ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(model.size(),-1);
    
    Rect roi = chamferImage->getROI();
    ImgChannel32s chan = pickChannel(chamferImage,0);
    
    if(m == hausdorff_mean){
      int n=0;
      double val=0;
      register int x,y;      
      for(unsigned int i=0;i<model.size();++i){
        x = model[i].x;
        y = model[i].y;
        if(roi.contains(x,y)){
          val+=chan(x,y);
          n++;
        }
      }
      return n ? val/n : 0;
    }else{
      icl32s maxVal = 0;
      register int x,y; 
      for(unsigned int i=0;i<model.size();++i){
        x = model[i].x;
        y = model[i].y;
        if(roi.contains(x,y)){
          maxVal = std::max(maxVal,chan(x,y));
        }
      }
      return maxVal;      
    }    
  }

  // }}}
  
  double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImageA, const Img32s *chamferImageB, ChamferOp::hausdorffMetric m){
    // {{{ open

    //    A:=model
 
    ICLASSERT_RETURN_VAL(chamferImageA,-1);
    ICLASSERT_RETURN_VAL(chamferImageB,-1);
    ICLASSERT_RETURN_VAL(chamferImageA->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(chamferImageB->getChannels() == 1,-1);

    Rect roi = chamferImageA->getROI() & chamferImageB->getROI();
    ImgChannel32s chanA = pickChannel(chamferImageA,0);
    ImgChannel32s chanB = pickChannel(chamferImageB,0);
    
    int xEnd = roi.right();
    int yEnd = roi.bottom();
    
    if(m == hausdorff_mean){
      int n=0;
      double val=0;
      for(int x=roi.x;x<xEnd;++x){
        for(int y=roi.y;y<yEnd;++y){
          if(!chanA(x,y)){
            val += chanB(x,y);
            n++;
          }
        }
      }
      return n ? val/n : 0;
    }else{
      icl32s maxVal = 0; 
      for(int x=roi.x;x<xEnd;++x){
        for(int y=roi.y;y<yEnd;++y){
          if(!chanA(x,y)){
            maxVal = std::max(maxVal,chanB(x,y));
          }
        }
      }
      return maxVal;
    }
  }
  
  // }}}

  double ChamferOp::computeSymmetricHausdorffDistance(const Img32s *chamferImageA, const Img32s *chamferImageB, ChamferOp::hausdorffMetric m){
    // {{{ open

    if(m==hausdorff_mean){
      return (computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m)+
              computeDirectedHausdorffDistance(chamferImageB,chamferImageA,m))/2;
    }else{
      return std::max(computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m),
                      computeDirectedHausdorffDistance(chamferImageB,chamferImageA,m));
    }
  }

  // }}}
  double ChamferOp::computeSymmetricHausdorffDistance(const std::vector<Point> setA, ImgBase **bufferA, 
                                                      const std::vector<Point> setB, ImgBase **bufferB,
                                                      const Size &imageSize,ChamferOp::hausdorffMetric m){
    // {{{ open

    ICLASSERT_RETURN_VAL(bufferA,-1);
    ICLASSERT_RETURN_VAL(bufferB,-1);
    ICLASSERT_RETURN_VAL(setA.size(),-1);
    ICLASSERT_RETURN_VAL(setB.size(),-1);
    
    ensureCompatible(bufferA,depth32s,imageSize,1);
    ensureCompatible(bufferB,depth32s,imageSize,1);
    
    Img32s *a = (*bufferA)->asImg<icl32s>();   
    Img32s *b = (*bufferA)->asImg<icl32s>();
    
    a->clear();
    b->clear();
    
    ImgChannel32s cA = pickChannel(a,0);
    ImgChannel32s cB = pickChannel(b,0);
    
    Rect roiA = a->getROI(); 
    Rect roiB = b->getROI(); 
    register int x,y;
    for(unsigned int i=0;i<setA.size();i++){
      x = setA[i].x;
      y = setA[i].y;
      if(roiA.contains(x,y)){
        cA(x,y)=255;
      }
    }
    for(unsigned int i=0;i<setB.size();i++){
      x = setB[i].x;
      y = setB[i].y;
      if(roiB.contains(x,y)){
        cB(x,y)=255;
      }
    }
  
    ChamferOp().apply(a,bufferA);
    ChamferOp().apply(b,bufferB);
    
    return computeSymmetricHausdorffDistance(a,b,m);
  }

  // }}}

  double  ChamferOp::computeSymmeticHausdorffDistance(const Img32s *chamferImage, 
                                                      const std::vector<Point> &model, 
                                                      ImgBase **bufferImage, 
                                                      ChamferOp::hausdorffMetric m){
    ICLASSERT_RETURN_VAL(chamferImage,-1);
    ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(bufferImage,-1);
    
    double hd1 = computeDirectedHausdorffDistance(chamferImage, model,m);

    ensureCompatible(bufferImage,depth32s,chamferImage->getSize(),1,formatMatrix);
    Img32s *bi = (*bufferImage)->asImg<icl32s>();
    ImgChannel32s biChannel = pickChannel(bi,0);
    bi->clear();
    Rect roi = bi->getROI();
    for(unsigned int i=0;i<model.size();i++){
      int x = model[i].x;
      int y = model[i].y;
      if(roi.contains(x,y)){
        biChannel(x,y) = 255;
      }
    }
    
    ChamferOp co;
    co.apply(bi,bufferImage);

    double hd2 = computeDirectedHausdorffDistance(chamferImage,bi,m);
    
    return m == hausdorff_mean ? (hd1+hd2)/2 : std::max(hd1,hd2);
  }



}
