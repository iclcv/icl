#include "iclChamferOp.h"
#include <iclImg.h>
#include <iclPoint.h>

#include <limits>
#include <math.h>
#include <vector>
using std::vector;

namespace icl{
  
  namespace{
    inline icl32s min3(icl32s a, icl32s b, icl32s c){
      // {{{ open

      return iclMin(iclMin(a,b),c);
    }

    // }}}
    inline icl32s min5(icl32s a, icl32s b, icl32s c, icl32s d, icl32s e){
      // {{{ open

      return min3(min3(a,b,c),d,e);
    }

    // }}}

    namespace co{
      inline Size scale(const Size &s, int f){
        return Size( (int)ceil(float(s.width)/f), (int)ceil(float(s.height)/f) );
      }
      inline Point scale(const Point &s, int f){
        return Point( (int)ceil(float(s.x)/f), (int)ceil(float(s.y)/f) );
      }
      inline Rect scale(const Rect &r, int f){
        return Rect(scale(r.ul(),f), scale(r.getSize(),f) );
      }
    }

    template<class T>
    void prepare_chamfer_image_unscaled(const Img<T> *poSrc, Img32s *poDst, int channel, icl32s maxVal){
      // {{{ open
      if(poSrc->hasFullROI() && poDst->hasFullROI()){
        
        Channel<T> src = (*poSrc)[channel];
        Channel32s dst = (*poDst)[channel];
        
        /// initialization
        for(int i=0;i<src.getDim();i++){
          dst[i] = src[i] ? 0 : maxVal;
        }

      }else{
        const ImgIterator<T> src = poSrc->beginROI(channel);
        const ImgIterator<T> srcEnd = poSrc->endROI(channel);
        ImgIterator<icl32s> dst = poDst->beginROI(channel);
        
        for(;src != srcEnd ;++src,++dst){
          *dst = *src ? 0 : maxVal;
        }
      }
    }

    // }}} 
    template<class T>
    void prepare_chamfer_image_scaled(const Img<T> *poSrc, Img32s *poDst, int channel, icl32s maxVal, int scaleFactor){
      // {{{ open
      
      ICLASSERT_RETURN(poSrc);
      ICLASSERT_RETURN(poDst);
      ICLASSERT_RETURN(co::scale(poSrc->getROISize(),scaleFactor) == poDst->getROISize());
      
      poDst->clear(channel,maxVal,true);
      
      if(poSrc->hasFullROI() && poDst->hasFullROI()){
        int xEnd = poSrc->getWidth();
        int yEnd = poSrc->getHeight();
        const Channel<T> src = (*poSrc)[channel];
        Channel32s dst = (*poDst)[channel];
        for(int x=0;x<xEnd;++x){
          for(int y=0;y<yEnd;++y){
            if(src(x,y)) dst(x/scaleFactor,y/scaleFactor) = 0;
          }
        }
      }else{
        int xSrcStart = poSrc->getROI().x;
        int xSrcEnd = poSrc->getROI().right();
        int ySrcStart = poSrc->getROI().y;
        int ySrcEnd = poSrc->getROI().bottom();
        int xDstStart = poDst->getROI().x;
        int yDstStart = poDst->getROI().y;

        const Channel<T> src = (*poSrc)[channel];
        Channel32s dst = (*poDst)[channel];
        
        poDst->clear(channel,maxVal,true);
        Rect r = poDst->getROI();
        for(int x=xSrcStart;x<xSrcEnd;++x){
          for(int y=ySrcStart;y<ySrcEnd;++y){
            if(src(x,y)) dst(xDstStart + (x-xSrcStart)/scaleFactor,yDstStart + (y-ySrcStart)/scaleFactor ) = 0;
          }
        }
      }
    }

    // }}}

  
    
    template<class T>
    void prepare_chamfer_image(const Img<T> *poSrc, Img32s *poDst, int channel, icl32s maxVal, int scaleFactor, Img32s &buffer, bool scaleUpResult){
      // {{{ open

      if(scaleFactor > 1){
        ImgBase *buf = scaleUpResult ? &buffer : poDst;
        
        ensureCompatible(&buf,depth32s,co::scale(poSrc->getSize(),scaleFactor), 
                         poSrc->getChannels(),poSrc->getFormat(),co::scale(poSrc->getROI(),scaleFactor));
        
        prepare_chamfer_image_scaled(poSrc,buf->asImg<icl32s>(),channel,maxVal,scaleFactor);
      }else{
        prepare_chamfer_image_unscaled(poSrc,poDst,channel,maxVal);
      }
    }
    // }}}

    void apply_chamfer_op_generic(Img32s *poDst, int channel, icl32s  d1, icl32s d2){
      // {{{ open
      
      if(poDst->hasFullROI()){
        int w = poDst->getWidth();
        int h = poDst->getHeight();
        Channel32s dst = (*poDst)[channel];
        
        // forward loop
        for(int x=1;x<w-1;++x){
          for(int y=1;y<h;++y){
            dst(x,y) = min5( dst(x,y),dst(x-1,y)+d1,dst(x-1,y-1)+d2,dst(x,y-1)+d1,dst(x+1,y-1)+d2 );
          }
        }
        
        // backward loop
        for(int x=w-2;x>=1; --x){
          for(int y=h-2;y>=0; --y){
            dst(x,y) = min5( dst(x,y),dst(x+1,y)+d1,dst(x+1,y+1)+d2,dst(x,y+1)+d1,dst(x-1,y+1)+d2 );
          }
        }

        // first  and last column / first and last row
        int h1 = h-1;
        int h2 = h1-1;
        int w1 = w-1;
        int w2 = w1-1;
        
        for(int y=0;y<h;++y){
          dst(0,y) = dst(1,y);
          dst(w1,y) = dst(w2,y) ;
        }
        for(int x=0;x<w;++x){
          dst(x,0) = dst(x,1);
          dst(x,h1) = dst(x,h2) ;
        }
      }else{

        Channel32s dst = (*poDst)[channel];

        Rect r = poDst->getROI();
     
        int rX = r.x;
        int rY = r.y;
        int rXEnd = r.right();
        int rYEnd = r.bottom();
        // forward loop
        for(int x=rX+1;x<rXEnd-1;++x){
          for(int y=rY+1;y<rYEnd;++y){
            dst(x,y) = min5( dst(x,y),dst(x-1,y)+d1,dst(x-1,y-1)+d2,dst(x,y-1)+d1,dst(x+1,y-1)+d2 );
          }
        }
        
        // backward loop
        for(int x=rXEnd-2;x>=rX+1; --x){
          for(int y=rYEnd-2;y>=rY; --y){
            dst(x,y) = min5( dst(x,y),dst(x+1,y)+d1,dst(x+1,y+1)+d2,dst(x,y+1)+d1,dst(x-1,y+1)+d2 );
          }
        }
        
        // first  and last column / first and last row
        int h1 = rYEnd-1;
        int h2 = h1-1;
        int w1 = rXEnd-1;
        int w2 = w1-1;
        
        for(int y=rY;y<rYEnd;++y){
          dst(rX,y) = dst(rX+1,y);
          dst(w1,y) = dst(w2,y) ;
        }
        for(int x=rX;x<rXEnd;++x){
          dst(x,rY) = dst(x,rY+1);
          dst(x,h1) = dst(x,h2) ;
        }
      }
    }    
  // }}}
    int compute_max_val(icl32s d1, icl32s d2, const Size &imageSize){
      // {{{ open

      return iclMax(d1,d2)*(imageSize.width+imageSize.height);
    }

    // }}}
  
  }
  
  ChamferOp::ChamferOp( icl32s horizontalAndVerticalNeighbourDistance, icl32s diagonalNeighborDistance, int scaleFactor, bool scaleUpResult)
    // {{{ open

    :m_iHorizontalAndVerticalNeighbourDistance(horizontalAndVerticalNeighbourDistance),
     m_iDiagonalNeighborDistance(diagonalNeighborDistance),
     m_iScaleFactor(scaleFactor),
     m_bScaleUpResult(scaleUpResult){
    setClipToROI(false);
  }

  // }}}
 

  
  void ChamferOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open
    ICLASSERT_RETURN(poSrc);
    ICLASSERT_RETURN(ppoDst);
    ICLASSERT_RETURN(poSrc != *ppoDst);

    bool needSetCheckOnlyToFalseCall = (!getCheckOnly()) && (*ppoDst == poSrc);
    if(needSetCheckOnlyToFalseCall) setCheckOnly(true);
    //    if(!prepare (ppoDst, poSrc, depth32s)){
    Size dstSize;
    Rect dstROI;
    if( (m_iScaleFactor == 1) || m_bScaleUpResult ){
      dstSize = poSrc->getSize();
      dstROI = poSrc->getROI();
    }else{
      dstSize = co::scale(poSrc->getSize(),m_iScaleFactor);
      dstROI = co::scale(poSrc->getROI(),m_iScaleFactor);
    }
    if(!prepare(ppoDst, depth32s, dstSize,  poSrc->getFormat(), poSrc->getChannels(), dstROI)){
      ERROR_LOG("unable to prepare image \n");
      if(needSetCheckOnlyToFalseCall) setCheckOnly(false);
      return;
    }      
    int C = (*ppoDst)->getChannels();
    
    Img32s *dst = (*ppoDst)->asImg<icl32s>();
    
    icl32s d1 = m_iHorizontalAndVerticalNeighbourDistance;
    icl32s d2 = m_iDiagonalNeighborDistance;
    icl32s maxVal = compute_max_val(d1,d2,poSrc->getSize());
    
    switch(poSrc->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                                          \
      case depth##D:                                                                                                      \
        for(int c=0;c<C;++c){                                                                                             \
          prepare_chamfer_image(poSrc->asImg<icl##D>(),dst,c,maxVal, m_iScaleFactor, m_oBufferImage, m_bScaleUpResult);   \
        }                                                                                                                 \
        break;                                                          
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
    if(m_iScaleFactor == 1 || !m_bScaleUpResult){
      for(int c=0;c<C;++c){
        apply_chamfer_op_generic(dst,c,d1,d2);
      }
    }else{
      for(int c=0;c<C;++c){
        apply_chamfer_op_generic(&m_oBufferImage,c,d1,d2);
      }      
      m_oBufferImage.scaledCopyROI(dst);
    }
    if(needSetCheckOnlyToFalseCall) setCheckOnly(false);
  }
  // }}}
  


  namespace{
    struct PenaltyModeNone{
      // {{{ open

      PenaltyModeNone( const Rect &roi, icl32s penalty):
        roi(roi),penalty(penalty){}
      inline icl32s operator()(icl32s val, int x, int y) const {(void)x; (void)y;  return val; }
      Rect roi;
      icl32s penalty;
    };

    // }}}
    struct PenaltyModeConst{
      // {{{ open

      PenaltyModeConst( const Rect &roi,icl32s penalty):
        roi(roi),penalty(penalty){}
      inline icl32s operator() (icl32s val, int x, int y) const { return roi.contains(x,y) ? val : penalty; }
      Rect roi;
      icl32s penalty;
    };

    // }}}
    struct PenaltyModeDist{
      // {{{ open

      PenaltyModeDist( const Rect &roi,icl32s penalty):
        roi(roi),penalty(penalty){}
      inline icl32s operator() (icl32s val, int x, int y) const { 
        if(roi.contains(x,y)){
          return val;
        }else{
          int dx = iclMax(0,abs(roi.x+roi.width/2 - x)-roi.width/2);
          int dy = iclMax(0,abs(roi.y+roi.height/2 - y)-roi.height/2);
          return (dx+dy)*penalty;
        }
      }
      Rect roi;
      icl32s penalty;
    };

    // }}}
  
    struct HausdorffMetricModeMean{
      // {{{ open

      HausdorffMetricModeMean():n(0),val(0){}
      inline void operator<<(icl32s x){ val+=x; n++; }
      double getResult() const{ return n ? val/n : -1; }
      int n;
      double val;
    };

    // }}}
    struct HausdorffMetricModeMax{
      // {{{ open

      HausdorffMetricModeMax():val(-1){}
      inline void operator<<(icl32s x){ val = iclMax(val,x); }
      double getResult() const{ return val; }
      icl32s val;
    };

    // }}}
    
    template<class HausdorffMetricMode, class PenaltyMode>
    inline double apply_directed_hausdorff_distance(const Img32s *chamferImage, const std::vector<Point> &model, HausdorffMetricMode hmm,PenaltyMode pm){
      // {{{ open

      Channel32s chan = (*chamferImage)[0];
      register int x,y;      
      Rect imageRect = Rect(Point::null,chamferImage->getSize());
      for(unsigned int i=0;i<model.size();++i){
        x = model[i].x;
        y = model[i].y;
        if(imageRect.contains(x,y)){
          hmm << pm(chan(x,y),x,y);
        }
      }
      return hmm.getResult();
    }

    // }}}
 
    template<class HausdorffMetricMode, class PenaltyMode>
    inline double apply_directed_hausdorff_distance_2(const Img32s *chamferImage, const Img32s *modelChamferImage, HausdorffMetricMode hmm,PenaltyMode pm){
      // {{{ open

      Channel32s chan = (*chamferImage)[0];
      Channel32s modelChan = (*modelChamferImage)[0];
      Rect imageRect = Rect(Point::null,chamferImage->getSize());
      Rect modelROI = modelChamferImage->getROI() & imageRect;
      
      int rX = modelROI.x;
      int rY = modelROI.y;
      int rXEnd = modelROI.right();
      int rYEnd = modelROI.bottom();
      for(int x = rX; x<rXEnd; ++x){
        for(int y = rY; y<rYEnd; ++y){
          if(modelChan(x,y)){
            hmm << pm(chan(x,y),x,y);
          }
        }
      }
      return hmm.getResult();
    }

    // }}}
  
}

  void ChamferOp::renderModel(const std::vector<Point> &model, ImgBase **image, const Size &size, icl32s bg, icl32s fg,  Rect roi){
    // {{{ open

    ICLASSERT_RETURN(image);
    if(roi == Rect::null){
      roi = Rect(Point::null,size);
    }
    ICLASSERT_RETURN(Rect(Point::null,size).contains(roi));
    
    ensureCompatible(image,depth32s, size, 1, formatMatrix, roi);
    (*image)->clear(0,bg);
    Channel32s c = (*(*image)->asImg<icl32s>())[0];
    for(unsigned int i=0;i<model.size();++i){
      if(roi.contains(model[i].x,model[i].y)){
        c(model[i].x,model[i].y) = fg;
      }
    }
  }

  // }}}
  
  double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage, 
                                                     const std::vector<Point> &model,
                                                     ChamferOp::hausdorffMetric m, 
                                                     ChamferOp::outerROIPenaltyMode pm,
                                                     icl32s penaltyValue){
    // {{{ open
    ICLASSERT_RETURN_VAL(chamferImage,-1);
    ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(model.size(),-1);
      
    Rect roi = chamferImage->getROI();
      
    switch(pm){
      case noPenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMax(), PenaltyModeNone(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMean(), PenaltyModeNone(roi,penaltyValue));
        }
      case constPenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMax(), PenaltyModeConst(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMean(), PenaltyModeConst(roi,penaltyValue));
        }
      case distancePenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMax(), PenaltyModeDist(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance(chamferImage,model,HausdorffMetricModeMean(), PenaltyModeDist(roi,penaltyValue));
        }
    }
    return -1;
  }

  // }}}
  
  double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage, 
                                                     const Img32s *modelChamferImage, 
                                                     ChamferOp::hausdorffMetric m, 
                                                     ChamferOp::outerROIPenaltyMode pm,
                                                     icl32s penaltyValue){
    // {{{ open
    ICLASSERT_RETURN_VAL(chamferImage,-1);
    ICLASSERT_RETURN_VAL(modelChamferImage,-1);
    ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(modelChamferImage->getChannels() == 1,-1);
    
    Rect roi = chamferImage->getROI();
      
    switch(pm){
      case noPenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMax(), PenaltyModeNone(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMean(), PenaltyModeNone(roi,penaltyValue));
        }
      case constPenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMax(), PenaltyModeConst(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMean(), PenaltyModeConst(roi,penaltyValue));
        }
      case distancePenalty:
        if( m == hausdorff_max) { 
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMax(), PenaltyModeDist(roi,penaltyValue));
        }else{
          return apply_directed_hausdorff_distance_2(chamferImage,modelChamferImage,HausdorffMetricModeMean(), PenaltyModeDist(roi,penaltyValue));
        }
    }
    return -1;
  }
   
  // }}}
  
  double ChamferOp::computeSymmetricHausdorffDistance(const Img32s *chamferImageA, 
                                                      const Img32s *chamferImageB,
                                                      hausdorffMetric m,
                                                      ChamferOp::outerROIPenaltyMode pm,
                                                      icl32s penaltyValue){
    // {{{ open
    
    double ab = computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m,pm,penaltyValue);
    double ba = computeDirectedHausdorffDistance(chamferImageB,chamferImageA,m,pm,penaltyValue);
    return m==hausdorff_mean ? (ab+ba)/2 : iclMax(ab,ba);
  }

  // }}}

 
  
  double ChamferOp::computeSymmetricHausdorffDistance(const std::vector<Point> setA, const Size &sizeA, const Rect &roiA, ImgBase **bufferA, 
                                                      const std::vector<Point> setB, const Size &sizeB, const Rect &roiB, ImgBase **bufferB, 
                                                      hausdorffMetric m,ChamferOp::outerROIPenaltyMode pm,icl32s penaltyValue,
                                                      ChamferOp coA, ChamferOp coB){
    // {{{ open
    
    renderModel(setA,bufferA,sizeA,0,255,roiA);
    renderModel(setB,bufferB,sizeB,0,255,roiB);
    coA.apply(*bufferA,bufferA);
    coB.apply(*bufferB,bufferB);
    return computeSymmetricHausdorffDistance((*bufferA)->asImg<icl32s>(),(*bufferB)->asImg<icl32s>(),m,pm,penaltyValue);
  }

  // }}}
  
  double ChamferOp::computeSymmeticHausdorffDistance(const Img32s *chamferImage, 
                                                     const std::vector<Point> &model, 
                                                     const Size &modelImageSize,
                                                     const Rect &modelImageROI,
                                                     ImgBase **bufferImageA, 
                                                     ImgBase **bufferImageB, 
                                                     ChamferOp::hausdorffMetric m,
                                                     ChamferOp::outerROIPenaltyMode pm,
                                                     icl32s penaltyValue,
                                                     ChamferOp co){
    // {{{ open
    ICLASSERT_RETURN_VAL(chamferImage,-1);
    ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
    ICLASSERT_RETURN_VAL(bufferImageA,-1);
    ICLASSERT_RETURN_VAL(bufferImageB,-1);
    
    double hd1 = computeDirectedHausdorffDistance(chamferImage, model,m,pm,penaltyValue);

    renderModel(model,bufferImageA, modelImageSize, 0, 255, modelImageROI);

    co.apply(*bufferImageA,bufferImageB);
    
    double hd2 = computeDirectedHausdorffDistance((*bufferImageB)->asImg<icl32s>(),chamferImage,m,pm,penaltyValue);
    
    return m == hausdorff_mean ? (hd1+hd2)/2 : iclMax(hd1,hd2);
    
  }

  // }}}

}
  /*    
      //OLD!!
      double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage, 
      const std::vector<Point> &model,
      ChamferOp::hausdorffMetric hm, 
      int penalty,
      ChamferOp::metric m){
      // {{{ open

      ICLASSERT_RETURN_VAL(chamferImage,-1);
      ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
      ICLASSERT_RETURN_VAL(model.size(),-1);
    
      Rect roi = chamferImage->getROI();
      Rect imagerect = Rect(Point::null,chamferImage->getSize());
      ImgChannel32s chan = pickChannel(chamferImage,0);
    
      penalty = penalty < 0 ? compute_max_val(m,chamferImage->getSize()) : penalty;
    
      if(hm == hausdorff_mean){
      int n=0;
      double val=0;
      register int x,y;      
      for(unsigned int i=0;i<model.size();++i){
      x = model[i].x;
      y = model[i].y;
      if(roi.contains(x,y)){
      val+=chan(x,y);
      n++;
      }else if(imagerect.contains(x,y)){
      val+=penalty;
      if(penalty) n++;
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
      maxVal = iclMax(maxVal,chan(x,y));
      }else if(imagerect.contains(x,y)){
      maxVal = iclMax(maxVal,penalty);
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
            maxVal = iclMax(maxVal,chanB(x,y));
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
      return iclMax(computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m),
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
    Img32s *b = (*bufferB)->asImg<icl32s>();
    
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
      ChamferOp::hausdorffMetric hm,
      int penalty,
      ChamferOp::metric m){
      ICLASSERT_RETURN_VAL(chamferImage,-1);
      ICLASSERT_RETURN_VAL(chamferImage->getChannels() == 1,-1);
      ICLASSERT_RETURN_VAL(bufferImage,-1);
    
      double hd1 = computeDirectedHausdorffDistance(chamferImage, model,hm, penalty,m);

      ensureCompatible(bufferImage,depth32s,chamferImage->getSize(),1,formatMatrix,chamferImage->getROI());
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

      double hd2 = computeDirectedHausdorffDistance(chamferImage,bi,hm);
    
      return hm == hausdorff_mean ? (hd1+hd2)/2 : iclMax(hd1,hd2);
      }
      */


