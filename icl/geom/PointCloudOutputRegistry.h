// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/PluginRegistry.h>
#include <icl/geom/PointCloudOutput.h>

#include <map>
#include <memory>
#include <string>

namespace icl::geom {
  /// Creation-time params for PointCloudOutput backends (mirror of
  /// `PointCloudGrabberData`).
  using PointCloudOutputData = std::map<std::string, std::string, std::less<>>;

  using PointCloudOutputRegistry =
      utils::ClassPluginRegistry<PointCloudOutput, const PointCloudOutputData&>;

  ICLGeom_API PointCloudOutputRegistry& pointCloudOutputRegistry();

  /// Per-backend creation-syntax hint. See PointCloudGrabberRegistry.h
  /// for the rationale (primitive's Entry carries only one description
  /// string).
  ICLGeom_API std::map<std::string, std::string>& pointCloudOutputSyntaxMap();

  /// TextTable of "ID | Description | Creation Syntax".
  ICLGeom_API std::string pointCloudOutputInfoTable();
} // namespace icl::geom

#define REGISTER_POINT_CLOUD_OUTPUT(NAME, CREATE_FUNC, DESC, SYNTAX)            \
  extern "C" __attribute__((constructor, used)) void                            \
  iclRegisterPointCloudOutput_##NAME() {                                        \
    ::icl::geom::pointCloudOutputRegistry().registerPlugin(#NAME,               \
      [](const ::icl::geom::PointCloudOutputData& d) {                          \
        return std::unique_ptr<::icl::geom::PointCloudOutput>(CREATE_FUNC(d));  \
      },                                                                        \
      DESC);                                                                    \
    ::icl::geom::pointCloudOutputSyntaxMap()[#NAME] = SYNTAX;                   \
  }
