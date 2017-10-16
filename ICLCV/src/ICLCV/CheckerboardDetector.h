/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CheckerboardDetector.h                 **
** Module : ICLCV                                                  **
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

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLUtils/Configurable.h>

namespace icl{

  namespace cv{

    /// Utility class wrapping OpenCV's cvFindChessboardCorners
    /** The CheckerboardDetector wrappes OpenCV's cvFindChessboardCorners and provides
        a flag to optionally optimize the detected corners using
        cvFindCornerSubPix */
    class ICLCV_API CheckerboardDetector : public utils::Configurable, public utils::Uncopyable{
      struct Data;   //!< internal data data
      Data *m_data;  //!< internal data pointer

      ///intializes configurable properties internally
      void init_properties();

      public:

      /// Default constructor (creates a null instance)
      CheckerboardDetector();

      /// Constructor with given checkerboard size
      /** Please note: the checkerboard size given relates to the inner checkerboard
          corners that are expected. So if the checkerboard has 5 by 5 fields, i.e.
          the first row is like BWBWB (Black/White), then you have to pass a size
          of 4x4 */
      CheckerboardDetector(const utils::Size &size);

      /// Destructor
      ~CheckerboardDetector();

      /// for deferred initialization
      /** Please note: the checkerboard size given relates to the inner checkerboard
          corners that are expected. So if the checkerboard has 5 by 5 fields, i.e.
          the first row is like BWBWB (Black/White), then you have to pass a size
          of 4x4 */
      void init(const utils::Size &size);

      /// Internally used and returned result structure
      struct Checkerboard{
        bool found;        //!< waes it found (i.e. all of the corners)
        utils::Size size;  //!< used size (see init)
        std::vector<utils::Point32f> corners; //!< found corners
        ICLCV_API utils::VisualizationDescription visualize() const;
      };

      /// returns whether this instance has been initilialized yet
      bool isNull() const;

      /// detects the defined checkerboard in the given image
      /** The image can have any format, but internally is is always converted
          to gray (if it is not of formatGray). If optSubPix was set in
          either the constructor or in init, the returned corners are
          automatically optimized using cvFindCornerSubPix */
      const Checkerboard &detect(const core::Img8u &image);

      /// convenience method that automatically scales the source images range to 0,255 if it is not already of type Img8u
      const Checkerboard &detect(const core::ImgBase *image);
    };

  } // namespace cv
} // namespace icl
