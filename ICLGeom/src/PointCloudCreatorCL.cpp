/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PointCloudCreatorCL.cpp                    **
** Module : ICLGeom                                                **
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

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#endif

#include <ICLGeom/PointCloudCreatorCL.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace geom{
    //OpenCL kernel code
    static char pointCloudCreatorKernel[] =
      "__kernel void                                                                                                              \n"
      "createRGB(__global float const * depthValues, __global uchar const * rIn, __global uchar const * gIn, __global uchar const * bIn, __global float const * dirs, __global float const * m, __global float4 * rgba, __global float * xyz, float4 const o, uint const colorW, uint const colorH, float const depthScaling, uchar const needsMapping) \n"
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
      "    float3 xyzM;                                                                                                           \n"
      "    xyzM.x = o.x + d * dirs[id*3+0];                                                                                       \n" 
      "    xyzM.y = o.y + d * dirs[id*3+1];                                                                                       \n"
      "    xyzM.z = o.z + d * dirs[id*3+2];                                                                                       \n"
      "    xyz[id*3+0] = xyzM.x;                                                                                                  \n"
      "    xyz[id*3+1] = xyzM.y;                                                                                                  \n"
      "    xyz[id*3+2] = xyzM.z;                                                                                                  \n"
      "    float phInv = 1.0/ ( m[0+4*3] * xyzM.x + m[1+4*3] * xyzM.y + m[2+4*3] * xyzM.z + m[3+4*3] );                           \n"
      "    int px = phInv * ( m[0+4*0] * xyzM.x + m[1+4*0] * xyzM.y + m[2+4*0] * xyzM.z + m[3+4*0] );                             \n"
      "    int py = phInv * ( m[0+4*1] * xyzM.x + m[1+4*1] * xyzM.y + m[2+4*1] * xyzM.z + m[3+4*1] );                             \n"
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
      "create(__global float const * depthValues, __global float const * dirs, __global float * xyz, float4 const o, float const depthScaling, uchar const needsMapping) \n"
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
      "    xyz[id*3+0] = o.x + d * dirs[id*3+0];                                                                                  \n" 
      "    xyz[id*3+1] = o.y + d * dirs[id*3+1];                                                                                  \n"
      "    xyz[id*3+2] = o.z + d * dirs[id*3+2];                                                                                  \n"
      "}                                                                                                                          \n"
      ;
  
                   
    PointCloudCreatorCL::PointCloudCreatorCL(Size size, const Array2D<Vec3> &dirs){
      clReady=false;
    #ifdef HAVE_OPENCL
      //create openCL context
      depthValuesArray=new float[size.width*size.height];
  	  rInArray=new cl_uchar[size.width*size.height];
  	  gInArray=new cl_uchar[size.width*size.height];
  	  bInArray=new cl_uchar[size.width*size.height];
  	  dirsArray=new float[size.width*size.height*3];
  	  xyzData=new float[size.width*size.height*3];
  	  rgbaData=new cl_float4[size.width*size.height];
  	  
      std::vector<cl::Platform> platformList;//get number of available openCL platforms
      int selectedDevice=0;//initially select platform 0
      try{
        if(cl::Platform::get(&platformList)==CL_SUCCESS){
          std::cout<<"openCL platform found"<<std::endl;
        }else{
          std::cout<<"no openCL platform available"<<std::endl;
        }
        std::cout<<"number of openCL platforms: "<<platformList.size()<<std::endl;
          
        //check devices on platforms
        for(unsigned int i=0; i<platformList.size(); i++){//check all platforms
          std::cout<<"platform "<<i+1<<":"<<std::endl;
          std::vector<cl::Device> deviceList;
          if(platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList)==CL_SUCCESS){
            std::cout<<"GPU-DEVICE(S) FOUND"<<std::endl;
            selectedDevice=i; //if GPU found on platform, select this platform
            clReady=true; //and mark CL context as available
          }else if(platformList.at(i).getDevices(CL_DEVICE_TYPE_CPU, &deviceList)==CL_SUCCESS){
            std::cout<<"CPU-DEVICE(S) FOUND"<<std::endl;
          }else{
            std::cout<<"UNKNOWN DEVICE(S) FOUND"<<std::endl;
          }
          std::cout<<"number of devices: "<<deviceList.size()<<std::endl;
        }
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        std::cout<<"OpenCL not ready"<<std::endl;
        clReady=false;//disable openCL on error
      }  
        
      if(clReady==true){//only if CL context is available
        try{
          cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[selectedDevice])(), 0};//get context properties of selected platform
          context = cl::Context(CL_DEVICE_TYPE_GPU, cprops);//select GPU device
          devices = context.getInfo<CL_CONTEXT_DEVICES>();
                
          std::cout<<"selected devices: "<<devices.size()<<std::endl;
                
          cl::Program::Sources sources(1, std::make_pair(pointCloudCreatorKernel, 0)); //kernel source
          program=cl::Program(context, sources); //program (bind context and source)
          program.build(devices);//build program

          //create buffer for memory access and allocation  
          dirsArray=(float*)dirs.begin();
          dirsBuffer = cl::Buffer(
      				         context, 
      				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
      				         size.width*size.height * 3 * sizeof(float), 
      				         (void *) &dirsArray[0]);  
      				           				         
          xyzBuffer = cl::Buffer(
      				         context, 
      				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
      				         size.width*size.height * 3 * sizeof(float), 
      				         (void *) &xyzData[0]);
      				         
          rgbaBuffer = cl::Buffer(
      				         context, 
      				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
      				         size.width*size.height * sizeof(cl_float4), 
      				         (void *) &rgbaData[0]);                                
  

          //create kernels    
          kernelCreate=cl::Kernel(program, "create");
          kernelCreateRGB=cl::Kernel(program, "createRGB");

          queue=cl::CommandQueue(context, devices[0], 0);//create command queue        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
          clReady=false;
        }
      }
        
    #else
      std::cout<<"no openCL parallelization available"<<std::endl;
      clReady=false;
    #endif   
    }
  
  
    PointCloudCreatorCL::~PointCloudCreatorCL(){
    #ifdef HAVE_OPENCL
      delete[] depthValuesArray;
  	  delete[] rInArray;
  	  delete[] gInArray;
  	  delete[] bInArray;
  	  delete[] dirsArray;
  	  delete[] xyzData;
  	  delete[] rgbaData;
    #endif
    }
  
    
    void PointCloudCreatorCL::createRGB(bool NEEDS_RAW_TO_MM_MAPPING,const Img32f *depthValues, const Mat M, 
                           const Vec O, const unsigned int COLOR_W, const unsigned int COLOR_H, const int DEPTH_DIM, 
                           DataSegment<float,3> xyz, DataSegment<float,4> rgba,
                           const Img8u *rgbIn,const Array2D<Vec3> &dirs, float depthScaling){
      
      cl_uchar needsMapping;
      if(NEEDS_RAW_TO_MM_MAPPING){
        needsMapping=1;
      }else{
        needsMapping=0;
      }
      
      #ifdef HAVE_OPENCL
        try{                             
          depthValuesArray = (float*)depthValues->begin(0);
          depthValuesBuffer = cl::Buffer(//write new image to buffer
      				         context, 
      				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      				         DEPTH_DIM * sizeof(float),
      				         (void *) &depthValuesArray[0]);
          matrixBuffer = cl::Buffer(
                       context, 
      				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
      				         16 * sizeof(float), 
      				         (void *) &M[0]);
          rInArray = (cl_uchar*)rgbIn->begin(0);
          rInBuffer  = cl::Buffer(//write new image to buffer
      				         context, 
      				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      				         DEPTH_DIM * sizeof(cl_uchar), 
      				         (void *) &rInArray[0]);
          gInArray = (cl_uchar*)rgbIn->begin(1);
          gInBuffer  = cl::Buffer(//write new image to buffer
      				         context, 
      				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      				         DEPTH_DIM * sizeof(cl_uchar), 
      				         (void *) &gInArray[0]);
      	  bInArray = (cl_uchar*)rgbIn->begin(2);
          bInBuffer  = cl::Buffer(//write new image to buffer
      				         context, 
      				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      				         DEPTH_DIM * sizeof(cl_uchar), 
      				         (void *) &bInArray[0]);
       
          cl_float4 oV;
      		oV.x=O[0];
      		oV.y=O[1];
      		oV.z=O[2];
      		oV.w=O[3];
          
          kernelCreateRGB.setArg(0, depthValuesBuffer);
          kernelCreateRGB.setArg(1, rInBuffer);
          kernelCreateRGB.setArg(2, gInBuffer);
          kernelCreateRGB.setArg(3, bInBuffer);
          kernelCreateRGB.setArg(4, dirsBuffer);
          kernelCreateRGB.setArg(5, matrixBuffer);
          kernelCreateRGB.setArg(6, rgbaBuffer);
          kernelCreateRGB.setArg(7, xyzBuffer);
          kernelCreateRGB.setArg(8, oV);
          kernelCreateRGB.setArg(9, COLOR_W);
          kernelCreateRGB.setArg(10, COLOR_H);   
          kernelCreateRGB.setArg(11, depthScaling);
          kernelCreateRGB.setArg(12, needsMapping);
          
          queue.enqueueNDRangeKernel(//run kernel
				     kernelCreateRGB, 
				     cl::NullRange, 
				     cl::NDRange(DEPTH_DIM), //input size for get global id
				     cl::NullRange);
				  
				  queue.enqueueReadBuffer(//read output from kernel
  				  xyzBuffer,
  				  CL_TRUE,
  				  0,
  				  DEPTH_DIM * 3 * sizeof(float),
  				  (float*) xyzData);
  				  
  				DataSegment<float,3>((float*)xyzData,sizeof(float)*3,DEPTH_DIM).deepCopy(xyz); //copy pointcloud data
  	      
  	      queue.enqueueReadBuffer(//read output from kernel
  				  rgbaBuffer,
  				  CL_TRUE,
  				  0,
  				  DEPTH_DIM * sizeof(cl_float4),
  				  (cl_float4*)rgbaData);
  		
  			DataSegment<float,4>((float*)rgbaData,sizeof(cl_float4),DEPTH_DIM).deepCopy(rgba); //copy pointcloud color data
      
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      #endif
    }
    
    void PointCloudCreatorCL::create(bool NEEDS_RAW_TO_MM_MAPPING,const Img32f *depthValues, 
                           const Vec O, const int DEPTH_DIM, 
                           DataSegment<float,3> xyz,const Array2D<Vec3> &dirs, float depthScaling){
      
      cl_uchar needsMapping;
      if(NEEDS_RAW_TO_MM_MAPPING){
        needsMapping=1;
      }else{
        needsMapping=0;
      }
      
      #ifdef HAVE_OPENCL
        try{              
          depthValuesArray = (float*)depthValues->begin(0);
          depthValuesBuffer = cl::Buffer(//write new image to buffer
      				         context, 
      				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
      				         DEPTH_DIM * sizeof(float),
      				         (void *) &depthValuesArray[0]);

          cl_float4 oV;
      		oV.x=O[0];
      		oV.y=O[1];
      		oV.z=O[2];
      		oV.w=O[3];
          
          kernelCreate.setArg(0, depthValuesBuffer);
          kernelCreate.setArg(1, dirsBuffer);
          kernelCreate.setArg(2, xyzBuffer);
          kernelCreate.setArg(3, oV);
          kernelCreate.setArg(4, depthScaling);
          kernelCreate.setArg(5, needsMapping);
          
          queue.enqueueNDRangeKernel(//run kernel
				     kernelCreate, 
				     cl::NullRange, 
				     cl::NDRange(DEPTH_DIM), //input size for get global id
				     cl::NullRange);
				  
				  queue.enqueueReadBuffer(//read output from kernel
  				  xyzBuffer,
  				  CL_TRUE,
  				  0,
  				  DEPTH_DIM * 3 * sizeof(float),
  				  (float*) xyzData);
  				
  				DataSegment<float,3>((float*)xyzData,sizeof(float)*3,DEPTH_DIM).deepCopy(xyz); //copy pointcloud data
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      #endif
    }
  	

    bool PointCloudCreatorCL::isCLReady(){
      return clReady;
    }
  
  } // namespace geom
}
