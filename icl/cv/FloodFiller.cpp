// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/FloodFiller.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {
  Rect FloodFiller::prepare(const Size &imageSize, const Point &seed){
    result.ffLUT.setChannels(1);
    result.ffLUT.setSize(imageSize);
    result.ffLUT.fill(0); // 0 not used, 1 visited

    result.pixels.clear();
    const Rect r(Point::null,imageSize);
    ICLASSERT_THROW(r.contains(seed.x,seed.y),ICLException("FloodFiller::apply: seedpoint lies outside the image boundaries"));
    futurePoints.resize(imageSize.getDim());
    return r;
  }

  const FloodFiller::Result &FloodFiller::apply(const ImgBase *image,
                                                              const Point &seed,
                                                              double referenceValue,
                                                              double threshold){
    ICLASSERT_THROW(image,ICLException("FloodFiller::apply: input image is null"));
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:                                                    \
        applyGeneric(*image->as##D(),                                   \
                     seed,                                              \
                     DefaultCriterion<icl##D>(referenceValue,           \
                                              threshold));              \
        break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default:
          ICL_INVALID_DEPTH;
      }
      return result;
    }

  const FloodFiller::Result &FloodFiller::applyColor(const ImgBase *image, const Point &seed,
                                                                   double refR, double refG, double refB, double threshold){
    ICLASSERT_THROW(image,ICLException("FloodFiller::apply: input image is null"));
    ICLASSERT_THROW(image->getChannels() >= 3,ICLException("FloodFiller::apply: input image has less then 3 channels"));

    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:                                                    \
        applyColorGeneric(*image->as##D(),                              \
                          seed,                                         \
                          ReferenceColorCriterion<icl##D>               \
                          (refR,refG,refB,                              \
                           threshold));                                 \
        break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default:
        ICL_INVALID_DEPTH;
    }
    return result;
  }

  } // namespace icl::cv