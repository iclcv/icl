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
  - [x] `markers::CalibrationTarget` pluggable backend interface scaffolded
        (detect→correspondences, generate→printable). Checkerboard / marker-grid / ChArUco
        backends to come.
  - [x] **native ChESS-style checkerboard saddle detector** (`cv::CheckerboardSaddleDetector`,
        OpenCV-free): ring 2nd-harmonic response (corner = 4 quadrants = strong 2-cycle signal;
        edge = 1-cycle) + NMS + parabolic sub-pixel; 2nd-harmonic phase gives per-corner
        orientation. LOCAL operator → distortion-robust by construction. Tests (clean +
        barrel-distorted): clean 64/64 @ 0px, 64 seeds; distorted 64/64 @ 0.49px. Emits
        `cv::CornerSeed{pos,score,orientation}`.
  - [x] **grid recovery v1** (distortion-tolerant, growth-based) — `cv::CheckerboardGrid` +
        `recoverCheckerboardGrid(seeds)`: turns the unordered ChESS seeds into an ordered integer
        `(col,row)` lattice. Seed-only: dedup near-duplicates → bootstrap local axes from the start
        seed's NEAREST NEIGHBOUR (not its orientation — the 2nd-harmonic phase points along the
        board *diagonals*, which would grow only the same-colour half-lattice) → BFS the lattice,
        refining the per-link step vectors so it tracks perspective + distortion → keep-best on a
        cell collision. Origin/axis order NOT canonicalised (fine for per-view calibration). This
        is the swappable layer behind a STABLE `CheckerboardGrid` boundary — the decided hybrid
        (below) can replace it without touching consumers. Tests
        `cv.checkergrid.{clean,square,distorted,perspective}` (full lattice incl. the square
        diagonal-trap, and `tilted_nonorthogonal` — a camera tilt makes the board axes
        non-orthogonal in the image; recovery bootstraps the 2nd axis from the nearest
        NON-COLLINEAR neighbour, not a 90° rotation, and grows by a fixed-point that re-derives
        each cell's local steps from its own neighbours). Visualised live in
        `icl-checkerboard-detection-lab`.
  - [x] **edge validation pass** (`scoreCheckerboardGridEdges`) — scores each lattice edge by
        the mean image gradient PERPENDICULAR to it (on a lightly-blurred gray): a real edge lies
        on a black/white square border (high), a wrong/diagonal edge crosses a uniform square
        (low). Fills `CheckerboardGrid.edgeRight/edgeDown`. Lab colours edges red→green by this
        confidence. Test `cv.checkergrid.edge_scoring` (real edges ~1.0; a corrupted through-square
        edge drops clearly). **NEXT: feed this confidence back into growth (guided growth)** —
        prefer/required high-evidence links to disambiguate hard cases (glare, partial board,
        strong distortion). This is the lightweight step toward the decided hybrid.
  - [x] **`CheckerboardTarget` backend** — the FIRST concrete `CalibrationTarget`
        (`markers::CheckerboardTarget`): detector + grid recovery → labels the lattice against the
        known board geometry (either axis order) → object↔image correspondences for
        `Camera::calibrate_*`; also `modelPoints()` + a detectable `generate()`. Test
        `markers.checkertarget.generate_detect_roundtrip` (complete, correctly-labelled). 981/981.
  - **Detector architecture (decided):** a HYBRID — region-quads (LocalThreshold→RegionDetector→
    QuadDetector, reusing the fiducial pipeline) own *structure / topology / `(row,col)` ordering /
    origin-disambiguation* and give approximate seeds; **ChESS owns precision** (sub-pixel refine
    at each junction); ChESS peaks also *split* the 8-connected quad merges at corners — the two
    methods fix each other's blind spots. Generalised later into a **seed-fusion/refinement
    framework** (pluggable `CornerSeed` providers + refiners) once the 2nd provider exists.
    **Distortion is a first-class test axis** (boards must survive bent edges).
  - ICL-native intrinsics (marker-grid optimizer) vs OpenCV comparison.
  - [x] **`calibrate_extrinsic` divergence resolved** — robust linear seed landed (see above).
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
- **FIX LANDED (framework):** `Camera::calibrate_extrinsic`'s linear seed is now robust —
  scale from average column norm, sign by cheirality (object in front), SVD orthonormalisation
  to the closest proper rotation (det=+1). The camera construction (pos=−RᵀT, axes from Rᵀ) is
  unchanged. The function now works directly via the public API: on the far/isometric object
  that used to diverge it gives **5.8mm** depth error vs the joint DLT's **245mm** (~42×), and
  its linear seed alone (LMA off) recovers the camera from perfect data. Full suite 966, no
  regressions in legacy calibration-dependent paths.
- **Implication for Phase B:** the decoupled path (planar intrinsics → fixed-intrinsics
  extrinsics) can now use `calibrate_extrinsic` directly, OR seed it from a homography
  (`CoplanarPointPoseEstimator`) for extra robustness — but the linear seed is no longer the
  blocker.
- **Locked regressions:** `fixed_intrinsics_beats_drift` (5.8mm vs 245mm),
  `extrinsic_linear_seed_is_valid` (LMA-off perfect recovery).

## The historical "marker-edge bias" drift
Corner-detection bias toward/away from quad centres shrinks/grows the apparent object →
focal↔depth drift. Two mitigations the harness will quantify: (a) decoupled intrinsics from a
near, frame-filling view; (b) checkerboard saddle corners (less biased than marker quad edges).
