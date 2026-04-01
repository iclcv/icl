// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/VisualizationDescription.h>
#include <utility>

namespace icl{
  namespace cv{

    /// Generic SURF Feature type
    struct ICLCV_API SurfFeature{
      float x;               //!< feature x-position
      float y;               //!< feature y-position
      float scale;           //!< feature size (scale factor)
      float orientation;     //!< feature direction/orientation
      int laplacian;         //!< laplacian sign
      int clusterIndex;      //!< cluster index
      float descriptor[64];  //!< 64-Dim feature descriptor
      float dx;              //!< can be used for point motion analysis
      float dy;              //!< can be used for point motion analysis

      /// distance to other feature (squared distance of descriptors)
      float operator-(const SurfFeature &other) const;

      /// visualizes this surf feature (optionally shifted by given offsets)
      utils::VisualizationDescription vis(int dx=0, int dy=0) const;
    };

    /// typedef for two matching features
    using SurfMatch = std::pair<SurfFeature,SurfFeature>;

  }
}
