/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLCV/OpenCVCamCalib.h                     **
** Module : ICLGeom                                                **
** Authors: Christian Groszewski                                   **
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

#include <ICLCore/Img.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLMath/DynMatrix.h>

#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>

#include <ICLCore/OpenCV.h>


namespace icl{
  namespace cv{
    
    /// Cameracalibration using OpenCV functions.
    class ICLGeom_API OpenCVCamCalib : public utils::Uncopyable{
      
      struct Data;
      ///Class for internal params and buffers.
      Data *m_data;
      
      public:
      ///Constructor
      /**boardWidth and boardHeight should not be equal
  	  @param boardWidth width of the chessboard
  	  @param boardHeight of the chessboard
  	  @param boardCount minimum number of chessboards to be found on images before calibration
          */
      ICLCV_API OpenCVCamCalib(unsigned int boardWidth=6, unsigned int boardHeight=9, unsigned int boardCount=8);
      
      ///Destructor
      ICLCV_API ~OpenCVCamCalib();
      
      ///Adds points from images to computation.
      /*@param img image to be searched for chessboard and points
          @return overall current number of found chessboard for calibration
          */
      ICLCV_API int addPoints(const core::ImgBase *img);
      
      ///Tries to calibrates the camera, if minimal number of found and valid chessboards  is greater zero
      ICLCV_API void calibrateCam();
        
      ///Computes the undistorted image.
      /*@return the new undistorted image*/
      ICLCV_API core::ImgBase *undisort(const core::ImgBase *img);
  
      ///resets internal data and sets given params
      /*@param width of the chessboard
  	  @param height of the chessboard
  	  @param count minimum number of chessboards to be found on images before calibration*/
      ICLCV_API void resetData(int width, int height, int count);
  
      ///Returns DynMatrix of intrinsic params
      /*@return intrinsic params*/
      ICLCV_API math::DynMatrix<icl64f> *getIntrinsics();
  
      ///Returns DynMatrix of distortion params
      /*@return distortion params*/
      ICLCV_API math::DynMatrix<icl64f> *getDistortion();
  
      ///loads intrinsic params from file (in OpenCV format)
      ICLCV_API void loadParams(const char* xmlfilename);
  
      ///saves intrinsic params to file (from OpenCV format)
      ICLCV_API void saveParams(const char* xmlfilename);
  
    };
  } // namespace geom

}

