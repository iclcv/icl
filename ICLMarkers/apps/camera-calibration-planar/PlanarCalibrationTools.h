// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/Point32f.h>
#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/GeomDefs.h>

namespace icl::markers {
    geom::Camera extract_camera_from_udist_file(const std::string &filename);

    /// returns the position variances of the last 10 frames var([x,y,z, rx, ry, rz])
    std::vector<float> estimate_pose_variance(const geom::Mat &T);
  }