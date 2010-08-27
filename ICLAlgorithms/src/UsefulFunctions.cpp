/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/src/UsefulFunctions.cpp                  **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLFilter/ProximityOp.h>
#include <ICLFilter/BinaryLogicalOp.h>
#ifdef HAVE_IPP
#include <ippi.h>
#endif 

#include <ICLQuick/Quick.h>
namespace icl{

  namespace{
    template<int N,typename BinaryCompare>
    inline void apply_inplace_threshold(Img8u &image,int dim, icl8u thresh, BinaryCompare cmp){
      int threshN = N*thresh;
      icl8u *pc[N];
      for(int j=0;j<N;++j){
        pc[j] = image.getData(j);
      }
      for(int i=0;i<dim;i++){
        int sumVal = 0;
        for(int j=0;j<N;++j){
          sumVal += pc[j][i];
        }
        pc[0][i] = 255 * cmp(sumVal,threshN);
      }          
    }

    template<int N,typename BinaryCompare>
    inline void apply_inplace_threshold_roi(Img8u &image, icl8u thresh, BinaryCompare cmp){
      const Rect &r = image.getROI();
      int xStart = r.x;
      int xEnd = r.right();
      int yStart = r.y;
      int yEnd = r.bottom();
      int w = image.getWidth();
      
      int threshN = N*thresh;
      icl8u *pc[N];
      for(int j=0;j<N;++j){
        pc[j] = image.getData(j);
      }

      for(int x=xStart;x<xEnd;++x){
        for(int y=yStart;y<yEnd;++y){
          int sumVal = 0;
          int idx = x+w*y;
          for(int i=0;i<N;++i){
            sumVal += pc[i][idx];
          }
          pc[0][idx] = 255 * cmp(sumVal,threshN);
        }          
      }
    }
  }
  
  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u &templ, 
                                     float significance,
                                     Img8u *bufferGiven,
                                     bool clipBuffersToROI,
                                     RegionDetector *rdGiven,
                                     bool useCrossCorrCoeffInsteadOfSqrDistance){    

    //DEBUG_LOG("src:" << src << "\ntempl:" << templ);
    Size bufSize = src.getROISize()-templ.getROISize()+Size(1,1);
    ICLASSERT_RETURN_VAL(bufSize.width > 0 && bufSize.height > 0, std::vector<Rect>());

    Img8u *useBuffer = bufferGiven ? bufferGiven : new Img8u;
    RegionDetector *useRD = rdGiven ? rdGiven : new RegionDetector;


    useBuffer->setChannels(src.getChannels());
    if(clipBuffersToROI){
      useBuffer->setSize(bufSize);
    }else{
      useBuffer->setSize(src.getSize());
      Point bufOffs = src.getROIOffset();
      bufOffs.x += templ.getROISize().width/2;
      bufOffs.y += templ.getROISize().height/2;
      useBuffer->setROI(Rect(bufOffs,bufSize));
    }
    for(int i=0;i<src.getChannels();i++){
#ifdef HAVE_IPP
      if(useCrossCorrCoeffInsteadOfSqrDistance){
        ippiCrossCorrValid_Norm_8u_C1RSfs(src.getROIData(i),src.getLineStep(),
                                          src.getROISize(), templ.getROIData(i),
                                          templ.getLineStep(),templ.getROISize(),
                                          useBuffer->getROIData(i),
                                          useBuffer->getLineStep(),-8);
      }else{
        ippiSqrDistanceValid_Norm_8u_C1RSfs(src.getROIData(i),src.getLineStep(),
                                            src.getROISize(), templ.getROIData(i),
                                            templ.getLineStep(),templ.getROISize(),
                                            useBuffer->getROIData(i),
                                            useBuffer->getLineStep(),-8);
      }
#else


  ERROR_LOG("not supported without IPP");
#endif
    }    

    Img8u &m = *useBuffer;
    
    //    show(cvt(m));

    icl8u t = useCrossCorrCoeffInsteadOfSqrDistance ? (icl8u)(float(255)*significance) : 255 - (icl8u)(float(255)*significance);
    
    if(m.hasFullROI()){
      if(!useCrossCorrCoeffInsteadOfSqrDistance){
        switch(m.getChannels()){
          case 1: apply_inplace_threshold<1,std::less<int> >(m,m.getDim(),t,std::less<int>());  break;
          case 2: apply_inplace_threshold<2,std::less<int> >(m,m.getDim(),t,std::less<int>());  break;
          case 3: apply_inplace_threshold<3,std::less<int> >(m,m.getDim(),t,std::less<int>());  break;
          default: 
            ERROR_LOG("this function is only supported for 1,2 and 3 channel images (channel count was " << m.getChannels() << ")");
            return std::vector<Rect>();
        }
      }else{
        switch(m.getChannels()){
          case 1: apply_inplace_threshold<1,std::greater<int> >(m,m.getDim(),t,std::greater<int>());  break;
          case 2: apply_inplace_threshold<2,std::greater<int> >(m,m.getDim(),t,std::greater<int>());  break;
          case 3: apply_inplace_threshold<3,std::greater<int> >(m,m.getDim(),t,std::greater<int>());  break;
          default: 
            ERROR_LOG("this function is only supported for 1,2 and 3 channel images (channel count was " << m.getChannels() << ")");
            return std::vector<Rect>();
        }
      }
    }else{
      if(!useCrossCorrCoeffInsteadOfSqrDistance){
        switch(m.getChannels()){
          case 1: apply_inplace_threshold_roi<1,std::less<int> >(m,t,std::less<int>());  break;
          case 2: apply_inplace_threshold_roi<2,std::less<int> >(m,t,std::less<int>());  break;
          case 3: apply_inplace_threshold_roi<3,std::less<int> >(m,t,std::less<int>());  break;
          default: 
            ERROR_LOG("this function is only supported for 1,2 and 3 channel images (channel count was " << m.getChannels() << ")");
            return std::vector<Rect>();
        }
      }else{
        switch(m.getChannels()){
          case 1: apply_inplace_threshold_roi<1,std::greater<int> >(m,t,std::greater<int>());  break;
          case 2: apply_inplace_threshold_roi<2,std::greater<int> >(m,t,std::greater<int>());  break;
          case 3: apply_inplace_threshold_roi<3,std::greater<int> >(m,t,std::greater<int>());  break;
          default: 
            ERROR_LOG("this function is only supported for 1,2 and 3 channel images (channel count was " << m.getChannels() << ")");
            return std::vector<Rect>();
        }
      }
    }

    
    Img8u c0 = p2o(useBuffer->selectChannel(0));

    //    show(norm(cvt(c0)));
    useRD->setConstraints(0,2<<20,1,255);
    const std::vector<ImageRegion> &blobData = useRD->detect(&c0);
    std::vector<Rect> resultVec(blobData.size());
    
    Point halfTemplROI(templ.getROISize().width/2,templ.getROISize().height/2);
    Point offs = src.getROIOffset()+halfTemplROI+Point(1,1);
    Rect templRect(halfTemplROI * (-1),templ.getROISize());
    templRect += offs;
    for(unsigned int i=0;i<blobData.size();i++){
      if(clipBuffersToROI){
        resultVec[i] =  (templRect + blobData[i].getCOG()) & src.getImageRect();
      }else{
        resultVec[i] = ( Rect(blobData[i].getCOG(),templ.getROISize())-halfTemplROI ) & src.getImageRect();
      }
      // const Img8u *tmp = src.shallowCopy(resultVec[i]);
      // show(copyroi(cvt(tmp)));
      // delete tmp;
    }
    
    
    if(!rdGiven) delete useRD;
    if(!bufferGiven) delete useBuffer;
    return resultVec;
  }
                


  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u *srcMask,
                                     const Img8u &templ, 
                                     const Img8u *templMask,
                                     float significance,
                                     Img8u *srcBuffer,
                                     Img8u *templBuffer,
                                     Img8u *buffer,
                                     bool clipBuffersToROI,
                                     RegionDetector *rd,
                                     bool useCrossCorrCoeffInsteadOfSqrDistance){
    Img8u *useSrcBuffer = 0;
    Img8u *useTemplBuffer = 0;
    if(srcMask){
      useSrcBuffer = srcBuffer ? srcBuffer : new Img8u;
      Img8u srcMaskRepChannel(ImgParams(srcMask->getSize(),0,srcMask->getROI()));
      for(int i=0;i<src.getChannels()/srcMask->getChannels();i++){
        srcMaskRepChannel.append(const_cast<Img8u*>(srcMask));
      }
      BinaryLogicalOp binop(BinaryLogicalOp::andOp);
      binop.setClipToROI(clipBuffersToROI);
      binop.apply(&src,&srcMaskRepChannel,bpp(useSrcBuffer));
    }else{
      useSrcBuffer = const_cast<Img8u*>(&src);
    }
    if(templMask){
      useTemplBuffer = templBuffer ? templBuffer : new Img8u;

      Img8u templMaskRepChannel(ImgParams(templMask->getSize(),0,templMask->getROI()));
      for(int i=0;i<templ.getChannels()/templMask->getChannels();i++){
        templMaskRepChannel.append(const_cast<Img8u*>(templMask));
      }
      
      BinaryLogicalOp binop(BinaryLogicalOp::andOp);
      binop.setClipToROI(clipBuffersToROI);
      binop.apply(&templ,&templMaskRepChannel,bpp(useTemplBuffer));
    }else{
      useTemplBuffer = const_cast<Img8u*>(&templ);
    }
    
    std::vector<Rect> results = iclMatchTemplate(*useSrcBuffer,
                                                 *useTemplBuffer,
                                                 significance,
                                                 buffer,
                                                 clipBuffersToROI,
                                                 rd,
                                                 useCrossCorrCoeffInsteadOfSqrDistance);
    
    if(clipBuffersToROI && srcMask){
      DEBUG_LOG("");
      for(unsigned int i=0;i<results.size();++i){
        results[i]+=src.getROIOffset();
      }
    }
    
    if(!srcBuffer && srcMask) delete useSrcBuffer;
    if(!templBuffer && templMask) delete useTemplBuffer;
    
    return results;
  }
                     
  
  
}

