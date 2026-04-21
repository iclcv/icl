// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Sergius Gaulik, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Img.h>

#include <vector>
#include <icl/core/Image.h>

namespace icl{
  namespace core{ class Image; }
  namespace cv{

    struct ContourImpl{
      virtual ~ContourImpl() = default;
      virtual bool hasHierarchy() const = 0;
      virtual int getID() const = 0;
      virtual bool isHole() const = 0;
      virtual const std::vector<int> &getChildren() const = 0;
      virtual const utils::Point *begin() const = 0;
      virtual const utils::Point *end() const = 0;
    };

    /// Utility class used by the ContourDetector
    class ICLCV_API Contour{
      ContourImpl *impl;

      public:
      Contour(ContourImpl *impl = 0):impl(impl){}

      inline bool isNull() const {
        return !impl;
      }
      inline operator bool() const {
        return !!impl;
      }
      inline bool hasHierarchy() const{
        return impl->hasHierarchy();
      }
      inline int getID() const {
        return impl->getID();
      }
      bool isHole() const{
        return impl->isHole();
      }
      const std::vector<int> &getChildren() const{
        return impl->getChildren();
      }
      const utils::Point *begin() const{
        return impl->begin();
      }
      const utils::Point *end() const{
        return impl->end();
      }
      const int getSize() const{
        return static_cast<int>(end() - begin());
      }
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

        \section ALG Algorithms

        Internally 2 different contour tracing algorithms are implemented. While
        the "Fast" method uses a 4-point neighbourhood, the Accurate method uses
        a 8-point neighborhood an can also optionally be used to obtain a region
        hierarchy. The fast method uses its own memory allocator to improve runtime
        performance.

    **/
    class ICLCV_API ContourDetector {

      /// internal data type
      struct Data;

      /// internal data pointer
      Data *m_data;

      public:
      ContourDetector(const ContourDetector&) = delete;
      ContourDetector& operator=(const ContourDetector&) = delete;


      /// contour tracing algorithm used
      enum Algorithm{
        Fast,                  //!< fast contour detection algorithm (using 4-point neighbourhood)
        Accurate,              //!< accurate contour detection algorithm (using 8-point neighborhood)
        AccurateWithHierarchy  //!< same as Accurate, but with Hierarchy estimation
      };

      // contructor
      /** @param thesh threshold for creating the binary image
          @param hierarchy shows if the relationship of the contours should be calculated
      */
      ContourDetector(const icl8u thresh=128, Algorithm a = Fast);

      // destructor
      virtual ~ContourDetector();

      // draws all contours to the first channel of the given image with the given value
      void drawAllContours(core::ImgBase *img, const icl64f &value);

      // calculates all contours (creates a deep copy/conversion to Img8u of the input image)
      const std::vector<Contour> &detect(const core::ImgBase *image);

      /// Image-based overload
      inline const std::vector<Contour> &detect(const core::Image &image) {
        return detect(image.ptr());
      }

      // calculates all controus (alters the values of the input image)
      const std::vector<Contour> &detect(core::Img8u &image);

      /// sets new binarization threshold
      void setThreshold(const icl8u &threshold);

      /// sets whether a contour hierarchy is created
      void setAlgorithm(Algorithm a);
    };

  } // namespace cv
}
