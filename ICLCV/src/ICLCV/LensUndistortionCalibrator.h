/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/LensUndistortionCalibrator.h       **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/Point32f.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/ImageUndistortion.h>

#include <vector>

namespace icl{

  namespace cv{
    
    /// Utility class for estimation of compensation parameters of lens-distortion 
    /** Internally, this is a wrapper class around OpenCV's cvCalibrateCamera2.
        For the calibration, a set of "grids", each consisting of a set of
        2D-image to 3D-object coordinates is needed. The 3D object coordinates
        are assumed to be coplanar and to be normalized to the range [0,1] in 
        the remaining x and y dimensions. Theses normalized x/y coordinates are
        defined by the LensUndistortionCalibrator::GridDefinition class.
        */
    class ICLCV_API LensUndistortionCalibrator : public utils::Uncopyable{
      struct Data;  //!< internal data
      Data *m_data; //!< internal data pointer
      
      public:
      
      /// Utility data class describing the grid structure that is used
      struct ICLCV_API GridDefinition : public std::vector<utils::Point32f>{
        
        /// creates an empty grid definition
        GridDefinition(){}
        
        /// creates a uniform grid with given dimensions
        GridDefinition(const utils::Size &dims, const utils::Size32f &size);
        
        /// creates a non-uniform grid resulting from a set of marker corners
        GridDefinition(const utils::Size &markerGridDims, 
                       const utils::Size32f &markerSize, 
                       const utils::Size32f &markerSpacing);
        
        /// as above but assuming square markers alligned in a uniform grid
        GridDefinition(const utils::Size &markerGridDims, float markerDim, float markerSpacing);
      };
      
      /// Internally used info structure (also returned by LensUndistortionCalibrator::getInfo)
      struct Info{
        utils::Size imageSize;  //!< current camera image size
        int numPointsAdded;     //!< total number of points added
        int numPointSetsAdded;  //!< number of sub-sets added
        const GridDefinition &gridDef; //!< current grid definition
      };

      
      /// empty default constructor (creates a null instance)
      LensUndistortionCalibrator();
      
      /// constructor with given parameters
      LensUndistortionCalibrator(const utils::Size &imagesSize, const GridDefinition &gridDef);
      
      /// Destructor
      ~LensUndistortionCalibrator();
      
      /// for deferred initialization
      void init(const utils::Size &imagesSize, const GridDefinition &gridDef);

      /// returns whether this instance was already initialized
      bool isNull() const;
      
      /// adds new points (the order must correspond to the GridDefinition that was provided)
      void addPoints(const std::vector<utils::Point32f> &imagePoints);

      /// adds new points with a given grid definition (the order must correspond to the GridDefinition that was provided)
      void addPoints(const std::vector<utils::Point32f> &imagePoints, const std::vector<utils::Point32f> &gridDef);

      /// removes all points added before
      void clear();
      
      /// returns current info
      Info getInfo();
      
      /// computes an image undistortion structure using all points added before
      io::ImageUndistortion computeUndistortion();
      
    };
  }
  
}
