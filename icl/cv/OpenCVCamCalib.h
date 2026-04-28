// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/ImgBase.h>
#include <icl/math/DynMatrix.h>

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
