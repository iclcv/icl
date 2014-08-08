/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PlanarRansacEstimator.cpp          **
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

#include <ICLGeom/PlanarRansacEstimator.h>

#ifdef ICL_HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

#include <ICLCore/Img.h>

namespace icl{
  namespace geom{
    
    #ifdef ICL_HAVE_OPENCL
    //OpenCL kernel code
    static char RansacKernel[] = 
    "  #pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable                                                           \n"
    "__kernel void                                                                                                                  \n"
    "checkRANSAC(int const passes, __global float4 const * n0, __global float const * dist, __global int * countAbove,              \n"
    "__global int * countBelow, __global int * countOn, float const threshold, int const numPoints, __global const float4 * points, \n"
    "int const subset)                                                                                                              \n"
    "{                                                                                                                              \n"
    "  size_t id =  get_global_id(0);                                                                                               \n"
    "  int pass = id%passes;                                                                                                        \n"
    "  int point = ((int)floor((float)id/(float)passes))*subset;                                                                    \n"
    "  float4 selPoint = points[point];                                                                                             \n"
    "  float4 seln0 = n0[pass];                                                                                                     \n"
    "  float distance1 = dist[pass];                                                                                                \n"
    "  float s1 = (selPoint.x*seln0.x+selPoint.y*seln0.y+selPoint.z*seln0.z)-distance1;                                             \n"
    "  if((s1>=-threshold && s1<=threshold)){                                                                                       \n"
    "    atomic_inc(&countOn[pass]);                                                                                                \n"
    "  }else if(s1>threshold){                                                                                                      \n"
    "    atomic_inc(&countAbove[pass]);                                                                                             \n"
    "  }else if(s1<-threshold){                                                                                                     \n"
    "    atomic_inc(&countBelow[pass]);                                                                                             \n"
    "  }                                                                                                                            \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "assignRANSAC(__global float4 const * xyz, __global uchar * mask, __global int * newLabel, float4 const n0,                     \n"
    "float const d, float const euclDist, __global int * oldLabel, int maxID, int desiredID)                                          \n"
    "{                                                                                                                              \n"
    "  size_t id =  get_global_id(0);                                                                                               \n"
    "  if(mask[id]==0)                                                                                                              \n"
    "  {                                                                                                                            \n"
    "   if(oldLabel[id]==maxID)                                                                                                     \n"
    "   {                                                                                                                           \n"
    "     newLabel[id]=desiredID;                                                                                                   \n"
    "     mask[id]=1;                                                                                                               \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     float4 xyzD=xyz[id];                                                                                                      \n"
    "     float s1 = (xyzD.x*n0.x+xyzD.y*n0.y+xyzD.z*n0.z)-d;                                                                       \n"
    "     if((s1>=-euclDist && s1<=euclDist) && mask[id]==0)                                                                        \n"
    "     {                                                                                                                         \n"
    "       newLabel[id]=desiredID;                                                                                                 \n"
    "       mask[id]=1;                                                                                                             \n"
    "     }                                                                                                                         \n"
    "     else                                                                                                                      \n"
    "     {                                                                                                                         \n"
    "       newLabel[id]=0;                                                                                                         \n"
    "       mask[id]=0;                                                                                                             \n"
    "     }                                                                                                                         \n"
    "   }                                                                                                                           \n"
    "  }                                                                                                                            \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "checkRANSACmatrix(__global float4 const * xyz, int const passes, __global float4 const * n0, __global float const * dist,      \n"
    " __global int * countAbove, __global int * countBelow, __global int * countOn, float const threshold,                          \n"
    "__global const int * points, __global const int * adjs, __global const int * start, __global const int * end)                  \n"
    "{                                                                                                                              \n"
    "  size_t id =  get_global_id(0);                                                                                               \n"
    "  int pass = id%passes;                                                                                                        \n"
    "  int point = ((int)floor((float)id/(float)passes));                                                                           \n"
    "  if(points[point]>0){                                                                                                         \n"
    "    int pointV = points[point]-1;                                                                                              \n"
    "    int startV = start[pointV];                                                                                                \n"
    "    int stopV = end[pointV];                                                                                                   \n"
    "    for(int i=startV; i<stopV; i++)                                                                                            \n"
    "    {                                                                                                                          \n"
    "      int adj=adjs[i];                                                                                                         \n"
    "      float4 selPoint = xyz[point];                                                                                            \n"
    "      float4 seln0 = n0[adj*passes+pass];                                                                                      \n"
    "      float distance1 = dist[adj*passes+pass];                                                                                 \n"
    "      float s1 = (selPoint.x*seln0.x+selPoint.y*seln0.y+selPoint.z*seln0.z)-distance1;                                         \n"
    "      if((s1>=-threshold && s1<=threshold)){                                                                                   \n"
    "        atomic_inc(&countOn[i*passes+pass]);                                                                                   \n"
    "      }else if(s1>threshold){                                                                                                  \n"
    "        atomic_inc(&countAbove[i*passes+pass]);                                                                                \n"
    "      }else if(s1<-threshold){                                                                                                 \n"
    "        atomic_inc(&countBelow[i*passes+pass]);                                                                                \n"
    "      }                                                                                                                        \n"
    "    }                                                                                                                          \n"
    "  }                                                                                                                            \n"
    "}                                                                                                                              \n"
    ;
    #endif  
    
    
    struct PlanarRansacEstimator::Data {
	    Data(Mode mode) {
		    clReady = false;

        if(mode==BEST || mode==GPU){
          useCL=true;
        }else{
          useCL=false;
        } 
	    }

	    ~Data() {
	    }

	    bool clReady;
	    bool useCL;
	
	    #ifdef ICL_HAVE_OPENCL        
        //OpenCL    
        cl::Context context;
        std::vector<cl::Device> devices;
        cl::Program program;
        cl::CommandQueue queue;
        
        cl::Kernel kernelCheckRANSAC; 
        cl::Kernel kernelAssignRANSAC;        
        cl::Kernel kernelCheckRANSACmatrix;  
      #endif
    };
    

    PlanarRansacEstimator::PlanarRansacEstimator(Mode mode) :
	    m_data(new Data(mode)) {
	    
	    if(m_data->useCL==true){
	      initOpenCL();
	    }
    }


    PlanarRansacEstimator::~PlanarRansacEstimator() {
	    delete m_data;
    }


    PlanarRansacEstimator::Result PlanarRansacEstimator::apply(core::DataSegment<float,4> &xyzh, 
                std::vector<int> &srcIDs, std::vector<int> &dstIDs, float threshold, int passes, 
                int subset, int tolerance, int optimization){
      
      std::vector<Vec> srcPoints(srcIDs.size());
      std::vector<Vec> dstPoints(dstIDs.size());
      for(unsigned int i=0; i<srcIDs.size(); i++){
        srcPoints[i]=xyzh[srcIDs.at(i)];
      }
      for(unsigned int i=0; i<dstIDs.size(); i++){
        dstPoints[i]=xyzh[dstIDs.at(i)];
      } 
                 
      return apply(srcPoints, dstPoints, threshold, passes, subset, tolerance, optimization);  
    }
    
        
    PlanarRansacEstimator::Result PlanarRansacEstimator::apply(std::vector<Vec> &srcPoints, 
                std::vector<Vec> &dstPoints, float threshold, int passes, int subset, int tolerance, int optimization){
   
      int numPoints=dstPoints.size();
      std::vector<Vec> n0(passes);
      std::vector<float> dist(passes);
      std::vector<int> cAbove(passes,0);
      std::vector<int> cBelow(passes,0);
      std::vector<int> cOn(passes,0);

      calculateRandomModels(srcPoints, n0, dist, passes);   
              
      if(m_data->useCL==true && m_data->clReady==true){
        calculateSingleCL(dstPoints, threshold, passes, subset, n0, dist, cAbove, cBelow, cOn);
      }else{             
        calculateSingleCPU(dstPoints, threshold, passes, subset, n0, dist, cAbove, cBelow, cOn);
      }
      
      return createResult(n0, dist, cAbove, cBelow, cOn, threshold, passes, tolerance, optimization, numPoints);      
    }
 
        
    math::DynMatrix<PlanarRansacEstimator::Result> PlanarRansacEstimator::apply(core::DataSegment<float,4> &xyzh, 
                std::vector<std::vector<int> > &pointIDs, math::DynMatrix<bool> &testMatrix, float threshold, 
                int passes, int tolerance, int optimization, core::Img32s labelImage){
      
      std::vector<std::vector<Vec> > n0Pre(testMatrix.rows(), std::vector<Vec>(passes));
      std::vector<std::vector<float> > distPre(testMatrix.rows(), std::vector<float>(passes));
      std::vector<Vec> n0(testMatrix.rows()*passes);
      std::vector<float> dist(testMatrix.rows()*passes);
      std::vector<int> cAbove;
      std::vector<int> cBelow;
      std::vector<int> cOn;
      
      std::vector<int> adjs;

      std::vector<int> start(testMatrix.rows());
      std::vector<int> end(testMatrix.rows());

      int count=0;
      for(size_t i=0; i<testMatrix.rows(); i++){

        start[i]=count;

        calculateRandomModels(xyzh, pointIDs.at(i), n0Pre.at(i), distPre.at(i), passes);

        for(int k=0; k<passes; k++){
          n0[i*passes+k]=n0Pre.at(i)[k];
          dist[i*passes+k]=distPre.at(i)[k];
        }

        for(size_t j=0; j<testMatrix.rows(); j++){
          if(testMatrix(i,j)==true){
            adjs.push_back(j);
            count++;
            for(int k=0; k<passes; k++){
              cAbove.push_back(0);
              cBelow.push_back(0);
              cOn.push_back(0);
            }
          }
        }
                 
        end[i]=count;
      }
      
      if(m_data->useCL==true && m_data->clReady==true){
        calculateMultiCL(xyzh, labelImage, testMatrix, threshold, passes, n0, dist, cAbove, cBelow, cOn, adjs, start, end);
      }else{
        calculateMultiCPU(xyzh, pointIDs, testMatrix, threshold, passes, n0Pre, distPre, cAbove, cBelow, cOn, adjs, start, end);
      }
    
      return createResultMatrix(testMatrix, start, end, adjs, cAbove, cBelow, cOn, pointIDs, n0Pre, distPre, threshold, passes, tolerance, optimization);                   
    }
    

    void PlanarRansacEstimator::relabel(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, 
                                core::Img32s &newLabel, int desiredID, int srcID, float threshold, Result &result){
         
      utils::Size size = newMask.getSize();
      int w = size.width;
      int h = size.height;
    
      //if(m_data->useCL==true && m_data->clReady==true){
      //  relabelCL(xyzh, newMask, oldLabel, newLabel, desiredID, srcID, threshold, result, w, h);
      //}else{//CPU                  
        relabelCPU(xyzh, newMask, oldLabel, newLabel, desiredID, srcID, threshold, result, w, h);
      //}
    }
    
    
    void PlanarRansacEstimator::calculateMultiCL(core::DataSegment<float,4> &xyzh, core::Img32s labelImage, math::DynMatrix<bool> &testMatrix, float threshold, int passes,
                    std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn,
                    std::vector<int> &adjs, std::vector<int> &start, std::vector<int> &end){
      #ifdef ICL_HAVE_OPENCL   
        
        try{
          if(adjs.size()>0){         
            cl::Event waitEvent;
            cl::Buffer n0Buffer;
            cl::Buffer distBuffer;
            cl::Buffer countAboveBuffer;
            cl::Buffer countBelowBuffer;
            cl::Buffer countOnBuffer;
            cl::Buffer adjsBuffer;
            cl::Buffer startBuffer;
            cl::Buffer endBuffer;
            cl::Buffer xyzBuffer;

            cl_mem n0Mem = n0Buffer();
            cl_mem distMem = distBuffer();
            cl_mem countAboveMem = countAboveBuffer();
            cl_mem countBelowMem = countBelowBuffer();
            cl_mem countOnMem = countOnBuffer();
          
            cl::Buffer RANSACpointsBuffer;
            cl_mem RANSACpointsMem = RANSACpointsBuffer();
            
            utils::Size size = xyzh.getSize();
            int w=size.width;
            int h=size.height;
            
            xyzBuffer = cl::Buffer(
                                   m_data->context, 
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                   w*h * sizeof(cl_float4), 
                                   (void *) &xyzh[0]);
            
            RANSACpointsBuffer = cl::Buffer(
                                            m_data->context, 
                                            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                            w*h * sizeof(int),
                                            (void *) labelImage.begin(0)); 
            
            n0Buffer = cl::Buffer(
                                  m_data->context, 
                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                  passes * testMatrix.rows() * sizeof(cl_float4), 
                                  (void *) &n0[0]);
            distBuffer = cl::Buffer(
                                    m_data->context, 
                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                    passes * testMatrix.rows() * sizeof(float), 
                                    (void *) &dist[0]); 
            countAboveBuffer = cl::Buffer(
                                          m_data->context, 
                                          CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                          passes * adjs.size() * sizeof(int), 
                                          (void *) &cAbove[0]);
            countBelowBuffer = cl::Buffer(
                                          m_data->context, 
                                          CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                          passes * adjs.size() * sizeof(int), 
                                          (void *) &cBelow[0]);
            countOnBuffer = cl::Buffer(
                                       m_data->context, 
                                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                       passes * adjs.size() * sizeof(int), 
                                       (void *) &cOn[0]);
            adjsBuffer = cl::Buffer(
                                    m_data->context, 
                                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                    adjs.size() * sizeof(int), 
                                    (void *) &adjs[0]); 
            startBuffer = cl::Buffer(
                                     m_data->context, 
                                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                     testMatrix.rows() * sizeof(int), 
                                     (void *) &start[0]); 
            endBuffer = cl::Buffer(
                                   m_data->context, 
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                   testMatrix.rows() * sizeof(int), 
                                   (void *) &end[0]);   

            m_data->kernelCheckRANSACmatrix.setArg(0, xyzBuffer);//set parameter for kernel
            m_data->kernelCheckRANSACmatrix.setArg(1, passes);
            m_data->kernelCheckRANSACmatrix.setArg(2, n0Buffer);
            m_data->kernelCheckRANSACmatrix.setArg(3, distBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(4, countAboveBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(5, countBelowBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(6, countOnBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(7, threshold);
            m_data->kernelCheckRANSACmatrix.setArg(8, RANSACpointsBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(9, adjsBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(10, startBuffer);
            m_data->kernelCheckRANSACmatrix.setArg(11, endBuffer);
                          
            int idSize=passes*w*h;
            m_data->queue.enqueueNDRangeKernel(//run kernel
                                       m_data->kernelCheckRANSACmatrix, 
                                       cl::NullRange, 
                                       cl::NDRange(idSize), //input size for get global id
                                       cl::NullRange,
                                       NULL,
                                       &waitEvent);
      	                	    
            m_data->queue.enqueueReadBuffer(//read output from kernel
                                    countAboveBuffer,
                                    CL_TRUE, // block 
                                    0,
                                    passes * adjs.size() * sizeof(int),
                                    (int*) &cAbove[0],
                                    NULL,&waitEvent);
                           
            m_data->queue.enqueueReadBuffer(//read output from kernel
                                    countBelowBuffer,
                                    CL_TRUE, // block 
                                    0,
                                    passes * adjs.size() * sizeof(int),
                                    (int*) &cBelow[0],
                                    NULL,&waitEvent);
                        
            m_data->queue.enqueueReadBuffer(//read output from kernel
                                    countOnBuffer,
                                    CL_TRUE, // block 
                                    0,
                                    passes * adjs.size() * sizeof(int),
                                    (int*) &cOn[0],
                                    NULL,&waitEvent);
             
            clFinish(m_data->queue()); 
              
            clReleaseMemObject(n0Mem);
            clReleaseMemObject(distMem);
            clReleaseMemObject(countAboveMem);
            clReleaseMemObject(countBelowMem);
            clReleaseMemObject(countOnMem);
            
            clReleaseMemObject(RANSACpointsMem);  
      		}
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      #endif
    }
    
    
    void PlanarRansacEstimator::calculateMultiCPU(core::DataSegment<float,4> &xyzh, std::vector<std::vector<int> > &pointIDs, math::DynMatrix<bool> &testMatrix, 
                    float threshold, int passes, std::vector<std::vector<Vec> > &n0Pre, std::vector<std::vector<float> > &distPre, std::vector<int> &cAbove, 
                    std::vector<int> &cBelow, std::vector<int> &cOn, std::vector<int> &adjs, std::vector<int> &start, std::vector<int> &end){
      for(size_t i=0; i<testMatrix.rows(); i++){
        for(int j=start[i]; j<end[i]; j++){
          int k=adjs[j];
          for(unsigned int m=0; m<pointIDs.at(k).size(); m++){
            for(int l=0; l<passes; l++){
              Vec n01 = n0Pre.at(i).at(l);
              Vec point = xyzh[pointIDs.at(k).at(m)];
              float s1 = (point[0]*n01[0]+point[1]*n01[1]+point[2]*n01[2])-distPre.at(i).at(l);      
              if((s1>=-threshold && s1<=threshold)){
                cOn[j*passes+l]++;
              }else if(s1>threshold){
                cAbove[j*passes+l]++;
              }else if(s1<threshold){
                cBelow[j*passes+l]++;
              } 
            }
          }
        }
      }
    }
    
    
    void PlanarRansacEstimator::calculateSingleCL(std::vector<Vec> &dstPoints, float threshold, int passes, int subset, 
                std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn){
     
      #ifdef ICL_HAVE_OPENCL    
        try{         
          int numPoints=dstPoints.size();
                              
          cl::Buffer RANSACpointsBuffer;
          cl_mem RANSACpointsMem = RANSACpointsBuffer();
          RANSACpointsBuffer = cl::Buffer(
                                          m_data->context, 
                                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                          numPoints * sizeof(cl_float4), 
                                          (void *) &dstPoints[0]);
                     
          cl::Event waitEvent;
          cl::Buffer n0Buffer;
          cl::Buffer distBuffer;
          cl::Buffer countAboveBuffer;
          cl::Buffer countBelowBuffer;
          cl::Buffer countOnBuffer;

          cl_mem n0Mem = n0Buffer();
          cl_mem distMem = distBuffer();
          cl_mem countAboveMem = countAboveBuffer();
          cl_mem countBelowMem = countBelowBuffer();
          cl_mem countOnMem = countOnBuffer();
          
          n0Buffer = cl::Buffer(
                                m_data->context, 
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                passes * sizeof(cl_float4), 
                                (void *) &n0[0]);
          distBuffer = cl::Buffer(
                                  m_data->context, 
                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                  passes * sizeof(float), 
                                  (void *) &dist[0]); 
          countAboveBuffer = cl::Buffer(
                                        m_data->context, 
                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                        passes * sizeof(int), 
                                        (void *) &cAbove[0]);
          countBelowBuffer = cl::Buffer(
                                        m_data->context, 
                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                        passes * sizeof(int), 
                                        (void *) &cBelow[0]);
          countOnBuffer = cl::Buffer(
                                     m_data->context, 
                                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                     passes * sizeof(int), 
                                     (void *) &cOn[0]);

          m_data->kernelCheckRANSAC.setArg(0, passes);
          m_data->kernelCheckRANSAC.setArg(1, n0Buffer);
          m_data->kernelCheckRANSAC.setArg(2, distBuffer);
          m_data->kernelCheckRANSAC.setArg(3, countAboveBuffer);
          m_data->kernelCheckRANSAC.setArg(4, countBelowBuffer);
          m_data->kernelCheckRANSAC.setArg(5, countOnBuffer);
          m_data->kernelCheckRANSAC.setArg(6, threshold);
          m_data->kernelCheckRANSAC.setArg(7, numPoints);
          m_data->kernelCheckRANSAC.setArg(8, RANSACpointsBuffer);
          m_data->kernelCheckRANSAC.setArg(9, subset);
                        
          int idSize=passes*numPoints/subset;
          m_data->queue.enqueueNDRangeKernel(//run kernel
                                     m_data->kernelCheckRANSAC, 
                                     cl::NullRange, 
                                     cl::NDRange(idSize), //input size for get global id
                                     cl::NullRange,
                                     NULL,
                                     &waitEvent);
    	                	    
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                  countAboveBuffer,
                                  CL_TRUE, // block 
                                  0,
                                  passes * sizeof(int),
                                  (int*) &cAbove[0],//cAboveRead
                                  NULL,&waitEvent);
                         
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                  countBelowBuffer,
                                  CL_TRUE, // block 
                                  0,
                                  passes * sizeof(int),
                                  (int*) &cBelow[0],
                                  NULL,&waitEvent);
                      
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                  countOnBuffer,
                                  CL_TRUE, // block 
                                  0,
                                  passes * sizeof(int),
                                  (int*) &cOn[0],
                                  NULL,&waitEvent);
           
          clFinish(m_data->queue()); 
            
          clReleaseMemObject(n0Mem);
          clReleaseMemObject(distMem);
          clReleaseMemObject(countAboveMem);
          clReleaseMemObject(countBelowMem);
          clReleaseMemObject(countOnMem);
    
          clReleaseMemObject(RANSACpointsMem);  
              
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }       
      #endif
    }
    
    
    void PlanarRansacEstimator::calculateSingleCPU(std::vector<Vec> &dstPoints, float threshold, int passes, int subset, 
                std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn){
      for(int p=0; p<passes; p++){
        for(unsigned int q=0; q<dstPoints.size(); q+=subset){
          Vec n01 = n0[p];
          Vec point = dstPoints.at(q);
          float s1 = (point[0]*n01[0]+point[1]*n01[1]+point[2]*n01[2])-dist[p];      
          if((s1>=-threshold && s1<=threshold)){
            cOn[p]++;
          }else if(s1>threshold){
            cAbove[p]++;
          }else if(s1<threshold){
            cBelow[p]++;
          } 
        }
      }            
    }
    
    
    void PlanarRansacEstimator::initOpenCL(){       
      #ifdef ICL_HAVE_OPENCL
      //create openCL context
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
            m_data->clReady=true; //and mark CL context as available
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
        m_data->clReady=false;//disable openCL on error
      }  
        
      if(m_data->clReady==true){//only if CL context is available
        try{
          cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[selectedDevice])(), 0};//get context properties of selected platform
          m_data->context = cl::Context(CL_DEVICE_TYPE_GPU, cprops);//select GPU device
          m_data->devices = m_data->context.getInfo<CL_CONTEXT_DEVICES>();
                
          std::cout<<"selected devices: "<<m_data->devices.size()<<std::endl;
                
          cl::Program::Sources sources(1, std::make_pair(RansacKernel, 0)); //kernel source
          m_data->program=cl::Program(m_data->context, sources); //program (bind context and source)
          m_data->program.build(m_data->devices);//build program
  
          //create kernels  
          m_data->kernelCheckRANSAC=cl::Kernel(m_data->program, "checkRANSAC");
          m_data->kernelAssignRANSAC=cl::Kernel(m_data->program, "assignRANSAC");
          m_data->kernelCheckRANSACmatrix=cl::Kernel(m_data->program, "checkRANSACmatrix");
          
          m_data->queue=cl::CommandQueue(m_data->context, m_data->devices[0], 0);//create command queue        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
          m_data->clReady=false;
        }
      }
        
      #else
      std::cout<<"no openCL parallelization available"<<std::endl;
      m_data->clReady=false;
      #endif   
    }
    
    
    void PlanarRansacEstimator::calculateRandomModels(std::vector<Vec> &srcPoints, std::vector<Vec> &n0, std::vector<float> &dist, int passes){
      for(int i=0; i<passes; i++){
        Vec p0i=srcPoints.at(rand()%srcPoints.size());
        Vec p1i=srcPoints.at(rand()%srcPoints.size());
        Vec p2i=srcPoints.at(rand()%srcPoints.size());
        Vec fa = p1i-p0i;
        Vec fb = p2i-p0i;
        Vec rPoint = p0i;
      
        calculateModel(fa, fb, rPoint, n0[i], dist[i]);
      }
    }
    
    
    void PlanarRansacEstimator::calculateRandomModels(core::DataSegment<float,4> &xyzh, std::vector<int> &srcPoints, std::vector<Vec> &n0, std::vector<float> &dist, int passes){
      for(int i=0; i<passes; i++){
        int p0i=srcPoints.at(rand()%srcPoints.size());
        int p1i=srcPoints.at(rand()%srcPoints.size());
        int p2i=srcPoints.at(rand()%srcPoints.size());
        Vec fa = xyzh[p1i]-xyzh[p0i];
        Vec fb = xyzh[p2i]-xyzh[p0i];
        Vec rPoint = xyzh[p0i];
        
        calculateModel(fa, fb, rPoint, n0[i], dist[i]);
      }
    }
    
    
    PlanarRansacEstimator::Result PlanarRansacEstimator::createResult(std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, 
                    std::vector<int> &cBelow, std::vector<int> &cOn, float threshold, int passes, int tolerance, int optimization, int numPoints){
      int maxMatch=0;
      int maxMatchID=0;
      int countAcc=0;
      int countNAcc=0;
      for(int i=0;i<passes; i++){
        if(optimization==ON_ONE_SIDE){
          if(cAbove[i]<tolerance || cBelow[i]<tolerance){
            countAcc++;
          }else{
            countNAcc++;
          }
        }
        if(optimization==MAX_ON){
          if(cOn[i]>maxMatch){
            maxMatch=cOn[i];
            maxMatchID=i;
          }
        }else if(optimization==ON_ONE_SIDE){
          if(cBelow[i]>maxMatch){
            maxMatch=cBelow[i];
            maxMatchID=i;
          }
          if(cAbove[i]>maxMatch){
            maxMatch=cAbove[i];
            maxMatchID=i;
          }
        }
      }
                
      Result result;
      result.numPoints=numPoints;
      result.countOn=cOn[maxMatchID];
      result.countAbove=cAbove[maxMatchID];
      result.countAbove=cBelow[maxMatchID];
      result.euclideanThreshold=threshold;
      result.n0=n0[maxMatchID];
      result.dist=dist[maxMatchID];
      result.tolerance=tolerance;
      result.acc=countAcc;
      result.nacc=countNAcc;
      return result;
    } 
    
    
    math::DynMatrix<PlanarRansacEstimator::Result> PlanarRansacEstimator::createResultMatrix(math::DynMatrix<bool> &testMatrix, std::vector<int> &start,
                   std::vector<int> &end, std::vector<int> &adjs, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn, 
                   std::vector<std::vector<int> > &pointIDs, std::vector<std::vector<Vec> > &n0Pre, std::vector<std::vector<float> > &distPre, 
                   float threshold, int passes, int tolerance, int optimization){
      Result init;
      math::DynMatrix<Result> result(testMatrix.cols(),testMatrix.rows(), init);
      
      for(size_t i=0; i<testMatrix.rows(); i++){
        for(int j=start[i]; j<end[i]; j++){
          int k=adjs[j];
          std::vector<int> above(passes);
          std::vector<int> below(passes);
          std::vector<int> on(passes);
          for(int l=0; l<passes; l++){
            above[l]=cAbove[j*passes+l];
            below[l]=cBelow[j*passes+l];
            on[l]=cOn[j*passes+l];
          }
          int numPoints = pointIDs.at(k).size();
          Result res = createResult(n0Pre[i], distPre[i], above, below, on, threshold, passes, tolerance, optimization, numPoints);
          result(i,k)=res;
        }
      }
        
      return result;                   
    }
    
    
    void PlanarRansacEstimator::calculateModel(Vec &fa, Vec &fb, Vec &rPoint, Vec &n0, float &dist){
      Vec n1;
      n1[0]=fa[1]*fb[2]-fa[2]*fb[1];
      n1[1]=fa[2]*fb[0]-fa[0]*fb[2];
      n1[2]=fa[0]*fb[1]-fa[1]*fb[0];
      n0[0]=n1[0]/norm3(n1);
      n0[1]=n1[1]/norm3(n1);
     	n0[2]=n1[2]/norm3(n1);
      dist = rPoint[0]*n0[0]+rPoint[1]*n0[1]+ rPoint[2]*n0[2];
    }
    
    
    void PlanarRansacEstimator::relabelCL(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, core::Img32s &newLabel,
                     int desiredID, int srcID, float threshold, Result &result, int w, int h){
      #ifdef ICL_HAVE_OPENCL
        try{
          cl::Buffer xyzBuffer = cl::Buffer(
                                 m_data->context, 
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                 w*h * sizeof(cl_float4), 
                                 (void *) &xyzh[0]);
                   
          cl::Buffer elementsBlobsBuffer = cl::Buffer(
				           m_data->context, 
				           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(cl_uchar), 
				           (void *) newMask.begin(0));

          cl::Buffer assignmentBlobsBuffer = cl::Buffer(
                                             m_data->context, 
                                             CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                             w*h * sizeof(int), 
                                             (void *) newLabel.begin(0));
				           
          cl::Buffer assignmentBuffer = cl::Buffer(
                                        m_data->context, 
                                        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                        w*h * sizeof(int), 
                                        (void *) oldLabel.begin(0));
                  
          m_data->kernelAssignRANSAC.setArg(0, xyzBuffer);
          m_data->kernelAssignRANSAC.setArg(1, elementsBlobsBuffer);
          m_data->kernelAssignRANSAC.setArg(2, assignmentBlobsBuffer);
          m_data->kernelAssignRANSAC.setArg(3, result.n0);
          m_data->kernelAssignRANSAC.setArg(4, result.dist);
          m_data->kernelAssignRANSAC.setArg(5, threshold);
          m_data->kernelAssignRANSAC.setArg(6, assignmentBuffer);
          m_data->kernelAssignRANSAC.setArg(7, srcID);
          m_data->kernelAssignRANSAC.setArg(8, desiredID);
                   
          m_data->queue.enqueueNDRangeKernel(//run kernel
				     m_data->kernelAssignRANSAC, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);
    	    
    	    std::vector<int> assignmentBlobs(w*h);
    	    std::vector<unsigned char> elementsBlobs(w*h);
    	    			          
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                  assignmentBlobsBuffer,
                                  CL_TRUE, // block 
                                  0,
                                  w*h * sizeof(int),
                                  (int*) &assignmentBlobs[0]);
          
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                  elementsBlobsBuffer,
                                  CL_TRUE, // block 
                                  0,
                                  w*h * sizeof(bool),
                                  (cl_uchar*) &elementsBlobs[0]);
        
          //newLabel = Img32s(Size(w,h),1,std::vector<int*>(1,assignmentBlobs.data()),false);//,true);//false);          
          //newMask = Img8u(Size(w,h),1,std::vector<unsigned char*>(1,elementsBlobs.data()),false);//,true);//false);
        
          for(int y=0; y<h; y++){
            for(int x=0; x<w; x++){
              newLabel(x,y,0)=assignmentBlobs[x+w*y];
              newMask(x,y,0)=elementsBlobs[x+w*y];
            }
          }
        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }       
      #endif
    }
    
    
    void PlanarRansacEstimator::relabelCPU(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, core::Img32s &newLabel,
                     int desiredID, int srcID, float threshold, Result &result, int w, int h){
      for(int y=0; y<h; y++){
        for(int x=0; x<w; x++){
          int i=x+y*w;
          if(newMask(x,y,0)==0){
            if(oldLabel(x,y,0)==srcID){
              newLabel(x,y,0)=desiredID;
              newMask(x,y,0)=1;
            }else{
              Vec n01 = result.n0;
              float dist = result.dist;
              float s1 = (xyzh[i][0]*n01[0]+xyzh[i][1]*n01[1]+xyzh[i][2]*n01[2])-dist;  
              if((s1>=-threshold && s1<=threshold) && newMask(x,y,0)==0){
                newLabel(x,y,0)=desiredID;
                newMask(x,y,0)=1;
              }else{
                newLabel(x,y,0)=0;
                newMask(x,y,0)=0;
              }
            }
          }
        }
      }    
    }
                    
  }
}
