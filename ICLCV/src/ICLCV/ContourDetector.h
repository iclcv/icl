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

#include <vector>

namespace icl{
  namespace cv{

    struct Contour : std::vector<utils::Point> {
      int id;
      int is_hole;
      int parent;
      std::vector<int> children;

      // draws the contour in the first channel of the given image
      template<class T> void drawContour(core::Img<T> &img, icl64f value) {
        T *d = img.getData(0);
        int lineStep = img.getLineStep();
        for (std::vector<utils::Point>::iterator it = begin(); it != end(); ++it) {
          *(d + it->y * lineStep + it->x) = value;
        }
      }

    };

    // The ContourDetector extracts all contours of a given image.
    // The image has to be gray with the type icl8u for the values.
    class ContourDetector {

      public:

      // contructor
      /** @param thesh threshold for creating the binary image
          @param hierarchy shows if the relationship of the contours should be calculated
      */
      ContourDetector(const icl8u thresh, const bool hierarchy = true);
  
      // destructor
      virtual ~ContourDetector();

      // draws all contours to the first channel of the given image with the given value
      template<class T> void drawAllContours(core::Img<T> &img, const icl64f val) {
        for (std::vector<Contour>::iterator it = contours.begin(); it != contours.end(); ++it) {
          it->drawContour(img, val);
        }
      }

      // calculates all contours
      std::vector<Contour> &findContours(core::Img<icl8u> &img);

      private:

      int id_count;
      icl8u threshold;
      bool hierarchy;
      std::vector<Contour> contours;

      // this function converts the image values to binary
      void createBinaryValues(core::Img<icl8u> &img);

      // calculation of contours without hierarchy information
      void findContoursWithoutHierarchy(core::Img<icl8u> &_img);

      // calculation of contours with hierarchy information
      void findContoursWithHierarchy(core::Img<icl8u> &_img);

    };
  
  } // namespace cv
}
