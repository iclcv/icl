// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#if !defined(ICL_HAVE_OPENGL) || !defined(ICL_HAVE_QT)
#warning "Scene2MouseHandler requires ICL_HAVE_OPENGL and ICL_HAVE_QT"
#else

#include <icl/utils/CompatMacros.h>
#include <icl/geom/Camera.h>
#include <icl/qt/MouseHandler.h>
#include <memory>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  class Scene2;

  /// Mouse handler for Scene2 camera navigation
  /** Provides the standard ICL mouse mappings:
      - Left drag: freeView (yaw/pitch)
      - Middle drag: strafe (pan)
      - Right drag: rotate around cursor
      - Wheel / Left+Right drag: roll & dolly
      - Shift+Ctrl+Click: place cursor (rotation center) via hit-testing */
  class ICLGeom2_API Scene2MouseHandler : public qt::MouseHandler {
  public:
    Scene2MouseHandler(int cameraIndex, Scene2 *scene);
    ~Scene2MouseHandler();

    void setSensitivities(float translation, float rotation = 1.0f,
                          float mouse = 1.0f, float wheel = 0.001f,
                          float modifier = 10.0f);

    void process(const qt::MouseEvent &e) override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2

#endif
