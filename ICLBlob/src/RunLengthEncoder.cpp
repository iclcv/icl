/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/src/RunLengthEncoder.cpp                       **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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


#include <ICLBlob/RunLengthEncoder.h>
#include <ICLBlob/RegionDetectorTools.h>


namespace icl{

  using namespace region_detector_tools;
  
  
  void RunLengthEncoder::prepare(const Rect &roi){
    if(m_imageROI == roi) return;
    m_data.resize(roi.width*roi.height);
    m_ends.resize(roi.height,0);
    m_imageROI = roi;
  }

  void RunLengthEncoder::resetLineSegments(){
    for(int y=0;y<m_imageROI.height;++y){
      std::for_each(begin(y),end(y),std::mem_fun_ref(&WorkingLineSegment::reset));
    }
  }
  
  template<class T>
  void RunLengthEncoder::encode_internal(const Img<T> &image){
    if(image.hasFullROI()){
      // optimized version form images without ROI
      const int W = image.getWidth();
      const int H = image.getHeight();
      
      WLS *sldata = m_data.data(), *sls = 0;
      
      const T *p = image.begin(0);
      const T *pEnd(0),*pLast(0),*pBegin(0);
      T curr(0);
      
      for(int y=0;y<H;++y){
        pBegin = p;    // pixel pointer to current image line begin
        pEnd = p+W;    // pixel pointer to current image line end
        curr = *p++;  
        sls = sldata+y*W;
        pLast = p-1;
        while(p<pEnd){
          p = find_first_not(p,pEnd,curr);
          sls->init((int)(pLast-pBegin), y,(int)(p-pBegin),curr); 
          ++sls;
          curr = *p; 
          pLast = p;
        }
        m_ends[y] = sls;
      }


    }else{
      // ROI-version (slighly slower due to some overhead for roi-handling 
      // and shifting of scanlines)
      const Rect &roi = image.getROI();
      
      const int rx = roi.x;
      const int ry = roi.y;
      const int rw = roi.width;
      const int rh = roi.height;
      
      WLS *sldata = m_data.data(), *sls = 0;
      
      const T *p = &*image.beginROI(0);
      const T *pEnd(0),*pLast(0),*pBegin(0);
      T curr(0);
      
      const int yEnd = ry+rh;
      const int xStep = image.getWidth()-rw;
      for(int y=ry;y<yEnd;++y){
        pBegin = p;    // pixel pointer to current image line begin
        pEnd = p+rw;   // pixel pointer to current image line end
        curr = *p++;  
        sls = sldata+rw*(y-ry);
        pLast = p-1;
        while(p<pEnd){
          p = find_first_not(p,pEnd,curr);
          sls->init(rx+(int)(pLast-pBegin), y,rx+(int)(p-pBegin),curr); 
          ++sls;
          curr = *p; 
          pLast = p;
        }
        p += xStep;
        m_ends[y-ry] = sls;
      }
    }
  }
  
  void RunLengthEncoder::encode(const ImgBase *image){
    ICLASSERT_THROW(image,ICLException(" RunLengthEncoder::encode :image is NULL"));

    resetLineSegments();
    prepare(image->getROI());
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: encode_internal<icl##D>(*image->asImg<icl##D>());  break;
      ICL_INSTANTIATE_ALL_INT_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default:
        ICL_INVALID_DEPTH;
    }
  }

}
