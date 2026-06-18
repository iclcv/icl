// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/MouseHandler.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::geom2 { class Scene2; }

namespace icl::physics2 {

  class PaperDriver;

  /// Modifier-selected paper interaction (one link in the handler chain).
  /** A plain qt::MouseHandler that routes a *modified* LEFT-drag on the paper to
      the right *behaviour driver* on the paper's node, chosen by the held
      keyboard modifier:
        - Ctrl  + drag        -> FoldDriver       (crease press-point -> release-point)
        - Shift + drag        -> PaperMoverDriver  point grab (kinematic, holds)
        - Shift + Ctrl + drag -> PaperMoverDriver  whole-sheet grab (rigid carry)
      (Meta is accepted as Ctrl — Qt remaps physical Ctrl to Meta on macOS.)
      Returns MouseResult::Processed for those gestures (and for any modified
      press, so a near-miss never reaches the camera); returns Forward for plain
      drags / right / wheel, so the camera handler installed *after* this one
      drives navigation. Install order: this handler first, camera last.
      Paper coordinates come from PaperDriver::hit; the behaviour drivers are
      resolved via getDriver<T>(). */
  class ICLPhysics2_API PaperMouseHandler : public qt::MouseHandler {
  public:
    PaperMouseHandler(int cameraIndex, geom2::Scene2 *scene, PaperDriver *paper);
    ~PaperMouseHandler() override;

    qt::MouseResult process(const qt::MouseEvent &e) override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
