/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/FloodFiller.cpp                        **
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

#include <ICLCV/FloodFiller.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{

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

  } // namespace cv
}
