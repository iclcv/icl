// /********************************************************************
// **                Image Component Library (ICL)                    **
// **                                                                 **
// ** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
// **                         Neuroinformatics Group                  **
// ** Website: www.iclcv.org and                                      **
// **          http://opensource.cit-ec.de/projects/icl               **
// **                                                                 **
// ** File   : ICLGeom/src/ICLGeom/IterativeClosestPoint.cpp          **
// ** Module : ICLGeom                                                **
// ** Authors: Matthias Esau                                          **
// **                                                                 **
// **                                                                 **
// ** GNU LESSER GENERAL PUBLIC LICENSE                               **
// ** This file may be used under the terms of the GNU Lesser General **
// ** Public License version 3.0 as published by the                  **
// **                                                                 **
// ** Free Software Foundation and appearing in the file LICENSE.LGPL **
// ** included in the packaging of this file.  Please review the      **
// ** following information to ensure the license requirements will   **
// ** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
// **                                                                 **
// ** The development of this software was supported by the           **
// ** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
// ** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
// ** Forschungsgemeinschaft (DFG) in the context of the German       **
// ** Excellence Initiative.                                          **
// **                                                                 **
// ********************************************************************/

// #include <ICLGeom/IterativeClosestPoint.h>
// #include <ICLGeom/IterativeClosestPointCLCode.h>
// #include <cfloat>
// #include <ICLMath/HomogeneousMath.h>


// #ifdef ICL_HAVE_OPENCL
// #include <ICLUtils/CLProgram.h>
// #include <CL/cl.h>
// #undef CL_VERSION_1_2
// #include <CL/cl.hpp>
// #endif


// namespace icl{
//   namespace geom{
//     using namespace utils;
//     using namespace math;
// #define MATRIX_SIZE 16
// #define MATRIX_WIDTH std::sqrt(MATRIX_SIZE)

// #ifdef ICL_HAVE_OPENCL
//     template<typename T>
//     struct IterativeClosestPoint<T>::Data{

//       void (*subFunc)(const char * a, const char *b, char *c);
//       Vec4 (*toVectorFunc)(const char* point);

//       CLProgram icpProgram;
//       CLKernel getClosestPointsKernel;
//       CLKernel getClosestPointsInDBKernel;
//       CLKernel reduceClosestPointsKernel;
//       CLKernel buildRepDBKernel;
//       CLKernel rotatePointsKernel;
//       CLKernel getErrorKernel;
//       CLKernel collectErrorKernel;
//       CLKernel getCovarianceSumKernel;
//       CLKernel collectCovarianceSumKernel;
//       CLKernel initMemoryKernel;
//       CLKernel sumPointsKernel;
//       CLKernel mulPointsKernel;
//       CLKernel subPointsKernel;

//       CLBuffer pointsABuf;
//       CLBuffer pointsBBuf;
//       CLBuffer centeredABuf;
//       CLBuffer centeredBBuf;
//       CLBuffer pointsRepsBuf;
//       CLBuffer pointsBRotatedBuf;
//       CLBuffer closestRepsABuf;
//       CLBuffer closestRepsBBuf;
//       CLBuffer closestPointsBuf;
//       CLBuffer countersBuf;
//       CLBuffer repDataBaseBuf;
//       CLBuffer rotationMatrixBuf;
//       CLBuffer error0Buf;
//       CLBuffer error1Buf;
//       CLBuffer covariance0Buf;
//       CLBuffer covariance1Buf;
//       CLBuffer center0Buf;
//       CLBuffer center1Buf;
//       CLBuffer centerABuf;
//       CLBuffer centerBBuf;

//       int localSizeX, localSizeY,localSize;
//       unsigned int dimA;
//       unsigned int dimB;
//       unsigned int dimClosestRepsA;
//       unsigned int dimClosestRepsB;
//       unsigned int dimReps;
//       unsigned int dimRepDataBase;
//       unsigned int dimCounters;
//       unsigned int dimClosestPoints;

//       Data(const std::string& clCode, int localSize2Dx, int localSize2Dy, int localSize1D, void (*subFunc)(const char * a, const char *b, char *c), math::Vec4 (*toVectorFunc)(const char* point)){
//         this->subFunc = subFunc;
//         this->toVectorFunc = toVectorFunc;
//         std::string code = clCode+IterativeClosestPointKernelCode;
//         icpProgram = CLProgram("gpu",code.c_str());
//         getClosestPointsKernel = icpProgram.createKernel("getClosestPoints");
//         getClosestPointsInDBKernel = icpProgram.createKernel("getClosestPointsInDB");
//         reduceClosestPointsKernel = icpProgram.createKernel("reduceClosestPoints");
//         buildRepDBKernel = icpProgram.createKernel("buildRepDB");
//         rotatePointsKernel = icpProgram.createKernel("rotatePoints");
//         getErrorKernel = icpProgram.createKernel("getError");
//         collectErrorKernel = icpProgram.createKernel("collectError");
//         getCovarianceSumKernel = icpProgram.createKernel("getCovarianceSum");
//         collectCovarianceSumKernel = icpProgram.createKernel("collectCovarianceSum");
//         initMemoryKernel = icpProgram.createKernel("initMemory");
//         sumPointsKernel = icpProgram.createKernel("sumPoints");
//         mulPointsKernel = icpProgram.createKernel("mulPoints");
//         subPointsKernel = icpProgram.createKernel("subPoints");
//         localSizeX = localSize2Dx;
//         localSizeY = localSize2Dy;
//         localSize = localSize1D;
//         srand(time(NULL));
//       }

//       void loadReps(const T* pointsReps, size_t typeSize, int sizeA, int sizeB, int sizeReps, int repWidth) {
//         dimReps = sizeReps * typeSize;
//         dimRepDataBase = sizeReps*repWidth*sizeof(int);
//         dimClosestRepsA = sizeA * sizeof(int);
//         dimClosestRepsB = sizeB * sizeof(int);
//         dimCounters = sizeReps*sizeof(int);
//         pointsRepsBuf = icpProgram.createBuffer("rw",dimReps);
//         closestRepsABuf = icpProgram.createBuffer("rw",dimClosestRepsA);
//         closestRepsBBuf = icpProgram.createBuffer("rw",dimClosestRepsB);
//         countersBuf = icpProgram.createBuffer("rw",dimCounters);
//         repDataBaseBuf = icpProgram.createBuffer("rw",dimRepDataBase);
//         pointsRepsBuf.write(pointsReps,dimReps);
//       }

//       void loadPoints(const T* pointsA, const T* pointsB, size_t typeSize, int sizeA, int sizeB) {

//         dimA = sizeA * typeSize;
//         dimB = sizeB * typeSize;
//         dimClosestPoints = sizeB*sizeof(int);

//         pointsABuf = icpProgram.createBuffer("r",dimA);
//         pointsBBuf = icpProgram.createBuffer("r",dimB);
//         centeredABuf = icpProgram.createBuffer("rw",dimA);
//         centeredBBuf = icpProgram.createBuffer("rw",dimB);
//         pointsBRotatedBuf = icpProgram.createBuffer("rw",dimB);
//         closestPointsBuf = icpProgram.createBuffer("rw",dimClosestPoints);
//         rotationMatrixBuf = icpProgram.createBuffer("r",MATRIX_SIZE*sizeof(float));

//         error0Buf = icpProgram.createBuffer("rw",sizeB*sizeof(float));
//         error1Buf = icpProgram.createBuffer("rw",sizeB*sizeof(float));


//         covariance0Buf = icpProgram.createBuffer("rw",sizeB*MATRIX_SIZE*sizeof(float));
//         covariance1Buf = icpProgram.createBuffer("rw",sizeB*MATRIX_SIZE*sizeof(float));

//         center0Buf = icpProgram.createBuffer("rw",std::max(sizeA,sizeB)*typeSize);
//         center1Buf = icpProgram.createBuffer("rw",std::max(sizeA,sizeB)*typeSize);

//         centerABuf = icpProgram.createBuffer("rw",typeSize);
//         centerBBuf = icpProgram.createBuffer("rw",typeSize);

//         pointsABuf.write(pointsA,dimA);
//         pointsBBuf.write(pointsB,dimB);
//         centeredABuf.write(pointsA,dimA);
//         centeredBBuf.write(pointsB,dimB);
//       }

//       void getClosestPoints(CLBuffer &pointsA, CLBuffer &pointsB, CLBuffer closestPoints, size_t typeSize, int sizeA, int sizeB){
//         int maxX = 1+(sizeB-1)/localSizeX;
//         int maxY = 1+(sizeA-1)/localSizeY;

//         unsigned int dimDistanceAcc = sizeA * maxX * sizeof(DistanceID);
//         CLBuffer distanceAccBuf = icpProgram.createBuffer("rw",dimDistanceAcc);
//         CLKernel::LocalMemory distances(localSizeY*localSizeX*sizeof(float));

//         getClosestPointsKernel.setArgs(pointsA, pointsB,distanceAccBuf, distances, (int)typeSize, sizeA, sizeB, maxX);
//         getClosestPointsKernel.apply(maxX*localSizeX,maxY*localSizeY,0,localSizeX,localSizeY,0);

//         reduceClosestPointsKernel.setArgs(closestPoints, distanceAccBuf, sizeA, maxX);
//         reduceClosestPointsKernel.apply((1+(sizeA-1)/localSize)*localSize,0,0,localSize,0,0);
//       }

//       void getClosestPointsInDB(CLBuffer &pointsA, CLBuffer &closestReps, CLBuffer &pointsB, CLBuffer pointsDB, CLBuffer counters, CLBuffer closestPoints, size_t typeSize, int sizeA, int widthDB){
//         int maxX = 1+(widthDB-1)/localSizeX;
//         int maxY = 1+(sizeA-1)/localSizeY;
//         unsigned int dimDistanceAcc = sizeA * maxX * sizeof(DistanceID);
//         CLBuffer distanceAccBuf = icpProgram.createBuffer("rw",dimDistanceAcc);
//         CLKernel::LocalMemory distances(localSizeY*localSizeX*sizeof(float));
//         getClosestPointsInDBKernel.setArgs(pointsA, closestReps, pointsB, pointsDB, counters, closestPoints, distanceAccBuf, distances, (int)typeSize, sizeA, widthDB, maxX);
//         getClosestPointsInDBKernel.apply(maxX*localSizeX,maxY*localSizeY,0,localSizeX,localSizeY,0);
//         reduceClosestPointsKernel.setArgs(closestPoints, distanceAccBuf, sizeA, maxX);
//         reduceClosestPointsKernel.apply((1+(sizeA-1)/localSize)*localSize,0,0,localSize,0,0);
//       }

//       void buildRepDB(CLBuffer &pointsABuf, CLBuffer &closestRepsABuf, CLBuffer &countersBuf, CLBuffer &repDataBaseBuf, int repWidth, int typeSize, int sizePoints, int sizeReps) {
//         initMemoryKernel.setArgs(countersBuf,dimCounters);
//         initMemoryKernel.apply(1);
//         buildRepDBKernel.setArgs(pointsABuf, closestRepsABuf, countersBuf, repDataBaseBuf, repWidth, typeSize, sizePoints, sizeReps);
//         buildRepDBKernel.apply((1+(sizePoints-1)/localSize)*localSize,0,0,localSize,0,0);
//       }

//       void getRotatedPoints(CLBuffer &points, CLBuffer &pointsRotated, const float* rotationMatrix, size_t typeSize, int size) {
//         rotationMatrixBuf.write(rotationMatrix,MATRIX_SIZE*sizeof(float));
//         rotatePointsKernel.setArgs(points, pointsRotated, rotationMatrixBuf, (int)typeSize, size);
//         rotatePointsKernel.apply((1+(size-1)/localSize)*localSize,0,0,localSize,0,0);
//       }

//       float getError(CLBuffer &pointsABuf, CLBuffer &pointsBBuf, CLBuffer closestPointsBuf, size_t typeSize, int sizeA) {
//         float error;
//         int length = sizeA;
//         CLKernel::LocalMemory scratch(localSize*sizeof(float));
//         CLBuffer* in = &error0Buf;
//         CLBuffer* out = &error1Buf;
//         getErrorKernel.setArgs(pointsABuf, pointsBBuf, closestPointsBuf,error0Buf, (int)typeSize, sizeA);
//         getErrorKernel.apply((1+(sizeA-1)/localSize)*localSize,0,0,localSize,0,0);
//         while(length > 1) {
//           collectErrorKernel.setArgs(*in,*out, scratch, length);
//           length = (1+(length-1)/localSize);
//           collectErrorKernel.apply(length*localSize,0,0,localSize,0,0);
//           CLBuffer *tmp = in;
//           in = out;
//           out = tmp;
//         }
//         in->read(&error,sizeof(float));
//         return error/sizeA;
//       }

//       void getCovarianceSum(CLBuffer &pointsA, CLBuffer &pointsB, CLBuffer closestPoints, float* covariance, size_t typeSize, int sizeA) {
//         float h[MATRIX_SIZE];
//         int length = sizeA;
//         CLKernel::LocalMemory scratch(localSize*MATRIX_SIZE*sizeof(float));
//         CLBuffer* in = &covariance0Buf;
//         CLBuffer* out = &covariance1Buf;
//         getCovarianceSumKernel.setArgs(pointsA, pointsB, closestPoints,covariance0Buf, (int)typeSize, sizeA);
//         getCovarianceSumKernel.apply((1+(sizeA-1)/localSize)*localSize,0,0,localSize,0,0);
//         while(length > 1) {
//           collectCovarianceSumKernel.setArgs(*in,*out, scratch, length);
//           length = (1+(length-1)/localSize);
//           collectCovarianceSumKernel.apply(length*localSize,0,0,localSize,0,0);
//           CLBuffer *tmp = in;
//           in = out;
//           out = tmp;
//         }
//         in->read(&h,MATRIX_SIZE*sizeof(float));
//         for(int i = 0; i< MATRIX_SIZE; i++)
//           covariance[i] = h[i];
//       }

//       void getCenter(CLBuffer &points, CLBuffer &center, char *centerOut, size_t typeSize, int size) {
//         int length = size;
//         CLKernel::LocalMemory scratch(localSize*typeSize);

//         sumPointsKernel.setArgs(points,center0Buf, scratch, (int)typeSize, length);
//         length = (1+(length-1)/localSize);
//         sumPointsKernel.apply(length*localSize,0,0,localSize,0,0);
//         CLBuffer* in = &center0Buf;
//         CLBuffer* out = &center1Buf;
//         while(length > 1) {
//           sumPointsKernel.setArgs(*in,*out, scratch, (int)typeSize, length);
//           length = (1+(length-1)/localSize);
//           sumPointsKernel.apply(length*localSize,0,0,localSize,0,0);
//           CLBuffer *tmp = in;
//           in = out;
//           out = tmp;
//         }
//         mulPointsKernel.setArgs(*in,center,(float)(1.f/size),(int)typeSize,1);
//         mulPointsKernel.apply(1);
//         center.read(centerOut,typeSize);
//       }

//       void subtract(CLBuffer &in, const char* v, CLBuffer &out, size_t typeSize, int size) {
//         CLBuffer vBuf = icpProgram.createBuffer("rw",typeSize);
//         vBuf.write(v,typeSize);
//         subPointsKernel.setArgs(in, vBuf, out, (int)typeSize, size);
//         subPointsKernel.apply((1+(size-1)/localSize)*localSize,0,0,localSize,0,0);
//       }

//     };

//     template<typename T>
//     void IterativeClosestPoint<T>::icp(const T* pointsA, const T* pointsB, int sizeA, int sizeB, float errorThreshold, float errorDeltaThreshold, int maxIterations, float* initialTransform, float* transformMatrix) {
//       float error = FLT_MAX;
//       float lowestError = FLT_MAX;
//       int sizeReps = std::sqrt(sizeA)+0.5;
//       int repWidth = sizeReps*2;
//       T* pointsReps = (T*)malloc(sizeReps*sizeof(T));
//       char centerA[sizeof(T)];
//       char centerB[sizeof(T)];
//       DynMatrix<float> rotation(4,4);

      //copy initial rotation
      for(int i = 0; i < MATRIX_SIZE; i++) {
        rotation.data()[i] = initialTransform[i];
      }
      //ignore translation because it is calculated internally
      rotation(3,0) = 0;
      rotation(3,1) = 0;
      rotation(3,2) = 0;
      //init output rotation
      for(int i = 0; i < MATRIX_SIZE; i++) {
        transformMatrix[i] = rotation.data()[i];
      }
      m_data->loadPoints(pointsA,pointsB,sizeof(T),sizeA,sizeB);
      m_data->getCenter(m_data->pointsABuf,m_data->centerABuf,centerA,sizeof(T),sizeA);
      m_data->getCenter(m_data->pointsBBuf,m_data->centerBBuf,centerB,sizeof(T),sizeB);
      m_data->subtract(m_data->pointsABuf,centerA,m_data->centeredABuf,sizeof(T),sizeA);
      m_data->subtract(m_data->pointsBBuf,centerB,m_data->centeredBBuf,sizeof(T),sizeB);
      //copy the representatives
      for(int i = 0; i < sizeReps; i++) {
        int index = rand()%sizeA;
        pointsReps[i] = pointsA[index];
      }
      m_data->loadReps(pointsReps,sizeof(T), sizeA, sizeB ,sizeReps, repWidth);
      m_data->subtract(m_data->pointsRepsBuf,centerA,m_data->pointsRepsBuf,sizeof(T),sizeReps);
      m_data->getClosestPoints(m_data->centeredABuf,m_data->pointsRepsBuf,m_data->closestRepsABuf,sizeof(T),sizeA,sizeReps);
      m_data->buildRepDB(m_data->centeredABuf, m_data->closestRepsABuf, m_data->countersBuf, m_data->repDataBaseBuf,repWidth,sizeof(T),sizeA,sizeReps);
      for(int i = 0; i < maxIterations; i++) {
        m_data->getRotatedPoints(m_data->centeredBBuf, m_data->pointsBRotatedBuf,rotation.inv().data(),sizeof(T),sizeB);
        m_data->getClosestPoints(m_data->pointsBRotatedBuf,m_data->pointsRepsBuf,m_data->closestRepsBBuf,sizeof(T),sizeB,sizeReps);
        m_data->getClosestPointsInDB(m_data->pointsBRotatedBuf,m_data->closestRepsBBuf,m_data->centeredABuf,m_data->repDataBaseBuf,m_data->countersBuf,m_data->closestPointsBuf,sizeof(T),sizeB,repWidth);
        //check the error to see if we can stop
        float prevError = error;
        error = m_data->getError(m_data->pointsBRotatedBuf,m_data->centeredABuf,m_data->closestPointsBuf,sizeof(T),sizeB);
        //only copy a new rotation if its better then the old one
        if(error<lowestError) {
          lowestError = error;
          for(int i = 0; i < MATRIX_SIZE; i++) {
            transformMatrix[i] = rotation.data()[i];
          }
        }
        if(((prevError - error) < errorDeltaThreshold) || (error < errorThreshold))break;
        DynMatrix<float> h(MATRIX_WIDTH,MATRIX_WIDTH), u(MATRIX_WIDTH,MATRIX_WIDTH),s(MATRIX_WIDTH,MATRIX_WIDTH),v(MATRIX_WIDTH,MATRIX_WIDTH),r(MATRIX_WIDTH,MATRIX_WIDTH);
        m_data->getCovarianceSum(m_data->pointsBRotatedBuf,m_data->centeredABuf,m_data->closestPointsBuf,h.data(),sizeof(T),sizeB);
        h.svd(u,s,v);
        r = u*v.transp();
        if(r.det() < 0) {
          r.at(2,0) *= -1;
          r.at(2,1) *= -1;
          r.at(2,2) *= -1;
        }
        rotation = r * rotation;
      }
      char translation[sizeof(T)];
      m_data->subFunc(centerB,centerA,translation);
      Vec4 trans = m_data->toVectorFunc(translation);
      transformMatrix[0*4+3] = trans[0];
      transformMatrix[1*4+3] = trans[1];
      transformMatrix[2*4+3] = trans[2];
      free(pointsReps);
    }
    template<typename T>
    IterativeClosestPoint<T>::IterativeClosestPoint(const std::string &clCode, int localSize2Dx, int localSize2Dy, int localSize1D, void (*subFunc)(const char * a, const char *b, char *c), math::Vec4 (*toVectorFunc)(const char* point)) {
        m_data = new Data(clCode, localSize2Dx, localSize2Dy, localSize1D, subFunc, toVectorFunc);
    }
#else
    template<class T>
    struct IterativeClosestPoint<T>::Data{};

    template<typename T>
    void IterativeClosestPoint<T>::icp(const char* pointsA, const char* pointsB, int sizeA, int sizeB, size_t typeSize, float errorThreshold, float errorDeltaThreshold, int maxIterations, float* initialTransform, float* transformMatrix, void (*subFunc)(const char * a, const char *b, char *c), void (*neutralElementFunc)(char * e), math::Vec4 (*toVectorFunc)(const char* point)){}
    template<typename T>
    IterativeClosestPoint<T>::IterativeClosestPoint(const std::string &clCode, void (*subFunc)(const char * a, const char *b, char *c), math::Vec4 (*toVectorFunc)(const char* point)) {}
    }
#endif

//     template<typename T>
//     IterativeClosestPoint<Vec4> IterativeClosestPoint<T>::icpVec4() {
//       return IterativeClosestPoint<Vec4>(IterativeClosestPointTypeCodeVec4,12,12,128,subVec4,toVectorVec4);
//     }

//     template<typename T>
//     IterativeClosestPoint<Vec8> IterativeClosestPoint<T>::icpVec8() {
//       return IterativeClosestPoint<Vec8>(IterativeClosestPointTypeCodeVec8,12,12,128,subVec8,toVectorVec8);
//     }

//     template class ICLGeom_API IterativeClosestPoint<Vec4>;
//     template class ICLGeom_API IterativeClosestPoint<Vec8>;

// //  template<typename T>
// //  void IterativeClosestPoint<T>::sub(const char * a, const char *b, char *c) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    const Vec4* pa = (const Vec4*)a;
// //    const Vec4* pb = (const Vec4*)b;
// //    Vec4* pc = (Vec4*)c;
// //    pc->x = pa->x-pb->x;
// //    pc->y = pa->y-pb->y;
// //    pc->z = pa->z-pb->z;
// //    pc->w = 1;
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::add(const char * a, const char *b, char *c) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    const Vec4* pa = (const Vec4*)a;
// //    const Vec4* pb = (const Vec4*)b;
// //    Vec4* pc = (Vec4*)c;
// //    pc->x = pa->x+pb->x;
// //    pc->y = pa->y+pb->y;
// //    pc->z = pa->z+pb->z;
// //    pc->w = 1;
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::mul(const char * a, float b, char *c) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    const Vec4* pa = (const Vec4*)a;
// //    Vec4* pc = (Vec4*)c;
// //    pc->x = pa->x*b;
// //    pc->y = pa->y*b;
// //    pc->z = pa->z*b;
// //    pc->w = pa->w;
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::neutralElement(char * e) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    Vec4* pe = (Vec4*)e;
// //    pe->x = 0;
// //    pe->y = 0;
// //    pe->z = 0;
// //    pe->w = 1;
// //  }

// //  template<typename T>
// //  Vec4 IterativeClosestPoint<T>::toVector(const char* point) {
// //    return *(const Vec4*)point;
// //  }


// //  template<typename T>
// //  void IterativeClosestPoint<T>::icp(const char* pointsA, const char* pointsB, int sizeA, int sizeB, size_t typeSize, float errorThreshold, float errorDeltaThreshold, int maxIterations, float* initialTransform, float* transformMatrix, float(*distanceFunc)(const char*, const char*), void(*covarianceFunc)(const char*, const char*, float*), void(*matrixMultiplyFunc)(const char * a, char *b, const float* r), void (*subFunc)(const char * a, const char *b, char *c), void (*addFunc)(const char * a, const char *b, char *c), void (*mulFunc)(const char * a, float b, char *c), void (*neutralElementFunc)(char * e), math::Vec4 (*toVectorFunc)(const char* point)) {
// //    int localSizeX = 12;
// //    int localSizeY = 12;
// //    int maxX = 1+(sizeB-1)/localSizeX;
// //    int maxY = 1+(sizeA-1)/localSizeY;
// //    float error = FLT_MAX;
// //    float lowestError = FLT_MAX;

// //    int sizeReps = std::sqrt(sizeA)+0.5;
// //    int repWidth = sizeReps*2;
// //    DistanceID* distanceAcc = (DistanceID*)malloc(sizeA*localSizeY*maxX*sizeof(DistanceID));
// //    char* centeredA = (char*)malloc(sizeA*typeSize);
// //    char* centeredB = (char*)malloc(sizeB*typeSize);
// //    char* pointsBRotated = (char*)malloc(sizeB*typeSize);
// //    char* pointsReps = (char*)malloc(sizeReps*typeSize);
// //    int* closestRepsA = (int*)malloc(sizeA*sizeof(int));
// //    int* closestRepsB = (int*)malloc(sizeB*sizeof(int));
// //    int* counters = (int*)malloc(sizeReps*sizeof(int));
// //    int* repDataBase = (int*)malloc(sizeReps*repWidth*sizeof(int));
// //    int* closestPoints = (int*)malloc(sizeB*sizeof(int));
// //    char centerA[typeSize];
// //    char centerB[typeSize];
// //    DynMatrix<float> rotation(4,4);

// //    //copy initial rotation
// //    for(int i = 0; i < MATRIX_SIZE; i++) {
// //      rotation.data()[i] = initialTransform[i];
// //    }
// //    //ignore translation because it is calculated internally
// //    rotation(3,0) = 0;
// //    rotation(3,1) = 0;
// //    rotation(3,2) = 0;
// //    //init output rotation
// //    for(int i = 0; i < MATRIX_SIZE; i++) {
// //      transformMatrix[i] = rotation.data()[i];
// //    }
// //    neutralElementFunc(centerA);
// //    for(int i = 0; i < sizeA; i++) {
// //      addFunc(centerA,pointsA+i*typeSize,centerA);
// //    }
// //    mulFunc(centerA,1.f/sizeA,centerA);
// //    neutralElementFunc(centerB);
// //    for(int i = 0; i < sizeB; i++) {
// //      addFunc(centerB,pointsB+i*typeSize,centerB);
// //    }
// //    mulFunc(centerB,1.f/sizeB,centerB);
// //    for(int i = 0; i < sizeA; i++) {
// //      subFunc(pointsA+i*typeSize,centerA,centeredA+i*typeSize);
// //    }
// //    for(int i = 0; i < sizeB; i++) {
// //      subFunc(pointsB+i*typeSize,centerB,centeredB+i*typeSize);
// //    }
// //    for(int i = 0; i < sizeReps; i++) {
// //      subFunc(pointsReps+i*typeSize,centerA,pointsReps+i*typeSize);
// //    }
// //    //copy the representatives
// //    for(int i = 0; i < sizeReps; i++) {
// //      int index = rand()%sizeA;
// //      subFunc(pointsA+typeSize*index,centerA,pointsReps+i*typeSize);
// //    }
// //    getClosestPoints(centeredA,pointsReps,closestRepsA,distanceAcc,distanceFunc,typeSize,sizeA,sizeReps,localSizeX,localSizeY);
// //    buildRepDB(centeredA,closestRepsA,counters,repDataBase,repWidth,typeSize,sizeA,sizeReps,localSizeX);
// //    for(int i = 0; i < maxIterations; i++) {
// //      getRotatedPoints(centeredB,pointsBRotated,rotation.inv().data(),matrixMultiplyFunc,typeSize,sizeB);
// //      getClosestPoints(pointsBRotated,pointsReps,closestRepsB,distanceAcc,distanceFunc,typeSize,sizeB,sizeReps,localSizeX,localSizeY);
// //      getClosestPointsInDB(pointsBRotated,closestRepsB,centeredA,repDataBase,counters,closestPoints,distanceAcc,distanceFunc,typeSize,sizeB,repWidth,localSizeX,localSizeY);
// //      //check the error to see if we can stop
// //      float prevError = error;
// //      error = getError(pointsBRotated,centeredA,closestPoints,distanceFunc,typeSize,sizeB);
// //      //only copy a new rotation if its better then the old one
// //      if(error<lowestError) {
// //        lowestError = error;
// //        for(int i = 0; i < MATRIX_SIZE; i++) {
// //          transformMatrix[i] = rotation.data()[i];
// //        }
// //      }
// //      if(((prevError - error) < errorDeltaThreshold) || (error < errorThreshold))break;
// //      DynMatrix<float> h(MATRIX_WIDTH,MATRIX_WIDTH), u(MATRIX_WIDTH,MATRIX_WIDTH),s(MATRIX_WIDTH,MATRIX_WIDTH),v(MATRIX_WIDTH,MATRIX_WIDTH),r(MATRIX_WIDTH,MATRIX_WIDTH);
// //      getCovarianceSum(pointsBRotated,centeredA,closestPoints,h.data(),covarianceFunc,typeSize,sizeB);
// //      h.svd(u,s,v);
// //      r = u*v.transp();
// //      if(r.det() < 0) {
// //        r.at(2,0) *= -1;
// //        r.at(2,1) *= -1;
// //        r.at(2,2) *= -1;
// //      }
// //      rotation = r * rotation;

// //    }
// //    char translation[typeSize];
// //    subFunc(centerB,centerA,translation);
// //    Vec4 trans = toVectorFunc(translation);
// //    transformMatrix[0*4+3] = trans[0];
// //    transformMatrix[1*4+3] = trans[1];
// //    transformMatrix[2*4+3] = trans[2];

// //    free(centeredA);
// //    free(centeredB);
// //    free(distanceAcc);
// //    free(pointsBRotated);
// //    free(pointsReps);
// //    free(closestRepsA);
// //    free(closestRepsB);
// //    free(counters);
// //    free(repDataBase);
// //    free(closestPoints);
// //  }

// //  template<typename T>
// //  float IterativeClosestPoint<T>::getDistance(const char * a, const char *b) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    const Vec4* pa = (const Vec4*)a;
// //    const Vec4* pb = (const Vec4*)b;
// //    Vec4 dist;
// //    dist.x = pa->x-pb->x;
// //    dist.y = pa->y-pb->y;
// //    dist.z = pa->z-pb->z;
// //    return std::sqrt(dist.x*dist.x+dist.y*dist.y+dist.z*dist.z);
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::getCovariance(const char * a, const char *b, float* h) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };
// //    const Vec4* pa = (const Vec4*)a;
// //    const Vec4* pb = (const Vec4*)b;
// //    h[0] = pa->x*pb->x;
// //    h[1] = pa->x*pb->y;
// //    h[2] = pa->x*pb->z;
// //    h[3] = 0;

// //    h[4] = pa->y*pb->x;
// //    h[5] = pa->y*pb->y;
// //    h[6] = pa->y*pb->z;
// //    h[7] = 0;

// //    h[8] = pa->z*pb->x;
// //    h[9] = pa->z*pb->y;
// //    h[10] = pa->z*pb->z;
// //    h[11] = 0;

// //    h[12] = 0;
// //    h[13] = 0;
// //    h[14] = 0;
// //    h[15] = 1;
// //  }


// //  template<typename T>
// //  void IterativeClosestPoint<T>::matrixMultiply(const char * a, char *b, const float* m) {
// //    struct Vec4{
// //      float x;
// //      float y;
// //      float z;
// //      float w;
// //    };

// //    const Vec4* pa = (const Vec4*)a;
// //    Vec4* pb = (Vec4*)b;
// //    pb->x = m[0]*pa->x+m[1]*pa->y+m[2]*pa->z+m[3]*pa->w;
// //    pb->y = m[4]*pa->x+m[5]*pa->y+m[6]*pa->z+m[7]*pa->w;
// //    pb->z = m[8]*pa->x+m[9]*pa->y+m[10]*pa->z+m[11]*pa->w;
// //    pb->w = m[12]*pa->x+m[13]*pa->y+m[14]*pa->z+m[15]*pa->w;
// //  }

// //  template<typename T>
// //  float IterativeClosestPoint<T>::getError(const char* pointsA, const char* pointsB, const int* closestPoints, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA) {
// //    float error_sum = 0;
// //    for(int i = 0; i < sizeA; i++) {
// //      error_sum += distanceFunc(pointsA+i*typeSize,pointsB+closestPoints[i]*typeSize);
// //    }
// //    return error_sum/sizeA;
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::getCovarianceSum(const char* pointsA, const char* pointsB, const int* closestPoints, float* covariance, void(*covarianceFunc)(const char*, const char*, float*), size_t typeSize, int sizeA) {
// //    float h[MATRIX_SIZE];
// //    for(int i = 0; i< MATRIX_SIZE; i++) {
// //      covariance[i] = 0;
// //    }
// //    for(int i = 0; i < sizeA; i++) {
// //      covarianceFunc(pointsA+i*typeSize,pointsB+closestPoints[i]*typeSize,h);
// //      for(int j = 0; j < MATRIX_SIZE; j++) {
// //        covariance[j]+=h[j];
// //      }
// //    }
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::getRotatedPoints(const char* pointsA, char* pointsRotated, const float* rotationMatrix, void(*matrixMultiplyFunc)(const char * a, char *b, const float* r), size_t typeSize, int sizeA) {
// //    for(int i = 0; i < sizeA; i++) {
// //      matrixMultiplyFunc(pointsA+i*typeSize,pointsRotated+i*typeSize,rotationMatrix);
// //    }
// //  }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::getClosestPoints(const char* pointsA, const char* pointsB, int* closestPoints, DistanceID* distanceAcc, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA, int sizeB, int localSizeX, int localSizeY) {
// //      //amount of localgroups in X direction rounded up
// //      int maxX = 1+(sizeB-1)/localSizeX;
// //      //amount of localgroups in Y direction rounded up
// //      int maxY = 1+(sizeA-1)/localSizeY;
// //      for(int i = 0; i < maxX;i++) {
// //        for(int j = 0; j < maxY;j++) {
// //          float distances[localSizeX*localSizeY];
// //          for(int localX = 0; localX < localSizeX; localX++) {
// //            for(int localY = 0; localY < localSizeY; localY++) {
// //              int globalX = i*localSizeX+localX;
// //              int globalY = j*localSizeY+localY;
// //              int localGroupX = i;
// //              int localGroupY = j;
// //              //___________________________________________________kernel code starts here_____________________________

// //              //calculate distance for current pair
// //              distances[localY*localSizeX+localX] = FLT_MAX;
// //              if(globalX < sizeB && globalY < sizeA)
// //                distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*globalX,pointsA+typeSize*globalY);

// //              //find closest distances in local group by letting only the last vertical line of threads in a local group calculate
// //              if(localX == (localSizeX - 1) && globalY < sizeA) {
// //                int localStartX = localGroupX*localSizeX;
// //                DistanceID min;
// //                min.distance = distances[localY*localSizeX];
// //                min.id = 0;
// //                for(int k = 1; k < localSizeX; k++) {
// //                  float distance = distances[localY*localSizeX+k];
// //                  if(distance < min.distance) {
// //                    min.distance = distance;
// //                    min.id = k;
// //                  }
// //                }
// //                //convert id from local to global space
// //                min.id += localStartX;
// //                distanceAcc[globalY*maxX+localGroupX] = min;
// //              }

// //              //_________global_barrier______

// //              //find closest point across all groups by letting only the last vertical line of threads calculate
// //              if(globalX == (localSizeX*maxX-1) && globalY < sizeA) {
// //                int startX = globalY*maxX;
// //                int endX = startX+maxX;
// //                DistanceID min = distanceAcc[startX];
// //                for(int k = startX+1; k < endX; k++) {
// //                  DistanceID point = distanceAcc[k];
// //                  if(point.distance < min.distance) {
// //                    min = point;
// //                  }
// //                }
// //                closestPoints[globalY] = min.id;
// //              }
// //            }
// //          }
// //        }
// //      }
// //    }

// //  template<typename T>
// //  void IterativeClosestPoint<T>::buildRepDB(const char* points, const int* closestReps, int* counters, int* repDataBase, int repWidth, size_t typeSize, int sizePoints, int sizeReps, int localSizeX) {
// //    //init counters because membars dont work in loops
// //    for(int i = 0; i < sizeReps; i++) {
// //      counters[i] = 0;
// //    }
// //    //amount of localgroups in X direction rounded up
// //    int maxX = 1+(sizePoints-1)/localSizeX;
// //    for(int i = 0; i < maxX;i++) {
// //      for(int localX = 0; localX < localSizeX; localX++) {
// //        int globalX = i*localSizeX+localX;
// //        int localGroupX = i;
// //        //___________________________________________________kernel code starts here_____________________________

// //        //_______global_barrier_______
// //        if(globalX < sizePoints) {
// //          int rep = closestReps[globalX];
// //          int pos = counters[rep]++;//atomic inc
// //          //check if the there are to many neighbours
// //          if(pos < repWidth) {
// //            repDataBase[repWidth*rep+pos] = globalX;
// //          }
// //        }
// //      }
// //    }
// //  }


// //  template<typename T>
// //  void IterativeClosestPoint<T>::getClosestPointsInDB(const char* pointsA, const int* closestRep, const char* pointsB, const int* pointsDB, const int* counters, int* closestPoints, DistanceID* distanceAcc, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA, int widthDB, int localSizeX, int localSizeY) {
// //    //amount of localgroups in X direction rounded up
// //    int maxX = 1+(widthDB-1)/localSizeX;
// //    //amount of localgroups in Y direction rounded up
// //    int maxY = 1+(sizeA-1)/localSizeY;
// //    for(int i = 0; i < maxX;i++) {
// //      for(int j = 0; j < maxY;j++) {
// //        float distances[localSizeX*localSizeY];
// //        for(int localX = 0; localX < localSizeX; localX++) {
// //          for(int localY = 0; localY < localSizeY; localY++) {
// //            int globalX = i*localSizeX+localX;
// //            int globalY = j*localSizeY+localY;
// //            int localGroupX = i;
// //            int localGroupY = j;
// //            //___________________________________________________kernel code starts here_____________________________

// //            //calculate distance for current pair
// //            distances[localY*localSizeX+localX] = FLT_MAX;
// //            int maxWidth = widthDB;
// //            if(globalY < sizeA)maxWidth = std::min(widthDB,counters[closestRep[globalY]]);
// //            if(globalY < sizeA && globalX < maxWidth)
// //              distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*pointsDB[closestRep[globalY]*widthDB+globalX],pointsA+typeSize*globalY);

// //            //_________local_barrier______

// //            //find closest distances in local group by letting only the first vertical line of threads in a local group calculate
// //            if(localX == (localSizeX-1) && globalY < sizeA) {
// //              int localStartX = localGroupX*localSizeX;
// //              DistanceID min;
// //              min.distance = distances[localY*localSizeX];
// //              min.id = 0;
// //              for(int k = 1; k < localSizeX; k++) {
// //                float distance = distances[localY*localSizeX+k];
// //                if(distance < min.distance) {
// //                  min.distance = distance;
// //                  min.id = k;
// //                }
// //              }
// //              //convert id from local to global space
// //              int id = min.id;
// //              min.id = pointsDB[closestRep[globalY]*widthDB+(min.id+localStartX)];
// //              distanceAcc[globalY*maxX+localGroupX] = min;
// //            }

// //            //_________global_barrier______

// //            //find closest point across all groups by letting only the last vertical line of threads calculate
// //            if(globalY < sizeA && globalX == (localSizeX*maxX-1)) {
// //              int startX = globalY*maxX;
// //              int endX = startX+maxX;
// //              DistanceID min = distanceAcc[startX];
// //              for(int k = startX+1; k < endX; k++) {
// //                DistanceID point = distanceAcc[k];
// //                if(point.distance < min.distance) {
// //                  min = point;
// //                }
// //              }
// //              closestPoints[globalY] = min.id;
// //            }
// //          }
// //        }
// //      }
// //    }
// //  }
//   } // namespace geom
// }
