/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/OpenCV.h                           **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Christian Groszewski              **
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

#include <opencv2/core/version.hpp>

#if CV_MAJOR_VERSION >= 3
#elif CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION >= 4
#else
  #error ICL requires at least OpenCV 2.4
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <ICLCore/CCFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/ImgBase.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace core{

    /// Modes that define whether to prefer the source image's or the destination image's depth
    enum DepthPreference{
      PREFERE_SRC_DEPTH, //!< prefer source depth
      PREFERE_DST_DEPTH  //!< prefer destination depth
    };

    /// converts icl image into opencv's C++ image type cv::Mat (data is deeply copied)
    /** If a destimation Mat is given, it will be set up to resemble the input images
        parameters exactly. Therefore, the data is always copied and never converted */
    ICLCore_API ::cv::Mat *img_to_mat(const ImgBase *src, ::cv::Mat *dst=0);

    /// converts cv::Mat to ImgBase (internally the pixel data is type-converted if needed)
    ICLCore_API ImgBase *mat_to_img(const ::cv::Mat *src, ImgBase *dstIn=0);

    /// converts cv::Mat to ImgBase (internally the pixel data is type-converted if needed)
    ICLCore_API void mat_to_img(const ::cv::Mat *src, ImgBase **dstIn);


    /// Very simply wrapper about the opencv C++ matrix type cv::Mat
    struct ICLCore_API MatWrapper{
      ::cv::Mat mat;
      MatWrapper();
      MatWrapper(depth d, const utils::Size &size, int channels);
      MatWrapper(const MatWrapper &other);
      explicit MatWrapper(const ::cv::Mat &other);

      void adapt(depth d, const utils::Size &size, int channels);

      MatWrapper &operator=(const ::cv::Mat &other);
      MatWrapper &operator=(const MatWrapper &other);
      MatWrapper &operator=(const ImgBase &image);
      void copyTo(ImgBase **dst);
      void convertTo(ImgBase &dst);

      utils::Size getSize() const;
      int getChannels() const;
      depth getDepth() const;

      template<class T> ICLCore_API
      T* getInterleavedData();

      template<class T> ICLCore_API
      const T* getInterleavedData() const;
    };

  } // namespace core
}
