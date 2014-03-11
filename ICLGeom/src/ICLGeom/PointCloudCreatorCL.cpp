/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/PointCloudCreatorCL.cpp            **
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

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#ifdef ICL_HAVE_OPENCL
#include <CL/cl.hpp>
#endif

#include <ICLGeom/PointCloudCreatorCL.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl {
  namespace geom {
    //OpenCL kernel code
    static char pointCloudCreatorKernel[] =
    "__kernel void                                                                                                              \n"
    "createRGB(__global float const * depthValues, __global uchar const * rIn, __global uchar const * gIn, __global uchar const * bIn, __global float4 const * dirs, float4 const m0, float4 const m1, float4 const m2, __global float4 * rgba, __global float4 * xyz, float4 const o, uint const colorW, uint const colorH, float const depthScaling, uchar const needsMapping) \n"
    "{                                                                                                                          \n"
    "    size_t id =  get_global_id(0);                                                                                         \n"
    "    float d;                                                                                                               \n"
    "    if (needsMapping==1)                                                                                                   \n"
    "    {                                                                                                                      \n"
    "       d = 1.046 * (depthValues[id]==2047 ? 0 : 1000. / (depthValues[id] * -0.0030711016 + 3.3309495161)) * depthScaling;  \n"
    "    }                                                                                                                      \n"
    "    else                                                                                                                   \n"
    "    {                                                                                                                      \n"
    "       d=depthValues[id] * depthScaling;                                                                                   \n"
    "    }                                                                                                                      \n"
    "    float4 xyzM = o+d*dirs[id];                                                                                            \n"
    "    xyzM.w=1.0;                                                                                                            \n"
    "    xyz[id]=xyzM;                                                                                                          \n"
    "    float phInv = 1.0/dot(m0,xyzM);                                                                                        \n"
    "    int px = phInv * dot(m1,xyzM);                                                                                         \n"
    "    int py = phInv * dot(m2,xyzM);                                                                                         \n"
    "    float4 rgbaD;                                                                                                          \n"
    "    if( ((uint)px) < colorW && ((uint)py) < colorH)                                                                        \n"
    "    {                                                                                                                      \n"
    "       int idx = px + colorW * py;                                                                                         \n"
    "       rgbaD.x= (float)rIn[idx]/255.;                                                                                      \n"
    "       rgbaD.y= (float)gIn[idx]/255.;                                                                                      \n"
    "       rgbaD.z= (float)bIn[idx]/255.;                                                                                      \n"
    "       rgbaD.w= 1.;                                                                                                        \n"
    "    }                                                                                                                      \n"
    "    else                                                                                                                   \n"
    "    {                                                                                                                      \n"
    "       rgbaD.x= 0.;                                                                                                        \n"
    "       rgbaD.y= 0.;                                                                                                        \n"
    "       rgbaD.z= 0.;                                                                                                        \n"
    "       rgbaD.w= 0.;                                                                                                        \n"
    "    }                                                                                                                      \n"
    "    rgba[id]=rgbaD;                                                                                                        \n"
    "}                                                                                                                          \n"
    "__kernel void                                                                                                              \n"
    "create(__global float const * depthValues, __global float4 const * dirs, __global float4 * xyz, float4 const o, float const depthScaling, uchar const needsMapping) \n"
    "{                                                                                                                          \n"
    "    size_t id =  get_global_id(0);                                                                                         \n"
    "    float d;                                                                                                               \n"
    "    if (needsMapping==1)                                                                                                   \n"
    "    {                                                                                                                      \n"
    "       d = 1.046 * (depthValues[id]==2047 ? 0 : 1000. / (depthValues[id] * -0.0030711016 + 3.3309495161)) * depthScaling;  \n"
    "    }                                                                                                                      \n"
    "    else                                                                                                                   \n"
    "    {                                                                                                                      \n"
    "       d=depthValues[id] * depthScaling;                                                                                   \n"
    "    }                                                                                                                      \n"
    "    float4 xyzM = o+d*dirs[id];                                                                                            \n"
    "    xyzM.w=1.0;                                                                                                            \n"
    "    xyz[id]=xyzM;                                                                                                          \n"
    "}                                                                                                                          \n";

    PointCloudCreatorCL::PointCloudCreatorCL(Size size, const Array2D<Vec> &dirs) {
      clReady = false;
      this->size = size;
#ifdef ICL_HAVE_OPENCL
      //create openCL context
      //      depthValuesArray=new float[size.width*size.height];
      //      rInArray=new unsigned char[size.width*size.height];
      //gInArray=new unsigned char[size.width*size.height];
      //bInArray=new unsigned char[size.width*size.height];
      //      dirsArray=new float[size.width*size.height*4];
      xyzData=new float[size.width*size.height*4];
      rgbaData=new math::FixedColVector<float, 4>[size.width*size.height];

      try {
        program = CLProgram("gpu", pointCloudCreatorKernel);
        clReady=true; //and mark CL context as available

      } catch (CLException &err) { //catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< std::endl;
        clReady = false;
      }

      if(clReady==true) { //only if CL context is available
        try {
          //create buffer for memory access and allocation
          dirsBuffer = program.createBuffer("rw", size.width*size.height * 4 * sizeof(float), dirs.begin());
          xyzBuffer = program.createBuffer("rw", size.width*size.height * 4 * sizeof(float), xyzData);
          rgbaBuffer = program.createBuffer("rw", size.width*size.height * 4 * sizeof(float), rgbaData);

          //create kernels
          kernelCreate = program.createKernel("create");
          kernelCreateRGB = program.createKernel("createRGB");
        } catch (CLException &err) { //catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< std::endl;
          clReady = false;
        }
      }

#else
      std::cout << "no openCL parallelization available" << std::endl;
      clReady = false;
#endif
    }

    PointCloudCreatorCL::~PointCloudCreatorCL() {
#ifdef ICL_HAVE_OPENCL
      //      delete[] depthValuesArray;
      //      delete[] rInArray;
      // delete[] gInArray;
      //delete[] bInArray;
      delete[] xyzData;
      delete[] rgbaData;
#endif
    }
    void PointCloudCreatorCL::setDirectionVectors(const utils::Array2D<Vec> &dirs) throw (ICLException){
      if(dirs.getSize() != this->size) throw ICLException("PointCloudCreatorCL::setDirectionVectors: size must not change");
      dirsBuffer.write(dirs.begin(), this->size.getDim()*sizeof(float)*4);
    }


    void PointCloudCreatorCL::createRGB(bool NEEDS_RAW_TO_MM_MAPPING,
                                        const Img32f *depthValues, const Mat M, const Vec O,
                                        const unsigned int COLOR_W, const unsigned int COLOR_H,
                                        const int DEPTH_DIM, DataSegment<float, 3> xyz
                                        , DataSegment<float, 4> rgba,
                                        const Img8u *rgbIn,const Array2D<Vec> &dirs, float depthScaling) {

      unsigned char needsMapping;
      if (NEEDS_RAW_TO_MM_MAPPING) {
        needsMapping = 1;
      } else {
        needsMapping = 0;
      }

#ifdef ICL_HAVE_OPENCL
      try {


        FixedColVector<float, 4> m0(M[0+4*3],M[1+4*3],M[2+4*3],M[3+4*3]);
        FixedColVector<float, 4> m1(M[0+4*0],M[1+4*0],M[2+4*0],M[3+4*0]);
        FixedColVector<float, 4> m2(M[0+4*1],M[1+4*1],M[2+4*1],M[3+4*1]);


        depthValuesBuffer = program.createBuffer("r", DEPTH_DIM * sizeof(float), depthValues->begin(0));
        rInBuffer = program.createBuffer("r", DEPTH_DIM * sizeof(unsigned char), rgbIn->begin(0));
        gInBuffer = program.createBuffer("r", DEPTH_DIM * sizeof(unsigned char), rgbIn->begin(1));
        bInBuffer = program.createBuffer("r", DEPTH_DIM * sizeof(unsigned char), rgbIn->begin(2));

        FixedColVector<float, 4> oV(O[0], O[1], O[2], O[3]);

        kernelCreateRGB.setArgs(depthValuesBuffer,
				rInBuffer,
				gInBuffer,
				bInBuffer,
				dirsBuffer,
				m0,
				m1,
				m2,
				rgbaBuffer,
				xyzBuffer,
				oV,
				COLOR_W,
				COLOR_H,
				depthScaling,
				needsMapping
                                );
        kernelCreateRGB.apply(DEPTH_DIM);

        if(xyz.getStride()==4*sizeof(float)) {
          xyzBuffer.read((float*) &xyz[0][0], DEPTH_DIM * 4 * sizeof(float));
        }
        else {
          xyzBuffer.read(xyzData, DEPTH_DIM * 4 * sizeof(float));
          DataSegment<float,3>((float*)xyzData,sizeof(float)*3,DEPTH_DIM).deepCopy(xyz); //copy pointcloud data
        }

        if(rgba.isPacked()) {
          rgbaBuffer.read((FixedColVector<float, 4>*) &rgba[0][0], DEPTH_DIM * sizeof(FixedColVector<float, 4>));
        }
        else {
          rgbaBuffer.read(rgbaData, DEPTH_DIM * sizeof(FixedColVector<float, 4>));
          DataSegment<float,4>((float*)rgbaData,sizeof(FixedColVector<float, 4>),DEPTH_DIM).deepCopy(rgba); //copy pointcloud color data
        }
      } catch (CLException &err) { //catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< std::endl;
      }
#endif
    }

    void PointCloudCreatorCL::create(bool NEEDS_RAW_TO_MM_MAPPING,
                                     const Img32f *depthValues, const Vec O, const int DEPTH_DIM,
                                     DataSegment<float, 3> xyz,const Array2D<Vec> &dirs
                                     , float depthScaling) {

      unsigned char needsMapping;
      if (NEEDS_RAW_TO_MM_MAPPING) {
        needsMapping = 1;
      } else {
        needsMapping = 0;
      }

#ifdef ICL_HAVE_OPENCL
      try {
        //        depthValuesArray = (float*)depthValues->begin(0);
        depthValuesBuffer = program.createBuffer("r", DEPTH_DIM * sizeof(float), depthValues->begin(0));
        FixedColVector<float, 4> oV(O[0], O[1], O[2], O[3]);

        kernelCreate.setArgs(depthValuesBuffer,
                             dirsBuffer,
                             xyzBuffer,
                             oV,
                             depthScaling,
                             needsMapping
                             );
        kernelCreate.apply(DEPTH_DIM);

        if(xyz.getStride()==4*sizeof(float)) {
          xyzBuffer.read((float*) &xyz[0][0], DEPTH_DIM * 4 * sizeof(float));
        }
        else {
          xyzBuffer.read(xyzData, DEPTH_DIM * 4 * sizeof(float));
          DataSegment<float,3>((float*)xyzData,sizeof(float)*3,DEPTH_DIM).deepCopy(xyz);//copy pointcloud data
        }
      } catch (CLException &err) { //catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< std::endl;
      }
#endif
    }

    bool PointCloudCreatorCL::isCLReady() {
      return clReady;
    }

  } // namespace geom
}
