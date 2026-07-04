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
// The sim helpers mirror geom2::OffscreenView's forward lens distortion so a
// rendered board carries a known ground-truth camera the recovered intrinsics can
// be checked against. See intrinsic-calib-app-plan.md.

#include <icl/markers/CalibrationTarget.h>
#include <icl/cv/IntrinsicCalibrator.h>
#include <icl/geom2/Node.h>
#include <icl/core/Img.h>
#include <icl/utils/Size.h>   // Size + Size32f (SizeT<float>)
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace icl::calibintr {

  /// Which kind of calibration target is in use.
  enum class TargetType { Checkerboard, Coded, MarkerGrid };

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
  geom2::NodePtr makeSceneNode(const TargetSpec &s);

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

  /// Apply the SAME forward radial lens distortion geom2::OffscreenView bakes into
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
    int   binsSeen()   const { return (int)m_bins.size(); }
    int   viewsAdded() const { return m_views; }

    /// Pseudo-color occupancy heatmap for display (blue = empty … red = dense).
    core::Img8u heatmap() const;

    const utils::Size &imageSize() const { return m_img; }

    private:
    int cellIndex(const utils::Point32f &p) const;
    int binKey(const ViewDescriptor &d) const;

    utils::Size      m_img;
    int              m_gw, m_gh;
    std::vector<int> m_occ;          ///< per-cell occupancy count (m_gw*m_gh)
    std::set<int>    m_bins;         ///< seen pose-bin keys
    int              m_views = 0;
    int              m_cellThresh = 1;   ///< a cell counts as covered at ≥ this
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
    bool            m_armed = true;   ///< false right after a capture until board moves away
    utils::Point32f m_armCentroid;    ///< centroid at last capture (for re-arm distance)
    int             m_stableNeeded = 4;
    float           m_moveTol = 3.f;      ///< max centroid move [px] to count as "still"
    float           m_rearmDist = 40.f;   ///< centroid must move this far to re-arm
  };

} // namespace icl::calibintr
