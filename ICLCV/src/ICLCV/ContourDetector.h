/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ContourDetector.h                      **
** Module : ICLCV                                                  **
** Authors: Sergius Gaulik                                         **
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

#include <vector>

namespace icl{
  namespace cv{
    /// Utility class used by the ContourDetector
    struct Contour : std::vector<utils::Point> {
      int id;             //!< contour ID
      int is_hole;        //!< is it a hole
      int parent;         //!< parent ID
      std::vector<int> children; //!< child contours

      // draws the contour in the first channel of the given image
      void drawTo(core::ImgBase *img, const icl64f &value);
    };

    /// The ContourDetector extracts all contours of a given image.
    /** Internally, the implementation works on binary images only,
        but a compatiblity layer is provided that allows for working
        on arbitraryly-typed images by internally converting the input
        image before the compuation takes place. The algorithms alters
        the values of the input image for performance reasons. If a
        const image is passed to the ContourDetector::detect method,
        the image is copied/converted before

        \section HIER Contour Hierarchy
        
        The ContourDetector can be set up to also extract a countour
        hierarchy.
    **/
    class ContourDetector : public utils::Uncopyable{
      
      /// internal data type
      class Data;

      /// internal data pointer
      Data *m_data;
      
      public:

      // contructor
      /** @param thesh threshold for creating the binary image
          @param hierarchy shows if the relationship of the contours should be calculated
      */
      ContourDetector(const icl8u thresh=128, const bool createHierarchy=true);
  
      // destructor
      virtual ~ContourDetector();

      // draws all contours to the first channel of the given image with the given value
      void drawAllContours(core::ImgBase *img, const icl64f &value);

      // calculates all contours (creates a deep copy/conversion to Img8u of the input image)
      const std::vector<Contour> &detect(const core::ImgBase *image);

      // calculates all controus (alters the values of the input image)
      const std::vector<Contour> &detect(core::Img8u &image);

      /// sets new binarization threshold
      void setThreshold(const icl8u &threshold);
      
      /// sets whether a contour hierarchy is created
      void setCreateHierarchy(bool createHierarchy);
    };
  
  } // namespace cv
}
