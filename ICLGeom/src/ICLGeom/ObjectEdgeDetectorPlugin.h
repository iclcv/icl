/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorPlugin.h         **
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

/** \cond */
//Please use the ObjectEdgeDetector class.
//This is the interface for the ObjectEdgeDetector implementations.

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Img.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/DataSegment.h>

namespace icl{
  namespace geom{
    class ObjectEdgeDetectorPlugin : public utils::Uncopyable{
    
     public:
    
      virtual ~ObjectEdgeDetectorPlugin() {}
      
      /// One call function for calculation of the complete processingpipeline
      /** Order:  ((filter)->normals->(normalAvg)->angles->binarization)
          @param depthImage the input depth image
          @param filter enable/disable filtering
          @param average enable/disable normal averaging
          @param gauss true=gauss smoothing, false=linear smoothing
          @return the binarized angle image */
      virtual const core::Img8u &calculate(const core::Img32f &depthImage, bool filter, bool average, bool gauss)=0;
	
      /// Sets the input depth image (input for median filter).
      /** @param depthImg the input depth image */
      virtual void setDepthImage(const core::Img32f &depthImg)=0;
  	
      /// Calculates a filtered image using a median filter. 
      /** The mask size is set by setMedianFilterSize(int size) */
      virtual void applyMedianFilter()=0;
  	
      /// Returns the filtered depth image
      /** @return the filtered depth image */
      virtual const core::Img32f &getFilteredDepthImage()=0;
  	
      /// Sets the (filtered) depth image (input for normal calculation) 
      /** This call is not necessary if medianFilter() is executed before
          @param filteredImg the (filtered) depth image */
      virtual void setFilteredDepthImage(const core::Img32f &filteredImg)=0; 
  	
      /// Calculates the point normals. 
      /** The range for calculation is set by 
          setNormalCalculationRange(int range). */
      virtual void applyNormalCalculation()=0;
  	
      /// Recalculates the normals by linear averaging
      /** This reduces the noise of the normal image. The function is called from 
          calculateNormals() if it is enabled width setUseNormalAveraging(bool use). 
          The range is set by setNormalAveragingRange(int range).\n
          Alternative: normalGaussSmoothing() with setUseGaussSmoothing(true). */
      virtual void applyLinearNormalAveraging()=0;
  
      /// Recalculates the normals by gaussian smoothing in a given range.
      /** Alternative to applyTemporalNormalAveraging that uses a simpler
          gaussian filter for smoothing

          if it is enabled width setUseNormalAveraging(bool use). 
          The range is set by setNormalAveragingRange(int range). 
          Alternative: normalAveraging() with setUseGaussSmoothing(false). */
      virtual void applyGaussianNormalSmoothing()=0;
  
      /// Returns the Pointer to the normals
      /** @return the point normals */
      virtual const core::DataSegment<float,4> getNormals()=0;
      
      /// Transforms the normals to the world space and calculates normal image.
      /**  @param cam the camera of the depth image */
      virtual void applyWorldNormalCalculation(const Camera &cam)=0;
  	  
      /// Returns the point normals in world space.
      /** @return the point normals in world space */
      virtual const core::DataSegment<float,4> getWorldNormals()=0;
  	  
      /// Returns the RGB normal image.
      /** @return the RGB normal image */
      virtual const core::Img8u &getRGBNormalImage()=0;
      
      /// Sets the point normals (input for angle image calculation). 
      /** This call is not necessary if normalCalculation() is executed before.
          @param pNormals the point normals */
      virtual void setNormals(core::DataSegment<float,4> pNormals)=0;
  	
      /// Calculates the angle image. 
      /** The mode is set by setAngleNeighborhoodMode(int mode).
          The range is set by setAngleNeighborhoodRange(int range) */
      virtual void applyAngleImageCalculation()=0;
  	
      /// Returns the angle image.
      /** @return the angle image */
      virtual const core::Img32f &getAngleImage()=0;
  	
      /// Sets the angle image (input for image binarization). 
      /** This call is not necessary if angleImageCalculation() is executed before.
          @param angleImg the angle image */
      virtual void setAngleImage(const core::Img32f &angleImg)=0;
  	
      /// Binarizes the angle image to detect edges. 
      /** The threshold is set by setBinarizationThreshold(float threshold). */
      virtual void applyImageBinarization()=0;
  	
      /// Returns the binarized angle image (final output).
      /** @return the (final) binarized angle image */
      virtual const core::Img8u &getBinarizedAngleImage()=0;  	
  	
      /// Sets the mask size for applyMedianFilter()
      /** size n corresponds to mask size n x n. (default 3, min 3, max 9, odd only)
          @param size the mask size */
      virtual void setMedianFilterSize(int size)=0;
  	
      /// Sets the range for applyNormalCalculation().
      /** (default 2)
          @param range the normal calculation range */
      virtual void setNormalCalculationRange(int range)=0;
  	
      /// Sets the averaging range for applyNormalAveraging()
      /** (default 1) @param range the normal averaging range */
      virtual void setNormalAveragingRange(int range)=0;
  	
      /// Sets the neighborhood mode for applyAngleImageCalculation()
      /** 0=max, 1=mean. (default 0)
          @param mode the neighborhood mode */
      virtual void setAngleNeighborhoodMode(int mode)=0;
  	
      /// Sets the neighborhood range for applyAngleImageCalculation()
      /** (default 3, min 1)
          @param range the neighborhood range */
      virtual void setAngleNeighborhoodRange(int range)=0;
  	
      /// Sets the binarization threshold for applyImageBinarization(). 
      /** Value n for acos(n). A value of 0 maps to 90 degree, a value of 1
          maps to o degree (default 0.89) 
          @param threshold binarization threshold */
      virtual void setBinarizationThreshold(float threshold)=0;
  	  	
      /// Sets normal averaging enabled/disabled. 
      /** (default true=enabled)
          @param use enable/disable normal averaging */
      virtual void setUseNormalAveraging(bool use)=0;
      
      /// Sets normal averaging by gauss smoothing enabled/disabled
      /** (default false=linear smoothing if normal averaging enabled)
          @param use enable/disable gauss smoothing */
      virtual void setUseGaussSmoothing(bool use)=0;
      
      /// Returns the openCL status 
      /** (true=openCL context ready, false=no openCL context available)
          @return openCL context ready/unavailable */
      virtual bool isCLReady()=0;  
      
      virtual void initialize(utils::Size size)=0;	
  	
    };
  }
}

/** \endcond */
