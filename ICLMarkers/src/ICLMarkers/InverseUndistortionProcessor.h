// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/Point32f.h>
#include <ICLUtils/Uncopyable.h>

#include <vector>

namespace icl{
  /// Utility class that performs gradient-descent based inverse undistortion mapping
  /** Internally, the class has two operation modes. An OpenCL-implementation and a C++-version.
      Unfortunately, the OpenCL version is not significantly faster for common numbers of points */
  class InverseUndistortionProcessor {
    struct Data;   //!< internal data structure
    Data *m_data;  //!< internal data pointer

    public:
    InverseUndistortionProcessor(const InverseUndistortionProcessor&) = delete;
    InverseUndistortionProcessor& operator=(const InverseUndistortionProcessor&) = delete;


    /// constructor
    InverseUndistortionProcessor(bool preferOpenCL);

    /// destructor
    ~InverseUndistortionProcessor();

    /// sets whether to try to use open cl
    void setPreferOpenCL(bool preferOpenCL);

    /// performs the mapping with given distortion coefficients
    const std::vector<utils::Point32f> &run(const std::vector<utils::Point32f> &p, const float kf[9]);
  };
}
