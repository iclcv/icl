// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/PluginRegistry.h>
#include <icl/geom/PointCloudGrabber.h>

#include <map>
#include <memory>
#include <string>

namespace icl::geom {
  /// Creation-time params for PointCloudGrabber backends.
  /** Historically a `std::map<std::string,std::string>` with a single
      `creation-string` key — preserved as a map for backend-API
      compatibility. */
  using PointCloudGrabberData = std::map<std::string, std::string, std::less<>>;

  /// Process-wide registry of PointCloudGrabber factories.
  /** Replaces the legacy `utils::PluginRegister<PointCloudGrabber>` shim
      with a direct `ClassPluginRegistry` accessor. */
  using PointCloudGrabberRegistry =
      utils::ClassPluginRegistry<PointCloudGrabber, const PointCloudGrabberData&>;

  ICLGeom_API PointCloudGrabberRegistry& pointCloudGrabberRegistry();

  /// Per-backend creation-syntax hint (e.g. `"filepattern[@loop=off]"`).
  /** Parallel side map because `PluginRegistry::Entry` carries only one
      description string. Mutated only from `REGISTER_POINT_CLOUD_GRABBER`
      at static-init time; read at runtime by `pointCloudGrabberInfoTable`. */
  ICLGeom_API std::map<std::string, std::string>& pointCloudGrabberSyntaxMap();

  /// Render a TextTable of "ID | Description | Creation Syntax" for
  /// `-pci list` style affordances.
  ICLGeom_API std::string pointCloudGrabberInfoTable();
} // namespace icl::geom

/// Self-register a PointCloudGrabber backend at static-init time.
/** CREATE_FUNC must be a function/callable returning `PointCloudGrabber*`
    (raw — caller takes ownership via the internal factory adapter).
    \code
      REGISTER_POINT_CLOUD_GRABBER(pcd, create_pcd_file_grabber,
                                   "Grabber for .pcd file patterns",
                                   "creation-string: filepattern[@loop=off]");
    \endcode */
#define REGISTER_POINT_CLOUD_GRABBER(NAME, CREATE_FUNC, DESC, SYNTAX)           \
  extern "C" __attribute__((constructor, used)) void                            \
  iclRegisterPointCloudGrabber_##NAME() {                                       \
    ::icl::geom::pointCloudGrabberRegistry().registerPlugin(#NAME,              \
      [](const ::icl::geom::PointCloudGrabberData& d) {                         \
        return std::unique_ptr<::icl::geom::PointCloudGrabber>(CREATE_FUNC(d)); \
      },                                                                        \
      DESC);                                                                    \
    ::icl::geom::pointCloudGrabberSyntaxMap()[#NAME] = SYNTAX;                  \
  }
