// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::geom2 { class Scene2; }

namespace icl::physics2 {

  class PaperDriver;

  /// Camera navigation + modifier-selected paper interaction.
  /** Extends geom2::Scene2MouseHandler (camera nav) and routes a *modified*
      LEFT-drag on the paper to the right *behaviour driver* on the paper's node,
      chosen by the held keyboard modifier:
        - Ctrl  + drag        -> FoldDriver       (crease press-point -> release-point)
        - Shift + drag        -> PaperMoverDriver  point grab (kinematic, holds)
        - Shift + Ctrl + drag -> PaperMoverDriver  whole-sheet grab (rigid carry)
        - plain drag / RIGHT / WHEEL -> camera (orbit / zoom)
      (Meta is accepted as Ctrl — Qt remaps physical Ctrl to Meta on macOS.)
      A modified left-drag that starts off the paper falls through to the camera.
      Paper coordinates come from PaperDriver::hit; the behaviour drivers are
      resolved via getDriver<T>(). */
  class ICLPhysics2_API PaperMouseHandler : public geom2::Scene2MouseHandler {
  public:
    PaperMouseHandler(int cameraIndex, geom2::Scene2 *scene, PaperDriver *paper);
    ~PaperMouseHandler() override;

    void process(const qt::MouseEvent &e) override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
