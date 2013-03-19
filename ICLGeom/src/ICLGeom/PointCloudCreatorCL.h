/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudCreatorCL.h              **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLCore/Img.h>

#ifdef HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

namespace icl{
  namespace geom{
    /**
       This class is used in PointCloudCreator for GPU-parallelized pointcloud creation*/
    class PointCloudCreatorCL{
  	
     public:
      ///Constructor
      /** Constructs an object of this class.
          @param size size of the input core::depth image and output pointcloud
          @param dirs view ray directions calculated in PointCloudCreator*/
      PointCloudCreatorCL(utils::Size size, const utils::Array2D<Vec> &dirs); 
  	
      ///Destructor
      ~PointCloudCreatorCL();
  	  	
  	  ///Creates a uncolored pointcloud (called from PointCloudCreator)
  	  void create(bool NEEDS_RAW_TO_MM_MAPPING,const core::Img32f *depthValues, 
                           const Vec O, const int DEPTH_DIM, 
                           DataSegment<float,3> xyz, const utils::Array2D<Vec> &dirs, float depthScaling);
      
      ///Creates a RGBD-mapped pointcloud (called from PointCloudCreator)
      void createRGB(bool NEEDS_RAW_TO_MM_MAPPING,const core::Img32f *depthValues, const Mat M, 
                           const Vec O, const unsigned int COLOR_W, const unsigned int COLOR_H, const int DEPTH_DIM, 
                           DataSegment<float,3> xyz, DataSegment<float,4> rgba,
                           const core::Img8u *rgbIn,const utils::Array2D<Vec> &dirs, float depthScaling);
  	
      /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
      /**        @return openCL context ready/unavailable */
      bool isCLReady();
  	  	
     private:
      bool clReady;

    #ifdef HAVE_OPENCL
      //OpenCL data
  	  float* depthValuesArray;
  	  cl_uchar* rInArray;
  	  cl_uchar* gInArray;
  	  cl_uchar* bInArray;
  	  float* dirsArray;
  	  float* xyzData;
  	  cl_float4* rgbaData;
  	
      //OpenCL    
      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;
        
      cl::Kernel kernelCreate;
      cl::Kernel kernelCreateRGB;

      //OpenCL buffer      
      cl::Buffer depthValuesBuffer;
      cl::Buffer matrixBuffer;
      cl::Buffer xyzBuffer;
      cl::Buffer rgbaBuffer;                    
      cl::Buffer rInBuffer;
      cl::Buffer gInBuffer;
      cl::Buffer bInBuffer;
      cl::Buffer dirsBuffer;                       
    #endif
    };
  } // namespace geom
}
