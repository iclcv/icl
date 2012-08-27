/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PointcloudSceneObject.h                **
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

#ifndef ICL_POINTCLOUD_SCENE_OBJECT_H
#define ICL_POINTCLOUD_SCENE_OBJECT_H

#include <ICLCV/Quick.h>
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointNormalEstimation.h>

#ifdef HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

namespace icl{
  namespace geom{
    /**
       This class visualize 2.5D grids as a 3D pointcloud including 2.5D to 3D projection and different color visualizations (e.g. point normals, color, depth).*/
    class PointcloudSceneObject : public SceneObject{
  	
    public:
      ///Constructor
      /** Constructs an object of this class. All default parameters are set. Use setters for desired values.
          @param size size of the input depth image */
      PointcloudSceneObject(Size size, const Camera &cam); 
  	
      ///Destructor
      ~PointcloudSceneObject();
  	    
      /// Calculates a depth image in pseudocolors (heatmap).
      /**        @return depth image in pseudocolors 
                 @param depthImg the input depth image
                 @param vSync enable/disable vSync (for openCL read)*/
      Img8u calculatePseudocolorDepthImage(const Img32f &depthImg, bool vSync);
      
      /// Calculates a pointcloud in a single color.
      /**        @param depthImg the input depth image
                 @param color the pointcloud color
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculateUniColor(const Img32f &depthImg, GeomColor color, bool vSync);
      
      /// Calculates a pointcloud colored with the RGB image.
      /**        @param depthImg the input depth image
                 @param colorImg the input color image
                 @param homogeneity the homogeneity matrix for mapping of the rgb colors to the points
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculateRGBColor(const Img32f &depthImg, const Img8u &colorImg, FixedMatrix<float,3,3> homogeneity, bool vSync);
      
      /// Calculates a pointcloud colored with the pseudocolor depth map (use if pseudocolor image is already calculated).
      /**        @param depthImg the input depth image
                 @param pseudoImg the pseudocolor image calculated by calculatePseudocolorDepthImage(const Img32f &depthImg, bool vSync) or another color mapping
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculatePseudoColor(const Img32f &depthImg, Img8u &pseudoImg, bool vSync);
      
      /// Calculates a pointcloud colored with the pseudocolor depth map (use if no previous pseudocolor image is calculated).
      /**        @param depthImg the input depth image
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculatePseudoColor(const Img32f &depthImg, bool vSync);
      
      /// Calculates a pointcloud colored with normal directions xyz->RGB in camera space (normals from PointNormalEstimation class).
      /**        @param depthImg the input depth image
                 @param depthImg the input point normals
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculateNormalDirectionColor(const Img32f &depthImg, PointNormalEstimation::Vec4* pNormals, bool vSync);
      
      /// Calculates a pointcloud colored with normal directions xyz->RGB in world space (normals from PointNormalEstimation class).
      /**        @param depthImg the input depth image
                 @param depthImg the input point normals
                 @param cam the input camera device
                 @param vSync enable/disable vSync (for openCL read)*/ 
      void calculateNormalDirectionColor(const Img32f &depthImg, PointNormalEstimation::Vec4* pNormals, Camera cam, bool vSync);
      
      /// Sets the depth scaling for the pointclouds.
      /**        @param scaling the scale factor*/
      void setDepthScaling(float scaling);
  	
  	  /// Sets openCL enabled/disabled. Enabling has no effect if no openCL context is available. (default true=enabled)
      /**        @param use enable/disable openCL */
  	  void setUseCL(bool use);
  	  
  	  /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
      /**        @return openCL context ready/unavailable */
  	  bool isCLReady();
  	  
  	  /// Returns the openCL activation status (true=openCL enabled, false=openCL disabled). The status can be set by setUseCL(bool use).
      /**        @return openCL enabled/disabled */
  	  bool isCLActive();
  	  
  	  /// Enables/disables the drawing of the normal lines for the calculated normal pointclouds.
      /**        @param use draw lines enabled/disabled */
  	  void setUseDrawNormalLines(bool use);
  	  
  	  /// Sets the length of the normal lines.
      /**        @param length the length of the normal lines */
  	  void setNormalLinesLength(float length);
  	  
  	  /// Sets the granularity of the normal lines (1=draw a line for every point, n=draw a line for every n-th point).
      /**        @param granularity the granularity of the normal lines */
  	  void setNormalLinesGranularity(int granularity);
  	  
  	  /// Returns the point normals in world space (only after calculateNormalDirectionColor(...)).
      /**        @return the point normals in world space */
  	  PointNormalEstimation::Vec4* getWorldNormals();
  	  
     private:
  	
  	  float depth_to_distance_mm(int d);
  	  float sprod(const Vec &a,const Vec &b);
  	  float getNormFactor(const ViewRay &a, const ViewRay &b);
  	  void create_view_rays(int w, int h, const Camera &cam);
  	  void drawNormalLines(PointNormalEstimation::Vec4* pNormals);
  	  void clearNormalLines();  
  	  
      int w,h,dim;
      Size s;
      std::vector<ViewRay> rays;
      std::vector<float> norms;
      FixedMatrix<float,3,3> H;
      Mat inverseCamCSMatrix;
      Mutex mutex;
      int highlightedIdx;
      float depthScaling;
      bool clReady;
      bool useCL;
      bool normalLinesSet;
      bool useNormalLines;
      
      float normalLinesLength;
      int normalLinesGranularity;
      
      PointNormalEstimation::Vec4* viewrayOffsets;
      PointNormalEstimation::Vec4* viewrayDirections;
      
      PointNormalEstimation::Vec4* pWNormals;
    	
    #ifdef HAVE_OPENCL
      //OpenCL    
      float* rawImageArray;
      PointNormalEstimation::Vec4 * outputVertices;
      PointNormalEstimation::Vec4 * outputColors;
      cl_uchar* colorImageRArray;
      cl_uchar* colorImageGArray;
      cl_uchar* colorImageBArray;
      cl_uchar* pseudoImageRArray;
      cl_uchar* pseudoImageGArray;
      cl_uchar* pseudoImageBArray;
      
      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;
      
      cl::Kernel kernelUnicolor;
      cl::Kernel kernelRGBcolor;
      cl::Kernel kernelPseudocolor;
      cl::Kernel kernelPseudodepth;
      cl::Kernel kernelNormalcolorCam;
      cl::Kernel kernelNormalcolorWorld;
      
      cl::Buffer rawImageBuffer;
      cl::Buffer vrOffsetBuffer;
      cl::Buffer vrDirectionBuffer;
      cl::Buffer normsBuffer;
      cl::Buffer foundBuffer;
      cl::Buffer verticesBuffer;
      cl::Buffer vertColorsBuffer;
      cl::Buffer colorRBuffer;
      cl::Buffer colorGBuffer;
      cl::Buffer colorBBuffer;
      cl::Buffer pseudoRBuffer;
      cl::Buffer pseudoGBuffer;
      cl::Buffer pseudoBBuffer;
      cl::Buffer homogeneityBuffer;
      cl::Buffer normalsBuffer;
      cl::Buffer camBuffer;
      cl::Buffer outNormalsBuffer;    
    #endif
    };
  } // namespace geom
}
#endif
