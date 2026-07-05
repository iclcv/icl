// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#if !defined(ICL_HAVE_OPENGL) || !defined(ICL_HAVE_QT)
#warning "SceneMouseHandler requires ICL_HAVE_OPENGL and ICL_HAVE_QT"
#else

#include <icl/utils/CompatMacros.h>
#include <icl/cv3d/Camera.h>
#include <icl/qt/MouseHandler.h>
#include <memory>

#ifndef ICLViz3d_API
#define ICLViz3d_API
#endif

namespace icl::viz3d {

  class Scene;

  /// Mouse handler for Scene camera navigation
  /** Provides the standard ICL mouse mappings:
      - Left drag: freeView (yaw/pitch)
      - Middle drag: strafe (pan)
      - Right drag: rotate around cursor
      - Wheel / Left+Right drag: roll & dolly
      - Shift+Ctrl+Click: place cursor (rotation center) via hit-testing */
  class ICLViz3d_API SceneMouseHandler : public qt::MouseHandler {
  public:
    SceneMouseHandler(int cameraIndex, Scene *scene);
    ~SceneMouseHandler();

    void setSensitivities(float translation, float rotation = 1.0f,
                          float mouse = 1.0f, float wheel = 0.0004f,
                          float modifier = 10.0f);

    qt::MouseResult process(const qt::MouseEvent &e) override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d

#endif
