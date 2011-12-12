/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2011 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PointNormalEstimation.h                **
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

#include <ICLQuick/Quick.h>
#include <ICLGeom/GeomDefs.h>

#ifdef HAVE_OPENCL    
#include <CL/cl.hpp>
#endif

/**
   This class is a normal estimator and edge detector for depth images. It uses OpenCL for hardware parallelization if a compatible GPU is found. The input is a depth image and the output a binarized edge image.
   All interim results can be accessed (e.g. point normals) or set to use only a part of the processing pipeline.*/
class PointNormalEstimation{
	
 public:
#ifdef HAVE_OPENCL
  typedef cl_float4 Vec4;
#else
  struct Vec4{
    inline Vec4(){}
    inline Vec4(float x,float y, float z, float w): x(x),y(y),z(z),w(w){}
    float x,y,z,w;
  };
#endif
    
  ///Constructor
  /** Constructs an object of this class. All default parameters are set. Use setters for desired values.
      @param size size of the input depth image */
  PointNormalEstimation(Size size); 
	
  ///Destructor
  ~PointNormalEstimation();
	
  /// Sets the input depth image (input for median filter).
  /**       @param depthImg the input depth image */
  void setDepthImage(const Img32f &depthImg);
	
  /// Calculates a filtered image using a median filter. The mask size is set by setMedianFilterSize(int size).
  void medianFilter();
	
  /// Returns the filtered depth image.
  /**        @return the filtered depth image */
  Img32f getFilteredImage();
	
  /// Sets the (filtered) depth image (input for normal calculation). This call is not necessary if medianFilter() is executed before.
  /**        @param filteredImg the (filtered) depth image */
  void setFilteredImage(Img32f &filteredImg); 
	
  /// Calculates the point normals. The range for calculation is set by setNormalCalculationRange(int range). 
  void normalCalculation();
	
  /// Recalculates the normals by averaging the normals in a given range. This reduces the noise and is called from calculateNormals() if it is enabled width setUseNormalAveraging(bool use). The range is set by setNormalAveragingRange(int range).
  void normalAveraging();

  /// Returns the point normals.
  /**        @return the point normals */
  Vec4* getNormals();
	
  /// Sets the point normals (input for angle image calculation). This call is not necessary if normalCalculation() is executed before.
  /**        @param pNormals the point normals */
  void setNormals(Vec4* pNormals);
	
  /// Calculates the angle image. The mode is set by setAngleNeighborhoodMode(int mode) and the range is set by setAngleNeighborhoodRange(int range)
  void angleImageCalculation();
	
  /// Returns the angle image.
  /**        @return the angle image */
  Img32f getAngleImage();
	
  /// Sets the angle image (input for image binarization). This call is not necessary if angleImageCalculation() is executed before.
  /**        @param angleImg the angle image */
  void setAngleImage(Img32f &angleImg);
	
  /// Binarizes the angle image to detect edges. The threshold is set by setBinarizationThreshold(float threshold).
  void imageBinarization();
	
  /// Returns the binarized angle image (final output).
  /**        @return the (final) binarized angle image */
  Img32f getBinarizedImage();
	
	
  /// Sets the mask size for medianFilter(). Size n corresponds to mask size n x n. (default 3, min 3, max 9, odd only)
  /**        @param size the mask size */
  void setMedianFilterSize(int size);
	
  /// Sets the range for normalCalculation(). (default 2)
  /**        @param range the normal calculation range */
  void setNormalCalculationRange(int range);
	
  /// Sets the averaging range for normalAveraging(). (default 1)
  /**        @param range the normal averaging range */
  void setNormalAveragingRange(int range);
	
  /// Sets the neighborhood mode for angleImageCalculation(). 0=max, 1=mean. (default 0)
  /**        @param mode the neighborhood mode */
  void setAngleNeighborhoodMode(int mode);
	
  /// Sets the neighborhood range for angleImageCalculation(). (default 3, min 1)
  /**        @param range the neighborhood range */
  void setAngleNeighborhoodRange(int range);
	
  /// Sets the binarization threshold for imageBinarization(). Value n for acos(n), so that 0 is 90° and 1 is 0° angle. (default 0.89) 
  /**        @param threshold binarization threshold */
  void setBinarizationThreshold(float threshold);
	
  /// Sets openCL enabled/disabled. Enabling has no effect if no openCL context is available. (default true=enabled)
  /**        @param use enable/disable openCL */
  void setUseCL(bool use);
	
  /// Sets normal averaging enabled/disabled. (default true=enabled)
  /**        @param use enable/disable normal averaging */
  void setUseNormalAveraging(bool use);
	
  /// Returns the openCL status (true=openCL context ready, false=no openCL context available)
  /**        @return openCL context ready/unavailable */
  bool isCLReady();
	
  /// Returns the openCL activation status (true=openCL enabled, false=openCL disabled). The status can be set by setUseCL(bool use).
  /**        @return openCL enabled/disabled */
  bool isCLActive();
	
	
  /// One call function for calculation of the complete pipeline ((filter)->normals->(normalAvg)->angles->binarization)
  /**        @param depthImage the input depth image
	     @param filter enable/disable filtering
	     @param average enable/disable normal averaging */
  Img32f calculate(const Img32f &depthImage, bool filter, bool average);
	
 private:
	
  int w,h;
  int medianFilterSize;
  int normalRange;
  int normalAveragingRange;
  int neighborhoodMode;
  int neighborhoodRange;
  float binarizationThreshold;
  bool clReady;
  bool useCL;
  bool useNormalAveraging;
  Vec4* normals;
  Vec4* avgNormals;
  Img32f rawImage;
  Img32f filteredImage;
  Img32f angleImage;
  Img32f binarizedImage;
	
#ifdef HAVE_OPENCL
  //OpenCL data
  Vec4 * outputNormals;
  float* outputFilteredImage;//output of kernel for image
  float* outputAngleImage;
  float* outputBinarizedImage;
  float* rawImageArray;//input for image to kernel
  float* filteredImageArray;
  float* angleImageArray;
  float* binarizedImageArray;
	
  //OpenCL    
  cl::Context context;
  std::vector<cl::Device> devices;
  cl::Program program;
  cl::CommandQueue queue;
    
  cl::Kernel kernelMedianFilter;
  cl::Kernel kernelNormalCalculation;
  cl::Kernel kernelNormalAveraging;
  cl::Kernel kernelAngleImageCalculation;
  cl::Kernel kernelImageBinarization;
  
  //OpenCL buffer
  cl::Buffer rawImageBuffer;
  cl::Buffer filteredImageBuffer;
  cl::Buffer normalsBuffer;
  cl::Buffer avgNormalsBuffer;
  cl::Buffer angleImageBuffer;
  cl::Buffer binarizedImageBuffer;
#endif
};
