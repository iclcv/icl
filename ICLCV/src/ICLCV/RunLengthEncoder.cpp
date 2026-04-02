// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#include <ICLCV/RunLengthEncoder.h>
#include <ICLCV/RegionDetectorTools.h>



//static char KernelText[] = "  __kernel void convolve(...)";
//CLProgram program("gpu", KernelText);
//program.listSelectedDevice();
//CLBuffer inputBuffer = program.createBuffer("r",sizeof(sizeof(unsigned int)) * inputSignalHeight * inputSignalWidth,inputSignal);
//CLBuffer outputSignalBuffer = program.createBuffer("w",sizeof(unsigned int) * outputSignalHeight * outputSignalWidth);
//CLKernel kernel = program.createKernel("convolve");
//kernel.setArgs(inputSignalBuffer, outputSignalBuffer, inputSignalWidth, maskWidth);
//kernel.apply(outputSignalWidth * outputSignalHeight, 0, 0);
//outputSignalBuffer.read(outputSignal,outputSignalWidth * outputSignalHeight * sizeof(unsigned int));

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {
    using namespace region_detector_tools;


    void RunLengthEncoder::prepare(const Rect &roi){
      if(m_imageROI == roi) return;
      m_data.resize(roi.width*roi.height);
      m_ends.resize(roi.height,0);
      m_imageROI = roi;
    }

    void RunLengthEncoder::resetLineSegments(){
      for(int y=0;y<m_imageROI.height;++y){
        std::for_each(begin(y),end(y),[](WorkingLineSegment &s){ s.reset(); });
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
          while(true){//p<pEnd){
            p = find_first_not(p,pEnd,curr);
            sls->init(static_cast<int>(pLast-pBegin), y,static_cast<int>(p-pBegin),curr);
            ++sls;
            if(p == pEnd) break;
            pLast = p;
            curr = *p;
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
            sls->init(rx+static_cast<int>(pLast-pBegin), y,rx+static_cast<int>(p-pBegin),curr);
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

  } // namespace icl::cv