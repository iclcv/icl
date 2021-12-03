/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/IterativeClosestPointCLCode.h      **
** Module : ICLGeom                                                **
** Authors: Matthias Esau                                          **
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
#include <string>
#include <ICLMath/HomogeneousMath.h>
namespace icl{
  namespace geom{
    void subVec4(const char * a, const char *b, char *c) {
      struct Vec4{
        float x;
        float y;
        float z;
        float w;
      };

      const Vec4* pa = (const Vec4*)a;
      const Vec4* pb = (const Vec4*)b;
      Vec4* pc = (Vec4*)c;
      pc->x = pa->x-pb->x;
      pc->y = pa->y-pb->y;
      pc->z = pa->z-pb->z;
      pc->w = 1;
    }

    Vec4 toVectorVec4(const char* point) {
      return *(const Vec4*)point;
    }

    Vec4 toVectorVec8(const char* point) {
      return *(const Vec4*)point;
    }

    void subVec8(const char * a, const char *b, char *c) {
      struct Vec4{
        float x;
        float y;
        float z;
        float w;
      };
      struct Vec8{
        Vec4 pos;
        Vec4 col;
      };
      const Vec8* pa = (const Vec8*)a;
      const Vec8* pb = (const Vec8*)b;
      Vec8* pc = (Vec8*)c;
      pc->pos.x = pa->pos.x-pb->pos.x;
      pc->pos.y = pa->pos.y-pb->pos.y;
      pc->pos.z = pa->pos.z-pb->pos.z;
      pc->pos.w = 1;
      pc->col.x = pa->col.x;
      pc->col.y = pa->col.y;
      pc->col.z = pa->col.z;
      pc->col.w = 1;
    }

    const std::string IterativeClosestPointKernelCode = (
          "#define MATRIX_SIZE 16\n"
          "\n"
          "__kernel void getClosestPoints(const __global char* pointsA, const __global char* pointsB, __global DistanceID* distanceAcc, __local float* distances, int typeSize, int sizeA, int sizeB, int maxX) {\n"
          "  int globalX = get_global_id(0);\n"
          "  int globalY = get_global_id(1);\n"
          "  int localX = get_local_id(0);\n"
          "  int localY = get_local_id(1);\n"
          "  int localSizeX = get_local_size(0);\n"
          "  int localGroupX = get_group_id(0);\n"
          "  //calculate distance for current pair\n"
          "  distances[localY*localSizeX+localX] = FLT_MAX;\n"
          "  if(globalX < sizeB && globalY < sizeA)\n"
          "    distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*globalX,pointsA+typeSize*globalY);\n"
          "\n"
          "  //_________local_barrier______\n"
          "  barrier(CLK_LOCAL_MEM_FENCE);\n"
          "\n"
          "  //find closest distances in local group by letting only the last vertical line of threads in a local group calculate\n"
          "  if(localX == 0 && globalY < sizeA) {\n"
          "    int localStartX = localGroupX*localSizeX;\n"
          "    DistanceID min;\n"
          "    min.distance = distances[localY*localSizeX];\n"
          "    min.id = 0;\n"
          "    for(int k = 1; k < localSizeX; k++) {\n"
          "      float distance = distances[localY*localSizeX+k];\n"
          "      if(distance < min.distance) {\n"
          "        min.distance = distance;\n"
          "        min.id = k;\n"
          "      }\n"
          "    }\n"
          "    //convert id from local to global space\n"
          "    min.id += localStartX;\n"
          "    distanceAcc[globalY*maxX+localGroupX] = min;\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void reduceClosestPoints(__global int* closestPoints, __global DistanceID* distanceAcc, int sizeA, int widthDistanceAcc) {\n"
          "  int globalX = get_global_id(0);\n"
          "\n"
          "  //find closest point across all groups by letting only the last vertical line of threads calculate\n"
          "  if(globalX < sizeA) {\n"
          "    int startX = globalX*widthDistanceAcc;\n"
          "    int endX = startX+widthDistanceAcc;\n"
          "    DistanceID min = distanceAcc[startX];\n"
          "    for(int k = startX+1; k < endX; k++) {\n"
          "      DistanceID point = distanceAcc[k];\n"
          "      if(point.distance < min.distance) {\n"
          "        min = point;\n"
          "      }\n"
          "    }\n"
          "    closestPoints[globalX] = min.id;\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void getClosestPointsInDB(const __global char* pointsA, const __global int* closestReps, const __global char* pointsB, const __global int* pointsDB, const __global int* counters, __global int* closestPoints, __global DistanceID* distanceAcc, __local float* distances, int typeSize, int sizeA, int widthDB, int maxX) {\n"
          "  int globalX = get_global_id(0);\n"
          "  int globalY = get_global_id(1);\n"
          "  int localX = get_local_id(0);\n"
          "  int localY = get_local_id(1);\n"
          "  int localSizeX = get_local_size(0);\n"
          "  int localGroupX = get_group_id(0);\n"
          "\n"
          "  //calculate distance for current pair\n"
          "  distances[localY*localSizeX+localX] = FLT_MAX;\n"
          "  int maxWidth;\n"
          "  if(globalY < sizeA)maxWidth = min(widthDB,counters[closestReps[globalY]]);\n"
          "  if(globalY < sizeA && globalX < maxWidth)\n"
          "    distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*pointsDB[closestReps[globalY]*widthDB+globalX],pointsA+typeSize*globalY);\n"
          "\n"
          "\n"
          "  //_________local_barrier______\n"
          "  barrier(CLK_LOCAL_MEM_FENCE);\n"
          "\n"
          "\n"
          "  //find closest distances in local group by letting only the first vertical line of threads in a local group calculate\n"
          "  if(localX == 0 && globalY < sizeA) {\n"
          "    int localStartX = localGroupX*localSizeX;\n"
          "    DistanceID min;\n"
          "    min.distance = distances[localY*localSizeX];\n"
          "    min.id = 0;\n"
          "    for(int k = 1; k < localSizeX; k++) {\n"
          "      float distance = distances[localY*localSizeX+k];\n"
          "      if(distance < min.distance) {\n"
          "        min.distance = distance;\n"
          "        min.id = k;\n"
          "      }\n"
          "    }\n"
          "    //convert id from local to global space\n"
          "    min.id = pointsDB[closestReps[globalY]*widthDB+(min.id+localStartX)];\n"
          "    distanceAcc[globalY*maxX+localGroupX] = min;\n"
          "  }\n"
          "}\n"
          "\n"
          "\n"
          "__kernel void initMemory(__global char* mem, int size) {\n"
          "  if(get_global_id(0) == 0) {\n"
          "    for(int i = 0; i < size; i++)mem[i] = 0;\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void buildRepDB(const __global char* points, const __global int* closestReps, __global int* counters, __global int* repDataBase, int repWidth, int typeSize, int sizePoints, int sizeReps) {\n"
          "    int globalX = get_global_id(0);\n"
          "    int localX = get_local_id(0);\n"
          "    int localSizeX = get_local_size(0);\n"
          "    int localGroupX = get_group_id(0);\n"
          "\n"
          "    if(globalX < sizePoints) {\n"
          "        int rep = closestReps[globalX];\n"
          "        int pos = atomic_inc(counters+rep);\n"
          "        //check if the there are to many neighbours\n"
          "        if(pos < repWidth) {\n"
          "            repDataBase[repWidth*rep+pos] = globalX;\n"
          "        }\n"
          "    }\n"
          "}\n"
          "\n"
          "__kernel void rotatePoints(const __global char* points, __global char* pointsRotated, const __global float* rotationMatrix, int typeSize, int sizePoints) {\n"
          "  int globalX = get_global_id(0);\n"
          "  if(globalX < sizePoints) {\n"
          "    int offset = typeSize*globalX;\n"
          "    matrixMultiply(points+offset, pointsRotated+offset, rotationMatrix);\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void getError(const __global char* pointsA, const __global char* pointsB, const __global int* closestPoints, __global float* errors, int typeSize, int sizeA) {\n"
          "  int globalX = get_global_id(0);\n"
          "  if(globalX < sizeA)errors[globalX] = distanceFunc(pointsA+globalX*typeSize,pointsB+closestPoints[globalX]*typeSize);\n"
          "}\n"
          "\n"
          "__kernel void collectError(const __global float* in, __global float* out, __local float* scratch, int length) {\n"
          "  int global_index = get_global_id(0);\n"
          "  int local_index = get_local_id(0);\n"
          "  if (global_index < length) {\n"
          "    scratch[local_index] = in[global_index];\n"
          "  } else {\n"
          "    scratch[local_index] = 0;\n"
          "  }\n"
          "  barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  for(int offset = get_local_size(0) / 2;\n"
          "      offset > 0;\n"
          "      offset >>= 1) {\n"
          "    if (local_index < offset) {\n"
          "      float other = scratch[local_index + offset];\n"
          "      float mine = scratch[local_index];\n"
          "      scratch[local_index] = mine+other;\n"
          "    }\n"
          "    barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  }\n"
          "  if (local_index == 0) {\n"
          "    out[get_group_id(0)] = scratch[0];\n"
          "  }\n"
          "}\n"
          "\n"
          "\n"
          "__kernel void getCovarianceSum(const __global char* pointsA, const __global char* pointsB, const __global int* closestPoints, __global float* covariances, int typeSize, int sizeA) {\n"
          "  int globalX = get_global_id(0);\n"
          "  if(globalX < sizeA) getCovariance(pointsA+globalX*typeSize,pointsB+closestPoints[globalX]*typeSize,covariances+globalX*MATRIX_SIZE);\n"
          "}\n"
          "\n"
          "__kernel void collectCovarianceSum(const __global float* in, __global float* out, __local float* scratch, int length) {\n"
          "  int global_index = get_global_id(0);\n"
          "  int local_index = get_local_id(0);\n"
          "  if (global_index < length) {\n"
          "    for(int i = 0; i < MATRIX_SIZE; i++)\n"
          "      scratch[local_index*MATRIX_SIZE+i] = in[global_index*MATRIX_SIZE+i];\n"
          "  } else {\n"
          "    for(int i = 0; i < MATRIX_SIZE; i++)\n"
          "      scratch[local_index*MATRIX_SIZE+i] = 0;\n"
          "  }\n"
          "  barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  for(int offset = get_local_size(0) / 2;\n"
          "      offset > 0;\n"
          "      offset >>= 1) {\n"
          "    if (local_index < offset) {\n"
          "      for(int i = 0; i < MATRIX_SIZE; i++) {\n"
          "        float other = scratch[(local_index + offset)*MATRIX_SIZE+i];\n"
          "        float mine = scratch[local_index*MATRIX_SIZE+i];\n"
          "        scratch[local_index*MATRIX_SIZE+i] = mine+other;\n"
          "      }\n"
          "    }\n"
          "    barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  }\n"
          "  if (local_index == 0) {\n"
          "    for(int i = 0; i < MATRIX_SIZE; i++)\n"
          "      out[get_group_id(0)*MATRIX_SIZE+i] = scratch[i];\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void sumPoints(const __global char* in, __global char* out, __local char* scratch, int typeSize, int length) {\n"
          "  int global_index = get_global_id(0);\n"
          "  int local_index = get_local_id(0);\n"
          "  int local_size = get_local_size(0);\n"
          "\n"
          "  if (global_index < length) {\n"
          "    for(int i = 0; i < typeSize; i++)\n"
          "      scratch[local_index*typeSize+i] = in[global_index*typeSize+i];\n"
          "  } else {\n"
          "      neutralElement_local(scratch+local_index*typeSize);\n"
          "  }\n"
          "  barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  for(int offset = get_local_size(0) / 2;\n"
          "      offset > 0;\n"
          "      offset >>= 1) {\n"
          "    if (local_index < offset) {\n"
          "      add_local(scratch+(local_index + offset)*typeSize,scratch+local_index*typeSize,scratch+local_index*typeSize);\n"
          "    }\n"
          "    barrier(CLK_LOCAL_MEM_FENCE);\n"
          "  }\n"
          "  if (local_index == 0) {\n"
          "    for(int i = 0; i < typeSize; i++)\n"
          "      out[get_group_id(0)*typeSize+i] = scratch[i];\n"
          "  }\n"
          "}\n"
          "\n"
          "__kernel void mulPoints(const __global char* in, __global char* out, float v, int typeSize, int length) {\n"
          "  int globalX = get_global_id(0);\n"
          "  if(globalX<length)mul_global(in+globalX*typeSize,v,out+globalX*typeSize);\n"
          "}\n"
          "\n"
          "__kernel void subPoints(const __global char* in, const __global char* v, __global char* out, int typeSize, int length) {\n"
          "  int globalX = get_global_id(0);\n"
          "  if(globalX< length)sub_global(in+globalX*typeSize,v,out+globalX*typeSize);\n"
          "}\n"
          );
    const std::string IterativeClosestPointTypeCodeVec4 = (
          "typedef struct Vec4_t{\n"
          "  float x;\n"
          "  float y;\n"
          "  float z;\n"
          "  float w;\n"
          "}Vec4;\n"
          "\n"
          "typedef struct DistanceID_t {\n"
          "  float distance;\n"
          "  int id;\n"
          "}DistanceID;\n"
          "\n"
          "void add(const char * a, const char *b, char * c) {\n"
          "  const Vec4* pa = (const Vec4*)a;\n"
          "  const Vec4* pb = (const Vec4*)b;\n"
          "  Vec4* pc = (Vec4*)c;\n"
          "  pc->x = pa->x+pb->x;\n"
          "  pc->y = pa->y+pb->y;\n"
          "  pc->z = pa->z+pb->z;\n"
          "}\n"
          "\n"
          "void add_global(const __global char * a, const __global char *b, __global char * c) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  const __global Vec4* pb = (const __global Vec4*)b;\n"
          "  __global Vec4* pc = (__global Vec4*)c;\n"
          "  pc->x = pa->x+pb->x;\n"
          "  pc->y = pa->y+pb->y;\n"
          "  pc->z = pa->z+pb->z;\n"
          "  pc->w = 1;\n"
          "}\n"
          "\n"
          "void add_local(const __local char * a, const __local char *b, __local char * c) {\n"
          "//  const __local Vec4* pa = (const __local Vec4*)a;\n"
          "//  const __local Vec4* pb = (const __local Vec4*)b;\n"
          "//  __local Vec4* pc = (__local Vec4*)c;\n"
          "//  pc->x = pa->x+pb->x;\n"
          "//  pc->y = pa->y+pb->y;\n"
          "//  pc->z = pa->z+pb->z;\n"
          "\n"
          "  char ac[16];\n"
          "  char bc[16];\n"
          "  char cc[16];\n"
          "  for(int i = 0; i < 16; i++) {\n"
          "      ac[i] = a[i];\n"
          "      bc[i] = b[i];\n"
          "  }\n"
          "  add(ac,bc,cc);\n"
          "  for(int i = 0; i < 16; i++) {\n"
          "      c[i] = cc[i];\n"
          "  }\n"
          "}\n"
          "\n"
          "void sub_global(const __global char * a, const __global char *b, __global char * c) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  const __global Vec4* pb = (const __global Vec4*)b;\n"
          "  __global Vec4* pc = (__global Vec4*)c;\n"
          "  pc->x = pa->x-pb->x;\n"
          "  pc->y = pa->y-pb->y;\n"
          "  pc->z = pa->z-pb->z;\n"
          "  pc->w = 1;\n"
          "}\n"
          "\n"
          "void sub_local(const __local char * a, const __local char *b, __local char * c) {\n"
          "  const __local Vec4* pa = (const __local Vec4*)a;\n"
          "  const __local Vec4* pb = (const __local Vec4*)b;\n"
          "  __local Vec4* pc = (__local Vec4*)c;\n"
          "  pc->x = pa->x-pb->x;\n"
          "  pc->y = pa->y-pb->y;\n"
          "  pc->z = pa->z-pb->z;\n"
          "  pc->w = 1;\n"
          "}\n"
          "\n"
          "void mul_global(const __global char * a, float b, __global char * c) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  __global Vec4* pc = (__global Vec4*)c;\n"
          "  pc->x = pa->x*b;\n"
          "  pc->y = pa->y*b;\n"
          "  pc->z = pa->z*b;\n"
          "  pc->w = pa->w;\n"
          "}\n"
          "\n"
          "void mul_local(const __local char * a, float b, __local char * c) {\n"
          "  const __local Vec4* pa = (const __local Vec4*)a;\n"
          "  __local Vec4* pc = (__local Vec4*)c;\n"
          "  pc->x = pa->x*b;\n"
          "  pc->y = pa->y*b;\n"
          "  pc->z = pa->z*b;\n"
          "  pc->w = pa->w;\n"
          "}\n"
          "\n"
          "\n"
          "void neutralElement(char* e) {\n"
          "  Vec4* pe = (Vec4*)e;\n"
          "  pe->x = 0;\n"
          "  pe->y = 0;\n"
          "  pe->z = 0;\n"
          "  pe->w = 1;\n"
          "}\n"
          "\n"
          "void neutralElement_global(__global char* e) {\n"
          "  __global Vec4* pe = (__global Vec4*)e;\n"
          "  pe->x = 0;\n"
          "  pe->y = 0;\n"
          "  pe->z = 0;\n"
          "  pe->w = 1;\n"
          "}\n"
          "\n"
          "void neutralElement_local(__local char* e) {\n"
          "//    __local Vec4* pe = (__local Vec4*)e;\n"
          "//    pe->x = 0;\n"
          "//    pe->y = 0;\n"
          "//    pe->z = 0;\n"
          "//    pe->w = 1;\n"
          "    char elem[16];\n"
          "    neutralElement(elem);\n"
          "    for(int i = 0; i < 16; i++) {\n"
          "        e[i] = elem[i];\n"
          "    }\n"
          "}\n"
          "\n"
          "\n"
          "float distanceFunc(const __global char * a, const __global char *b) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  const __global Vec4* pb = (const __global Vec4*)b;\n"
          "  Vec4 dist;\n"
          "  dist.x = pa->x-pb->x;\n"
          "  dist.y = pa->y-pb->y;\n"
          "  dist.z = pa->z-pb->z;\n"
          "  return sqrt(dist.x*dist.x+dist.y*dist.y+dist.z*dist.z);\n"
          "}\n"
          "\n"
          "void getCovariance(const __global char * a, const __global char *b, __global float* h) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  const __global Vec4* pb = (const __global Vec4*)b;\n"
          "  h[0] = pa->x*pb->x;\n"
          "  h[1] = pa->x*pb->y;\n"
          "  h[2] = pa->x*pb->z;\n"
          "  h[3] = 0;\n"
          "\n"
          "  h[4] = pa->y*pb->x;\n"
          "  h[5] = pa->y*pb->y;\n"
          "  h[6] = pa->y*pb->z;\n"
          "  h[7] = 0;\n"
          "\n"
          "  h[8] = pa->z*pb->x;\n"
          "  h[9] = pa->z*pb->y;\n"
          "  h[10] = pa->z*pb->z;\n"
          "  h[11] = 0;\n"
          "\n"
          "  h[12] = 0;\n"
          "  h[13] = 0;\n"
          "  h[14] = 0;\n"
          "  h[15] = 1;\n"
          "}\n"
          "\n"
          "\n"
          "void matrixMultiply(const __global char * a, char __global *b, const __global float* m) {\n"
          "  const __global Vec4* pa = (const __global Vec4*)a;\n"
          "  __global Vec4* pb = (__global Vec4*)b;\n"
          "  pb->x = m[0]*pa->x+m[1]*pa->y+m[2]*pa->z+m[3]*pa->w;\n"
          "  pb->y = m[4]*pa->x+m[5]*pa->y+m[6]*pa->z+m[7]*pa->w;\n"
          "  pb->z = m[8]*pa->x+m[9]*pa->y+m[10]*pa->z+m[11]*pa->w;\n"
          "  pb->w = m[12]*pa->x+m[13]*pa->y+m[14]*pa->z+m[15]*pa->w;\n"
          "}\n"
          );
    const std::string IterativeClosestPointTypeCodeVec8 = (
          "typedef struct Vec4_t{\n"
          "  float x;\n"
          "  float y;\n"
          "  float z;\n"
          "  float w;\n"
          "}Vec4;\n"
          "\n"
          "typedef struct Vec8_t{\n"
          "  Vec4 pos;\n"
          "  Vec4 col;\n"
          "}Vec8;\n"
          "\n"
          "typedef struct DistanceID_t {\n"
          "  float distance;\n"
          "  int id;\n"
          "}DistanceID;\n"
          "\n"
          "void add(const char * a, const char *b, char * c) {\n"
          "  const Vec8* pa = (const Vec8*)a;\n"
          "  const Vec8* pb = (const Vec8*)b;\n"
          "  Vec8* pc = (Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x+pb->pos.x;\n"
          "  pc->pos.y = pa->pos.y+pb->pos.y;\n"
          "  pc->pos.z = pa->pos.z+pb->pos.z;\n"
          "  pc->pos.w = 1;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "void add_global(const __global char * a, const __global char *b, __global char * c) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  const __global Vec8* pb = (const __global Vec8*)b;\n"
          "  __global Vec8* pc = (__global Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x+pb->pos.x;\n"
          "  pc->pos.y = pa->pos.y+pb->pos.y;\n"
          "  pc->pos.z = pa->pos.z+pb->pos.z;\n"
          "  pc->pos.w = 1;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "void add_local(const __local char * a, const __local char *b, __local char * c) {\n"
          "//  const __local Vec8* pa = (const __local Vec8*)a;\n"
          "//  const __local Vec8* pb = (const __local Vec8*)b;\n"
          "//  __local Vec8* pc = (__local Vec8*)c;\n"
          "//  pc->pos.x = pa->pos.x+pb->pos.x;\n"
          "//  pc->pos.y = pa->pos.y+pb->pos.y;\n"
          "//  pc->pos.z = pa->pos.z+pb->pos.z;\n"
          "//  pc->pos.w = 1;\n"
          "//  pc->col.x = pa->col.x;\n"
          "//  pc->col.y = pa->col.y;\n"
          "//  pc->col.z = pa->col.z;\n"
          "//  pc->col.w = 1;\n"
          "\n"
          "  char ac[32];\n"
          "  char bc[32];\n"
          "  char cc[32];\n"
          "  for(int i = 0; i < 32; i++) {\n"
          "      ac[i] = a[i];\n"
          "      bc[i] = b[i];\n"
          "  }\n"
          "  add(ac,bc,cc);\n"
          "  for(int i = 0; i < 32; i++) {\n"
          "      c[i] = cc[i];\n"
          "  }\n"
          "}\n"
          "\n"
          "void sub_global(const __global char * a, const __global char *b, __global char * c) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  const __global Vec8* pb = (const __global Vec8*)b;\n"
          "  __global Vec8* pc = (__global Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x-pb->pos.x;\n"
          "  pc->pos.y = pa->pos.y-pb->pos.y;\n"
          "  pc->pos.z = pa->pos.z-pb->pos.z;\n"
          "  pc->pos.w = 1;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "void sub_local(const __local char * a, const __local char *b, __local char * c) {\n"
          "  const __local Vec8* pa = (const __local Vec8*)a;\n"
          "  const __local Vec8* pb = (const __local Vec8*)b;\n"
          "  __local Vec8* pc = (__local Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x-pb->pos.x;\n"
          "  pc->pos.y = pa->pos.y-pb->pos.y;\n"
          "  pc->pos.z = pa->pos.z-pb->pos.z;\n"
          "  pc->pos.w = 1;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "void mul_global(const __global char * a, float b, __global char * c) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  __global Vec8* pc = (__global Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x*b;\n"
          "  pc->pos.y = pa->pos.y*b;\n"
          "  pc->pos.z = pa->pos.z*b;\n"
          "  pc->pos.w = pa->pos.w;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "void mul_local(const __local char * a, float b, __local char * c) {\n"
          "  const __local Vec8* pa = (const __local Vec8*)a;\n"
          "  __local Vec8* pc = (__local Vec8*)c;\n"
          "  pc->pos.x = pa->pos.x*b;\n"
          "  pc->pos.y = pa->pos.y*b;\n"
          "  pc->pos.z = pa->pos.z*b;\n"
          "  pc->pos.w = pa->pos.w;\n"
          "  pc->col.x = pa->col.x;\n"
          "  pc->col.y = pa->col.y;\n"
          "  pc->col.z = pa->col.z;\n"
          "  pc->col.w = 1;\n"
          "}\n"
          "\n"
          "\n"
          "void neutralElement(char* e) {\n"
          "  Vec8* pe = (Vec8*)e;\n"
          "  pe->pos.x = 0;\n"
          "  pe->pos.y = 0;\n"
          "  pe->pos.z = 0;\n"
          "  pe->pos.w = 1;\n"
          "  pe->col.x = 0;\n"
          "  pe->col.y = 0;\n"
          "  pe->col.z = 0;\n"
          "  pe->col.w = 1;\n"
          "}\n"
          "\n"
          "void neutralElement_global(__global char* e) {\n"
          "  __global Vec8* pe = (__global Vec8*)e;\n"
          "  pe->pos.x = 0;\n"
          "  pe->pos.y = 0;\n"
          "  pe->pos.z = 0;\n"
          "  pe->pos.w = 1;\n"
          "  pe->col.x = 0;\n"
          "  pe->col.y = 0;\n"
          "  pe->col.z = 0;\n"
          "  pe->col.w = 1;\n"
          "}\n"
          "\n"
          "void neutralElement_local(__local char* e) {\n"
          "//    __local Vec8* pe = (__local Vec8*)e;\n"
          "//    pe->pos.x = 0;\n"
          "//    pe->pos.y = 0;\n"
          "//    pe->pos.z = 0;\n"
          "//    pe->pos.w = 1;\n"
          "//  pe->col.x = 0;\n"
          "//  pe->col.y = 0;\n"
          "//  pe->col.z = 0;\n"
          "//  pe->col.w = 1;\n"
          "    char elem[32];\n"
          "    neutralElement(elem);\n"
          "    for(int i = 0; i < 32; i++) {\n"
          "        e[i] = elem[i];\n"
          "    }\n"
          "}\n"
          "\n"
          "\n"
          "float distanceFunc(const __global char * a, const __global char *b) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  const __global Vec8* pb = (const __global Vec8*)b;\n"
          "  Vec8 dist;\n"
          "  dist.pos.x = pa->pos.x-pb->pos.x;\n"
          "  dist.pos.y = pa->pos.y-pb->pos.y;\n"
          "  dist.pos.z = pa->pos.z-pb->pos.z;\n"
          "  dist.col.x = pa->pos.x-pb->col.x;\n"
          "  dist.col.y = pa->pos.y-pb->col.y;\n"
          "  dist.col.z = pa->pos.z-pb->col.z;\n"
          "  return sqrt(dist.pos.x*dist.pos.x+dist.pos.y*dist.pos.y+dist.pos.z*dist.pos.z+dist.col.x*dist.col.x+dist.col.y*dist.col.y+dist.col.z*dist.col.z);\n"
          "}\n"
          "\n"
          "void getCovariance(const __global char * a, const __global char *b, __global float* h) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  const __global Vec8* pb = (const __global Vec8*)b;\n"
          "  h[0] = pa->pos.x*pb->pos.x;\n"
          "  h[1] = pa->pos.x*pb->pos.y;\n"
          "  h[2] = pa->pos.x*pb->pos.z;\n"
          "  h[3] = 0;\n"
          "\n"
          "  h[4] = pa->pos.y*pb->pos.x;\n"
          "  h[5] = pa->pos.y*pb->pos.y;\n"
          "  h[6] = pa->pos.y*pb->pos.z;\n"
          "  h[7] = 0;\n"
          "\n"
          "  h[8] = pa->pos.z*pb->pos.x;\n"
          "  h[9] = pa->pos.z*pb->pos.y;\n"
          "  h[10] = pa->pos.z*pb->pos.z;\n"
          "  h[11] = 0;\n"
          "\n"
          "  h[12] = 0;\n"
          "  h[13] = 0;\n"
          "  h[14] = 0;\n"
          "  h[15] = 1;\n"
          "}\n"
          "\n"
          "\n"
          "void matrixMultiply(const __global char * a, char __global *b, const __global float* m) {\n"
          "  const __global Vec8* pa = (const __global Vec8*)a;\n"
          "  __global Vec8* pb = (__global Vec8*)b;\n"
          "  pb->pos.x = m[0]*pa->pos.x+m[1]*pa->pos.y+m[2]*pa->pos.z+m[3]*pa->pos.w;\n"
          "  pb->pos.y = m[4]*pa->pos.x+m[5]*pa->pos.y+m[6]*pa->pos.z+m[7]*pa->pos.w;\n"
          "  pb->pos.z = m[8]*pa->pos.x+m[9]*pa->pos.y+m[10]*pa->pos.z+m[11]*pa->pos.w;\n"
          "  pb->pos.w = m[12]*pa->pos.x+m[13]*pa->pos.y+m[14]*pa->pos.z+m[15]*pa->pos.w;\n"
          "}\n"
          );
  } // namespace geom
}
