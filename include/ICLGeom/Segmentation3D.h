/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Segmentation3D.h                       **
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

#pragma once

#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Img.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLCV/RegionDetector.h>

#ifdef HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

namespace icl{
  namespace geom{
    /**
       This class includes segmentation algorithms for depth images. It uses OpenCL for hardware parallelization if a compatible GPU is found. The input is a depth image, a binarized edge image from the PointNormalEstimation class and the xyz DataSegment from the PointCloudObject class. The output is a color image (e.g. as input for setColorsFromImage() method of the PointCloudObject class).*/
    class Segmentation3D{
  	
     public:
      /// Constructor
      /** Constructs an object of this class. All default parameters are set. Use setters for desired values.
          @param size size of the input data */
      Segmentation3D(utils::Size size); 
  	
      /// Destructor
      ~Segmentation3D();
  	
      /// One line call for the complex segmentation using probabilitstic surface composition.
      /**       @param xyz the xyzh DataSegment from the PointCloudObject class
          @param edgeImg the edge image from the PointNormalEstimation class
          @param depthImg the input depth image
          @return the color image of the segments */
      core::Img8u segmentation(DataSegment<float,4> xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg);
      
      /// One line call for the simple support plane and blobs segmentation.
      /**       @param xyz the xyzh DataSegment from the PointCloudObject class
          @param edgeImg the edge image from the PointNormalEstimation class
          @param depthImg the input depth image
          @return the color image of the segments */
      core::Img8u segmentationBlobs(DataSegment<float,4> xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg);
  	  		
      /// Sets openCL enabled/disabled. Enabling has no effect if no openCL context is available. (default true=enabled)
      /**        @param use enable/disable openCL */
      void setUseCL(bool use);
  		
  		/// Sets ROI enabled/disabled. (default false=disabled)
  		/**        @param use enable/disable ROI */
  		void setUseROI(bool use);
      
      /// Sets the ROI in world coordinates
      /**       @param xMin xMin in mm
                @param xMax xMax in mm
                @param yMin yMin in mm
                @param yMax yMax in mm */
      void setROI(float xMin, float xMax, float yMin, float yMax);
  		
      /// Sets the minimum cluster size for region growing (default 25)
      /**       @param size minimum cluster size in points */
      void setMinClusterSize(unsigned int size);
      
      /// Sets fast growing enabled/disabled. Fast growing uses the region detector, normal growing uses classical region growing. (default false=disabled)
      /**        @param use enable/disable fast growing */
      void setUseFastGrowing(bool use);
      
      /// Sets the assignment radius for the edge point assignment after region growing. (default 5)
      /**       @pram radius the assignment radius in points */
      void setAssignmentRadius(int radius);
      
      /// Sets the maximum distance for the edge point assignment after region growing. (default 15)
      /**       @param maxDistance maximum distance in mm */
      void setAssignmentMaxDistance(float maxDistance);
      
      /// Sets the maximum euclidean distance for RANSAC. (default 15)
      /**       @param distance the maximum euclidean distance for RANSAC in mm */
      void setRANSACeuclDistance(int distance);
      
      /// Sets the RANSAC passes (default 20)
      /**       @param passes the RANSAC passes */ 
      void setRANSACpasses(int passes); 
      
      /// Sets the RANSAC tolerance for the cutfree neighbours calculation. (default 30)
      /**       @param tolerance the tolerance in points */
      void setRANSACtolerance(int tolerance); 
      
      /// Sets the subset for the RANSAC check (every n-th point). (default 2)
      /**       @param subset the point subset for RANSAC */
      void setRANSACsubset(int subset);
  		
      /// Sets the minimum euclidean BLOB distance for remaining points or the blob segmentation
      /**       @param distance the minimum euclidean distance for blob segmentation */
      void setBLOBSeuclDistance(int distance);
      
      /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
      /**        @return openCL context ready/unavailable */
      bool isCLReady();
  	
      /// Returns the openCL activation status (true=openCL enabled, false=openCL disabled). The status can be set by setUseCL(bool use).
      /**        @return openCL enabled/disabled */
      bool isCLActive();
      
      /// Returns the color segment image. (Use one line calls)
      /**        @return the color segment image */
      core::Img8u getSegmentColorImage();
      
      /// Returns the cluster.
      /**        @return a vector of cluster. Every entry contains a vector with the point indices */
      std::vector<std::vector<int> > getCluster();
      
      /// Returns the blobs of the complex segmentation.
      /**        @return a vector of blobs. Every entry contains a vector with the cluster indices */
      std::vector<std::vector<int> > getBlobs();
      
      /// Returns the boolean neighbourhood matrix.
      /**       @return the neighbourhood/adjacency matrix */
      math::DynMatrix<bool> getNeigboursMatrix();
      
      /// Returns the probability matrix.
      /**      @return returns the probability matrix for cluster composition */
      math::DynMatrix<float> getProbabilityMatrix();
      
      /// Sets the xyzh DataSegment from the PointCloudObject class. (Use one line calls)
      /**       @param xyz the xyz DataSegment */
      void setXYZH(DataSegment<float,4> xyz);
      
      /// Sets the edge image from the PointNormalEstimation class. (Use one line calls)
      /**       @param edgeImage the edge image */
      void setEdgeImage(const core::Img8u &edgeImage);
      
      /// Sets the depth image
      /**       @param depth the depth image */
      void setDepthImage(const core::Img32f &depth);
      
      /// Clears the data. Must be called before every iteration. (Use one line calls)
      void clearData();
  		
      /// Region growing. (Use one line calls)
      void regionGrow();
	  	
      /// Calculates the edge point assignment and the neighbourhood matrix. (Use one line calls)
      void calculatePointAssignmentAndAdjacency();
  		
      /// Calculates the cutfree neighbouring cluster. (Use one line calls)
      void calculateCutfreeMatrix();
  		
      /// Greedy composition with probability matrix. (Use one line calls)
      void greedyComposition();
  		
      /// Calculates the assignment of the remaining points. (Use one line calls)
      void calculateRemainingPoints();
  		
      /// Calculates the blob segmentation (support plane and blobs). (Use one line calls)
      void blobSegmentation();
  		
      /// Calculates the color segment image. (Use one line calls)
      void colorPointcloud();
      
     private:
     
      int w,h,dim;
      utils::Size s;
      
      float xMinROI, xMaxROI, yMinROI, yMaxROI;
      bool useROI;
      
      bool* elements;
		  int* assignment;
		  int* assignmentRemaining;
		  bool* elementsBlobs;
		  int* assignmentBlobs;
      
      DataSegment<float,4> xyzData;
      core::Img8u normalEdgeImage;
      core::Img32f depthImage;
      
      core::Img8u segmentColorImage;
      
      std::vector<std::vector<int> > cluster;
      std::vector<std::vector<int> > blobs;
      unsigned int minClusterSize;
      bool useFastGrowing;
      int assignmentRadius;
      float assignmentMaxDistance;
      int RANSACeuclDistance; 
      int RANSACpasses; 
      int RANSACtolerance; 
      int RANSACsubset;
      int BLOBSeuclDistance;
      
      math::DynMatrix<bool> neighbours;
      math::DynMatrix<bool> cutfree;
      math::DynMatrix<float> probabilities;
      
      cv::RegionDetector* region;
   
      bool clReady;
      bool useCL;
      
      void checkNeighbourGrayThreshold(int x, int y, int zuw, int threshold, std::vector<int> *data);
      
      void checkNeighbourDistanceRemaining(int x, int y, int zuw, std::vector<int> *data);
      
      void regionGrowBlobs();
      
      void checkNeighbourDistance(int x, int y, int zuw, std::vector<int> *data);
      
      bool checkNotExist(int zw, std::vector<int> &nb);
      
      float dist3(const Vec &a, const Vec &b);
      
    #ifdef HAVE_OPENCL
      //OpenCL data
      cl_uchar* segmentColorImageRArray;
      cl_uchar* segmentColorImageGArray;
      cl_uchar* segmentColorImageBArray;
      
      //OpenCL    
      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;
      
      cl::Kernel kernelSegmentColoring; 
      cl::Kernel kernelPointAssignment; 
      cl::Kernel kernelCheckRANSAC; 
      cl::Kernel kernelAssignRANSAC; 

      //OpenCL buffer
      cl::Buffer segmentColorImageRBuffer;
      cl::Buffer segmentColorImageGBuffer;
      cl::Buffer segmentColorImageBBuffer;
      cl::Buffer assignmentBuffer;      
      cl::Buffer neighboursBuffer;
      cl::Buffer elementsBuffer;
      cl::Buffer assignmentOutBuffer;
      cl::Buffer xyzBuffer;
      cl::Buffer assignmentBlobsBuffer;
      cl::Buffer elementsBlobsBuffer;
    #endif
    };
  } // namespace geom
}
