// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/ChamferOp.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/Point.h>

#include <limits>
#include <cmath>
#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    namespace{
      inline icl32s min3(icl32s a, icl32s b, icl32s c){

        return iclMin(iclMin(a,b),c);
      }

      inline icl32s min5(icl32s a, icl32s b, icl32s c, icl32s d, icl32s e){

        return min3(min3(a,b,c),d,e);
      }


      namespace co{
        inline Size scale(const Size &s, int f){
          return Size( static_cast<int>(ceil(float(s.width)/f)), static_cast<int>(ceil(float(s.height)/f)) );
        }
        inline Point scale(const Point &s, int f){
          return Point( static_cast<int>(ceil(float(s.x)/f)), static_cast<int>(ceil(float(s.y)/f)) );
        }
        inline Rect scale(const Rect &r, int f){
          return Rect(scale(r.ul(),f), scale(r.getSize(),f) );
        }
      }

      template<class T>
      void prepare_chamfer_image_unscaled(const Img<T> &src, Img32s &dst, int channel, icl32s maxVal){
        visitROILinesWith(src, channel, dst, channel,
          [maxVal](const T *s, icl32s *d, int w) {
            for(int i = 0; i < w; ++i) {
              d[i] = s[i] ? 0 : maxVal;
            }
          });
      }

      template<class T>
      void prepare_chamfer_image_scaled(const Img<T> &src, Img32s &dst, int channel, icl32s maxVal, int scaleFactor){
        ICLASSERT_RETURN(co::scale(src.getROISize(),scaleFactor) == dst.getROISize());

        dst.clear(channel,maxVal,true);

        if(src.hasFullROI() && dst.hasFullROI()){
          int xEnd = src.getWidth();
          int yEnd = src.getHeight();
          const Channel<T> srcCh = src[channel];
          Channel32s dstCh = dst[channel];
          for(int x=0;x<xEnd;++x){
            for(int y=0;y<yEnd;++y){
              if(srcCh(x,y)) dstCh(x/scaleFactor,y/scaleFactor) = 0;
            }
          }
        }else{
          int xSrcStart = src.getROI().x;
          int xSrcEnd = src.getROI().right();
          int ySrcStart = src.getROI().y;
          int ySrcEnd = src.getROI().bottom();
          int xDstStart = dst.getROI().x;
          int yDstStart = dst.getROI().y;

          const Channel<T> srcCh = src[channel];
          Channel32s dstCh = dst[channel];

          for(int x=xSrcStart;x<xSrcEnd;++x){
            for(int y=ySrcStart;y<ySrcEnd;++y){
              if(srcCh(x,y)) dstCh(xDstStart + (x-xSrcStart)/scaleFactor,yDstStart + (y-ySrcStart)/scaleFactor ) = 0;
            }
          }
        }
      }




      template<class T>
      void prepare_chamfer_image(const Img<T> &src, Img32s &dst, int channel, icl32s maxVal, int scaleFactor, Img32s &buffer, bool scaleUpResult){
        if(scaleFactor > 1){
          Img32s &buf = scaleUpResult ? buffer : dst;
          ImgBase *bufPtr = &buf;
          ensureCompatible(&bufPtr,depth32s,co::scale(src.getSize(),scaleFactor),
                           src.getChannels(),src.getFormat(),co::scale(src.getROI(),scaleFactor));
          prepare_chamfer_image_scaled(src, *bufPtr->asImg<icl32s>(), channel, maxVal, scaleFactor);
        }else{
          prepare_chamfer_image_unscaled(src, dst, channel, maxVal);
        }
      }

      void apply_chamfer_op_generic(Img32s &img, int channel, icl32s d1, icl32s d2){
        if(img.hasFullROI()){
          int w = img.getWidth();
          int h = img.getHeight();
          Channel32s dst = img[channel];

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
          Channel32s dst = img[channel];
          Rect r = img.getROI();

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
      int compute_max_val(icl32s d1, icl32s d2, const Size &imageSize){

        return iclMax(d1,d2)*(imageSize.width+imageSize.height);
      }


    }

    ChamferOp::ChamferOp( icl32s horizontalAndVerticalNeighbourDistance, icl32s diagonalNeighborDistance, int scaleFactor, bool scaleUpResult)

      :m_iHorizontalAndVerticalNeighbourDistance(horizontalAndVerticalNeighbourDistance),
       m_iDiagonalNeighborDistance(diagonalNeighborDistance),
       m_iScaleFactor(scaleFactor),
       m_bScaleUpResult(scaleUpResult){
      setClipToROI(false);
    }




    void ChamferOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(!src.isNull());

      Size dstSize;
      Rect dstROI;
      if( (m_iScaleFactor == 1) || m_bScaleUpResult ){
        dstSize = src.getSize();
        dstROI = src.getROI();
      }else{
        dstSize = co::scale(src.getSize(),m_iScaleFactor);
        dstROI = co::scale(src.getROI(),m_iScaleFactor);
      }
      if(!prepare(dst, depth32s, dstSize, src.getFormat(), src.getChannels(), dstROI, src.getTime())){
        ERROR_LOG("unable to prepare image");
        return;
      }
      int C = dst.getChannels();
      Img32s &d = dst.as32s();

      icl32s d1 = m_iHorizontalAndVerticalNeighbourDistance;
      icl32s d2 = m_iDiagonalNeighborDistance;
      icl32s maxVal = compute_max_val(d1,d2,src.getSize());

      src.visit([&](const auto &s) {
        for(int c = 0; c < C; ++c){
          prepare_chamfer_image(s, d, c, maxVal, m_iScaleFactor, m_oBufferImage, m_bScaleUpResult);
        }
      });

      if(m_iScaleFactor == 1 || !m_bScaleUpResult){
        for(int c = 0; c < C; ++c){
          apply_chamfer_op_generic(d, c, d1, d2);
        }
      }else{
        for(int c = 0; c < C; ++c){
          apply_chamfer_op_generic(m_oBufferImage, c, d1, d2);
        }
        m_oBufferImage.scaledCopyROI(&d);
      }
    }



    namespace{
      struct PenaltyModeNone{

        PenaltyModeNone( const Rect &roi, icl32s penalty):
          roi(roi),penalty(penalty){}
        inline icl32s operator()(icl32s val, int x, int y) const {(void)x; (void)y;  return val; }
        Rect roi;
        icl32s penalty;
      };

      struct PenaltyModeConst{

        PenaltyModeConst( const Rect &roi,icl32s penalty):
          roi(roi),penalty(penalty){}
        inline icl32s operator() (icl32s val, int x, int y) const { return roi.contains(x,y) ? val : penalty; }
        Rect roi;
        icl32s penalty;
      };

      struct PenaltyModeDist{

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


      struct HausdorffMetricModeMean{

        HausdorffMetricModeMean():n(0),val(0){}
        inline void operator<<(icl32s x){ val+=x; n++; }
        double getResult() const{ return n ? val/n : -1; }
        int n;
        double val;
      };

      struct HausdorffMetricModeMax{

        HausdorffMetricModeMax():val(-1){}
        inline void operator<<(icl32s x){ val = iclMax(val,x); }
        double getResult() const{ return val; }
        icl32s val;
      };


      template<class HausdorffMetricMode, class PenaltyMode>
      inline double apply_directed_hausdorff_distance(const Img32s *chamferImage, const std::vector<Point> &model, HausdorffMetricMode hmm,PenaltyMode pm){

        Channel32s chan = (*chamferImage)[0];
        int x,y;
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


      template<class HausdorffMetricMode, class PenaltyMode>
      inline double apply_directed_hausdorff_distance_2(const Img32s *chamferImage, const Img32s *modelChamferImage, HausdorffMetricMode hmm,PenaltyMode pm){

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

    }

    void ChamferOp::renderModel(const std::vector<Point> &model, ImgBase **image, const Size &size, icl32s bg, icl32s fg,  Rect roi){

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


    double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage,
                                                       const std::vector<Point> &model,
                                                       ChamferOp::hausdorffMetric m,
                                                       ChamferOp::outerROIPenaltyMode pm,
                                                       icl32s penaltyValue){
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


    double ChamferOp::computeDirectedHausdorffDistance(const Img32s *chamferImage,
                                                       const Img32s *modelChamferImage,
                                                       ChamferOp::hausdorffMetric m,
                                                       ChamferOp::outerROIPenaltyMode pm,
                                                       icl32s penaltyValue){
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


    double ChamferOp::computeSymmetricHausdorffDistance(const Img32s *chamferImageA,
                                                        const Img32s *chamferImageB,
                                                        hausdorffMetric m,
                                                        ChamferOp::outerROIPenaltyMode pm,
                                                        icl32s penaltyValue){

      double ab = computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m,pm,penaltyValue);
      double ba = computeDirectedHausdorffDistance(chamferImageB,chamferImageA,m,pm,penaltyValue);
      return m==hausdorff_mean ? (ab+ba)/2 : iclMax(ab,ba);
    }




    double ChamferOp::computeSymmetricHausdorffDistance(const std::vector<Point> setA, const Size &sizeA, const Rect &roiA, ImgBase **bufferA,
                                                        const std::vector<Point> setB, const Size &sizeB, const Rect &roiB, ImgBase **bufferB,
                                                        hausdorffMetric m,ChamferOp::outerROIPenaltyMode pm,icl32s penaltyValue,
                                                        ChamferOp coA, ChamferOp coB){

      renderModel(setA,bufferA,sizeA,0,255,roiA);
      renderModel(setB,bufferB,sizeB,0,255,roiB);
      coA.apply(*bufferA,bufferA);
      coB.apply(*bufferB,bufferB);
      return computeSymmetricHausdorffDistance((*bufferA)->asImg<icl32s>(),(*bufferB)->asImg<icl32s>(),m,pm,penaltyValue);
    }


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
  
  } // namespace filter

}
