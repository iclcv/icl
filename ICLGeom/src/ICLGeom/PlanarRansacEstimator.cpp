// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#include <ICLGeom/PlanarRansacEstimator.h>

#include <ICLUtils/CLIncludes.h>

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

        #ifdef ICL_HAVE_OPENCL
        context = nullptr;
        device = nullptr;
        program = nullptr;
        queue = nullptr;
        kernelCheckRANSAC = nullptr;
        kernelAssignRANSAC = nullptr;
        kernelCheckRANSACmatrix = nullptr;
        #endif
	    }

	    ~Data() {
	      #ifdef ICL_HAVE_OPENCL
	      if(kernelCheckRANSAC) clReleaseKernel(kernelCheckRANSAC);
	      if(kernelAssignRANSAC) clReleaseKernel(kernelAssignRANSAC);
	      if(kernelCheckRANSACmatrix) clReleaseKernel(kernelCheckRANSACmatrix);
	      if(program) clReleaseProgram(program);
	      if(queue) clReleaseCommandQueue(queue);
	      if(context) clReleaseContext(context);
	      #endif
	    }

	    bool clReady;
	    bool useCL;

	    #ifdef ICL_HAVE_OPENCL
        //OpenCL
        cl_context context;
        cl_device_id device;
        cl_program program;
        cl_command_queue queue;

        cl_kernel kernelCheckRANSAC;
        cl_kernel kernelAssignRANSAC;
        cl_kernel kernelCheckRANSACmatrix;
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
                std::vector<std::vector<int> > &pointIDs, math::DynMatrixBase<bool> &testMatrix, float threshold,
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


    void PlanarRansacEstimator::calculateMultiCL(core::DataSegment<float,4> &xyzh, core::Img32s labelImage, math::DynMatrixBase<bool> &testMatrix, float threshold, int passes,
                    std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn,
                    std::vector<int> &adjs, std::vector<int> &start, std::vector<int> &end){
      #ifdef ICL_HAVE_OPENCL

        if(adjs.size()>0){
          cl_int err = CL_SUCCESS;
          cl_mem xyzBuffer = nullptr;
          cl_mem RANSACpointsBuffer = nullptr;
          cl_mem n0Buffer = nullptr;
          cl_mem distBuffer = nullptr;
          cl_mem countAboveBuffer = nullptr;
          cl_mem countBelowBuffer = nullptr;
          cl_mem countOnBuffer = nullptr;
          cl_mem adjsBuffer = nullptr;
          cl_mem startBuffer = nullptr;
          cl_mem endBuffer = nullptr;

          utils::Size size = xyzh.getSize();
          int w=size.width;
          int h=size.height;

          xyzBuffer = clCreateBuffer(
                                     m_data->context,
                                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     w*h * sizeof(cl_float4),
                                     static_cast<void *>(&xyzh[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(xyzBuffer) error: " << err);
            return;
          }

          RANSACpointsBuffer = clCreateBuffer(
                                              m_data->context,
                                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                              w*h * sizeof(int),
                                              static_cast<void *>(labelImage.begin(0)), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(RANSACpointsBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            return;
          }

          n0Buffer = clCreateBuffer(
                                    m_data->context,
                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    passes * testMatrix.rows() * sizeof(cl_float4),
                                    static_cast<void *>(&n0[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(n0Buffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            return;
          }

          distBuffer = clCreateBuffer(
                                      m_data->context,
                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      passes * testMatrix.rows() * sizeof(float),
                                      static_cast<void *>(&dist[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(distBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            return;
          }

          countAboveBuffer = clCreateBuffer(
                                            m_data->context,
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                            passes * adjs.size() * sizeof(int),
                                            static_cast<void *>(&cAbove[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(countAboveBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            return;
          }

          countBelowBuffer = clCreateBuffer(
                                            m_data->context,
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                            passes * adjs.size() * sizeof(int),
                                            static_cast<void *>(&cBelow[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(countBelowBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            clReleaseMemObject(countAboveBuffer);
            return;
          }

          countOnBuffer = clCreateBuffer(
                                         m_data->context,
                                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         passes * adjs.size() * sizeof(int),
                                         static_cast<void *>(&cOn[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(countOnBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            clReleaseMemObject(countAboveBuffer);
            clReleaseMemObject(countBelowBuffer);
            return;
          }

          adjsBuffer = clCreateBuffer(
                                      m_data->context,
                                      CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      adjs.size() * sizeof(int),
                                      static_cast<void *>(&adjs[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(adjsBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            clReleaseMemObject(countAboveBuffer);
            clReleaseMemObject(countBelowBuffer);
            clReleaseMemObject(countOnBuffer);
            return;
          }

          startBuffer = clCreateBuffer(
                                       m_data->context,
                                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                       testMatrix.rows() * sizeof(int),
                                       static_cast<void *>(&start[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(startBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            clReleaseMemObject(countAboveBuffer);
            clReleaseMemObject(countBelowBuffer);
            clReleaseMemObject(countOnBuffer);
            clReleaseMemObject(adjsBuffer);
            return;
          }

          endBuffer = clCreateBuffer(
                                     m_data->context,
                                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                     testMatrix.rows() * sizeof(int),
                                     static_cast<void *>(&end[0]), &err);
          if(err != CL_SUCCESS){
            ERROR_LOG("clCreateBuffer(endBuffer) error: " << err);
            clReleaseMemObject(xyzBuffer);
            clReleaseMemObject(RANSACpointsBuffer);
            clReleaseMemObject(n0Buffer);
            clReleaseMemObject(distBuffer);
            clReleaseMemObject(countAboveBuffer);
            clReleaseMemObject(countBelowBuffer);
            clReleaseMemObject(countOnBuffer);
            clReleaseMemObject(adjsBuffer);
            clReleaseMemObject(startBuffer);
            return;
          }

          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 0, sizeof(cl_mem), &xyzBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 1, sizeof(int), &passes);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 2, sizeof(cl_mem), &n0Buffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 3, sizeof(cl_mem), &distBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 4, sizeof(cl_mem), &countAboveBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 5, sizeof(cl_mem), &countBelowBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 6, sizeof(cl_mem), &countOnBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 7, sizeof(float), &threshold);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 8, sizeof(cl_mem), &RANSACpointsBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 9, sizeof(cl_mem), &adjsBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 10, sizeof(cl_mem), &startBuffer);
          clSetKernelArg(m_data->kernelCheckRANSACmatrix, 11, sizeof(cl_mem), &endBuffer);

          size_t idSize = static_cast<size_t>(passes*w*h);
          cl_event waitEvent = nullptr;
          err = clEnqueueNDRangeKernel(//run kernel
                                       m_data->queue,
                                       m_data->kernelCheckRANSACmatrix,
                                       1,
                                       nullptr,
                                       &idSize, //input size for get global id
                                       nullptr,
                                       0, nullptr,
                                       &waitEvent);
          if(err != CL_SUCCESS){
            ERROR_LOG("clEnqueueNDRangeKernel error: " << err);
          }

          if(waitEvent){
            clWaitForEvents(1, &waitEvent);
            clReleaseEvent(waitEvent);
          }

          clEnqueueReadBuffer(//read output from kernel
                              m_data->queue,
                              countAboveBuffer,
                              CL_TRUE, // block
                              0,
                              passes * adjs.size() * sizeof(int),
                              reinterpret_cast<int*>(&cAbove[0]),
                              0, nullptr, nullptr);

          clEnqueueReadBuffer(//read output from kernel
                              m_data->queue,
                              countBelowBuffer,
                              CL_TRUE, // block
                              0,
                              passes * adjs.size() * sizeof(int),
                              reinterpret_cast<int*>(&cBelow[0]),
                              0, nullptr, nullptr);

          clEnqueueReadBuffer(//read output from kernel
                              m_data->queue,
                              countOnBuffer,
                              CL_TRUE, // block
                              0,
                              passes * adjs.size() * sizeof(int),
                              reinterpret_cast<int*>(&cOn[0]),
                              0, nullptr, nullptr);

          clFinish(m_data->queue);

          clReleaseMemObject(xyzBuffer);
          clReleaseMemObject(RANSACpointsBuffer);
          clReleaseMemObject(n0Buffer);
          clReleaseMemObject(distBuffer);
          clReleaseMemObject(countAboveBuffer);
          clReleaseMemObject(countBelowBuffer);
          clReleaseMemObject(countOnBuffer);
          clReleaseMemObject(adjsBuffer);
          clReleaseMemObject(startBuffer);
          clReleaseMemObject(endBuffer);
    		}
      #endif
    }


    void PlanarRansacEstimator::calculateMultiCPU(core::DataSegment<float,4> &xyzh, std::vector<std::vector<int> > &pointIDs, math::DynMatrixBase<bool> &testMatrix,
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
        cl_int err = CL_SUCCESS;
        int numPoints=dstPoints.size();

        cl_mem RANSACpointsBuffer = clCreateBuffer(
                                                   m_data->context,
                                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                   numPoints * sizeof(cl_float4),
                                                   static_cast<void *>(&dstPoints[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(RANSACpointsBuffer) error: " << err);
          return;
        }

        cl_mem n0Buffer = clCreateBuffer(
                                         m_data->context,
                                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         passes * sizeof(cl_float4),
                                         static_cast<void *>(&n0[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(n0Buffer) error: " << err);
          clReleaseMemObject(RANSACpointsBuffer);
          return;
        }

        cl_mem distBuffer = clCreateBuffer(
                                           m_data->context,
                                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           passes * sizeof(float),
                                           static_cast<void *>(&dist[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(distBuffer) error: " << err);
          clReleaseMemObject(RANSACpointsBuffer);
          clReleaseMemObject(n0Buffer);
          return;
        }

        cl_mem countAboveBuffer = clCreateBuffer(
                                                 m_data->context,
                                                 CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                                 passes * sizeof(int),
                                                 static_cast<void *>(&cAbove[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(countAboveBuffer) error: " << err);
          clReleaseMemObject(RANSACpointsBuffer);
          clReleaseMemObject(n0Buffer);
          clReleaseMemObject(distBuffer);
          return;
        }

        cl_mem countBelowBuffer = clCreateBuffer(
                                                 m_data->context,
                                                 CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                                 passes * sizeof(int),
                                                 static_cast<void *>(&cBelow[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(countBelowBuffer) error: " << err);
          clReleaseMemObject(RANSACpointsBuffer);
          clReleaseMemObject(n0Buffer);
          clReleaseMemObject(distBuffer);
          clReleaseMemObject(countAboveBuffer);
          return;
        }

        cl_mem countOnBuffer = clCreateBuffer(
                                              m_data->context,
                                              CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                              passes * sizeof(int),
                                              static_cast<void *>(&cOn[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(countOnBuffer) error: " << err);
          clReleaseMemObject(RANSACpointsBuffer);
          clReleaseMemObject(n0Buffer);
          clReleaseMemObject(distBuffer);
          clReleaseMemObject(countAboveBuffer);
          clReleaseMemObject(countBelowBuffer);
          return;
        }

        clSetKernelArg(m_data->kernelCheckRANSAC, 0, sizeof(int), &passes);
        clSetKernelArg(m_data->kernelCheckRANSAC, 1, sizeof(cl_mem), &n0Buffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 2, sizeof(cl_mem), &distBuffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 3, sizeof(cl_mem), &countAboveBuffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 4, sizeof(cl_mem), &countBelowBuffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 5, sizeof(cl_mem), &countOnBuffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 6, sizeof(float), &threshold);
        clSetKernelArg(m_data->kernelCheckRANSAC, 7, sizeof(int), &numPoints);
        clSetKernelArg(m_data->kernelCheckRANSAC, 8, sizeof(cl_mem), &RANSACpointsBuffer);
        clSetKernelArg(m_data->kernelCheckRANSAC, 9, sizeof(int), &subset);

        size_t idSize = static_cast<size_t>(passes*numPoints/subset);
        cl_event waitEvent = nullptr;
        err = clEnqueueNDRangeKernel(//run kernel
                                     m_data->queue,
                                     m_data->kernelCheckRANSAC,
                                     1,
                                     nullptr,
                                     &idSize, //input size for get global id
                                     nullptr,
                                     0, nullptr,
                                     &waitEvent);
        if(err != CL_SUCCESS){
          ERROR_LOG("clEnqueueNDRangeKernel error: " << err);
        }

        if(waitEvent){
          clWaitForEvents(1, &waitEvent);
          clReleaseEvent(waitEvent);
        }

        clEnqueueReadBuffer(//read output from kernel
                            m_data->queue,
                            countAboveBuffer,
                            CL_TRUE, // block
                            0,
                            passes * sizeof(int),
                            reinterpret_cast<int*>(&cAbove[0]),
                            0, nullptr, nullptr);

        clEnqueueReadBuffer(//read output from kernel
                            m_data->queue,
                            countBelowBuffer,
                            CL_TRUE, // block
                            0,
                            passes * sizeof(int),
                            reinterpret_cast<int*>(&cBelow[0]),
                            0, nullptr, nullptr);

        clEnqueueReadBuffer(//read output from kernel
                            m_data->queue,
                            countOnBuffer,
                            CL_TRUE, // block
                            0,
                            passes * sizeof(int),
                            reinterpret_cast<int*>(&cOn[0]),
                            0, nullptr, nullptr);

        clFinish(m_data->queue);

        clReleaseMemObject(RANSACpointsBuffer);
        clReleaseMemObject(n0Buffer);
        clReleaseMemObject(distBuffer);
        clReleaseMemObject(countAboveBuffer);
        clReleaseMemObject(countBelowBuffer);
        clReleaseMemObject(countOnBuffer);
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
      cl_uint numPlatforms = 0;
      cl_platform_id selectedPlatform = nullptr;
      bool gpuFound = false;

      cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms);
      if(err != CL_SUCCESS || numPlatforms == 0){
        std::cout<<"no openCL platform available"<<std::endl;
        m_data->clReady = false;
        return;
      }
      std::cout<<"openCL platform found"<<std::endl;
      std::cout<<"number of openCL platforms: "<<numPlatforms<<std::endl;

      std::vector<cl_platform_id> platforms(numPlatforms);
      clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

      //check devices on platforms
      for(cl_uint i=0; i<numPlatforms; i++){
        std::cout<<"platform "<<i+1<<":"<<std::endl;
        cl_uint numDevices = 0;
        cl_int devErr = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
        if(devErr == CL_SUCCESS && numDevices > 0){
          std::cout<<"GPU-DEVICE(S) FOUND"<<std::endl;
          selectedPlatform = platforms[i];
          gpuFound = true;
        }else{
          devErr = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
          if(devErr == CL_SUCCESS && numDevices > 0){
            std::cout<<"CPU-DEVICE(S) FOUND"<<std::endl;
          }else{
            std::cout<<"UNKNOWN DEVICE(S) FOUND"<<std::endl;
          }
        }
        std::cout<<"number of devices: "<<numDevices<<std::endl;
      }

      if(!gpuFound){
        std::cout<<"OpenCL not ready"<<std::endl;
        m_data->clReady = false;
        return;
      }

      m_data->clReady = true;

      // Get GPU device
      cl_uint numDevices = 0;
      err = clGetDeviceIDs(selectedPlatform, CL_DEVICE_TYPE_GPU, 1, &m_data->device, &numDevices);
      if(err != CL_SUCCESS || numDevices == 0){
        ERROR_LOG("clGetDeviceIDs error: " << err);
        m_data->clReady = false;
        return;
      }

      std::cout<<"selected devices: 1"<<std::endl;

      // Create context
      cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(selectedPlatform), 0};
      m_data->context = clCreateContext(cprops, 1, &m_data->device, nullptr, nullptr, &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateContext error: " << err);
        m_data->clReady = false;
        return;
      }

      // Create program from source
      const char *src = RansacKernel;
      size_t srcLen = strlen(RansacKernel);
      m_data->program = clCreateProgramWithSource(m_data->context, 1, &src, &srcLen, &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateProgramWithSource error: " << err);
        m_data->clReady = false;
        return;
      }

      // Build program
      err = clBuildProgram(m_data->program, 1, &m_data->device, nullptr, nullptr, nullptr);
      if(err != CL_SUCCESS){
        // Retrieve build log for diagnostics
        size_t logLen = 0;
        clGetProgramBuildInfo(m_data->program, m_data->device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLen);
        if(logLen > 0){
          std::vector<char> log(logLen);
          clGetProgramBuildInfo(m_data->program, m_data->device, CL_PROGRAM_BUILD_LOG, logLen, log.data(), nullptr);
          ERROR_LOG("clBuildProgram error: " << err << "\n" << log.data());
        }else{
          ERROR_LOG("clBuildProgram error: " << err);
        }
        m_data->clReady = false;
        return;
      }

      // Create kernels
      m_data->kernelCheckRANSAC = clCreateKernel(m_data->program, "checkRANSAC", &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateKernel(checkRANSAC) error: " << err);
        m_data->clReady = false;
        return;
      }

      m_data->kernelAssignRANSAC = clCreateKernel(m_data->program, "assignRANSAC", &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateKernel(assignRANSAC) error: " << err);
        m_data->clReady = false;
        return;
      }

      m_data->kernelCheckRANSACmatrix = clCreateKernel(m_data->program, "checkRANSACmatrix", &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateKernel(checkRANSACmatrix) error: " << err);
        m_data->clReady = false;
        return;
      }

      // Create command queue
      m_data->queue = clCreateCommandQueue(m_data->context, m_data->device, 0, &err);
      if(err != CL_SUCCESS){
        ERROR_LOG("clCreateCommandQueue error: " << err);
        m_data->clReady = false;
        return;
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


    math::DynMatrix<PlanarRansacEstimator::Result> PlanarRansacEstimator::createResultMatrix(math::DynMatrixBase<bool> &testMatrix, std::vector<int> &start,
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
        cl_int err = CL_SUCCESS;

        cl_mem xyzBuffer = clCreateBuffer(
                                          m_data->context,
                                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          w*h * sizeof(cl_float4),
                                          static_cast<void *>(&xyzh[0]), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(xyzBuffer) error: " << err);
          return;
        }

        cl_mem elementsBlobsBuffer = clCreateBuffer(
                                                    m_data->context,
                                                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                                    w*h * sizeof(cl_uchar),
                                                    static_cast<void *>(newMask.begin(0)), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(elementsBlobsBuffer) error: " << err);
          clReleaseMemObject(xyzBuffer);
          return;
        }

        cl_mem assignmentBlobsBuffer = clCreateBuffer(
                                                      m_data->context,
                                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                      w*h * sizeof(int),
                                                      static_cast<void *>(newLabel.begin(0)), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(assignmentBlobsBuffer) error: " << err);
          clReleaseMemObject(xyzBuffer);
          clReleaseMemObject(elementsBlobsBuffer);
          return;
        }

        cl_mem assignmentBuffer = clCreateBuffer(
                                                 m_data->context,
                                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                 w*h * sizeof(int),
                                                 static_cast<void *>(oldLabel.begin(0)), &err);
        if(err != CL_SUCCESS){
          ERROR_LOG("clCreateBuffer(assignmentBuffer) error: " << err);
          clReleaseMemObject(xyzBuffer);
          clReleaseMemObject(elementsBlobsBuffer);
          clReleaseMemObject(assignmentBlobsBuffer);
          return;
        }

        // Convert Vec (result.n0) to cl_float4 for the kernel argument
        cl_float4 n0Val = {{result.n0[0], result.n0[1], result.n0[2], result.n0[3]}};

        clSetKernelArg(m_data->kernelAssignRANSAC, 0, sizeof(cl_mem), &xyzBuffer);
        clSetKernelArg(m_data->kernelAssignRANSAC, 1, sizeof(cl_mem), &elementsBlobsBuffer);
        clSetKernelArg(m_data->kernelAssignRANSAC, 2, sizeof(cl_mem), &assignmentBlobsBuffer);
        clSetKernelArg(m_data->kernelAssignRANSAC, 3, sizeof(cl_float4), &n0Val);
        clSetKernelArg(m_data->kernelAssignRANSAC, 4, sizeof(float), &result.dist);
        clSetKernelArg(m_data->kernelAssignRANSAC, 5, sizeof(float), &threshold);
        clSetKernelArg(m_data->kernelAssignRANSAC, 6, sizeof(cl_mem), &assignmentBuffer);
        clSetKernelArg(m_data->kernelAssignRANSAC, 7, sizeof(int), &srcID);
        clSetKernelArg(m_data->kernelAssignRANSAC, 8, sizeof(int), &desiredID);

        size_t globalSize = static_cast<size_t>(w*h);
        err = clEnqueueNDRangeKernel(//run kernel
                                     m_data->queue,
                                     m_data->kernelAssignRANSAC,
                                     1,
                                     nullptr,
                                     &globalSize, //input size for get global id
                                     nullptr,
                                     0, nullptr, nullptr);
        if(err != CL_SUCCESS){
          ERROR_LOG("clEnqueueNDRangeKernel error: " << err);
        }

    	  std::vector<int> assignmentBlobs(w*h);
    	  std::vector<unsigned char> elementsBlobs(w*h);

        clEnqueueReadBuffer(//read output from kernel
                            m_data->queue,
                            assignmentBlobsBuffer,
                            CL_TRUE, // block
                            0,
                            w*h * sizeof(int),
                            reinterpret_cast<int*>(&assignmentBlobs[0]),
                            0, nullptr, nullptr);

        clEnqueueReadBuffer(//read output from kernel
                            m_data->queue,
                            elementsBlobsBuffer,
                            CL_TRUE, // block
                            0,
                            w*h * sizeof(bool),
                            reinterpret_cast<cl_uchar*>(&elementsBlobs[0]),
                            0, nullptr, nullptr);

        //newLabel = Img32s(Size(w,h),1,std::vector<int*>(1,assignmentBlobs.data()),false);//,true);//false);
        //newMask = Img8u(Size(w,h),1,std::vector<unsigned char*>(1,elementsBlobs.data()),false);//,true);//false);

        for(int y=0; y<h; y++){
          for(int x=0; x<w; x++){
            newLabel(x,y,0)=assignmentBlobs[x+w*y];
            newMask(x,y,0)=elementsBlobs[x+w*y];
          }
        }

        clReleaseMemObject(xyzBuffer);
        clReleaseMemObject(elementsBlobsBuffer);
        clReleaseMemObject(assignmentBlobsBuffer);
        clReleaseMemObject(assignmentBuffer);
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
          }else{
            newLabel(x,y,0)=oldLabel(x,y,0);
          }
        }
      }
    }

  }
}
