// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLCore/Img.h>
#include <ICLMath/FixedVector.h>

#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLBuffer.h>
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
      ICLGeom_API PointCloudCreatorCL(utils::Size size, const utils::Array2D<Vec> &dirs);

      ///Destructor
      ICLGeom_API ~PointCloudCreatorCL();

      /// updates the internally used direction vectors
      /** the underlying chip size must not change, otherwise, and exception is thrown */
      ICLGeom_API void setDirectionVectors(const utils::Array2D<Vec> &dirs);

      ///Creates a uncolored pointcloud (called from PointCloudCreator)
      ICLGeom_API void create(bool NEEDS_RAW_TO_MM_MAPPING,const core::Img32f *depthValues,
                        const Vec O, const int DEPTH_DIM,
                        core::DataSegment<float,3> xyz, const utils::Array2D<Vec> &dirs, float depthScaling);

      ///Creates a RGBD-mapped pointcloud (called from PointCloudCreator)
      ICLGeom_API void createRGB(bool NEEDS_RAW_TO_MM_MAPPING, const core::Img32f *depthValues, const Mat M,
                           const Vec O, const unsigned int COLOR_W, const unsigned int COLOR_H, const int DEPTH_DIM,
                           core::DataSegment<float,3> xyz, core::DataSegment<float,4> rgba,
                           const core::Img8u *rgbIn,const utils::Array2D<Vec> &dirs, float depthScaling);

      /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
      /**        @return openCL context ready/unavailable */
      ICLGeom_API bool isCLReady();

     private:
      bool clReady;
      utils::Size size;
    #ifdef ICL_HAVE_OPENCL
      //OpenCL data

      //float* depthValuesArray;
      //	  unsigned char* rInArray;
      //unsigned char* gInArray;
      //unsigned char* bInArray;
  	  float* xyzData;
  	  math::FixedColVector<float, 4>* rgbaData;

      //OpenCL
      utils::CLProgram program;

      utils::CLKernel kernelCreate;
      utils::CLKernel kernelCreateRGB;

      utils::CLBuffer depthValuesBuffer;
      utils::CLBuffer matrixBuffer;
      utils::CLBuffer xyzBuffer;
      utils::CLBuffer rgbaBuffer;
      utils::CLBuffer rInBuffer;
      utils::CLBuffer gInBuffer;
      utils::CLBuffer bInBuffer;
      utils::CLBuffer dirsBuffer;

    #endif
    };
  } // namespace geom
}
