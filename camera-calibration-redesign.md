# Camera-calibration redesign (geom2) — plan + findings

The last geom→geom2 retirement blocker is the camera-calibration apps. Rather than
transliterate them, we're re-thinking the pipeline (the legacy 3D-object joint DLT was
chronically drift-prone). This is a multi-session arc.

## Grounding facts (verified in-tree)
- **Extrinsic calibration is 100% ICL-native** — no OpenCV. `Camera::calibrate_pinv`
  (joint DLT, intrinsics+extrinsics from one 3D view), `Camera::calibrate_extrinsic`
  (extrinsics with fixed intrinsics), `CoplanarPointPoseEstimator` (planar/homography pose,
  needs known intrinsics).
- **Only the intrinsic/lens step has an OpenCV path** (`icl-lens-undistortion-calibration`).
  ICL ALSO has an OpenCV-free marker-grid intrinsic optimizer
  (`MarkerGridBasedUndistortionOptimizer` + `InverseUndistortionProcessor` + `ImageUndistortion`).
- The legacy 3D app already branches: given intrinsics → `calibrate_extrinsic`, else
  `calibrate_pinv`. ICL's own `Camera.h` docs (~line 247) document the depth drift:
  a far/isometric object makes focal↔distance inseparable → ">10cm z errors" → recommends
  the two-step (intrinsics first, then fixed-intrinsics extrinsics) decoupling.

## Decisions (with the user)
- **2D vs 3D:** keep 3D for extrinsics *for now*; build the harness/comparison first and
  decide later if 3D is worth keeping. Planar is the expected primary path.
- **Build order:** synthetic harness FIRST (Phase A).
- **Intrinsics backend:** keep OpenCV as a comparison baseline; retire it once the
  ICL-native path matches accuracy+speed.
- **Calibration-object abstraction (key new architecture):** a registerable backend —
  `detect(Image) -> [(objectPos2D/3D, imagePos2D)]` correspondences, plus
  `generate(params) -> printable target (PDF/SVG/image)`. Backends: **checkerboard**
  (NEW, ICL-native — user's thesis: checkerboard corners localise more accurately in 2D than
  marker quads, so should be superior; tune/harness with synthetic lighting/reflection/noise)
  and **marker-grid** (existing `AdvancedMarkerGridDetector`). Keep both as modes.

## Phased plan
- **Phase A — synthetic harness** (headless, testable here).
  - [x] A.1 projection-based accuracy harness (`tests/test-geom2-calibration-harness.cpp`):
        project known target → known camera → +noise → recover → measure vs truth.
        Reproduces the drift (near 700mm: 2mm err; far 3000mm: 351±265mm). LANDED.
  - [ ] A.2 rendering-based harness: render the calibration target in a geom2 Scene2
        through known cameras (offscreen `BVHSceneCapture`), with synthetic
        lighting/reflection/noise, → detect → calibrate → measure. Closes the full image loop.
- **Phase B — planar intrinsic+extrinsic** (the primary path):
  - checkerboard `CalibrationTarget` backend (detect + generate) — ICL-native, harness-tuned.
  - ICL-native intrinsics (marker-grid optimizer) vs OpenCV comparison.
  - **resolve the `calibrate_extrinsic` divergence finding** (below) → working decoupled fix
    + passing regression.
- **Phase C — multi-camera one-click extrinsic** (3D object, fixed intrinsics,
  `calibrate_extrinsic`): preserve the 6-cam single-click, drift-free. GridIndicatorObject →
  geom2 nodes.

## Resolved finding — the `calibrate_extrinsic` "divergence" (RCA + fix)
Dug in (probes + `test-geom2-calibration-harness`):
- **Root cause:** `Camera::calibrate_extrinsic`'s internal **linear SVD seed** is non-robust.
  For the 3D two-plane object it returns a **cheirality-flipped, mis-scaled** camera
  (z=−2100 vs the true +700 — wrong sign AND ~3× scale). The LMA then starts in that bad
  basin and can't escape → the ~1.8m "divergence". `calibrate_pinv` is unaffected (it gets +700).
- **The LMA is correct.** `optimize_camera_calibration_lma` IS the fixed-intrinsics /
  extrinsic-only solver (`Q = P·m(p)·T`, P=intrinsics held; only the 6 extrinsic params vary).
  Seeded near truth it gives **1.5mm** depth error vs the joint DLT's **191mm** under the same
  noise, and it has a **WIDE basin**: seed pose off by ±300mm→~5mm, ±600mm→~15mm (at 3m).
- **pinv's pose is too unreliable a seed** in the ill-conditioned far case (its orientation, not
  just depth, is far off → outside the basin → 700mm). So the seed must come from elsewhere.
- **Fix (Phase B):** seed the extrinsic LMA from a reliable pose — a **homography/PnP init**
  (`CoplanarPointPoseEstimator` for planar) — not the broken linear solve. This is exactly the
  planar-first path, and it makes the decoupled calibration robust. Optionally also fix the
  framework `calibrate_extrinsic` linear seed (cheirality + SVD-orthonormal rotation + scale)
  so it stops handing the LMA a poisoned seed — benefits all callers, but touches a shared
  function used by the legacy apps, so do it with the harness as the regression guard.
- **Locked regression:** `decoupled_seeded_beats_drift` (homography-quality ±200mm seed) →
  4.4mm vs joint 249mm (~56×).

## The historical "marker-edge bias" drift
Corner-detection bias toward/away from quad centres shrinks/grows the apparent object →
focal↔depth drift. Two mitigations the harness will quantify: (a) decoupled intrinsics from a
near, frame-filling view; (b) checkerboard saddle corners (less biased than marker quad edges).
