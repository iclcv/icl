// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/Point.h>
#include <icl/viz3d/Driver.h>
#include <icl/physics2/Units.h>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  class PaperDriver;

  /// A *behaviour* driver that folds the paper it shares a node with.
  /** FoldDriver owns no physics state — it attaches to the SAME viz3d::MeshNode as
      a PaperDriver, resolves that substrate via `node()->getDriver<PaperDriver>()`
      in `onAttach`, and turns fold *requests* (from a mouse handler) into
      `PaperDriver::foldAlongLine` calls. Its own tunables (auto-extend) live here,
      separate from the substrate's. This is the composition the driver model
      enables: one node, a substrate + stacked behaviours, dispatched by input. */
  class ICLPhysics2_API FoldDriver : public viz3d::Driver,
                                     public utils::Configurable {
  public:
    FoldDriver();
    void onAttach() override;

    /// Create a crease along the paper-space segment a->b (paper coords [0,1]^2).
    void foldAlongLine(const utils::Point32f &a, const utils::Point32f &b);

    PaperDriver *paper() const { return m_paper; }

    /// Live fold-line preview (world coords, ICL units) shown while the user drags
    /// out a crease — a render overlay reads it via getPreview().
    void setPreview(const Vec &a, const Vec &b);
    void clearPreview();
    /// Returns true and fills a,b if a preview line is currently active.
    bool getPreview(Vec &a, Vec &b) const;

  private:
    PaperDriver *m_paper = nullptr;
    bool m_previewActive = false;
    Vec m_previewA, m_previewB;
  };

} // namespace icl::physics2
