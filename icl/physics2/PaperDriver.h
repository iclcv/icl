// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/Point.h>
#include <icl/utils/Size.h>
#include <icl/core/Img.h>
#include <icl/geom2/Driver.h>
#include <icl/physics2/Units.h>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

class btSoftBody;

namespace icl::geom { struct ViewRay; class Camera; }

namespace icl::physics2 {

  class PhysicsWorld;

  /// The fold-aware *paper* substrate driver — the crown-jewel transplant of the
  /// legacy `PhysicsPaper3` onto geom2 + the driver model.
  /** PaperDriver owns a manually-built `btSoftBody` whose dual mesh (corner grid +
      per-cell centre vertices) carries first-order structural links and a
      second-order bending graph; folding *splits* triangles along a crease line,
      inserting weak fold links recorded in a FoldMap so the paper bends there.
      The driver binds that soft body to a `geom2::MeshNode`: it builds the mesh
      topology, registers a post-step capture hook (the sim thread snapshots node
      positions + — when a fold changed the topology — the new triangle list into a
      `PaperStateBuffer`), and on the UI thread copies positions into the mesh each
      `sync`, rebuilding the topology only when the structure version changes.

      This is the *substrate*: it exposes the Bullet-level paper operations
      (fold / drag / whole-sheet move / picking) as a public API. Behaviour drivers
      (`FoldDriver`, `PaperMoverDriver`) attach to the SAME node and drive these
      ops, dispatched by input — the composition the monolithic legacy class could
      not express. All structural mutations route through the world command queue
      (sim-thread safe), closing the legacy `// TODO IMPLEMENT LOCKER` gaps.

      Runs only in a `SoftBodyMode::SoftRigid` world: paper needs the legacy
      cluster self-collision + per-link solver constants the deformable pipeline
      has no equivalent for. Host node MUST be a geom2::MeshNode. */
  class ICLPhysics2_API PaperDriver : public geom2::Driver,
                                      public utils::Configurable {
  public:
    using Point32f = utils::Point32f;
    using LinkCoords = std::pair<Point32f, Point32f>;

    /// Opaque PIMPL state (defined in the .cpp); public only so the file-local
    /// sim-side fold primitives can name it.
    struct Data;

    /// \a cells = grid resolution (corner vertices per axis). \a corners (4 in
    /// ul,ur,ll,lr order, ICL units) define the flat sheet; null = a default
    /// A4-ish sheet. \a initialStiffness <=0 reads stiffness from the FoldMap;
    /// \a maxLinkDist is the bending-constraint range in paper units.
    PaperDriver(PhysicsWorld &world, const utils::Size &cells,
                const Vec *corners = nullptr, bool enableSelfCollision = false,
                float initialStiffness = -1.f, float maxLinkDist = 0.4f);
    ~PaperDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    btSoftBody *softBody() const;
    int getNumNodes() const;

    // --- paper-space operations (sim-thread safe; routed via the command queue) ---
    /// Create a fold (crease) along the paper-space segment a->b. With
    /// \a autoExtendToEdges the line is extended to the sheet boundary first.
    void foldAlongLine(const Point32f &a, const Point32f &b, bool autoExtendToEdges = true);
    /// Soft, Gaussian-falloff drag: pull the node nearest \a paperCoords toward
    /// \a worldTarget (ICL units), influencing neighbours within \a radius. This is
    /// a one-shot velocity nudge (decays once you stop calling it).
    void dragPoint(const Point32f &paperCoords, const Vec &worldTarget,
                   float strength = 1.f, float radius = 0.1f);

    /// Begin a *persistent* kinematic grab: pin the nodes within \a radius (paper
    /// units) of \a paperCoords (inverse-mass 0) so they hold their position even
    /// when not moving. Follow with updateGrab() to move them, endGrab() to let go.
    void beginGrab(const Point32f &paperCoords, float radius = 0.06f);
    /// Move the grabbed patch so its centre follows \a worldTarget (ICL units),
    /// preserving the grabbed nodes' relative offsets. No-op if not grabbing.
    void updateGrab(const Vec &worldTarget);
    /// Release the grab, restoring the pinned nodes' original masses.
    void endGrab();
    /// Rigidly translate the whole sheet by \a deltaIcl (offsets every node).
    void wholeSheetMove(const Vec &deltaIcl);
    /// Flatten the paper back to its initial corners with zero velocity (keeps
    /// the current fold topology).
    void reset();

    // --- picking / queries (UI thread; lock the world internally) ---
    /// Paper coordinates ([0,1]^2) hit by \a ray, or (-1,-1) on a miss.
    Point32f hit(const geom::ViewRay &ray) const;
    /// Crease line for a screen-space drag, computed by *cutting* the paper with
    /// the plane spanned by the camera centre and the two endpoint rays
    /// (\a rayA = press, \a rayB = release; both share the eye as offset). The
    /// intersection of that plane with the paper faces is the crease, so the
    /// drag may start or end off the sheet. Returns the two extreme crease points
    /// (paper coords), or null if the plane misses the paper. Locks the world.
    std::shared_ptr<LinkCoords> projectScreenLine(const geom::ViewRay &rayA,
                                                  const geom::ViewRay &rayB) const;
    /// World position (ICL units) of a paper coordinate (barycentric).
    Vec interpolatePosition(const Point32f &paperCoords) const;
    /// The fold link nearest screen pixel \a pix (paper-coord endpoints), or null.
    std::shared_ptr<LinkCoords> getLinkCoords(const Point32f &pix, const geom::Camera &cam) const;
    /// Adapt the stiffness of the fold at \a coords (optionally memorize the
    /// current deformation as the new rest state).
    void adaptFoldStiffness(const LinkCoords &coords, float stiffness, bool memorize = false);

    /// The current fold map (discretized crease stiffness) for display.
    const core::Img32f &getFoldMap() const;

    /// World-space (ICL units) segments of every crease (fold) link — for drawing
    /// a crease highlight overlay. Locks the world internally (UI-thread safe).
    std::vector<std::pair<Vec, Vec>> getCreaseSegments() const;

    /// Categorized soft-body geometry (world-space, ICL units) for optional debug
    /// overlays. Each vector holds line segments. Locks the world internally.
    struct DebugGeometry {
      std::vector<std::pair<Vec, Vec>> faces;        ///< triangle edges (mesh wireframe)
      std::vector<std::pair<Vec, Vec>> firstOrder;   ///< structural links (distance preservation between neighbours)
      std::vector<std::pair<Vec, Vec>> secondOrder;  ///< bending links (non-neighbour constraints)
      std::vector<std::pair<Vec, Vec>> creases;      ///< fold links
    };
    DebugGeometry getDebugGeometry() const;

  private:
    void buildBody();   ///< UI thread (onAttach): construct soft body + mesh topology

    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
