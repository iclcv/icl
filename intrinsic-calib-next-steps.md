# `icl-cam-calib-intrinsic` — Continuation / Next Steps

**Status: PAUSED (2026-07-05).** The app works end-to-end in simulation and the detection/
calibration core is solid, but making it *robust on hard real-world poses* (steep, partial,
board overrunning the frame) turned out harder than expected — this doc captures exactly where
we are so it can be resumed cleanly.

Related docs/memories: [`intrinsic-calib-app-plan.md`](intrinsic-calib-app-plan.md) (original
plan), [`camera-calibration-redesign.md`](camera-calibration-redesign.md) (this is Phase B),
memories `project_intrinsic_calib_app`, `project_coded_checkerboard`, `project_coded_checkerboard2`.

## What the app is
`icl-cam-calib-intrinsic`: pick a target type+size → "wave" it in front of the camera →
**auto-capture** views where coverage is poor (live image-space corner heatmap + per-orientation
gauges) → calibrate → error report. Sim-first (a rendered board through a known GT lens); real
`-i` input still TODO. Sibling app later: `icl-cam-calib-extrinsic` (Phase C).

## What is LANDED and works
- **Headless core** `icl/markers/apps/icl-cam-calib-intrinsic-core.{h,cpp}`: `TargetSpec` +
  `makeTarget`/`makeSceneNode`, `IntrinsicSession` (masked `cv::IntrinsicCalibrator`, per-view
  RMS), `CoverageMap` (occupancy heatmap + pose bins region×scale×tilt), `AutoCaptureController`.
- **`--sim-selftest`** headless path (all PASS): `--synthetic` (analytic solver → exact
  fx/fy/cx/cy/k1/k2), render (tight focal/principal + k2 through detection), `--auto` (coverage
  climbs, selective capture).
- **Targets**: checkerboard, coded-checkerboard (white/black cells), **coded2 (dual-polarity +
  edge-ring stubs — new this session)**, marker-grid. See `project_coded_checkerboard2`.
- **Detection robustness (this session)**: border-margin marker rejection + a distortion-/
  perspective-immune spatial corner filter (second-difference of the robust-homography RESIDUAL
  field for isolated mislabels + a gross-magnitude test for clustered ones). Steep-pose mislabels
  ~2% → <1%, peripheral ring preserved.
- **Orientation cursor**: robust RANSAC-homography-decomposition board normal (was a jittery
  depth-gradient correlation).
- **UI**: gauge-view background dimmed toward mid-grey (display-only) so overlays read.
- Fixed three latent framework bugs (DynMatrix (rows,cols) migration sites) found via this app.

## Why it's hard (the crux)
Getting *accurate calibration on hard poses* is the real difficulty, not the plumbing:
- **k2 (r⁴) is only observable when corners reach the image periphery** → the board must overrun
  the frame → every useful view is PARTIAL and steep → detection must survive foreshortening,
  clipping, and (with coded2) dense in-marker saddle clutter.
- **False-positive corners** at steep/close poses fed calibration; mostly fixed, but a residual
  ~0.8% of CLUSTERED sub-cell mislabels remain (a whole 2×2 patch shifted by one bad marker —
  locally consistent, so only a global homography sees them, which conflates with distortion at
  the very periphery). Proven harmless to k1/k2 by the end-to-end test, but not zero.
- **Coverage/pose descriptor** must be stable enough to drive auto-capture without a real
  intrinsic estimate (uses a guessed K).

## Remaining work — prioritized

1. **🔶 Auto-capture QUALITY GATE (highest value, do first).** `AutoCaptureController::update`
   currently captures on `valid(≥4 corners)` + stability + under-represented pose bin only — no
   floor on corner count or quality. A marginal/heavily-partial frame (or one with residual
   false-positive corners) is accepted like a rich one. Add, before accepting a view:
   - **min corner count** — absolute (~16–20) or a fraction of the visible lattice; surface as a
     tunable like the existing stability slider.
   - **homography inlier-RMS / inlier-ratio gate** — fit the robust board→image homography over
     the view's corners; reject high inlier-RMS or low inlier-fraction. Reuses the machinery
     already in `CodedCheckerboardTarget2::detect` / `CoverageMap::describe`.
   Purely additive to `update()` (+ a couple of tunables). **Also verify** whether, in a steep
   partial view, the top row of detected corners sitting in the dark region are genuine edge-ring
   points (good for k2) or off-board false positives — dump `detect()` output for such a pose.

2. **Region-level clustered-mislabel rejection (optional, for the last ~0.8%).** A patch-based
   check (detect a locally-consistent group of corners collectively offset from the surrounding
   lattice) would remove clustered mislabels the current per-corner filter can't. Only worth it
   if the quality gate + downstream calibration robustness prove insufficient.

3. **Real `ImageSource` input (`-i`).** Currently sim-only. Plan step 6. Swap the rendered frame
   source for a live `ImageSource`; keep the sim path behind a flag.

4. **Promote `CoverageMap` / `IntrinsicSession` → installed `markers` classes + gtest.** They live
   in the app's `apps/` today. Move to the module, fold the `--sim-selftest` asserts into a gtest.

5. **GUI on a real display + tune.** Cannot run interactively in-sandbox (Cocoa `QOpenGLWidget`
   crashes); verified compile + init-to-GL-context only. Needs a real display to exercise & tune
   the stability/coverage/auto-capture feel.

6. **Cursor sign convention.** The robust orientation cursor is now *stable*; confirm its azimuth
   points the intuitive way (near vs far side) on a real display; flip `n[0],n[1]` in
   `CoverageMap::describe` if not.

7. **Nice-to-haves**: undistortion preview, per-bin coverage gauges, save-path via `-o`.

## Key files
- App: `icl/markers/apps/icl-cam-calib-intrinsic{,-core}.{h,cpp}`
- Targets: `icl/markers/CodedCheckerboardTarget2.{h,cpp}`, `CodedCheckerboardTarget.*`,
  `CheckerboardTarget.*`, `MarkerGridTarget.*`, `CalibrationTarget.h`
- Corner infra: `icl/cv/CheckerboardSaddleDetector.*`, `icl/cv/CheckerboardGrid.*`,
  `icl/math/transform/Homography2D.*`
- Tests: `tests/test-markers-coded-checkerboard2.cpp`, `tests/test-markers-intrinsic-endtoend.cpp`

## Verify / run
```bash
ninja -C builddir -j 16
builddir/bin/icl-tests -j 1 -f 'markers.*'                       # unit + end-to-end
QT_QPA_PLATFORM=cocoa builddir/bin/icl-cam-calib-intrinsic --sim-selftest -t coded2
QT_QPA_PLATFORM=cocoa builddir/bin/icl-cam-calib-intrinsic --sim-selftest --auto -t coded2
```
