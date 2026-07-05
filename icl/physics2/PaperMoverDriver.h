// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/Point.h>
#include <icl/viz3d/scene/Driver.h>
#include <icl/physics2/Units.h>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  class PaperDriver;

  /// A *behaviour* driver that moves the paper it shares a node with — in two
  /// modes: a soft, Gaussian-falloff point drag, and a rigid whole-sheet move.
  /** Like FoldDriver, it owns no physics state: it resolves the sibling
      PaperDriver in `onAttach` and forwards drag / move requests to it. Its drag
      strength + radius are its own Configurable tunables, kept separate from the
      substrate and the fold behaviour — so a node can carry a substrate plus
      several behaviours, each with its own UI, dispatched by input. */
  class ICLPhysics2_API PaperMoverDriver : public viz3d::Driver,
                                           public utils::Configurable {
  public:
    PaperMoverDriver();
    void onAttach() override;

    /// Begin a persistent grab at \a paperCoords (uses the configured grab radius).
    /// The grabbed patch holds its position until released — even when the mouse
    /// stops moving (unlike the one-shot dragPoint nudge).
    void beginGrab(const utils::Point32f &paperCoords);
    /// Begin a persistent grab of the WHOLE sheet (carry it rigidly). Same hold
    /// semantics as beginGrab — it stays where you leave it until release.
    void beginSheetGrab(const utils::Point32f &paperCoords);
    /// Move the active grab so its centre follows \a worldTarget (ICL units).
    void updateGrab(const Vec &worldTarget);
    /// Release the grab.
    void endGrab();

    PaperDriver *paper() const { return m_paper; }

  private:
    PaperDriver *m_paper = nullptr;
  };

} // namespace icl::physics2
