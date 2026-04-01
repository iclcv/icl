// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/PointCloudObjectBase.h>

namespace icl{
  namespace geom{

    /// Generic interface for PointCloud sources
    struct PointCloudOutput{
      /// fills the given point cloud with grabbed information
      virtual void send(const PointCloudObjectBase &dst) = 0;

      /// virtual, but empty destructor
      virtual ~PointCloudOutput(){}
    };
  } // namespace geom
}
