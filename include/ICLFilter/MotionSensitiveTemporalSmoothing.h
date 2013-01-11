/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/MotionSensitiveTemporalSmoothing.h   **
** Module : ICLFilter                                              **
** Authors: Andre Ueckermann                                       **
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

#pragma once

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

#ifdef HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

namespace icl{
  namespace filter{
  
    class TemporalSmoothingCL{
      public:
      
      /// creates a new TemporalSmoothingCL with given parameters (implementation of the MotionSensitiveTemporalSmoothing Filter)
      /** @param size size of the input image 
          @param depth depth of the image (32f and 8u supported)
          @param maxFilterSize maximum size of the filter */
      TemporalSmoothingCL(utils::Size size, core::depth depth, int iMaxFilterSize, int iNullValue);
  
      ///Destructor
      ~TemporalSmoothingCL();
      
      /// Execution of the temporal smoothing for float images
      /** @param inputImage the next input image for the smoothing sequence
          @return the smoothed image */ 
      core::Img32f temporalSmoothingF(core::Img32f &inputImage);
      
      /// Execution of the temporal smoothing for uchar images
      /** @param inputImage the next input image for the smoothing sequence
          @return the smoothed image */
      core::Img8u temporalSmoothingC(core::Img8u &inputImage);
      
      /// Sets openCL enabled/disabled. Enabling has no effect if no openCL context is available. (default true=enabled)
      /**        @param use enable/disable openCL */
	    void setUseCL(bool use);
	    
	    /// Sets the filter size (smaller than maxFilterSize in Constructor)
	    /** @param iFilterSize the filter size */
	    void setFilterSize(int iFilterSize);
	    
	    ///Sets the difference separating noise from movement (smaller=noise, bigger=movement)
	    /**       @param iDifference the difference */
	    void setDifference(int iDifference);
	    
	    ///Returns the motionImage (visualize the movement in the image, usable as motion detector)
	    /**   @return the motion image */
	    core::Img32f getMotionImage();
	    
	    /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
      /**        @return openCL context ready/unavailable */
	    bool isCLReady();
	    
	    /// Returns the openCL activation status (true=openCL enabled, false=openCL disabled). The status can be set by setUseCL(bool use).
      /**        @return openCL enabled/disabled */
	    bool isCLActive();
	    	  
     private:
     
      int w,h;
      core::depth d;
      bool clReady;
      bool useCL;
      
      int imgCount;
      
      int filterSize;
      int currentFilterSize;
      int maxFilterSize;
      
      int currentDifference;
      int nullValue;
        
      std::vector<core::Img32f> inputImagesF;
      core::Img32f outputImageF;         
      std::vector<core::Img8u> inputImagesC;
      core::Img8u outputImageC; 
      core::Img32f motionImage;     
     	
    #ifdef HAVE_OPENCL
      //OpenCL    
      float* inputImage1ArrayF;
      float* inputImagesArrayF;
      float* outputImageArrayF;
      
      cl_uchar* inputImage1ArrayC;
      cl_uchar* inputImagesArrayC;
      cl_uchar* outputImageArrayC;
      
      float* motionImageArray;
      
      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;
      
      cl::Kernel kernelTemporalSmoothingFloat;
      cl::Kernel kernelTemporalSmoothingChar;
      
      cl::Buffer inputImageBufferF;      
      cl::Buffer outputImageBufferF;
      cl::Buffer inputImageBufferC;
      cl::Buffer outputImageBufferC;
      cl::Buffer motionImageBuffer;
      
    #endif
  
    };
  
    class MotionSensitiveTemporalSmoothing : public UnaryOp, public utils::Uncopyable{
      public:
      
      /// creates a new MotionSensitiveTemporalSmoothing filter with given parameters
      /** @param iNullValue the value with no image information (e.g. Kinect data) -1=no nullValues
          @param iMaxFilterSize the maximum size of the filter */
      MotionSensitiveTemporalSmoothing(int iNullValue, int iMaxFilterSize);
  
      ///Destructor
      ~MotionSensitiveTemporalSmoothing();
      
      ///applies the MotionSensitiveTemporalSmoothing
      /**
          @param src the source image
          @param dst pointer to the destination image
      */
      virtual void apply(const core::ImgBase *src, core::ImgBase **dst);
          
      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;
  
      /// Sets openCL enabled/disabled. Enabling has no effect if no openCL context is available. (default true=enabled)
      /**        @param use enable/disable openCL */
	    void setUseCL(bool use);
	    
	    ///Sets the filter size (smaller than maxFilterSize in Constructor)
	    /**       @param filterSize the filter size */
	    void setFilterSize(int filterSize);
	    
	    ///Sets the difference separating noise from movement (smaller=noise, bigger=movement)
	    /**       @param difference the difference */
	    void setDifference(int difference);
	    
	    ///Returns the motionImage (visualize the movement in the image, usable as motion detector)
	    /**   @return the motion image */
	    core::Img32f getMotionImage();
	     
	    /// Returns the openCL activation status (true=openCL enabled, false=openCL disabled). The status can be set by setUseCL(bool use).
      /**        @return openCL enabled/disabled */
	    bool isCLActive();
	    	  
     private:
     
      void init(int iChannels, core::depth iDepth, utils::Size iSize);
	    
      bool useCL;
      
      int currentFilterSize;
      int currentDifference;
      
      int nullValue;
      int maxFilterSize;
       
      int numChannels;
      utils::Size size;
      core::depth depth;
 
      std::vector<TemporalSmoothingCL*> clPointer;     
    };
    
  } // namespace filter
}
