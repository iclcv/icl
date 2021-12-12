/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/IterativeClosestPoint.h            **
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
#include <ICLGeom/Geom.h>
#include <ICLMath/HomogeneousMath.h>
namespace icl{
  namespace geom{
    struct Vec8{
      math::Vec4 pos;
      math::Vec4 col;
    };
    template<typename T>
    class IterativeClosestPoint{
    public:

      struct Data;
      Data *m_data;

      struct DistanceID {
        float distance;
        int id;
      };

      IterativeClosestPoint(const std::string &clCode, int localSize2Dx, int localSize2Dy, int localSize1D, void (*subFunc)(const char * a, const char *b, char *c), math::Vec4 (*toVectorFunc)(const char* point));
      void icp(const T* pointsA, const T* pointsB, int sizeA, int sizeB, float errorThreshold, float errorDeltaThreshold, int maxIterations, float* initialTransform, float* transformMatrix);
      static IterativeClosestPoint<math::Vec4> icpVec4();
      static IterativeClosestPoint<Vec8> icpVec8();

//      static void sub(const char * a, const char *b, char *c);
//      static void add(const char * a, const char *b, char *c);
//      static void mul(const char * a, float b, char *c);
//      static void neutralElement(char * e);
//      static math::Vec4 toVector(const char* point);
//      static float getDistance(const char* a, const char* b);
//      static void getCovariance(const char * a, const char *b, float* h);
//      static void matrixMultiply(const char * a, char *b, const float* h);
//      static float getError(const char* pointsA, const char* pointsB, const int* closestPoints, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA);
//      static void getCovarianceSum(const char* pointsA, const char* pointsB, const int* closestPoints, float* covariance, void(*covarianceFunc)(const char*, const char*, float*), size_t typeSize, int sizeA);
//      static void getRotatedPoints(const char* pointsA, char* pointsRotated, const float* rotationMatrix, void(*matrixMultiplyFunc)(const char * a, char *b, const float* r), size_t typeSize, int sizeA);

//      void icp(const char* pointsA, const char* pointsB, int sizeA, int sizeB, size_t typeSize, float errorThreshold, float errorDeltaThreshold, int maxIterations, float* initialTransform, float* transformMatrix, float(*distanceFunc)(const char*, const char*), void(*covarianceFunc)(const char*, const char*, float*), void(*matrixMultiplyFunc)(const char * a, char *b, const float* r), void (*subFunc)(const char * a, const char *b, char *c), void (*addFunc)(const char * a, const char *b, char *c), void (*mulFunc)(const char * a, float b, char *c), void (*neutralElementFunc)(char * e), math::Vec4 (*toVectorFunc)(const char* point));
//      //searches for a closest point for every point in pointsA in the list of points from pointsB
//      /// searches for a closest point for every point in pointsA in the list of points from pointsB
//      /** @param pointsA the points of which the nearest neighbours are to be found
//       *  @param pointsB the points in which the nearest neighbours are searched
//       *  @param closestPoints indices of the nearest neighbours of pointsA in pointsB with the size of sizeA
//       *  @param distanceAcc pointer to a global memory with the size of sizeA*(sizeB+localSizeX-1)/localSizeX used to store temporary values for each local group for a point in PointsA
//       *  @param distanceFunc a function that works on the datatype of pointsA and pointsB and provides a distance between those 2 points
//       *  @param typeSize size of the type of pointsA and pointsB
//       *  @param sizeA number of points in pointsA
//       *  @param sizeB number of points in pointsB
//       *  @param localSizeX width of a local group
//       *  @param localsizeY height of a local group
//       */
//      void getClosestPoints(const char* pointsA, const char* pointsB, int* closestPoints, DistanceID* distanceAcc, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA, int sizeB, int localSizeX, int localSizeY);

//      /// builds a database with the closest neighbours of the representatives
//      /** @param points list of points to be added to the database
//       *  @param closestPoints list of closest representatives to points
//       *  @param counters list of counters to avoid conflicts in
//       *  @param maxCounter maximum amount of neighbours for a single representative
//       *  @param repWidth maximum number of neighbours of a representativethe database
//       *  @param typeSize size of the type of points
//       *  @param sizePoints number of points in points
//       *  @param sizeReps number of representatives
//       *  @param localSizeX width of a local group
//       */
//      void buildRepDB(const char* points, const int* closestreps, int* counters, int* repDataBase, int repWidth, size_t typeSize, int sizePoints, int sizeReps, int localSizeX);

//      void getClosestPointsInDB(const char* pointsA, const int* closestRep, const char* pointsB, const int* pointsDB, const int* counters, int* closestPoints, DistanceID* distanceAcc, float(*distanceFunc)(const char*, const char*), size_t typeSize, int sizeA, int widthDB, int localSizeX, int localSizeY);
    };
  } // namespace geom
}
