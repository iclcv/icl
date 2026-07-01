// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <icl/utils/Size.h>
#include <icl/core/ImgBase.h>
#include <icl/math/la/DynMatrix.h>
#include <vector>

namespace icl::cv {
    /// Cameracalibration using OpenCV functions.
    class ICLCV_API OpenCVCamCalib {

      struct Data;
      ///Class for internal params and buffers.
      Data *m_data;

      public:
      OpenCVCamCalib(const OpenCVCamCalib&) = delete;
      OpenCVCamCalib& operator=(const OpenCVCamCalib&) = delete;

      ///Constructor
      /**boardWidth and boardHeight should not be equal
  	  @param boardWidth width of the chessboard
  	  @param boardHeight of the chessboard
  	  @param boardCount minimum number of chessboards to be found on images before calibration
          */
      OpenCVCamCalib(unsigned int boardWidth=6, unsigned int boardHeight=9, unsigned int boardCount=8);

      ///Destructor
      ~OpenCVCamCalib();

      ///Adds points from images to computation.
      /*@param img image to be searched for chessboard and points
          @return overall current number of found chessboard for calibration
          */
      int addPoints(const core::ImgBase *img);

      /// Add one view's correspondences directly, bypassing findChessboardCorners.
      /** For feeding correspondences from an external detector (e.g. ICL's own
          CalibrationTarget) or a synthetic harness, so the OpenCV calibration can
          be compared to the native one on IDENTICAL points. \a objectMM are the
          planar object points [mm] (z=0 assumed); \a imagePx the matching image
          points [px]; the two must be the same length. Remember to setImageSize()
          before calibrateCam() (the image-based addPoints sets it automatically).
          @return overall current number of accumulated views */
      int addPoints(const std::vector<utils::Point32f> &objectMM,
                    const std::vector<utils::Point32f> &imagePx);

      /// Set the calibration image size [px] (required by calibrateCam() when the
      /// correspondence-based addPoints overload is used).
      void setImageSize(const utils::Size &size);

      ///Tries to calibrates the camera, if minimal number of found and valid chessboards  is greater zero
      void calibrateCam();

      ///Computes the undistorted image.
      /*@return the new undistorted image*/
      core::ImgBase *undisort(const core::ImgBase *img);

      ///resets internal data and sets given params
      /*@param width of the chessboard
  	  @param height of the chessboard
  	  @param count minimum number of chessboards to be found on images before calibration*/
      void resetData(int width, int height, int count);

      ///Returns DynMatrix of intrinsic params
      /*@return intrinsic params*/
      math::DynMatrix<icl64f> *getIntrinsics();

      ///Returns DynMatrix of distortion params
      /*@return distortion params*/
      math::DynMatrix<icl64f> *getDistortion();

      ///loads intrinsic + distortion params from file (OpenCV FileStorage / YAML or XML)
      void loadParams(const char* filename);

      ///saves intrinsic + distortion params to file (OpenCV FileStorage / YAML or XML)
      void saveParams(const char* filename);

    };
  } // namespace icl::cv
