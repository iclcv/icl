/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/IterativeClosestPointCLCode.cl     **
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

typedef struct Vec4_t{
  float x;
  float y;
  float z;
  float w;
}Vec4;

typedef struct DistanceID_t {
  float distance;
  int id;
}DistanceID;

void add(const char * a, const char *b, char * c) {
  const Vec4* pa = (const Vec4*)a;
  const Vec4* pb = (const Vec4*)b;
  Vec4* pc = (Vec4*)c;
  pc->x = pa->x+pb->x;
  pc->y = pa->y+pb->y;
  pc->z = pa->z+pb->z;
}

void add_local(const __local char * a, const __local char *b, __local char * c) {
//  const __local Vec4* pa = (const __local Vec4*)a;
//  const __local Vec4* pb = (const __local Vec4*)b;
//  __local Vec4* pc = (__local Vec4*)c;
//  pc->x = pa->x+pb->x;
//  pc->y = pa->y+pb->y;
//  pc->z = pa->z+pb->z;

  char ac[16];
  char bc[16];
  char cc[16];
  for(int i = 0; i < 16; i++) {
      ac[i] = a[i];
      bc[i] = b[i];
  }
  add(ac,bc,cc);
  for(int i = 0; i < 16; i++) {
      c[i] = cc[i];
  }
}

void sub_global(const __global char * a, const __global char *b, __global char * c) {
  const __global Vec4* pa = (const __global Vec4*)a;
  const __global Vec4* pb = (const __global Vec4*)b;
  __global Vec4* pc = (__global Vec4*)c;
  pc->x = pa->x-pb->x;
  pc->y = pa->y-pb->y;
  pc->z = pa->z-pb->z;
  pc->w = 1;
}

void mul_global(const __global char * a, float b, __global char * c) {
  const __global Vec4* pa = (const __global Vec4*)a;
  __global Vec4* pc = (__global Vec4*)c;
  pc->x = pa->x*b;
  pc->y = pa->y*b;
  pc->z = pa->z*b;
  pc->w = pa->w;
}

void neutralElement(char* e) {
  Vec4* pe = (Vec4*)e;
  pe->x = 0;
  pe->y = 0;
  pe->z = 0;
  pe->w = 1;
}

void neutralElement_local(__local char* e) {
//    __local Vec4* pe = (__local Vec4*)e;
//    pe->x = 0;
//    pe->y = 0;
//    pe->z = 0;
//    pe->w = 1;
    char elem[16];
    neutralElement(elem);
    for(int i = 0; i < 16; i++) {
        e[i] = elem[i];
    }
}


float distanceFunc(const __global char * a, const __global char *b) {
  const __global Vec4* pa = (const __global Vec4*)a;
  const __global Vec4* pb = (const __global Vec4*)b;
  Vec4 dist;
  dist.x = pa->x-pb->x;
  dist.y = pa->y-pb->y;
  dist.z = pa->z-pb->z;
  return sqrt(dist.x*dist.x+dist.y*dist.y+dist.z*dist.z);
}

void getCovariance(const __global char * a, const __global char *b, __global float* h) {
  const __global Vec4* pa = (const __global Vec4*)a;
  const __global Vec4* pb = (const __global Vec4*)b;
  h[0] = pa->x*pb->x;
  h[1] = pa->x*pb->y;
  h[2] = pa->x*pb->z;
  h[3] = 0;

  h[4] = pa->y*pb->x;
  h[5] = pa->y*pb->y;
  h[6] = pa->y*pb->z;
  h[7] = 0;

  h[8] = pa->z*pb->x;
  h[9] = pa->z*pb->y;
  h[10] = pa->z*pb->z;
  h[11] = 0;

  h[12] = 0;
  h[13] = 0;
  h[14] = 0;
  h[15] = 1;
}


void matrixMultiply(const __global char * a, char __global *b, const __global float* m) {
  const __global Vec4* pa = (const __global Vec4*)a;
  __global Vec4* pb = (__global Vec4*)b;
  pb->x = m[0]*pa->x+m[1]*pa->y+m[2]*pa->z+m[3]*pa->w;
  pb->y = m[4]*pa->x+m[5]*pa->y+m[6]*pa->z+m[7]*pa->w;
  pb->z = m[8]*pa->x+m[9]*pa->y+m[10]*pa->z+m[11]*pa->w;
  pb->w = m[12]*pa->x+m[13]*pa->y+m[14]*pa->z+m[15]*pa->w;
}

#define MATRIX_SIZE 16

__kernel void getClosestPoints(const __global char* pointsA, const __global char* pointsB, __global DistanceID* distanceAcc, __local float* distances, int typeSize, int sizeA, int sizeB, int maxX) {
  int globalX = get_global_id(0);
  int globalY = get_global_id(1);
  int localX = get_local_id(0);
  int localY = get_local_id(1);
  int localSizeX = get_local_size(0);
  int localGroupX = get_group_id(0);
  //calculate distance for current pair
  distances[localY*localSizeX+localX] = FLT_MAX;
  if(globalX < sizeB && globalY < sizeA)
    distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*globalX,pointsA+typeSize*globalY);

  //_________local_barrier______
  barrier(CLK_LOCAL_MEM_FENCE);

  //find closest distances in local group by letting only the last vertical line of threads in a local group calculate
  if(localX == 0 && globalY < sizeA) {
    int localStartX = localGroupX*localSizeX;
    DistanceID min;
    min.distance = distances[localY*localSizeX];
    min.id = 0;
    for(int k = 1; k < localSizeX; k++) {
      float distance = distances[localY*localSizeX+k];
      if(distance < min.distance) {
        min.distance = distance;
        min.id = k;
      }
    }
    //convert id from local to global space
    min.id += localStartX;
    distanceAcc[globalY*maxX+localGroupX] = min;
  }
}

__kernel void reduceClosestPoints(__global int* closestPoints, __global DistanceID* distanceAcc, int sizeA, int widthDistanceAcc) {
  int globalX = get_global_id(0);

  //find closest point across all groups by letting only the last vertical line of threads calculate
  if(globalX < sizeA) {
    int startX = globalX*widthDistanceAcc;
    int endX = startX+widthDistanceAcc;
    DistanceID min = distanceAcc[startX];
    for(int k = startX+1; k < endX; k++) {
      DistanceID point = distanceAcc[k];
      if(point.distance < min.distance) {
        min = point;
      }
    }
    closestPoints[globalX] = min.id;
  }
}

__kernel void getClosestPointsInDB(const __global char* pointsA, const __global int* closestReps, const __global char* pointsB, const __global int* pointsDB, const __global int* counters, __global int* closestPoints, __global DistanceID* distanceAcc, __local float* distances, int typeSize, int sizeA, int widthDB, int maxX) {
  int globalX = get_global_id(0);
  int globalY = get_global_id(1);
  int localX = get_local_id(0);
  int localY = get_local_id(1);
  int localSizeX = get_local_size(0);
  int localGroupX = get_group_id(0);

  //calculate distance for current pair
  distances[localY*localSizeX+localX] = FLT_MAX;
  int maxWidth;
  if(globalY < sizeA)maxWidth = min(widthDB,counters[closestReps[globalY]]);
  if(globalY < sizeA && globalX < maxWidth)
    distances[localY*localSizeX+localX] = distanceFunc(pointsB+typeSize*pointsDB[closestReps[globalY]*widthDB+globalX],pointsA+typeSize*globalY);


  //_________local_barrier______
  barrier(CLK_LOCAL_MEM_FENCE);


  //find closest distances in local group by letting only the first vertical line of threads in a local group calculate
  if(localX == 0 && globalY < sizeA) {
    int localStartX = localGroupX*localSizeX;
    DistanceID min;
    min.distance = distances[localY*localSizeX];
    min.id = 0;
    for(int k = 1; k < localSizeX; k++) {
      float distance = distances[localY*localSizeX+k];
      if(distance < min.distance) {
        min.distance = distance;
        min.id = k;
      }
    }
    //convert id from local to global space
    min.id = pointsDB[closestReps[globalY]*widthDB+(min.id+localStartX)];
    distanceAcc[globalY*maxX+localGroupX] = min;
  }
}


__kernel void initMemory(__global char* mem, int size) {
  if(get_global_id(0) == 0) {
    for(int i = 0; i < size; i++)mem[i] = 0;
  }
}

__kernel void buildRepDB(const __global char* points, const __global int* closestReps, __global int* counters, __global int* repDataBase, int repWidth, int typeSize, int sizePoints, int sizeReps) {
    int globalX = get_global_id(0);
    int localX = get_local_id(0);
    int localSizeX = get_local_size(0);
    int localGroupX = get_group_id(0);

    if(globalX < sizePoints) {
        int rep = closestReps[globalX];
        int pos = atomic_inc(counters+rep);
        //check if the there are to many neighbours
        if(pos < repWidth) {
            repDataBase[repWidth*rep+pos] = globalX;
        }
    }
}

__kernel void rotatePoints(const __global char* points, __global char* pointsRotated, const __global float* rotationMatrix, int typeSize, int sizePoints) {
  int globalX = get_global_id(0);
  if(globalX < sizePoints) {
    int offset = typeSize*globalX;
    matrixMultiply(points+offset, pointsRotated+offset, rotationMatrix);
  }
}

__kernel void getError(const __global char* pointsA, const __global char* pointsB, const __global int* closestPoints, __global float* errors, int typeSize, int sizeA) {
  int globalX = get_global_id(0);
  if(globalX < sizeA)errors[globalX] = distanceFunc(pointsA+globalX*typeSize,pointsB+closestPoints[globalX]*typeSize);
}

__kernel void collectError(const __global float* in, __global float* out, __local float* scratch, int length) {
  int global_index = get_global_id(0);
  int local_index = get_local_id(0);
  if (global_index < length) {
    scratch[local_index] = in[global_index];
  } else {
    scratch[local_index] = 0;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for(int offset = get_local_size(0) / 2;
      offset > 0;
      offset >>= 1) {
    if (local_index < offset) {
      float other = scratch[local_index + offset];
      float mine = scratch[local_index];
      scratch[local_index] = mine+other;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    out[get_group_id(0)] = scratch[0];
  }
}


__kernel void getCovarianceSum(const __global char* pointsA, const __global char* pointsB, const __global int* closestPoints, __global float* covariances, int typeSize, int sizeA) {
  int globalX = get_global_id(0);
  if(globalX < sizeA) getCovariance(pointsA+globalX*typeSize,pointsB+closestPoints[globalX]*typeSize,covariances+globalX*MATRIX_SIZE);
}

__kernel void collectCovarianceSum(const __global float* in, __global float* out, __local float* scratch, int length) {
  int global_index = get_global_id(0);
  int local_index = get_local_id(0);
  if (global_index < length) {
    for(int i = 0; i < MATRIX_SIZE; i++)
      scratch[local_index*MATRIX_SIZE+i] = in[global_index*MATRIX_SIZE+i];
  } else {
    for(int i = 0; i < MATRIX_SIZE; i++)
      scratch[local_index*MATRIX_SIZE+i] = 0;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for(int offset = get_local_size(0) / 2;
      offset > 0;
      offset >>= 1) {
    if (local_index < offset) {
      for(int i = 0; i < MATRIX_SIZE; i++) {
        float other = scratch[(local_index + offset)*MATRIX_SIZE+i];
        float mine = scratch[local_index*MATRIX_SIZE+i];
        scratch[local_index*MATRIX_SIZE+i] = mine+other;
      }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    for(int i = 0; i < MATRIX_SIZE; i++)
      out[get_group_id(0)*MATRIX_SIZE+i] = scratch[i];
  }
}

__kernel void sumPoints(const __global char* in, __global char* out, __local char* scratch, int typeSize, int length) {
  int global_index = get_global_id(0);
  int local_index = get_local_id(0);
  int local_size = get_local_size(0);

  if (global_index < length) {
    for(int i = 0; i < typeSize; i++)
      scratch[local_index*typeSize+i] = in[global_index*typeSize+i];
  } else {
      neutralElement_local(scratch+local_index*typeSize);
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for(int offset = get_local_size(0) / 2;
      offset > 0;
      offset >>= 1) {
    if (local_index < offset) {
      add_local(scratch+(local_index + offset)*typeSize,scratch+local_index*typeSize,scratch+local_index*typeSize);
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    for(int i = 0; i < typeSize; i++)
      out[get_group_id(0)*typeSize+i] = scratch[i];
  }
}

__kernel void mulPoints(const __global char* in, __global char* out, float v, int typeSize, int length) {
  int globalX = get_global_id(0);
  if(globalX<length)mul_global(in+globalX*typeSize,v,out+globalX*typeSize);
}

__kernel void subPoints(const __global char* in, const __global char* v, __global char* out, int typeSize, int length) {
  int globalX = get_global_id(0);
  if(globalX< length)sub_global(in+globalX*typeSize,v,out+globalX*typeSize);
}
