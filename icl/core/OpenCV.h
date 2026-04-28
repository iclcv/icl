// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Christian Groszewski

#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#if CV_MAJOR_VERSION < 4
  #error ICL requires OpenCV >= 4.0
#endif

#include <icl/core/cc/CCFunctions.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>

namespace icl::core {

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

  } // namespace icl::core
