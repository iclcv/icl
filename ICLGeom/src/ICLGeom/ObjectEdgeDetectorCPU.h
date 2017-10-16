/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorCPU.h            **
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
//Please use the ObjectEdgeDetector Class.
//This is the CPU implementation of the interface.

#pragma once

#include <ICLGeom/ObjectEdgeDetectorPlugin.h>

namespace icl{
  namespace geom{

    struct Vec4 {
	    inline Vec4() {
	    }
	    inline Vec4(float x, float y, float z, float w) :
			    x(x), y(y), z(z), w(w) {
	    }
	    float x, y, z, w;
    };

     class ICLGeom_API ObjectEdgeDetectorCPU : public ObjectEdgeDetectorPlugin{

     struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

     public:

      /// Create new ObjectEdgeDetectorCPU with given internal image size
      /** Constructs an object of this class. All default parameters are set.
          Use setters for desired values. */
      ObjectEdgeDetectorCPU();

      ///Destructor
      virtual ~ObjectEdgeDetectorCPU();

      /// One call function for calculation of the complete processingpipeline
      /** Order:  ((filter)->normals->(normalAvg)->angles->binarization)
          @param depthImage the input depth image
          @param filter enable/disable filtering
          @param average enable/disable normal averaging
          @param gauss true=gauss smoothing, false=linear smoothing
          @return the binarized angle image */
      const core::Img8u &calculate(const core::Img32f &depthImage, bool filter, bool average, bool gauss);

      /// Sets the input depth image (input for median filter).
      /** @param depthImg the input depth image */
      void setDepthImage(const core::Img32f &depthImg);

      /// Calculates a filtered image using a median filter.
      /** The mask size is set by setMedianFilterSize(int size) */
      void applyMedianFilter();

      /// Returns the filtered depth image
      /** @return the filtered depth image */
      const core::Img32f &getFilteredDepthImage();

      /// Sets the (filtered) depth image (input for normal calculation)
      /** This call is not necessary if medianFilter() is executed before
          @param filteredImg the (filtered) depth image */
      void setFilteredDepthImage(const core::Img32f &filteredImg);

      /// Calculates the point normals.
      /** The range for calculation is set by
          setNormalCalculationRange(int range). */
      void applyNormalCalculation();

      /// Recalculates the normals by linear averaging
      /** This reduces the noise of the normal image. The function is called from
          calculateNormals() if it is enabled width setUseNormalAveraging(bool use).
          The range is set by setNormalAveragingRange(int range).\n
          Alternative: normalGaussSmoothing() with setUseGaussSmoothing(true). */
      void applyLinearNormalAveraging();

      /// Recalculates the normals by gaussian smoothing in a given range.
      /** Alternative to applyTemporalNormalAveraging that uses a simpler
          gaussian filter for smoothing

          if it is enabled width setUseNormalAveraging(bool use).
          The range is set by setNormalAveragingRange(int range).
          Alternative: normalAveraging() with setUseGaussSmoothing(false). */
      void applyGaussianNormalSmoothing();

      /// Returns the Pointer to the normals
      /** @return the point normals */
      const core::DataSegment<float,4> getNormals();

      /// Transforms the normals to the world space and calculates normal image.
      /**  @param cam the camera of the depth image */
      void applyWorldNormalCalculation(const Camera &cam);

      /// Returns the point normals in world space.
      /** @return the point normals in world space */
      const core::DataSegment<float,4> getWorldNormals();

      /// Returns the RGB normal image.
      /** @return the RGB normal image */
      const core::Img8u &getRGBNormalImage();

      /// Sets the point normals (input for angle image calculation).
      /** This call is not necessary if normalCalculation() is executed before.
          @param pNormals the point normals */
      void setNormals(core::DataSegment<float,4> pNormals);

      /// Calculates the angle image.
      /** The mode is set by setAngleNeighborhoodMode(int mode).
          The range is set by setAngleNeighborhoodRange(int range) */
      void applyAngleImageCalculation();

      /// Returns the angle image.
      /** @return the angle image */
      const core::Img32f &getAngleImage();

      /// Sets the angle image (input for image binarization).
      /** This call is not necessary if angleImageCalculation() is executed before.
          @param angleImg the angle image */
      void setAngleImage(const core::Img32f &angleImg);

      /// Binarizes the angle image to detect edges.
      /** The threshold is set by setBinarizationThreshold(float threshold). */
      void applyImageBinarization();

      /// Returns the binarized angle image (final output).
      /** @return the (final) binarized angle image */
      const core::Img8u &getBinarizedAngleImage();

      /// Sets the mask size for applyMedianFilter()
      /** size n corresponds to mask size n x n. (default 3, min 3, max 9, odd only)
          @param size the mask size */
      void setMedianFilterSize(int size);

      /// Sets the range for applyNormalCalculation().
      /** (default 2)
          @param range the normal calculation range */
      void setNormalCalculationRange(int range);

      /// Sets the averaging range for applyNormalAveraging()
      /** (default 1) @param range the normal averaging range */
      void setNormalAveragingRange(int range);

      /// Sets the neighborhood mode for applyAngleImageCalculation()
      /** 0=max, 1=mean. (default 0)
          @param mode the neighborhood mode */
      void setAngleNeighborhoodMode(int mode);

      /// Sets the neighborhood range for applyAngleImageCalculation()
      /** (default 3, min 1)
          @param range the neighborhood range */
      void setAngleNeighborhoodRange(int range);

      /// Sets the binarization threshold for applyImageBinarization().
      /** Value n for acos(n). A value of 0 maps to 90 degree, a value of 1
          maps to o degree (default 0.89)
          @param threshold binarization threshold */
      void setBinarizationThreshold(float threshold);

      /// Sets normal averaging enabled/disabled.
      /** (default true=enabled)
          @param use enable/disable normal averaging */
      void setUseNormalAveraging(bool use);

      /// Sets normal averaging by gauss smoothing enabled/disabled
      /** (default false=linear smoothing if normal averaging enabled)
          @param use enable/disable gauss smoothing */
      void setUseGaussSmoothing(bool use);

      /// Returns the openCL status
      /** (true=openCL context ready, false=no openCL context available)
          @return openCL context ready/unavailable */
      bool isCLReady();

  	  void initialize(utils::Size size);


  	 private:

  	  float scalar(math::FixedColVector<float,4> &a, math::FixedColVector<float,4> &b);
  	  float flipAngle(float angle);
  	  float scalarAndFlip(math::FixedColVector<float,4> &a, math::FixedColVector<float,4> &b);
  	  float maxAngle(float snr, float snl, float snt, float snb,
                     float snbl, float snbr, float sntl, float sntr);

    };
  }
}

/** \endcond */
