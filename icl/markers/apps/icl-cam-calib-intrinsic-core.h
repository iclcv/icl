// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// Headless core for icl-cam-calib-intrinsic (easy intrinsic calibration).
//
// Everything here is GUI-free and typed so it can be driven by the --sim-selftest
// path (and a future gtest) with no widgets: pick a calibration target, feed
// detected object↔image correspondences per view into an IntrinsicSession, and
// solve for the pinhole + radial/tangential intrinsics via cv::IntrinsicCalibrator.
// The sim helpers mirror viz3d::OffscreenView's forward lens distortion so a
// rendered board carries a known ground-truth camera the recovered intrinsics can
// be checked against. See intrinsic-calib-app-plan.md.

#include <icl/markers/CalibrationTarget.h>
#include <icl/cv/IntrinsicCalibrator.h>
#include <icl/viz3d/Node.h>
#include <icl/core/Img.h>
#include <icl/utils/Size.h>   // Size + Size32f (SizeT<float>)
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace icl::viz3d { class MeshNode; }

namespace icl::calibintr {

  /// Which kind of calibration target is in use.
  enum class TargetType { Checkerboard, Coded, Coded2, MarkerGrid };

  /// Typed description of a calibration target + its metric geometry.
  /** The single source of truth for both the detector (makeTarget) and the sim
      scene geometry (makeSceneNode) — they MUST agree on physical scale (mm) for
      the recovered focal length to be metrically correct. */
  struct TargetSpec {
    TargetType type = TargetType::Checkerboard;

    // checkerboard / coded-checkerboard: counts are CHECKER SQUARES; the
    // detectable inner corners are (cols-1)×(rows-1).
    int   cols     = 9;
    int   rows     = 7;
    float squareMM = 25.f;
    bool  codedBlackCells = false;   ///< coded: markers on the BLACK squares (needs
                                     ///< pp.filter=dilatation; White is the default)

    // marker-grid only
    utils::Size    gridCells  = utils::Size(4, 3);
    utils::Size32f markerMM   = utils::Size32f(20, 20);
    float          markerGapMM = 10.f;

    std::string describe() const;   ///< human-readable one-liner
  };

  /// Build the detector/generator for a spec (a concrete markers::CalibrationTarget).
  std::unique_ptr<markers::CalibrationTarget> makeTarget(const TargetSpec &s);

  /// Build the scene geometry for a spec at the correct metric scale (mm), so the
  /// sim render matches the target's modelPoints(). Checkerboard → CheckerboardNode;
  /// coded / marker-grid → a flat textured quad from the target's generate().
  viz3d::NodePtr makeSceneNode(const TargetSpec &s);

  /// Re-populate an existing coded / marker-grid board MeshNode for \a s (texture +
  /// metric-sized quad), locked via ScopedEdit so it is safe to call from a worker
  /// loop on a node already in the scene. Lets the GUI change board geometry in
  /// place (node add/remove from a worker thread is a data race). Checkerboards use
  /// CheckerboardNode::setCells / setWidth instead.
  void rebuildBoardNode(viz3d::MeshNode &node, const TargetSpec &s);

  /// The pinhole + distortion intrinsics of a camera.
  struct Intrinsics {
    double fx = 0, fy = 0, cx = 0, cy = 0;   ///< focal [px] + principal point [px]
    double k1 = 0, k2 = 0;                    ///< radial distortion
    std::string toString() const;
  };

  // --- sim ground truth -----------------------------------------------------
  // The sim camera uses a horizontal FOV chosen so its pinhole focal equals the
  // forward-distortion focal (max(w,h)/2), making the injected lens model a
  // self-consistent MatlabModel5Params camera the calibration can recover exactly.

  /// Horizontal FOV [deg] making the pinhole focal == the distortion focal.
  float simHFovDeg(const utils::Size &imgSize);

  /// Ground-truth intrinsics of the sim camera with injected distortion k1,k2.
  Intrinsics groundTruthIntrinsics(const utils::Size &imgSize, float k1, float k2);

  /// Apply the SAME forward radial lens distortion viz3d::OffscreenView bakes into
  /// its captured frames (MatlabModel5Params, focal max(w,h)/2, principal center).
  /// k1==k2==0 → the input is returned unchanged.
  core::Img8u forwardDistort(const core::Img8u &img, float k1, float k2);

  /// Accumulates detected views and solves for the intrinsics. Fully headless.
  class IntrinsicSession {
    public:
    IntrinsicSession(const TargetSpec &spec, const utils::Size &imgSize);

    /// Add one detected view. Correspondences are labelled to full-board point
    /// indices; a view is kept only if it contributes ≥4 labelled points. Returns
    /// the number of labelled points kept (0 = view rejected).
    int addView(const std::vector<markers::CalibrationCorrespondence> &corr);

    int viewCount()       const { return (int)m_views.size(); }
    int boardPointCount() const { return (int)m_model.size(); }
    bool anyPartial()     const { return m_partial; }

    /// Solve for the intrinsics over all added views. False if <4 views. On
    /// success result()/recovered()/reprojRMS() are valid.
    bool calibrate();

    const cv::IntrinsicCalibrator::Result &result() const { return m_result; }
    Intrinsics recovered() const;             ///< result() as a plain Intrinsics
    double reprojRMS() const { return m_rms; } ///< overall reprojection RMS [px], -1 if none

    /// How close the observed corners got to the frame periphery: max corner radius
    /// over ALL views / the image half-diagonal (1.0 = a corner reached the image
    /// corner). Radial distortion (esp. k2) is only well-observed when this is high
    /// (≳0.85) — a full checkerboard that stays in view can't reach it.
    float edgeReach() const;

    /// Write the recovered intrinsics (ImageUndistortion XML) to \a file. False if
    /// not calibrated yet or the file can't be opened.
    bool save(const std::string &file) const;

    const TargetSpec &spec()      const { return m_spec; }
    const utils::Size &imageSize() const { return m_size; }

    private:
    int objectPointIndex(const geom::Vec &objectPos) const;
    double computeReprojRMS() const;

    TargetSpec  m_spec;
    utils::Size m_size;
    std::vector<geom::Vec> m_model;   ///< full-board model points (mm), canonical order
    struct View { std::vector<std::pair<int, utils::Point32f>> pts; };  ///< (index, px)
    std::vector<View> m_views;
    cv::IntrinsicCalibrator::Result m_result;
    double m_rms = -1;
    bool   m_partial = false;         ///< any view saw a strict subset → mask path
  };

  /// A compact descriptor of one detected board view, derived purely from its
  /// correspondences (no metric camera needed): where it sits in the image, how
  /// big it appears, and how tilted it is. Used for coverage binning + the
  /// stability gate.
  struct ViewDescriptor {
    utils::Point32f centroid;     ///< mean image position [px]
    float           scale = 0;    ///< apparent px-per-mm (geo-mean of the affine's singular values)
    float           tiltMag = 0;  ///< foreshortening anisotropy in [0,1) (0 = fronto-parallel)
    float           tiltDir = 0;  ///< tilt axis angle [rad] (meaningful only when tiltMag is large)
    std::vector<int> cells;       ///< image occupancy cells the corners fall in (row-major idx)
    int             region = 0;   ///< 3×3 image region of the centroid (0..8)
    int             scaleBand = 0;///< apparent-size band (0=far .. 2=near)
    int             tiltOct = 8;  ///< tilt-direction octant (0..7), or 8 = ~fronto-parallel
    bool valid = false;           ///< false if too few points to describe
  };

  /// Image-space + pose coverage tracker (headless). Accumulates, over kept views,
  /// an image occupancy grid (→ heatmap) and the set of seen pose bins
  /// (image-region × apparent-scale × tilt-direction), and answers whether a new
  /// view would extend coverage. The heatmap/pose bins drive the auto-capture and
  /// the live UI so the user can see which regions/angles still need filling.
  class CoverageMap {
    public:
    CoverageMap(const utils::Size &imgSize, int gridW = 16, int gridH = 12);

    /// Describe a detection (image position/scale/tilt + occupancy cells + bins).
    ViewDescriptor describe(const std::vector<markers::CalibrationCorrespondence> &corr) const;

    /// Would this view add coverage? True if it fills any under-filled image cell
    /// OR its pose bin (region × scale × tilt) has not been seen.
    bool isUnderRepresented(const ViewDescriptor &d) const;

    /// Commit a kept view (bumps cell occupancy + records the pose bin).
    void add(const ViewDescriptor &d);

    float coveragePercent() const;   ///< % of image cells with occupancy ≥ 1
    int   binsSeen()   const;         ///< total captured gauge/pose bins
    int   viewsAdded() const { return m_views; }

    /// Pseudo-color occupancy heatmap for display (blue = empty … red = dense).
    core::Img8u heatmap() const;

    // --- orientation gauge (an alternative coverage viz) ------------------------
    // Per coarse image region, a radial "compass": a centre disc (fronto-parallel)
    // + \a gaugeRings() concentric rings, each split into \a gaugeSegs() angular
    // segments. A kept view fills the segment for its tilt magnitude (→ ring; the
    // centre for near-fronto) and tilt direction (→ segment). The app renders it
    // over the frame so the user sees which viewing angles are still missing at
    // each image location. Bin code: 0 = centre; ring r∈[1..rings], seg s∈[0..segs)
    // → 1 + (r-1)*segs + s.
    int gaugeCols()  const { return m_ggw; }
    int gaugeRows()  const { return m_ggh; }
    int gaugeRings() const { return m_grings; }
    int gaugeSegs()  const { return m_gsegs; }
    static int gaugeCode(int ring, int seg, int segs) { return ring == 0 ? 0 : 1 + (ring-1)*segs + seg; }
    /// captured orientation bins for gauge cell (gx,gy).
    const std::set<int> &gaugeBins(int gx, int gy) const { return m_orient[gy*m_ggw + gx]; }

    /// Where a view lands on the gauge grid: cell (gx,gy) + tilt ring (0 = centre)
    /// + segment. add() records this; the UI highlights it as the live "you are
    /// here" cursor. `valid` is false for an undescribable detection.
    struct GaugeHit { int gx = -1, gy = -1, ring = 0, seg = 0; bool valid = false; };
    GaugeHit gaugeLocate(const ViewDescriptor &d) const;

    /// Continuous radius fraction [0,1] of the gauge outer radius for a tilt
    /// magnitude, aligned with the ring boundaries — for the live needle length.
    float gaugeRadiusFrac(float tiltMag) const;

    const utils::Size &imageSize() const { return m_img; }

    private:
    int cellIndex(const utils::Point32f &p) const;

    utils::Size      m_img;
    int              m_gw, m_gh;
    std::vector<int> m_occ;          ///< per-cell occupancy count (m_gw*m_gh)
    int              m_views = 0;
    int              m_cellThresh = 1;   ///< a cell counts as covered at ≥ this

    // orientation gauge state
    int   m_ggw = 4, m_ggh = 3;      ///< coarse gauge grid (legible glyphs)
    int   m_grings = 2, m_gsegs = 8; ///< tilt-magnitude rings + angular segments
    // tiltMag ≈ 1-cos(θ): 0.03≈14°, 0.13≈30°. centre = ~fronto, ring1 = a bit
    // tilted, ring2 = tilted more (the user's three zones).
    float m_gt0 = 0.03f, m_gt1 = 0.13f;   ///< tiltMag thresholds: centre|ring1|ring2
    std::vector<std::set<int>> m_orient;  ///< captured orientation bins per gauge cell
  };

  /// Decides when to auto-capture: fires once per dwell when the board is held
  /// STILL (stability gate) over a view that would extend coverage, then disarms
  /// until the board moves away (debounce). Headless; the caller does the actual
  /// addView()/CoverageMap::add() on a Capture decision.
  class AutoCaptureController {
    public:
    enum class Decision { Skip, Capture };

    explicit AutoCaptureController(CoverageMap &coverage);

    /// Feed the latest detection every frame. Returns Capture when the board is
    /// stable AND under-represented AND armed; the returned descriptor is the
    /// caller's to add() on Capture.
    Decision update(const std::vector<markers::CalibrationCorrespondence> &corr,
                    ViewDescriptor &outDesc);

    bool  stable() const { return m_stableFrames >= m_stableNeeded; }
    int   stableFrames() const { return m_stableFrames; }

    // tunables (exposed so the GUI can offer a sensitivity slider)
    void setStabilityFrames(int n) { m_stableNeeded = n; }
    void setMoveTolerancePx(float px) { m_moveTol = px; }

    private:
    CoverageMap    &m_cov;
    bool            m_have = false;   ///< have a previous frame to compare
    utils::Point32f m_lastCentroid;
    float           m_lastScale = 0;
    int             m_stableFrames = 0;
    int             m_stableNeeded = 4;
    float           m_moveTol = 3.f;      ///< max centroid move [px] to count as "still"
  };

} // namespace icl::calibintr
