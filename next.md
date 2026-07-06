# ICL — Continuation Guide

📋 **Open-todo index: [`backlog.md`](backlog.md)** — terse running list so nothing is forgotten.

## Next Step

### ✅ DONE (S100) — pose enum collapse + cv3d/pose framework steps 2 & 4
Branch `further-restructuring-and-cleanup`; suite **1084→1088**, all green. **Nothing pushed.**
1. **PlanarPoseEstimator enum collapse** (`990f765ef`) — `PoseEstimationAlgorithm` is now just
   `HomographyBasedOnly` vs `Refined` (the former se(3)-NelderMead `SimplexSampling`, now the sole
   refinement). Dropped the bespoke brute-force `optimize_error`/`compute_error_opt` and the five
   sampling properties + dead `#if 0` helpers. Consumers (camera-calibration-planar, CameraCalibrationUtils,
   MarkerGridPoseEstimator, test) retargeted to `Refined`.
2. **Framework step 2** (`2cf43fd34`) — `RigidTransformFitter : ModelFitter<PointPair,Mat>` over
   `RigidTransformEstimator::map` (new `PointPair` struct). Composes with `RobustFitter<PointPair,Mat>`
   for robust point-cloud alignment / ICP correspondence solving. New test-cv3d-rigid-transform.
3. **Framework step 4** (`6188badf7`) — `PlaneFitter : ModelFitter<Vec,PlaneModel>` (PCA least-squares
   plane). Chose the *face* approach: `RansacPlaneFitter`'s RANSAC is fused with the tuned OpenCL
   multi-surface pipeline (ON_ONE_SIDE/MAX_ON, adjacency, relabel) that RobustFitter can't express, so
   left that engine intact and added the generic face for unstructured points. `PlaneModel` shares the
   `(n0,dist)` representation with `RansacPlaneFitter::Result`. New test-cv3d-plane-fitter.
**cv3d/pose framework migration steps 1–4 now ALL done** (1 RobustPoseEstimator→RobustFitter,
2 RigidTransformFitter face, 3 PlanarPoseEstimator se(3)+enum collapse, 4 PlaneFitter face).

### ✅ DONE (S100) — ICP Phase 2 (both halves, GPU-verified) + RobustPoseEstimator sim
Suite **1088→1092**, all green.
1. **ICP color-aware C++ backend** (`61f2a4e2e`) — `ColorNN : ICP::Backend`, exact brute-force NN under
   `dist² = ||Δpos||² + colorWeight²·||Δrgb||²`. Colours out-of-band (setTarget/SourceColors, don't
   transform with pose); protected virtual `distanceSq()` = free-form-metric seam (the C++ edge). Own
   `ICP_ColorNN.cpp` — Vec4 hot path untouched (CE constraint). NB `math::KDTree` NOT used: its
   nearestNeighbour is a no-backtrack descent (approximate), wrong for ICP.
2. **ICP OpenCL backend** (`54b642210`) — `CLNN : ICP::Backend` (`ICP_OpenCL.cpp`, `ICL_HAVE_OPENCL`),
   brute-force NN kernel, one work-item/query. **GPU-verified in-sandbox**: correspondences match
   OctreeNN exactly (delta=0), ICP recovers transform. Fresh kernel (the rep-DB approx-NN seed is a
   further optimisation). Graceful `isValid()` fallback. See [[project_icp_consolidation]].
3. **RobustPoseEstimator sim** (`693431a4e`) — did the "real-display check" as a headless sim.
   New `icl-robust-pose-sim` (no Qt): warps a textured template (mandril) to known poses, runs the
   REAL clsurf-SURF → `RobustPoseEstimator::fit` → corner-reprojection scoring. 12/12 frames @1.7px
   clean, 16/16 @2.0px under 8px noise. Also fixed the mirrored app's stale `"opensurf"` backend
   (retired) → `"best"`. **cv3d/pose framework migration steps 1–4 all done** (see S100 block above).

### ▶ NEXT SESSION — pick one (S99/S100 recap below)
Branch `further-restructuring-and-cleanup`; **nothing pushed** (SSH blocked in-sandbox — CE pushes).
Candidate directions:

1. **ICP color-aware, further** — Vec8 GPU color backend (extend the CLNN kernel with the weighted
   colour term + upload colour buffers), and/or the rep-DB approximate-NN acceleration from the
   preserved `icp/IterativeClosestPoint.*` seed for large clouds. See [[project_icp_consolidation]].
2. **Filament rendering backend** (big, offset) — rework viz3d/render onto Google Filament.
   See [[project_filament_backend]].
3. **Resume paused arcs** — `icl-cam-calib-intrinsic` (auto-capture quality gate; see
   `intrinsic-calib-next-steps.md`) or the camera-calibration redesign / Phase C.
4. **Retire the preserved ICP OpenCL seed** — now that CLNN exists, decide the fate of the
   821-line commented `icp/IterativeClosestPoint.*` (mine rep-DB NN then delete, or delete outright).

### ✅ DONE (S99) — fit-framework adoption COMPLETE
Every `math/fit` tool now sits behind the generic bases, or is a documented internal engine:
- **`Optimizer<V>`** = `NelderMeadOptimizer`, `CMAESOptimizer` (+ generic `minimizeRestarts`);
  `SimplexOptimizer` is the internal engine (two bugs fixed — UB + degenerate simplex).
- **`ModelFitter<Data,Model>`** = `PrimitiveFitter2D` family (Line/Circle/Ellipse), `TaubinCircleFitter`,
  `HalirFlusserEllipseFitter`, `RobustFitter`/`SeededFitter`, and new **`LMFitter`** (LevMar + robust
  `RobustKernel`); `LeastSquareModelFitting(2D)` is the internal algebraic engine.
- **Stragglers finished (`204388c69`):** deleted deprecated `StochasticOptimizer` (superseded by CMAES);
  documented `LeastSquareModelFitting` as the engine + migrated the qt example onto the ModelFitter
  faces; documented `PolynomialRegression` as a standalone tool (robust poly-fit = LMFitter).
The old `RansacFitter` is gone (`RobustFitter` is the sole RANSAC). Full P3–P5 essentially landed.

<details><summary>fit-framework hardening detail (collapsed)</summary>
Audit of `math/fit`: ON the framework = `Optimizer<V>` (NelderMead, CMAES), `ModelFitter`
(PrimitiveFitter2D + Line/Circle/Ellipse, Taubin, HalirFlusser, Robust/Seeded), `RefiningFitter`
(GeometricCircleRefiner). OFF (stragglers): `SimplexOptimizer` (engine behind NelderMead),
`StochasticOptimizer`, `LeastSquareModelFitting(2D)` (overlaps PrimitiveFitter2D!),
`LevenbergMarquardtFitter`, `PolynomialRegression`.

**`SimplexOptimizer` engine — two real bugs FIXED (the "fix divergence" task):**
1. `cc63851ae` — **UB/non-determinism**: `create_zero_vector` only handled `FixedColVector<float,3>`;
   all other fixed sizes fell to `Vector(dim,0.0)` → FixedMatrix 2-arg element ctor → uninitialised
   tail → garbage in the optimizer's internal buffers. Pose6D results swung 1.2px↔86000px across runs
   of the same binary. Added zero-fill specialisations N=2..6 float+double. NOW DETERMINISTIC.
2. `36088e80d` — default simplex was multiplicative (`x[i]*=1.05`), degenerate at zero components;
   now additive (scipy heuristic). NelderMead now works from a zero init.

**Engine-internalisation DONE (`8bb6d9391`):** enriched `Optimizer<V>` with a generic multi-start
helper `minimizeRestarts(f, initGen, nStarts)` (the last engine-only capability a real consumer needed);
migrated `MarkerGridBasedUndistortionOptimizer` off raw `SimplexOptimizer` → `NelderMeadOptimizer` +
`minimizeRestarts`. `SimplexOptimizer` is now documented as the internal engine behind NelderMead. A
literal `detail/` move stays blocked (NelderMead is an installed template that must include the engine),
so it's **internal-by-contract**. Remaining direct engine users are legit: the simplex-2D/3D demos
(visualise the simplex mechanics) and PlanarPoseEstimator (deferred to its redesign).

**LevMar folded (`a9d2c7146`):** new `LMFitter<Scalar> : ModelFitter<(x,y),β>` wraps
`LevenbergMarquardtFitter` + a `RobustKernel` (Huber/Cauchy/Tukey M-estimators, MAD scale) doing IRLS
(weights fold √w into y and f — no LM-core change). Robust recovers a line under ¼ outliers
(plain m=2.019 → Cauchy m=2.00002). LevMar is still SOTA for dense small-medium NLS; the modern win is
this robust-loss option, complementary to the RANSAC `RobustFitter`.

**Stragglers finished (`204388c69`):** StochasticOptimizer deleted (superseded by CMAES);
LeastSquareModelFitting documented as PrimitiveFitter2D's engine + qt example migrated onto the faces;
PolynomialRegression documented (robust poly-fit = LMFitter). See [[project_fit_framework]].
</details>

**✅ PlanarPoseEstimator se(3) refinement DONE (`c6ad14966`).** `SimplexSampling` now refines the
closed-form seed over a LOCAL se(3) tangent δ=(ω,v) — `minimise reproj( ΔT(δ)·T_seed )`,
`ΔT=[Rodrigues(ω)|v]` — through the framework `NelderMeadOptimizer<Pose6D>`. Rodrigues is
singularity-free near 0 so the euler pathology is gone: exact data 1.23px(+divergent) → 9.4e-5px
(deterministic, seed quality); under noise it improves on the seed (0.4713→0.4701px). This is the
"closed-form seed + pluggable Optimizer" shape. **Remaining:** the brute-force `Sampling*` enum modes
still use bespoke `optimize_error`/`compute_error_opt` (could fold into the framework or drop); the
`PoseEstimationAlgorithm` enum could collapse to HomographyBasedOnly vs Refined(Optimizer).
See [[project_pose_estimation_bugs]].

**⏭ TODO (offset, S99) — Filament rendering backend.** Consider reworking the whole viz3d/render
backend onto **Google Filament** as a renderer backend at some point. Big architectural item, not
scoped yet. See [[project_filament_backend]].

### ✅ DONE (S99) — pose-correctness pass: FALSE ALARM + one real milder bug
The "getPose is broken" alarm was a **test bug**, not a code bug. `getPose` (HomographyBasedOnly)
and `getPoses` (IPPE) both recover translated/off-axis planar targets to ~1e-4–2e-5 px. The bad
~40px/EMPTY readings came from the diagnostic's `markerPose` writing translation into ROW 3
(`T(3,0)`) while ICL applies `T*v` with translation in COLUMN 3 (`T(0,3)`) — leaks into homogeneous
w, only bites when tw≠0, so the centred-only test never caught it. Fixed `test-cv3d-planar-pose`'s
markerPose + added `geom.coplanarpose.translated_target_recovers`; `RobustPoseEstimator` validated
(`cv3d.robustpose.*`, ~8e-5px). Suite 1080/1080. See memory `project_pose_estimation_bugs`.

**🟡 Remaining real (milder) TODO — `SimplexSampling` degrades a perfect seed.** It is the DEFAULT
`PlanarPoseEstimator` algorithm; it seeds from the homography pose (~1e-4px) then `SimplexOptimizer`
diverges to 10–34px (euler round-trip is clean ~1e-7 → optimizer / SimplexErrorFunction fault,
likely mixed rad/mm scaling with a coarse 0.5 step). RobustPoseEstimator unaffected (forces
HomographyBasedOnly). Fix/replace the Simplex refinement or change the default.

### ✅ DONE (S99) — cv3d/pose renames + pose-estimator framework migration (steps 1–4, partial)
Estimators renamed (memory: none; see commit `e62ee722e`):
`PoseEstimator`→`RigidTransformEstimator`, `CoplanarPointPoseEstimator`→`PlanarPoseEstimator`,
`PlanarRansacEstimator`→`RansacPlaneFitter` (was misnamed — fits a PLANE, not a pose),
`RansacBasedPoseEstimator`→`RobustPoseEstimator`. Deleted dead `Posit`+`SoftPosit`.

**RANSAC de-duplication DONE:** retired the legacy `std::function`-based `math::fit::RansacFitter`;
`math::RobustFitter` (ModelFitter-based, MSAC/RANSAC/trimmed + LO) is now the sole RANSAC.
- `RobustPoseEstimator` migrated onto RobustFitter (commit `82f52a16a`) — also **fixed a latent
  euler-round-trip bug** in its old scoring that made it find NOTHING even on clean data.
- qt `model-fitting` example + `test-math` line test migrated; `RansacFitter.h` deleted (`97ca02d52`).

**Framework migration steps 1–4 status:** step 1 (RobustPoseEstimator→RobustFitter) ✅;
step 2 (RigidTransformEstimator ModelFitter face) ⏭ not done; step 3 (PlanarPoseEstimator internal
`SimplexOptimizer`→framework `Optimizer<V>`/NelderMead + ModelFitter face) ⏭ deferred — blocked on
the pose-correctness pass above; step 4 (RansacPlaneFitter→ModelFitter/RobustFitter, has GPU path)
⏭ deferred. Old `SimplexOptimizer` still has consumers (PlanarPoseEstimator) so it stays for now.
Suite 1078/1078 throughout.

### ✅ DONE (S99) — cv3d cosmetic renames + ICP consolidation (Phase 1)
Branch `further-restructuring-and-cleanup`. Suite **1078/1078** (was 1074 + 4 new ICP tests).

**Cosmetic renames (finished the module cleanup S98 started):**
- `ICLGeom_API` → `ICLCv3d_API` (20 cv3d files) / `ICLViz3d_API` (viz3d; was self-defined in
  `nodes/Node.h`, now central in `CompatMacros.h`, self-define removed). `core/DataSegmentBase.h`
  was wrongly on the geom macro → `ICLCore_API`.
- `Scene2`/`DemoScene2`/`Scene2MouseHandler` → `Scene`/`DemoScene`/`SceneMouseHandler` (6 files
  `git mv`, ~350 refs, meson+tests). Disambiguated `ccl::Scene` in `CyclesRenderer.cpp` where the
  unqualified name then collided with the new `viz3d::Scene`.
- `icl/cv3d/GeomDefs.h` → `icl/cv3d/Types.h` (pure include rename; `Vec`/`Mat`/`GeomColor` aliases).

**ICP consolidation — ONE great ICP with a backend seam (Phase 1 of 2):**
Three parallel ICP impls existed in `cv3d/icp/`: classic `ICP` (DynMatrix, KDTree — rough: cout spam
in the loop, stubbed `compute()`, zero users), `ICP3D` (octree, robust — the best, only used by its
own test app), and `IterativeClosestPoint<T>` (OpenCL, **821 lines 100% commented out**, zero users).
- New unified `cv3d::ICP` (`icp/ICP.{h,cpp}`) built on ICP3D's algorithm (octree NN, error-delta
  convergence, Gram-Schmidt re-ortho, maxDist outlier rejection), PIMPL'd, params private, auto-fit
  octree AABB (dropped the manual-bounds API).
- **Polymorphic backend seam**: `ICP::Backend` (batch `build(target)` + `nearest(queries,out)`),
  default `OctreeNN` (C++) in `icp/ICP_Cpp.cpp`. `setBackend()` swaps it — GPU backend slots in
  without touching the loop. Real interface, not a std::function/tag registry.
- Deleted `ICP3D.{h,cpp}` + classic `ICP` impl + `apps/icp3d-test.cpp`. New gtest
  `tests/test-cv3d-icp.cpp` (4 tests: recovers known transform to err 4.7e-6 in 4 iters, noop on
  aligned, empty-safe, backend swappable). Added `icl_cv3d_dep` to base test_deps.
- Fixed stale `DoxygenMainPage.h` Geom block (→ Cv3d + Viz3d).

**⏭ ICP Phase 2 (OpenCL backend):** `icp/IterativeClosestPoint.{h,cpp,CLCode.h,CLCode.cl}` are
PRESERVED as the seed (excluded from meson, banner added to the .h). Implement a `CLNN : ICP::Backend`
in `icp/ICP_OpenCL.cpp` (guarded by `ICL_HAVE_OPENCL`) mining those kernels (rep-DB approximate NN),
ported to the current `icl::utils::cl` CLProgram API. Sandbox OpenCL needs the Metal-cache patch
(see `reference_sandbox_opencl`); verify on Linux/Docker too.

**⏭ TODO — Vec8 (pos+color) ICP** (an older OpenCL `IterativeClosestPoint<Vec8>` had this). Color
matters ONLY in the correspondence/NN metric, never in the rigid-body transform. Do it as a
**color-aware backend** (the `ICP::Backend` seam already isolates this) with a **fully-configurable
distance function** (weighted pos+color; hard/impossible to keep free on OpenCL — so the free-form
metric is the C++ backend's selling point). Constraints from CE:
- MUST NOT slow the position-only path → either template everything (`ICP<T>` + template point/metric)
  OR keep two versions (pos-only `ICP` + a separate color-aware class/backend). Do NOT bolt an
  always-present color branch onto the hot Vec4 loop.
- C++ route is cheap: ICL's N-D `math::KDTree` on 6D `[x,y,z, w·r,w·g,w·b]` gives combined NN for
  free; OpenCL route = the old Vec8 pluggable-distance path.
Decide template-vs-two-versions when picked up; fold in with Phase 2.

### ✅ DONE (S98) — geom / geom2 DE-DUPLICATION + functional sub-folders
The big multi-module restructuring is complete. **End state: two clean modules, no `geom`:**
- **`cv3d`** (`icl::cv3d`, **Qt-free** 3D CV, below ICLQt → headless vision) — root = foundation
  (Camera, GeomDefs, PlaneEquation, ViewRay, RGBDMapping, Primitive3D) + subdirs `pose/ icp/ features/
  edge/ segmentation/`.
- **`viz3d`** (`icl::viz3d`, 3D scene/render, above ICLQt) — root = Loader + Primitive3DConverter +
  subdirs `nodes/ render/ scene/ plot/ pointcloud/ detail/`.

Phases 0–7 + sub-folders, **~21 commits** `551476bda`..`35d3bf1dd`, **suite 1074/1074** throughout.
Full record: [`geom-dedup-scoping.md`](geom-dedup-scoping.md); memory `project_geom_cv3d_split`.
Key facts: old geom (136 files) was mostly CV algorithms, not a scene-graph "duplicate"; the old
point-cloud pipeline was ~all dead (`viz3d::PointCloud` already covers it); physics2 was already on the
new scene graph. Both legacy calib apps mechanically ported onto viz3d (scene classes only; the
calibration ALGORITHM rethink stays deferred).

**Immediate cosmetic follow-ups (non-blocking, quick):**
- `ICLGeom_API` → `ICLCv3d_API` / `ICLViz3d_API` (kept as a globally-defined empty alias for now; a
  mechanical rename + self-define guards).
- `Scene2` / `DemoScene2` still carry the `2` (the old `geom::Scene` they disambiguated is gone) →
  could drop to `Scene` / `DemoScene`.

**Candidate directions for next session (pick one):**
1. The two cosmetic renames above (fast, finishes the module cleanup).
2. Resume the **PAUSED intrinsic-calib** work (below) — auto-capture quality gate is the open thread.
3. The **camera-calibration redesign / Phase C** (deferred algorithm arc; the ported apps are on viz3d
   now) — see `camera-calibration-redesign.md`, [[project_intrinsic_calib_app]].
4. More general re-sorting cleanup if any remains (the `further-restructuring-and-cleanup` branch theme).

### PAUSED — `icl-cam-calib-intrinsic` app (easy intrinsic calibration)
**Full continuation doc: [`intrinsic-calib-next-steps.md`](intrinsic-calib-next-steps.md)** —
current state, why it's harder than expected, and prioritized remaining work (auto-capture quality
gate first). Works end-to-end in sim; robustness on hard partial/steep poses is the crux. Details
of the original plan below and in [`intrinsic-calib-app-plan.md`](intrinsic-calib-app-plan.md).

<details><summary>Original intrinsic-calib plan + landed detail (collapsed — see next-steps doc)</summary>

Easy intrinsic calib:
pick target type+size → wave it → **auto-capture** where coverage is poor with a live **image-space
corner heatmap** → calibrate → error report. Phase B of `camera-calibration-redesign.md`, BEFORE
Phase C. Sibling app later: `icl-cam-calib-extrinsic` (= Phase C).

**LANDED so far (headless core + verified selftest — plan steps 1–4):**
- `icl/markers/apps/icl-cam-calib-intrinsic-core.{h,cpp}` — typed headless core: `TargetSpec` +
  `makeTarget`/`makeSceneNode` (metric-accurate scene geometry), `IntrinsicSession`
  (`addView`/`calibrate` over `cv::IntrinsicCalibrator` with metric worldpoints + partial-board mask +
  `objectPointIndex` labelling + per-view reprojection RMS), `CoverageMap` (image occupancy heatmap +
  pose bins region×scale×tilt from an affine-fit descriptor), `AutoCaptureController` (stability gate
  + under-representation + debounce), sim GT helpers (`groundTruthIntrinsics`, `forwardDistort`
  mirroring OffscreenView's MatlabModel5Params warp).
- `icl-cam-calib-intrinsic.cpp` — `--sim-selftest` headless path (QGuiApplication + `GLSceneCapture`,
  `QT_QPA_PLATFORM=cocoa`). Modes: `--synthetic` (analytic-projection solver check → recovers
  fx/fy/cx/cy/k1/k2 EXACTLY); render (recovers fx/cx/cy tight + RMS 0.46px); `--auto` (drives
  CoverageMap+AutoCaptureController → coverage climbs 36→88%, captures fire selectively, still
  calibrates). All **PASS**. Flags `--sim-input <size=VGA>`, `-t`, `--cells`, `--square-mm`,
  `--sim-k1/k2`, `--auto`, `--heatmap`, `-v`, `--dump`.
- 🔴 **Fixed THREE latent framework bugs found via this app — all missed sites from the DynMatrix
  (rows,cols) migration** (a background audit of ~272 sites confirmed these are the only ones):
  (1) `IntrinsicCalibrator::resetData` allocated `distortion_coeffs` as `create(5,1)` vs the ctor's
  `create(1,5)` → `offset=5+cols()` became 6 not 10 → extrinsic params overwrote distortion → **k1
  never moved** (this cost most of the debug time); (2) `DynMatrixBase::setBounds` internal
  `M(cols,rows)`→`M(rows,cols)` (latent holdContent+non-square transpose); (3) `Homography2D.cpp:151`
  `create(1,8)`→`create(8,1)` — `A.solve(rhs)` threw every iter (swallowed by `catch(...){break}`)
  so **`Homography2D::refined()`'s LM refinement was silently DEAD**, always returning the DLT seed.
  Suite still 1064/1064. See [`project_matrix_rowcol_migration`]. NB `setBounds` params are still
  `(cols,rows)` — the one API whose order is opposite the now-`(rows,cols)` ctor/create.
  (Audit also flagged pre-existing NON-migration defects in dead branches — `IntrinsicCalibrator`
  :542 `for(int i=0; m_data->bSize; ++i)` infinite loop, :785 `hashvec(idx,i)` OOB, :713
  `dvar1dtheta[1]` unset; SoftPosit sinkhorn square-only — left untouched, not this task.)

- **Sim GUI shell landed (plan step 5, compile-verified only).** `guiInit`/`guiRun` in the app:
  `HSplit(Canvas3D scene | Canvas view | control VBox)` — orbit/zoom the target ("waving"),
  target combo + checkerboard cell/square sliders (3 pre-built boards, visibility-toggled),
  auto-capture checkbox, capture-now / reset / calibrate / save buttons, coverage-heatmap toggle,
  `Prop(&OffscreenView)` for backend+lens distortion, live status labels (views / coverage% / bins /
  reproj RMS + sim fx-vs-GT error). Camera FOV = simHFovDeg so render focal == distortion focal
  (self-consistent GT). Runs via `ICLApp(-sim-input(size=VGA) -o(1) ...)`. Can't run interactively
  in-sandbox (Cocoa `QOpenGLWidget` crashes) — verified it compiles + inits up to GL-context;
  **needs a real display to exercise**.

**TODO (next):** exercise the GUI on a real display + tune; then real `ImageSource` input (`-i`,
step 6, currently sim-only); then promote `CoverageMap`/`IntrinsicSession` → `markers` as installed
classes with a gtest (fold in the `--sim-selftest` asserts). Nice-to-haves: undistortion preview,
per-bin coverage gauges, save-path via `-o`. Distortion-through-render needs the CODED partial-board
target (full checkerboard can't reach frame corners → k2 unobservable; already proven by
`markers.intrinsic.endtoend_coded_partial_k2`).
- **UI polish DONE:** the black/white checkerboard background fought the coverage-gauge/heatmap
  overlay — now `gaugeOverlay` builds an independent RGB display copy and pulls it strongly toward
  mid-grey (`mid + 0.28*(p-mid)`, DISPLAY-ONLY) before compositing the gauges, so the magenta ink
  reads clearly on the low-contrast frame. Detection still runs on the untouched frame; no
  calibration impact. Verified via `--sim-selftest --auto --gauge-dump`.

- **LANDED — `CodedCheckerboardTarget2` (dual-polarity + edge-ring stubs).** New target
  `icl/markers/CodedCheckerboardTarget2.{h,cpp}`, old targets untouched. See
  [`project_coded_checkerboard2`]. Markers in EVERY cell (normal in white cells, INVERTED
  white-on-black in black cells; `detect()` runs the fiducial detector on the frame AND its
  inverse and merges) → ~2× identity anchors + exposure robustness. Edge-ring black "stubs" in
  the quiet zone make the board-edge grid intersections saddles → extended `(cols+1)×(rows+1)`
  lattice (peripheral corners for k1/k2). Wired into the app as `TargetType::Coded2` (combo
  "coded2 (dual-pol)", `-t coded2`). Tests: `markers.codedcheckerboard2.*` (80/80 lattice, 32
  edge-ring), `markers.intrinsic.endtoend_coded2_partial_k2` (recovers k1/k2), suite 1070/1070.
  Commits `1b713b91d`, `37d767a9a`.

- **LANDED — false-positive corner rejection + robust orientation cursor.** Commits `1594e6c24`,
  `b2d7a482d`. (1) coded2 `detect()`: drop markers whose corners touch the frame edge
  (`BORDER_MARGIN_PX`, clipped); then a two-criterion filter on the RESIDUAL of a robust
  board→image homography — perspective is exactly a homography so its residual is smooth
  lens-distortion + isolated mislabel spikes; a centered second-difference of the RESIDUAL field
  (median of 4 opposite-neighbour midpoints) catches ISOLATED sub-cell mislabels distortion-
  AND perspective-immune (tight 0.08·cell), plus a gross-magnitude test (0.35·cell) catches
  CLUSTERED mislabels (a shifted 2×2 patch the neighbour test can't see). Steep-pose mislabels
  ~2%→<1% with the peripheral ring fully preserved (endtoend edge 496→495, k2 0.0395). Guard
  test `markers.intrinsic.coded2_steep_pose_no_mislabels`. (2) `CoverageMap::describe`: replaced
  the jittery depth-gradient-correlation `tiltDir` with a RANSAC board→image homography
  decomposed (guessed pinhole K) into the board normal → steady orientation cursor;
  `tiltMag = 1-|nz|`. **Note the residual ~0.8% mislabels are clustered sub-cell — proven
  harmless to k1/k2 by endtoend; a fully-clean fix would need region-level patch detection.**

- **🔶 OPEN (continue here) — auto-capture has NO quality gate.** `AutoCaptureController::update`
  captures on `valid(≥4 corners)` + stability + under-represented pose bin only — no minimum
  corner count, no coverage-fraction, no corner-quality/outlier check. A marginal/heavily-partial
  frame (or one carrying the residual <1% false-positive corners) is accepted like a rich one.
  **Proposed (not yet done):** before accepting a view, require (a) a **min corner count**
  (absolute ~16–20 or a fraction of the visible lattice, surfaced as a tunable like the stability
  slider) and (b) a **homography inlier-RMS / inlier-ratio** gate (fit the robust board→image
  homography over the view's corners; reject high inlier-RMS or low inlier-fraction). Purely
  additive to `update()`. **Also verify:** in a steep partial view the top row of green corners
  sitting in the dark region — are they genuine edge-ring points (good for k2) or off-board false
  positives? Dump `detect()` output for such a pose to confirm.

</details>

### THEN — Phase C: multi-cam one-click extrinsics (old geom already deleted, S98)
Build the **extrinsic-calibration app / Phase C**: multi-camera one-click extrinsics in 3D with
FIXED intrinsics (intrinsics path now fully done — native + coded/ChArUco, see S94 below). Reuse
`getPoses` (closed-form IPPE, S90) + the native checkerboard / marker-grid / coded-checkerboard
detectors. Once Phase C lands, the old monolithic `geom` module can be retired in favour of `geom2`.
Backlog has the calibration-redesign tree.

Optional follow-ups this session opened up (all deferred, none blocking Phase C):
- **`Homography2D::robust()`** now exists (RANSAC) — reuse it anywhere a homography is fit on
  possibly-outlier correspondences (Phase C's per-view/marker fits are candidates).
- **Calibration-target study** (`studies/coded-target-study/`) has deferred tiers: image-size sweep
  (1280×960), marker-fill sweep, Cycles realism re-render, quad-assisted plain-checkerboard recon.
- Default `pp.filter=dilatation` for `MarkerCells::Black` coded targets (black-cell board IS
  detectable with dilatation — see S96); and the `project_memorypool` channel-based QuickContext
  buffer-reuse fix (latent aliasing, `isExclusivelyOwned` only checks the ImgBase handle).

### Session 97 — math/fit review → full model-fitting framework + geev + robustness
On `further-restructuring-and-cleanup`, suite **1044→1063**. A deep dive into `icl/math/fit`
(triggered by a review request); Phase C untouched and remains NEXT. Memory:
[`project_fit_framework`], [`project_eigen_ordering`]. Big session, ~16 commits.

- **🔴 Fixed a live, silent correctness regression: `DynMatrix::eigen()` ordering was
  backend-dependent.** The Jacobi→LAPACK migration flipped eigenvalue order (LAPACK `syev`
  ascending vs the C++ Jacobi descending) and the wrapper didn't reconcile — on Accelerate/Eigen
  this made `LeastSquareModelFitting` return garbage (circle centre (10,0)→(-1.3,0.5), proven) and
  broke PoseEstimator/PCA-normal callers. Fix: `eigen()` now sorts **descending** unconditionally +
  documented contract + regressions. There were ZERO eigen tests before. See [`project_eigen_ordering`].
- **Fit-tool bugs fixed:** SimplexOptimizer multi-restart returned the wrong result; PolynomialRegression
  mixed-term `float` accumulation for `T=double`; toString sign/indexing.
- **Model-fitting FRAMEWORK (P0–P5, `math-fit-framework-plan.md`):** virtual `ModelFitter<Data,Model>`
  + `Optimizer<V>` interfaces (Configurable); `RobustFitter` (RANSAC/MSAC + LO **+ trimmed/LTS
  threshold-free** decorator); `SeededFitter` chaining; `TaubinCircleFitter`, `GeometricCircleRefiner`,
  Halíř–Flusser ellipse; `CMAESOptimizer` (StochasticOptimizer `[[deprecated]]`); shared
  `FitUtils`/`VectorTraits`. `icl-model-fitting-playground` app. LeastSquareModelFitting modernised to
  SVD null-space.
- **`geev` general (non-symmetric) eigensolver** added across the whole LapackOps backend split
  (Accelerate `dgeev`, Eigen `EigenSolver`, C++ Faddeev–LeVerrier fallback) → `DynMatrix::eigenGeneral()`;
  unblocked the ellipse fitter. Also `DynMatrix` returning `eigen()/svd()` overloads + `eigenVector()`.
- **`PlotWidget` gained a `lock aspect ratio` property** (isotropic scaling — circles render circular).
- **`superquadric-fitting-demo`** (`geom2/demos/`): CMA-ES fits a superquadric's size+squareness to
  noisy surface points (Solina inside-outside error), 3D `Plot3D` scatter+fitted `surf`. Fit runs
  DETACHED on the worker `run()` (slider callbacks just snapshot inputs + flag an atomic dirty) →
  responsive sliders. Verified on a display: near-perfect recovery (size 2.98/1.99/1.40, e 0.70/0.79
  vs truth 3/2/1.4, 0.70/0.80). `icl-model-fitting-playground` stays 2D-only in `math/apps` (fusing
  2D+3D into one tabbed app got clumsy — split back). Headless regression `cmaes_superquadric_shape`.
  NOTE: the SQ fit is outlier-sensitive (exponents collapse under many outliers) — a robust/trimmed
  SQ cost is a future nicety.
- **`geom2::PlotWidget3D` fixes (verified on display):** axis labels were detaching from their ticks
  (`makeAxis` placed inverted-axis labels at `-r`); and `Plot3D::setViewPort` must be called on the
  GUI thread at init (calling it from a worker after the first paint left Y/Z showing `inf`/`nan`).

### Session 96 — morphology flicker fix, calibration-target study, Homography2D toolkit
On `further-restructuring-and-cleanup`, suite **1044/1044**. Big session, ~14 commits. Three arcs:
apps/robustness cleanup, a calibration-target detectability study, and a `Homography2D` redesign.

- **n×n BCH apps generalized (the old NEXT — DONE).** `icl-marker-detection` + `icl-create-marker`
  accept `-m bchNxN`; square types default their id set to the code's `orientationSafeIds()` (not the
  6×6 4096 space); region-corner overlay for any bch\*. `icl-create-marker` applies border-width to
  any bch\* type. End-to-end verified (render→detect recovers ids for 3×3/4×4/5×5/6×6).
- **`icl-marker-detection` null-image crash fixed** — `-i ws` hands back nothing before the first
  frame / during reconnect; `detect()` threw uncaught → abort. Guard skips the iteration.
- **`WSSink` last-frame replay** — retains the last broadcast frame and replays it to newly-connected
  clients, so a late-joining consumer isn't stuck on null until the next `send()`.
- **`LabelHandle` accepts wide/unsigned integers** (`long/unsigned/size_t/…`) via a constrained
  `operator=` + `AssignRegistry` enrollment — `gui["label"] = container.size()` used to abort with
  `UnassignableTypesException`.
- **`calib-target-detection-lab`** gains `-o` (ImageSink) and streams the right-pane image on every
  loop iteration (not only on change).
- **MorphologicalOp opening/closing FLICKER + SIGSEGV fixed** (both C++ and Accelerate/vImage
  backends). The composite ops routed BOTH passes through the op's REUSED member buffer, which carried
  ROI/offset state across calls → the 2nd apply sampled a misaligned/stale region → non-binary,
  per-call-drifting output (the on-screen "flicker", diagnosed from captured frames). Fix: fresh local
  intermediate per call; pass 1 always allocates (checkOnly propagation was the historic SIGSEGV);
  border replicated between passes. Re-enabled the disabled Quick2 opening/closing tests + added
  binary-in⇒binary-out & repeat-stability regressions.
- **Black-cell coded checkerboard IS detectable** — but with `pp.filter=dilatation` (grow white →
  break the diagonally-touching black cells), NOT closing/opening (size-preserving, can't separate) or
  erosion (merges). 17/17 markers, 42–44/48 corners under tilt+blur+noise. Updates the S95
  "undetectable by design" note.
- **Calibration-target study** (`studies/coded-target-study/`, standalone, `run.sh`): analytic
  projective renderer with EXACT ground-truth corners (verified 0.056px) driving the real
  detect→calibrate pipeline. Tier A (detectability/corner RMS vs marker size/distortion) + Tier B
  (end-to-end intrinsics, multi-seed, outlier reject). **Key finding:** saddle noise averages out in
  the bundle; **gross MISLABELS** are what poison calibration — and white-cell's smaller (0.62×cell)
  markers hit the mislabel regime before black-cell's full-cell markers. A robust-homography outlier
  reject removes them (coded-white 3.87%→0.11% f-error). See `findings.md`.
- **`Homography2D` redesign.** Factory API `fit(src,dst,n)` (normalized DLT, intuitive src→dst
  direction), `refined()` (DLT seed + Levenberg-Marquardt on geometric error), `robust()` (RANSAC →
  `HomographyFit{H,inliers,rms,ok}`). Constructor `[[deprecated]]` (delegates; it maps pBs→pAs, which
  is why fit flips the arg order); all in-tree callers migrated. **dlt_fit now solves the HOMOGENEOUS
  null space** via the 9×9 `AᵀA` + smallest-eigenvalue eigenvector (NOT a full 2n×2n SVD) — general
  (correct as H(2,2)→0) AND ~6× faster than the old inhomogeneous solve (fit n=1000: 90µs→15µs). Fixed
  the backwards header doc. Tests (`test-math.cpp`) + benchmarks (`bench-homography.cpp`; also
  un-rotted `bench-cv.cpp`). Migrated `CodedCheckerboardTarget`'s board-pose outlier reject + the
  study's `rejectOutliers()` to `robust()`.

### Session 95 — BCH codes reimplemented + generalized to n×n; coded checkerboard on SquareBCHCode
On `further-restructuring-and-cleanup`, suite **1034/1034**. Turned the copied-as-is 36-bit BCH marker
code into a clean, parameterized family and rebuilt the coded checkerboard on top of it. 8 commits.
Memory: none yet — key facts below.

- **`markers::BCHCoder` reimplemented** (`BCHCode.cpp` 608→384 lines). Retired the 4096-entry string
  table + the undocumented magic XOR `0x8f80b8750`: proved `table[id] == reverse36(systematic_encode(id)
  ^ WHITENING)`, so a real systematic LFSR encoder + a named whitening mask replace both. The whitening
  is a distance-preserving anti-degeneracy mask (keeps id 0 off the all-black marker; ARToolKitPlus-
  compatible), not part of the coding math. Berlekamp/Chien split into named phases. Characterization
  tests locked the contract first (round-trip all 4096, marker geometry, rotation, 1–4-bit correction,
  **true min distance = 9**, and low ids have larger distance: 11 for ids 0–15, 10 for 0–63).
- **`markers::SquareBCHCode`** — n×n generalization (3×3/4×4/5×5/6×6). `BCHEngine(length,t)` derives the
  GF(2^m), builds the generator, computes k. Presets (`SquareBCHPreset` + `presetTable()`): the fully
  rotation-safe ones carry an `_RS` suffix — **BCH_4x4_t2_RS** (64 ids, d5), **BCH_5x5_t4_RS** (32, d9),
  **BCH_6x6_t4_RS** (4096, d9). Rotation metrics: `rotationSafeIdCount()`, `rotatedMinDistance()`,
  `selectRotationRobustIds()`, and **`isOrientationSafe()`/`orientationSafeIds()`** (decode2D recovers
  id AND rotation — stronger than id-only rotation-safety; needed when a consumer trusts per-marker pose).
- **`FiducialDetectorPluginSquareBCH`** — n×n analogue of the BCH plugin (reuses the quad pipeline).
  Wired into the `FiducialDetector` factory as `bch3x3`(3,1) / `bch4x4`(4,2) / `bch5x5`(5,4) /
  `bch6x6`(6,4). End-to-end detection test renders `SquareBCHCode` markers and recovers ids.
- **`CodedCheckerboardTarget` now runs on `SquareBCHCode` presets** (default BCH_4x4_t2_RS; also 5×5/6×6
  for big boards — the k2 board needs ~196 markers → 6×6). Three problems solved: (1) require unique-id
  markers — the board's solid black squares all decode to one fixed id, so false positives are
  *duplicates*; (2) allocate only `orientationSafeIds` to cells; (3) **bootstrap detection**: run the
  decoder at FULL t (noise tolerance), keep unique-id markers, fit a global image→board homography from
  their centres (geometric outlier rejection replaces `max bch errors=0`), recover the discrete
  marker→board rotation R by voting, then label each marker's corners LOCALLY from cell + R (distortion-
  robust at the image corners — a global homography can't model radial distortion). Position from the
  per-marker homography + saddle snap. Noise-tolerance test (±55) added.
- **Lab (`calib-target-detection-lab`)**: coded marker-code combo (4×4/5×5/6×6), a "markers on black
  cells" toggle, and a **live-tunable `Prop` of the coded target's FiducialDetector** (persistent
  `DetectorHost` swaps the detector as a child Configurable on the GUI thread via
  `ICLApplication::executeInGUIThread`; Prop live-refreshes).
- **`MarkerCells::Black` — RENDERS but is UNDETECTABLE by design.** "The black cells ARE the markers"
  (full-cell marker = quad border + inner BCH code, no inversion). Renders fine, but the markers touch
  at every diagonal corner → one connected black lattice the quad detector can't segment (every quad
  decodes to the all-black id). Confirmed via a headless threshold/region sweep + the live lab: no
  detector setting fixes connectivity. This is exactly why ChArUco uses white cells. White-cell markers
  are the usable variant; leave black-cell as a documented experiment.

### Session 94b — coded-checkerboard (ICL's ChArUco) + partial-board k2 intrinsics DONE
On `further-restructuring-and-cleanup`, suite **1014/1014**. Built the "marker+checkerboard stuff for
intrinsics": ICL's ChArUco analogue with BCH markers + a partial-board intrinsic calibrator. 4 commits.
See memory `project_coded_checkerboard.md`.
- **`markers::CodedCheckerboardTarget`** — a checkerboard whose interior white cells carry shrunk BCH
  markers (fill≈0.62, corners survive). detect(): markers → per-marker homography → predict the 4
  surrounding checker corners at `markerPos/fill` → snap to nearest ChESS saddle → absolute (col,row)
  label → partial `CheckerboardGrid` → same sub-pixel polish as CheckerboardTarget. Round-trip 0.16px;
  cropped board still labels corners. Enables the board to OVERRUN the frame (corners reach image edges).
- **`cv::IntrinsicCalibrator::calibrate(impoints, worldpoints, validMask)`** — PARTIAL-board overload
  (variable points per view). Opt-in mask; full-grid path byte-for-byte unchanged. Per-view homography
  init via `GenericHomography2D` on valid points; extrinsic seed by K⁻¹·H decomposition; LMA zeros
  masked residual+Jacobian columns.
- **k2 OBSERVABILITY LESSON (important):** k2 (r⁴) only observable when frame corners are at large
  normalized radius. Narrow FOV (f=600 @640×480, r_max≈0.67) → k2 drifts to garbage even on
  zero-distortion GT. The E2E test uses WIDE FOV (f=640 @1024×768, r_max≈1.0) + board overrunning the
  frame → recovers k1=-0.15, k2=0.05 through real detection. Tests: `cv.intrinsic.partial_board_recovers_k2`
  (pure-math, exact), `markers.intrinsic.endtoend_coded_partial_k2` (full render→detect→calibrate),
  `markers.codedcheckerboard.*` (detection). Wired into `calib-target-detection-lab` (3rd target).

### Session 94 — FixedMatrix `<T,COLS,ROWS>` → `<T,ROWS,COLS>` flip DONE (migration complete)
On `further-restructuring-and-cleanup`, suite **1010/1010**. Finished the matrix (row,col) migration:
FixedMatrix template/ctor dim order is now `(rows,cols)`, matching the standard `(row,col)` accessors
(DynMatrix half was S93). Four commits.

**Design decision first (user prompted): make `FixedColVector`/`FixedRowVector` ALIAS templates, not
subclasses.** They added ZERO members over FixedMatrix — only ctors (the base already has variadic +
initializer_list + range). The subclass identity bought nothing (no overloads distinguished the two)
but created friction (FixedMatrix arithmetic returns FixedMatrix, needing the converting-ctor
round-trip) AND made the flip riskier. As aliases, the ~94 vector sites absorb the flip via the 2
alias lines. Commit `197509e99`.
- Fallout the alias collapse surfaced: (1) `SimplexOptimizer.cpp` instantiated both the vector alias
  and the raw matrix form (now the same type) → dropped the dup. (2) FixedMatrix's templated range
  ctor `(begin,end)` had no call sites but, once vectors were plain FixedMatrix, hijacked two-value
  element calls like `Pos(0,0)`/`Pt(rx,ry)` as exact `(int,int)`/`(URand,URand)` matches → converted
  it to a named factory `FixedMatrix::fromRange(begin,end)`.

**Phase A** (commit `96de14b0a`): spelled ~77 raw `FixedMatrix<T,1,N>`/`<T,N,1>` call sites as
`FixedColVector`/`FixedRowVector` (identity rename while still same-type) so they absorb the flip.

**Phase B — the flip** (commit `52eb09c19`). The elegant mechanism:
- Reorder ONLY the primary class template-param decl `<T,COLS,ROWS>`→`<T,ROWS,COLS>`. This single edit
  flips the EXTERNAL meaning of `FixedMatrix<T,A,B>` (cols,rows→rows,cols); method bodies use COLS/ROWS
  BY NAME so stay correct untouched.
- Swap arg2↔arg3 of EVERY explicit `FixedMatrix<...>` spelling (internal + external) via a
  `perl` regex — behaviour-preserving re-spelling under the reordered convention. Free-function param
  decls need NO reorder (deduction binds the swapped-spelling names correctly). Squares are no-ops.
- Flip the aliases (a column vector is now `FixedMatrix<T,DIM,1>`); swap the genuine non-square
  external sites: Camera 3×4/4×3 blocks, PoseEstimator `NUM_POINTS×3/4` point matrices, and
  `gramSchmidtOrtho` (deduced COLS/ROWS would bind backwards → iterate the wrong axis for non-square).
- Completeness by tree-wide grep of every non-square/symbolic `FixedMatrix<...>` spelling, each
  reviewed (the planned `static_assert(ROWS==COLS)` pass was unnecessary — grep covered it). Left the
  shape-agnostic helpers: `PixelRef::operator=` (linear DIM copy) + `DrawWidget` color/fill
  (`<T,COLS,3/COLS>` orientation-agnostic, accepts both column & row vectors either spelling).
- `.dyn()` bridge verified consistent: member uses correctly-bound ROWS,COLS + `DynMatrix::fromData`
  is `(rows,cols)` since S93.

Backlog ⚠️ URGENT item closed; memory `project_matrix_rowcol_migration.md` marked DONE.

### Session 93 — DynMatrix (row,col) constructor migration (compiler-forced, scripted)
On `further-restructuring-and-cleanup`, suite **1010/1010**. Completed the DynMatrix half of the
matrix-convention migration (FixedMatrix still pending — see ⚠️ URGENT in backlog).

**What landed:** flipped `DynMatrixBase` dim ctors from `(cols,rows)` to `(rows,cols)` (matching the
already-standard `(row,col)` accessors). Migrated all ~186 construction sites via the user's
forcing recipe: made the raw dim-ctor private + added `create()/fromData()` factories → the compiler
pinpointed every site → Python scripts (`flip_dynmatrix.py` token-level, `flip_decls.py`
declaration-level, in the scratchpad) did the bulk arg-swap; hand-fixed the ~5% the scripts can't do
(typedef aliases `Matrix`/`DMat`/`DMatF`/`mat`, multi-declarators, `new`-heap sites, C-array data
ctors, base-initializer lists, a comment the token regex spanned). Finally restored a public
`(rows,cols)` ctor (create/fromData kept as self-documenting aliases).

**The bug the process caught** (why compiler-forcing + full-suite invariance matters): constructing a
`DynMatrix` base from a `fromData(...,false)` **shallow** temporary via the copy ctor silently
deep-copied → broke write-through views → the LMA's `y_est` rows went stale → `calibrate_extrinsic`
regressed 5.8→70px (seed byte-identical, only the LMA output wrong). Fix: added a **move ctor** to
`DynMatrixBase`/`DynMatrix` that transfers the buffer (preserving shallow wraps); restored copy-assign
but deliberately NO move-assign (so `view = rvalue` still deep-copies into the view's memory).

**NEXT:** either (a) the k2/ChArUco hybrid (fix distortion observability — the original S92 plan), or
(b) the FixedMatrix template-param flip (higher risk, no compiler forcing). Then the extrinsic-calib app.

### Session 92 — Phase B kickoff: native intrinsic calibration verified + tilt requirement locked
On `further-restructuring-and-cleanup`, suite **1006/1006**. Moved up a layer from detection to the
actual calibration math. Key finding: **the native `cv::IntrinsicCalibrator` works** (it was
completely unexercised — no caller anywhere — so a real risk it had bitrotted; it hadn't).

**Landed — `test-cv-intrinsic-calibration`** (3 tests, cv-only, no Qt):
- Synthetic Phase-B intrinsic harness: a centred planar point grid projected through a KNOWN GT
  pinhole camera in N poses → feed correspondences to `IntrinsicCalibrator::calibrate` → check
  recovered intrinsics. Isolates the calibration MATH from detection (perfect points in).
- **`tilted_recovers_gt`**: diverse out-of-plane tilted views → recovers fx/fy/cx/cy **exactly**
  (650.000/620.000/330.000/250.000), k1/k2≈0, on clean data.
- **`tilted_noise_robust`**: 0.3px correspondence noise → still within ~4px.
- **`recovers_distortion`**: GT radial+tangential (k1=-0.18, k2=0.05, p1/p2) recovered **exactly** on
  clean data (large frame-spanning board so the distortion radius is actually excited). The forward
  model — `cdist=1+k1r²+k2r⁴+k3r⁶` + Brown tangential + normalized-alpha skew, `kc=[k1,k2,p1,p2,k3]` —
  was reverse-engineered from `project_points2` to match the estimator exactly.
- **`frontoparallel_is_degenerate`**: fronto-parallel-only views (no tilt) can't constrain the focal
  length — fx error diverges (~4e18) vs 0.0 for tilted. Proves tilt out-of-plane is a REQUIREMENT
  for planar (Zhang) calibration, not a nicety (foreshortening is the missing signal).
- **Input contract reverse-engineered** (the calibrator is a 1476-line Bouguet/Matlab transliteration
  with no example usage): `DynMatrix(cols,rows)` ctor + `operator()(row,col)`; `impoints` is
  `(bSize, 2*views)` indexed `(2v[+1], pt)`, `worldpoints` is `(bSize,3)` indexed `(coord, pt)`,
  planar Z=0. Native path uses `math::DynMatrix`+LAPACK, zero OpenCV.

**Native vs OpenCV — LANDED (parity confirmed):** `cv.intrinsic.native_vs_opencv` feeds IDENTICAL
correspondences (shared `makeViews` projection, incl. distortion + 0.2px noise) to BOTH
`cv::IntrinsicCalibrator` and `cv::calibrateCamera` (via `OpenCVCamCalib`). They agree to **~0.02px**
on fx/fy and track each other on every parameter — even where both struggle (k2 under noise, a shared
observability limit, not a calibrator difference). Both recover the well-conditioned GT params
(fx/fy/cx/cy/k1). Redesign's core thesis validated: the native path is on par with OpenCV.
- Added a correspondence-based `OpenCVCamCalib::addPoints(objMM, imgPx)` + `setImageSize` (bypasses
  its internal `findChessboardCorners` so both calibrators see the same points); dropped
  `CALIB_FIX_ASPECT_RATIO` (fx≠fy now recovered faithfully); fixed a latent `getDistortion()` shape
  bug (built `DynMatrix(1,5)` but wrote `at(0,i)` col-indexed → threw "col index too large" — a direct
  casualty of the cols-first ctor vs (row,col) accessor asymmetry; see below).

**End-to-end render→detect→calibrate — LANDED** (`markers.intrinsic.endtoend_checkerboard`). Renders
GT-camera views of a tilted board (homography H=K[r1 r2 t], auto-centred on the optical axis,
inverse-warp + 3× supersample + pixel noise), detects with the real `CheckerboardTarget` (ChESS
saddle + sub-pixel), orders detected corners canonically by objectPos, and calibrates with
`IntrinsicCalibrator`. 10/10 boards detected; recovers fx=600.3/fy=600.2 (GT 600), cx=320.0, cy=239.7
— sub-pixel accuracy THROUGH genuine detection noise (a small spurious k1≈0.05 absorbs a little). No
lens distortion in the render yet (needs an inverse-distortion warp) — deferred.

**Distortion-render gap CLOSED** (`markers.intrinsic.endtoend_distortion`). The render now applies an
inverse-distortion warp (`undistortNorm` fixed-point: pixel→distorted-norm→undistort→pinhole-norm→
board; identity when kc=0 so the clean test is unchanged). GT barrel k1=-0.15/k2=0.03 → detection
survives the curved boards (10/10), recovers fx=599.9/cx=320.5/cy=239.7 and **k1=-0.155 (within
0.005)** with a big enough board (13×9 — corners reach larger image radius). k2 (r⁴) stays weakly
observable (a complete-board detector can't reach the extreme corners where r⁴ dominates) → not
asserted; same limit seen in the perfect-points parity test, not a render/detect bug.

**NEXT:** Phase C multi-cam one-click extrinsics (3D, fixed intrinsics) — then old `geom` can be
deleted. NOTE (matrix convention, ⚠️ URGENT backlog):
`DynMatrix`/`FixedMatrix` accessors are standard `(row,col)` but the CONSTRUCTOR/template dim order is
still column-first — half-done migration; recipe recorded (temp `CONSTRUCTOR(rows,cols)` factory →
private ctor → script-rewrite sites → restore ctor with new order).

### Session 91 — native-checkerboard sub-pixel corner polish (Phase-B gap closed)
On `further-restructuring-and-cleanup`, suite **1003/1003**. Landed the S90 NEXT: a final gradient
sub-pixel corner polish on the native checkerboard path, closing the accuracy gap to OpenCV.

**Landed:**
- **`cv::refineCheckerboardCornersSubPix(grid, image, params)`** (`CheckerboardGrid.{h,cpp}`) — the
  classic OpenCV `cornerSubPix` gradient-orthogonality criterion: a true X-junction sits where the
  image gradient g(p) is ⟂ q−p for every window pixel, so minimise Σ w·(g·(q−p))² → the 2×2 normal
  system `(Σ w gg^T) q = Σ w gg^T p`, solved + iterated. Central-difference gradients bilinear-sampled
  on a full-res (unblurred) gray field; Gaussian window weights; window half-size capped to 0.4× the
  median cell spacing so it can never reach a neighbour corner; divergence guard (reject a step
  leaving the search window). `SubPixelParams{winRadius=5, maxIters=20, eps=0.02}`.
- **Wired default-ON into all three native backends** (Native/Ransac/Graph `CheckerboardDetector`)
  via `m_subpixel`/`setSubPixel`, applied AFTER the optional LAP cleanup. Lab got a `subpixel`
  checkbox (default on).
- **Phase-B re-run** — checkerboard native now **0.002 / 0.014 / 0.012 px** (frontal/keystone/noisy),
  vs OpenCV 0.005 / 0.017 / 0.015 → native now MATCHES/BEATS OpenCV (was ~0.6 / 0.43 / 0.8).
- **Ground-truth metric added to Phase-B** (`checkerTrueCorners`/`checkerGroundTruthRMS`): the
  self-consistency homography residual doesn't measure ABSOLUTE accuracy, so the harness now also maps
  the analytically-known gen-image corners through the exact gen→view homography and measures each
  detected corner's distance to its NEAREST true corner (nearest-match because native doesn't
  canonicalise the (col,row) board frame — a per-label compare is meaningless). Result confirms it
  independently: native **g=0.007/0.015/0.016** vs opencv g=0.009/0.020/0.018. So native is genuinely
  on par / a hair better, not just self-consistent. Asserts native GT sub-0.1px and < 1.5× opencv.
- **Test** `cv.checkergrid.subpixel_refine_improves_corners`: on a keystone-warped synthetic the
  homography residual drops 0.56→0.08px (the test's 4×-supersampled render is the floor here).

**NEXT options** (from S90 follow-ups + backlog): (a) reuse `getPoses` in the marker/Fiducial pose
getters + temporal disambiguation; (b) grid-based multi-marker pose (baseline removes the IPPE
ambiguity); (c) coded corners (BCH/PuzzleBoard) to make grid association ABSOLUTE (backlog:64);
(d) native-graph partial-grid robustness OR retire as experimental.

### Session 90 — marker sub-pixel refiners, closed-form IPPE pose, calib-target lab + demo, Phase-B harness
On `further-restructuring-and-cleanup`, suite **1002/1002**. Long session, 8 commits, continuing the
calibration arc. **NEXT (start here): wire a sub-pixel corner refinement into the NATIVE CHECKERBOARD
path** — the Phase-B harness (below) showed the native ChESS-saddle corners sit at ~0.6px residual
while OpenCV's refined corners hit ~0.01px. The native checkerboard path has no final corner-polish
step; add one (reuse `cv::SubPixelCornerRefiner`, or a saddle-specific parabolic/gradient polish on
the ChESS response) and re-run the Phase-B harness to confirm the gap closes.

**Landed this session (commit order):**
1. **ROCHADE graph checkerboard backend + Delaunay primitive** (`160406868`). New
   `math::delaunayTriangulation`/`delaunayEdges` (Bowyer-Watson) in ICLMath; `cv::Graph
   CheckerboardDetector` ("native-graph") = Delaunay adjacency pruned to grid edges by image
   border evidence → BFS topological (col,row). Wired into the lab. EVALUATION: best on COMPLETE
   lattices (survives 45° oblique shear), but degrades on real PARTIAL frames (hole breaks the
   4-connectivity) — experimental, not default. Phase-B later confirmed it also fails under noise.
2. **Sub-pixel marker-corner refiner (edge)** (`7c07ba879`). `cv::SubPixelCornerRefiner` (border
   edge-line fit + intersection) → 2× on the marker-grid affine residual (0.32→0.16px). Default-on
   in `MarkerGridTarget` via `setSubPixelRefine`.
3. **Generalized `calib-target-detection-lab`** (`276f315a6`). Renamed/moved the checkerboard lab
   into `markers/apps` (spans cv+geom2+markers), driven by a `target` combo over `CalibrationTarget`
   (checkerboard + marker-grid), context-sensitive controls.
4. **BCH-pattern exposure-robust refiner** (`6012cb3b0`). Confirmed (blur+gain+clip experiment) an
   exposure-driven radial corner drift (~2% at heavy overexposure) that the outer-edge refiner can't
   fix. `markers::MarkerPatternRefiner` aligns the KNOWN decoded pattern's INTERIOR edges (both
   polarities → bias cancels) + a few outer edges to pin corners. `RefineMode{None,Edge,Pattern}`.
   Matches edge accuracy (0.19px) at <½ the exposure drift (0.78% vs 1.8%).
5. **Two-solution planar pose `getPoses`** (`36cb48bad`, then refactored) — the IPPE flip ambiguity.
6. **`single-marker-pose` demo** (`def684109`). Visualises BOTH pose frames + live ambiguity ratio;
   marker-type combo rebuilds the detector + its `Prop` panel on the fly (filter-playground pattern);
   raw/edge/pattern corner-refine combo.
7. **Closed-form IPPE refactor** (`b2e93a008`). `getPoses` is now analytic (Collins-Bartoli):
   homography → canonical frame (Rv: centre→optical axis) → complete a 2×2 block's third row/col,
   whose SIGN freedom IS the flip → both rotations; translation per solution by linear LS.
   Deterministic, no iteration, exact (~1e-5px vs the old simplex's ~0.03). Sign completion
   enumerates (a,b) combos, reprojection arbitrates (b=-ga/a unstable for small a).
8. **Phase-B comparison harness** (`efb986e03`). `test-markers-calibration-comparison`: render a
   perspective view (warp generate() + noise) → detect() → completeness + homography residual, for
   BOTH targets × every backend × {frontal,keystone,noisy}. Table + sanity asserts.

**Phase-B first numbers** (640×480; frontal/keystone/noisy):
- checkerboard native-growth/ransac 24/24 r≈0.6/0.43/0.8px; **opencv 24/24 r≈0.005-0.017px**;
  native-graph ok clean, **fails under noise**.
- marker-grid none/edge/pattern 48/48 r≈0.52/0.39/0.56px (pattern's win is exposure, not clean px).

**Follow-ups noted:** (a) NEXT = native-checkerboard sub-pixel polish (above); (b) reuse getPoses in
the marker/Fiducial pose getters + temporal disambiguation; (c) grid-based pose (multi-marker baseline
removes ambiguity); (d) ROCHADE graph needs a Delaunay-based ROCHADE proper OR retire as experimental.

### Session 89 — deep research on robust association + RANSAC backend landed (trap + perspective fixed)
On `further-restructuring-and-cleanup`, suite **991/991**. Ran the deep-research pass handed off
from S88, then prototyped + productionized the top recommendation, incl. closing the perspective gap.

**Research** (`checkerboard-association-research.md`, cited + license-tagged): SOTA association
methods surveyed. Families: greedy growth (Geiger/libcbdetect [GPL!], OpenCV SB, CNNs — all share
our trap), model-anchored growth (Hoffmann VISAPP 2017 — structural trap fix), graph-topology
(ROCHADE; Deltille [LGPL-2.1]), coded targets (PuzzleBoard [CC0]). ICL already has every primitive
(Hungarian, GenericHomography2D, KDTree, LevenbergMarquardt, ImageUndistortion). Only Delaunay is
missing (needed only for a future ROCHADE backend). Ranked #1: global homography-RANSAC seed →
model-anchored re-fit (reuses `refineCheckerboardGrid`).

**LANDED — `cv::RansacCheckerboardDetector` (`"native-ransac"`), a 3rd CheckerboardDetector backend.**
`recoverCheckerboardGridRansac(seeds)` in `CheckerboardGrid.cpp` (image-free, parameter-free):
(1) **RANSAC** over (centre, axis-pair) affine hypotheses, scored by distinct integer-cell inlier
count — a diagonal basis snaps only half the corners (half-integers) → loses → defeats the trap;
(2) affine-snap a central **core** + **de-shear it to the TRUE axes** (compactness = max fill,
unimodular relabel); (3) grow ONCE from a central cell with those true axes via the shared
`growFixedPoint` local-step engine (so it inherits growth's perspective + lens-distortion tracking);
(4) final de-shear. Shared helpers `growFixedPoint`/`buildGridFromCells`/`deshearCells`/`dedupSeeds`/
`medianSpacing` extracted (greedy `recoverCheckerboardGrid` now also uses them). Wired into the lab
`backend` combo (`native-growth,native-ransac,opencv`). Tests `ransac_clean`, `ransac_diagonal_trap`
(φ=55/45/35°: recovers full 9×6/54 where growth-from-interior-seed traps), `ransac_keystone`.
Real saved frame: complete 4×6/24 @ 0.65px.

**PERSPECTIVE GAP CLOSED.** First prototype used homography ICP for step 2 → brittle under strong
keystone. Replaced with RANSAC-seed → de-shear-to-true-axes → local-step growth (above). Now a clean
**Pareto win over greedy growth**: recovers the full grid everywhere growth does, PLUS the diagonal-
trap region (oblique shear φ≈40–55°) where growth collapses. Only φ≲30° + keystone k≳0.5 (extreme,
growth fails there too) remain. KEY INSIGHT: growth needs the TRUE axes, not RANSAC's arbitrary
unimodular basis — a sheared basis makes growth reach cells only via a 4-connected staircase and
stall, so the core must be de-sheared BEFORE growth.

**NEXT:** (a) resume the Phase-B comparison harness — now THREE detector backends
(growth / ransac / opencv) × both CalibrationTargets through `Camera::calibrate_*`; (b) optional
ROCHADE graph backend (needs a Delaunay util in ICLMath); (c) consider migrating backend selection
onto ICL's generic backend-dispatching framework once a 4th backend appears.

### Session 88 — lab backend/cleanup toggles, OpenCV crash fix, + diagonal-trap investigation (→ deep research next)
On `further-restructuring-and-cleanup`, suite 988/988. Wired the new detection paths into the
lab, fixed a real crash, and investigated the association bug the user spotted live. **Conclusion:
the grid-association robustness needs a rethink — starting a DEEP RESEARCH pass next** (the current
ChESS-saddle → greedy-growth → optional-LAP-cleanup stack is fragile under steep oblique views).

**Committed this session:**
1. **Lab backend switch + LAP cleanup toggle** (`eec894fa9`) — `icl-checkerboard-detection-lab`
   options panel got a `detector backend` combo (`native-growth` | `opencv`, via the
   `CheckerboardDetector` interface) and a `cleanup (LAP)` checkbox (`NativeCheckerboardDetector::
   setCleanup` → `refineCheckerboardGrid`). Both feed `resultDirty` (re-process cached frame).
2. **OpenCV null-Mat crash fix** (`d4e6405b7`) — the lab's opencv backend crashed in
   `findChessboardCorners` (EXC_BAD_ACCESS). Pre-existing bug inherited from the legacy detector:
   `img_to_mat(useImage, m_data->mat)` allocates+returns a fresh Mat when the ptr is null but the
   result was discarded → `*m_data->mat` deref'd null. Fixed (store the result back; init
   `Data::mat`). Regression test `cv.opencvcheckerboard.detect`.
3. **Lab `save frame` button** (`5bb6269d5`) — dumps the exact distorted detector-input image to
   `checkerboard-frame.png` so a failing pose can be replayed offline against real render data.

**Investigation findings (the association bug):**
- The user's screenshots showed a **diagonal-lattice trap**: all seeds detected, but growth
  bootstrapped its axes along the board DIAGONALS → sparse diagonal sub-lattice.
- **Orientation-gate attempt (REVERTED, unvalidated).** Idea: the saddle `orientation` (2nd-harmonic
  phase) gives each corner's local axes, a foreshortening-robust cue to reject diagonal links.
  Rigorously confirmed the convention: the phase points along the square DIAGONAL (bright-quadrant
  bisector), so the row/col axis = `orientation + 45°`. Built a bootstrap gate on it. Result: **zero
  regression** across all synthetics, but **completely inert** — no synthetic reproduces the trap
  (clean synthetic squares never mislead the edge-evidence bootstrap), and under the EXTREME shear
  that causes the trap the orientation cue itself degrades (local axes stop being 90° apart). Refused
  to ship unvalidated, inert association logic → reverted.
- **Real saved frame analyzed** (a MILDER pose than the screenshots): 31 seeds, growth → `6×5/25`
  **axis-aligned but slightly incomplete** (only 2 unconnected seeds, both score ≈0.353 = right at
  the 0.35 threshold), `refineCheckerboardGrid` → `6×4/24` complete+correct. So on this frame the
  association is FINE — the lab just shows growth-only (cleanup defaults off). The dramatic diagonal
  trap is a steeper pose not captured; couldn't reproduce it synthetically (needs the real render's
  lighting to mislead edge-evidence). `checkerboard-frame.png` kept as a future fixture.

**NEXT — DEEP RESEARCH (user's call): robust checkerboard grid association.** The greedy growth +
edge-evidence bootstrap is fragile; the user's intuition is "this should be easy." Survey the SOTA
and pick a principled, foreshortening-robust association: e.g. libcbdetect/Geiger energy-based
growth, OpenCV `findChessboardCornersSB` (ROCHADE refinement), graph/Delaunay topology + the
orientation cue, or the homography-ICP/Hungarian framing done globally rather than as a cleanup.
Goal: replace the bootstrap-then-grow heuristic with something that doesn't fall into the diagonal
trap under steep oblique views, validated against real lab frames (use the save-frame tool).

**Then resume the Phase-B arc:** comparison harness (both `CalibrationTarget`s + both detector
backends through `Camera::calibrate_*`), then the `CalibrationSession` undistortion bootstrap.

### Session 87 — MarkerGridTarget (2nd CalibrationTarget)
On `further-restructuring-and-cleanup`. **COMMITTED** (`964ebf830`), full suite **987/987 green**.
Continues the calibration arc (`camera-calibration-redesign.md`, Phase B).

**Landed:** `markers::MarkerGridTarget` — the 2nd concrete `CalibrationTarget`, wrapping
`markers::AdvancedMarkerGridDetector`. Both calibration backends (checkerboard + marker-grid) now
feed the identical `detect()`/`modelPoints()`/`generate()` contract = the comparison-harness
substrate.
- **`detect()`** runs the detector, emits each *found* marker's **4 corner correspondences**
  (grid-space mm ↔ sub-pixel image px), mirroring `MarkerGridPoseEstimator`'s `appendCornersTo`.
  Unlike the checkerboard: markers self-identify → a **partial/occluded grid still calibrates** and
  the frame is **absolute** (no arbitrary origin / no transpose ambiguity).
- **`modelPoints()`** — all markers' grid-space corners from a reference grid.
- **`generate()`** — renders each marker (`FiducialDetector::createMarker`, BCH default) into its
  grid-space rect at a uniform mm→px scale with margin.
- **PIMPL** `Data{def, detector, model grid}` cleanly hides the *non-const* `detect()` of the
  wrapped detector inside the `const CalibrationTarget::detect()`.
- Test `markers.markergridtarget.generate_detect_roundtrip`: a 4×3 BCH grid generates + detects
  **all 12 markers (48 corrs)**, object frame spans the known grid bounds, sub-pixel affine-labelling
  residual (0.32 / 0.31 px).

**NEXT (pick up here):**
- **Comparison harness** — drive each `CalibrationTarget` / `CheckerboardDetector` backend through
  `Camera::calibrate_*` across a sweep (distance/noise/distortion/viewpoint) → error-vs-truth tables.
  Targets AND detector backends are now all in place behind their interfaces — this is the next big
  payoff (the 2D-checkerboard-vs-3D-marker, native-vs-opencv numbers).
- **Iterative undistortion bootstrap** (`CalibrationSession` above `IntrinsicCalibrator`) — see the
  Session 86 design notes (detect/calibrate in RAW coords; predict-and-refine rather than full-frame
  inverse warp since `createInverseWarpMap` diverges for k1≳0.2; accept a param update only if
  held-out reprojection error improves; coverage map + re-process stored frames). This is what makes
  the opt-in `setCleanup()` safe-by-default.
- **RESEARCH — survey better checkerboard detectors** (findChessboardCornersSB/ROCHADE,
  libcbdetect/Geiger growth, DL corner detectors) vs our ChESS+growth; wrap the best as another
  `CheckerboardDetector` backend if it beats native.

### Session 86 — CheckerboardDetector technique interface + homography/Hungarian false-positive suppression
On `further-restructuring-and-cleanup`. **Both items below COMMITTED**, full suite **986/986
green**. Continues the calibration arc (`camera-calibration-redesign.md`, Phase B). This session
acted on the user's idea: reframe checkerboard point↔grid association as a linear-assignment
problem (Hungarian) + a global homography model, and give detection techniques a real polymorphic
interface so the native path and OpenCV can be compared like-for-like.

**Landed (commit order):**
1. **`cv::CheckerboardDetector` technique interface** (`4a97d7a93`). Abstract seam under
   `markers::CheckerboardTarget`: `detect(Img8u, Hints) → Result{vector<CheckerboardGrid>}` +
   `name()`. `Hints` carry `boardCells` (known inner-corner dims; OpenCV requires them) and a
   non-owning `filter::ImageUndistortion*` — **the feedback channel for the future iterative
   undistortion bootstrap**. Backends:
   - `NativeCheckerboardDetector` — the existing ChESS-saddle + growth pipeline ("native-growth").
   - `OpenCVCheckerboardDetector` — the **former `cv::CheckerboardDetector`** (OpenCV
     findChessboardCorners wrapper) renamed and made a backend (its row-major output maps straight
     onto a complete `CheckerboardGrid`; legacy concrete API preserved for `camera-calibration-planar`).
   `CheckerboardTarget` holds a `shared_ptr<CheckerboardDetector>` (defaults native) +
   `setDetector()`/`getDetector()`. Both techniques now feed one interface = the comparison-harness
   substrate (NEXT below). *Gotcha:* nested `Hints` with DMIs can't be `= {}`-defaulted inside the
   enclosing class → gave it a user-provided default ctor; OpenCV override drops the default arg to
   avoid colliding with its legacy 1-arg `detect` overloads.
2. **`refineCheckerboardGrid()` — homography + Hungarian cleanup** (`d14d95509`). Global,
   model-based re-association that suppresses spurious border detections (the flagged
   "phantom rows/cols inflate the dims" failure): robust homography fit (col,row)→image (iterative
   high-residual rejection so phantoms don't bias it) → predict nodes → **Hungarian** seed↔node
   one-to-one assignment (square cost matrix, per-seed/per-node reject dummies, gate 0.4× local
   spacing) → trim boundary ranks that end up <50% filled or (with image) contrastless. Opt-in
   `NativeCheckerboardDetector::setCleanup()` (name → "native-growth+lap"). **OFF by default**: a
   plain homography mispredicts strongly lens-distorted borders, so it stays opt-in until a
   distortion estimate is fed back via `Hints`. Tests `cv.checkergrid.refine_suppresses_phantom`
   (9×6/50 → 8×6/48) + `refine_clean_noop`. *Gotcha:* `Homography2D(x,y)` maps **y→x** — the
   lattice is the 2nd arg.

**Design notes for later:** (a) the cleanup kills *irregular/partial* phantoms (support trim) and
*geometrically-inconsistent* ones (homography gate), but a phantom rank sitting on the **extended
regular lattice** survives both — that's the edge-score trim's job, but the current edge score is
weakly discriminative (a through-square edge scores ~0.69), so `EDGE_MIN` is conservative; revisit
with the distortion-aware model. (b) The bootstrap loop (see below) belongs in a NEW multi-frame
`CalibrationSession` ABOVE `IntrinsicCalibrator`, NOT inside a detector — detect on the RAW frame
and calibrate in raw coords (rectify only to *reach* border boards; `createInverseWarpMap` diverges
for k1≳0.2, so predict-and-refine rather than full-frame rectify); accept a param update only if
held-out reprojection error improves (EM with a validation gate); keep a coverage map + re-process
stored frames.

**NEXT (pick up here):**
- **`MarkerGridTarget`** — wrap `AdvancedMarkerGridDetector` as the 2nd `CalibrationTarget`, so both
  backends feed the identical interface.
- **Comparison harness** — drive each `CalibrationTarget` / `CheckerboardDetector` backend through
  `Camera::calibrate_*` across a sweep (distance/noise/distortion/viewpoint) → error-vs-truth tables.
  THE 2D-checkerboard-vs-3D-marker (and native-vs-opencv) numbers.
- **Iterative undistortion bootstrap** (`CalibrationSession`) — see design notes above; this is what
  makes the opt-in cleanup safe-by-default (distortion-aware prediction reaches the border boards).
- **RESEARCH — survey better checkerboard detectors** (findChessboardCornersSB/ROCHADE,
  libcbdetect/Geiger growth, DL corner detectors) vs our ChESS+growth; wrap the best as another
  `CheckerboardDetector` backend / `CornerSeed` provider if it beats native.

### Session 85 — checkerboard calibration: detector ~13× faster, grid recovery + first CalibrationTarget, edge validation + guided growth
On `further-restructuring-and-cleanup`. **Everything below is COMMITTED**, full suite **984/984
green**. The detailed calibration arc lives in [`camera-calibration-redesign.md`](camera-calibration-redesign.md)
(Phase B is now well advanced). Sandbox OpenCL/Metal cache fix from S84 is live.

**Landed this session (commit order):**
1. **WarpOp OOB → black is the default** + OpenCL edge-smear fix. The kernel now decides
   out-of-bounds explicitly (coord outside `[0,w-1]×[0,h-1]` → black) instead of relying on
   Apple's `CLK_ADDRESS_CLAMP` (which clamp-to-EDGEs, smearing Cycles' sky into the border).
   OffscreenView distort + lab undistort switched Clamp→Zero. Tests `Filter.WarpOp.border_*`.
2. **Lab: detect on the DISTORTED image**; "apply undistortion" is now a view-only rectified
   preview (no detection) — real pipelines detect on the raw frame and undistort the corner
   *coordinates*.
3. **`cv::CheckerboardSaddleDetector` ~13× faster** (1000×1000: 38.9 → 3.0 ms): precomputed
   ring bilinear weights (no per-pixel floor), squared-magnitude flat gate (skip sqrt on flat
   areas), OpenMP on the dense response + the NMS scan (ordered row-bands → bit-identical),
   `toGray` parallel. `Params::multithreaded` is the only knob (disable to avoid oversubscription
   in an already-parallel context). Tried an OpenCL backend — SLOWER (upload/readback dwarfs the
   compute), removed. `core::cc` not used (no SIMD on mac + wrong (R+G+B)/3 weighting). Tests
   `cv.checkersaddle.*` (incl. `benchmark`, `multithreaded_matches_singlethreaded`).
4. **Grid recovery v1** (`cv::CheckerboardGrid` + `recoverCheckerboardGrid`) — unordered seeds →
   ordered integer `(col,row)` lattice. Dedup near-duplicates → bootstrap axes → fixed-point
   growth (each cell re-derives local step vectors from its own neighbours; iterate till nothing
   new). **GUIDED** when given the image: axes bootstrapped by EDGE EVIDENCE (true axis link = a
   B/W border = high; diagonal = crosses a uniform square = low) — fixes the diagonal lattice
   pure geometry grows under strong foreshortening. Tests `cv.checkergrid.*`.
5. **Edge validation pass** (`scoreCheckerboardGridEdges`) — per-edge confidence = mean image
   gradient PERPENDICULAR to the edge, probed at ±0.4× the **perpendicular cell spacing** (so
   foreshortened "back" edges score correctly). Lab colours edges red→green by it.
6. **`markers::CheckerboardTarget`** — the FIRST concrete `CalibrationTarget` (interface was
   scaffolded, no impls). detector + guided recovery → object↔image correspondences for
   `Camera::calibrate_*`; `modelPoints()` + a detectable `generate()`. Test
   `markers.checkertarget.generate_detect_roundtrip`.

**Pipeline now:** detector (seeds) → grid recovery (ordered lattice) → CheckerboardTarget
(correspondences) → `Camera::calibrate_*`. `CheckerboardGrid` is the stable seam the decided
region-quad/ChESS hybrid can replace later without touching consumers.

**NEXT (pick up here):**
- **False-positive suppression** (deferred, the user flagged it): extreme views yield a few
  spurious *border* detections that inflate the lattice dims. Build a heuristic on the edge
  confidence — reject seeds whose best edges are weak, or require a complete rectangular
  sub-block. (See the unchecked box in the plan doc.)
- **`MarkerGridTarget`** — wrap `AdvancedMarkerGridDetector` as the 2nd `CalibrationTarget`, so
  both backends feed the identical interface.
- **Comparison harness** — drive each `CalibrationTarget` through `Camera::calibrate_*`
  (decoupled intrinsics → fixed-intrinsics extrinsics) across a sweep (distance/noise/distortion/
  viewpoint) → error-vs-truth tables. THE 2D-checkerboard-vs-3D-marker numbers (the whole point).
- **RESEARCH — survey better checkerboard detectors.** Detection quality bounds calibration
  accuracy, and our native ChESS-saddle + growth still struggles on hard cases (strong
  foreshortening false positives, glare/blur, partial boards). Survey the state of the art
  (e.g. OpenCV `findChessboardCornersSB`/ROCHADE-style refinement, libcbdetect / Geiger's
  growth method, deflectometric / deep-learning corner detectors) and benchmark robustness vs our
  native path — keep ICL-native if competitive, else wrap the best as another `CornerSeed`
  provider in the planned seed-fusion framework.

### Session 84 — Node→Scene2 backpointer, OffscreenView lens distortion, WarpOp bugs, sandbox OpenCL fix
**(LANDED — committed in Session 85; the "UNCOMMITTED" notes below are historical.)**
On `further-restructuring-and-cleanup`. **All work below is UNCOMMITTED** in the working tree
(git status: WarpOp.{h,cpp} + WarpOp_{Cpp,Ipp,OpenCL}.cpp, geom2 Node/GroupNode/Scene2/
OffscreenView/CheckerboardNode + the lab, tests/test-filter.cpp, new scripts/sandbox-cl-*).
Full suite was **971/971 green** before the OpenCL sandbox cache broke (see ⚠️ below).

**LANDED (built + tested, except where noted):**
1. **Node→Scene2 back-pointer** (`project_node_scene_backpointer`). `Node` has a non-owning
   `Scene2*` (set by `Scene2::addNode`, propagated by `GroupNode`); `Node::ScopedEdit` RAII
   self-locks the owning scene + marks it changed; `Scene2::touch()`/`sceneVersion()`;
   `OffscreenView::next()` auto-resyncs on a version bump. Lab `setBoardCells` is gone — just
   `board->setCells(gui["xc"], gui["yc"])` per frame.
2. **OffscreenView forward lens distortion** — `distortion.k1/k2` (`Range<float>`) +
   `distortion.reset` (`Command`) props; applies a `filter::WarpOp` (createWarpMap — the EXACT
   forward map; createInverseWarpMap diverges for k1≳0.2) to the captured frame. Lab keeps only
   the optional rectify (createInverseWarpMap). See `reference_inverse_warpmap_divergence`.
3. **API rework**: `poll()`+`image()` → single `next() → Frame{core::Img8u image; bool isNew}`.
4. **WarpOp bug #1 (the "distortion frozen" one)**: OpenCL backend cached the GPU warp map by
   SIZE only → a same-size `setWarpMap` was ignored. Fixed with a **version/dirty flag**:
   `WarpOp::m_warpMapVersion` (bumped in setWarpMap), threaded through `WarpSig` (Cpp/Ipp ignore
   it), `CLWarpState` re-uploads on version-or-size change. Backend is per-WarpOp (addStateful =
   per-clone), so no cross-op collision. Test `Filter.WarpOp.reuse_setWarpMap`.
5. **WarpOp `BorderMode` (Zero/Clamp)** — Zero (default) = black OOB; Clamp = replicate edge.
   **(Session 85)** OOB→black is now the chosen default everywhere: the OpenCL kernel makes the
   call explicitly (`fX/fY` out of `[0,w-1]×[0,h-1]` → black) instead of relying on the input
   sampler — Apple's CL→Metal `CLK_ADDRESS_CLAMP` behaves as clamp-to-edge and was smearing the
   source edge (Cycles' sky) across the whole OOB region (the "sky ring"). OffscreenView distort
   + lab undistort switched Clamp→Zero. Kernel also dropped the unused filter-mode arg (integer
   images can only be NEAREST-sampled). Tests `Filter.WarpOp.border_clamp`, `.border_zero_is_black`.

**IN PROGRESS / NOT DONE:**
- **WarpOp bug #2 — OpenCL kernel skipped the 1-px outer frame** — DONE/verified (sandbox
  OpenCL now compiles). Kernel writes every pixel. Test `Filter.WarpOp.border_pixels_written`.
- **User's requested auto-scale distortion** — DONE. `ImageUndistortion::createWarpMap(bool
  autoScale)` / `createInverseWarpMap(bool autoScale)` take an opt-in flag: when set,
  `fillWarpMap` first scans all mapped source coords and picks the largest uniform scale (≤1)
  about the distortion centre (principal point) that keeps the WHOLE frame in source bounds
  (alpha=0 crop — no OOB/black border, fills the frame). Cache key now includes the flag.
  `OffscreenView` exposes it as the `distortion.fill frame` Flag prop (atomic `autoScale`,
  threaded through `syncDistortion`/`refreshOutput`'s warp-map rebuild key); the lab gets the
  toggle for free via `Prop(&view)`. Test `Filter.WarpOp.auto_scale_fills_frame` (map-coord
  only, no OpenCL). Full suite 973/973.

**⚠️ SANDBOX OpenCL FIX (apply on host, then relaunch mscc):** changing the WarpOp kernel source
invalidated the `opencl_c.pcm` Metal module cache, and rebuilding it inside mscc fails:
`cl2Metal failed / opencl_c-*.pcm: Operation not permitted`. Root cause: the Metal compile runs
in the `com.apple.MTLCompilerService` XPC daemon, which writes the cache only via a **sandbox
extension** the sandboxed client must ISSUE — and mscc denied `file-issue-extension`
(`deny(1) file-issue-extension target:.../com.apple.metalfe extension-class:
com.apple.app-sandbox.read-write`). **Fix scripted:** run `scripts/sandbox-cl-patch.sh` on the
host (adds `(allow file-issue-extension (require-all (extension-class
"com.apple.app-sandbox.read-write") (subpath "/private/var/folders")))` to
`~/margin.mscc/default-profile.darwin`), then relaunch mscc. Confirm with
`scripts/sandbox-cl-smoke.sh` (host; its "+candidate" run should print `OK: kernel built`).

**RESUME AFTER RESTART (fresh mscc):**
1. `ninja -C builddir -j 16` then `builddir/bin/icl-tests -j 1 -f 'Filter.WarpOp*'` — with the
   sandbox patch live, OpenCL should compile; confirm `reuse_setWarpMap`, `border_clamp`,
   `border_pixels_written` all PASS (the last verifies the 1-px kernel fix).
2. Implement the **auto-scale distortion** (user's idea) on `filter::ImageUndistortion`
   createWarpMap/createInverseWarpMap; wire into OffscreenView's distort. Validate via map coords.
3. Real-display pass on `icl-checkerboard-detection-lab` (still owed): drag camera, GL↔Cycles,
   k1/k2 sliders (now live), reset, undistort toggle — confirm no edge false-positives.
4. Commit the lot (was 971/971 green pre-sandbox-break).

### Session 83 — geom2 offscreen-render tooling + checkerboard-lab overhaul
On `further-restructuring-and-cleanup`, full build green throughout. Headless GL **and** Cycles
both verified in-sandbox (Session 82 work). This session built the reusable "interactive GL scene
+ switchable offscreen renderer" stack and rebuilt the checkerboard-detection lab on it.

**Commits (oldest→newest):** `8548851d3` headless GL verified + 2 renderer bug fixes ·
`d0ec8e28a` Cycles fixes + docs + `OffscreenView` born · `71e31d8e5` `CheckerboardNode` + NN
texture filter · `544f21f6e` warp-map (un)distortion + `LightNode` factories · `876ee2f4c` lab
3-pane layout + `OffscreenView` poll()/image() consolidation.

**New reusable pieces (geom2 unless noted):**
- **`OffscreenView` (`OffscreenView.{h,cpp}`)** — THE pattern for a real on-screen Scene2 view +
  a switchable GL/Cycles offscreen capture, threading baked in (GL renders on the GUI thread in
  the widget context — a worker-thread owned context stalls macOS; Cycles is GL-free, polled on
  the worker). A `utils::Configurable`: `Prop(&view)` surfaces `backend`, `cycles.*`, and the
  capture scene as a `scene.*` child. API: `poll()` (drives Cycles + auto-requests a GL capture on
  camera/backend change), `image()` (cached latest frame), `invalidate()` (scene edit → refresh
  both), `requestCapture()`, `setBackend()`/`cycles()`.
- **`CheckerboardNode`** — calibration board as a node: one quad, **1-texel-per-cell
  nearest-neighbour** texture (crisp at any scale; renderer AA smooths edges), `innerCorners()` =
  the (cols-1)×(rows-1) ground-truth saddle corners. Reuses its material across `setCells()`.
- **`Material::TexFilter` (Linear/Nearest)** — honoured by GL (`GL_NEAREST`) + Cycles
  (`INTERPOLATION_CLOSEST`).
- **`filter::ImageUndistortion::createInverseWarpMap()`** — the DISTORTION (forward) map
  (`createWarpMap` rectifies). Lab applies both via `filter::WarpOp` (precomputed, rebuilt on
  k1/k2 change). Both share one `fillWarpMap()` helper.
- **`LightNode::point()/directional()`** factories (point() = warm-white shadow-caster).
- Cycles backend **documented** (`CyclesRenderer.h`/`Raytracer.h`: drive models + non-black
  recipe). Fixed: 3-channel RGB texture corruption (`SceneSynchronizer`); `Scene2::add/removeNode`
  now invalidate the renderer cache (the x/y-cells stale-texture lag).

**Verify these HERE (headless):** `geom2-headless-gl-capture-demo`,
`geom2-headless-cycles-capture-demo` (both render crisp images in-sandbox).

### NEXT — pick up here
1. **Real-display pass on `icl-checkerboard-detection-lab`** (owed — built + headless-checked, never
   run on a real display). 3 panes: 3D view | options | result. Check: drag camera (smooth FPS),
   toggle GL↔Cycles in the `Prop(&view)` panel (live Cycles refinement), "apply undistortion"
   toggle, x/y cells (crisp board, no stale texture), k1/k2 distortion, `scene.enable lighting` +
   shadows. Also eyeball the other geom2 GL apps (still build-checked only).
2. **Node→Scene2 back-pointer** — planned, memory `project_node_scene_backpointer`: self-locking
   high-level mutators + auto-invalidation (Cycles `SceneSynchronizer` version-aware) → drop the
   manual `scene.lock()`/`view.invalidate()` in `setBoardCells` and simplify to a per-frame
   `board->setCells(gui["xc"], gui["yc"])` (already idempotent).
3. **Shadow-casting disturber objects** in the lab → realistic shaded test images for the detector.
4. **camera-calibration redesign** (last geom→geom2 retirement item) — plan
   `camera-calibration-redesign.md` + backlog. `CheckerboardNode` + the lab now provide the
   target/visualization half; resume at distortion-tolerant grid recovery → `CheckerboardTarget`
   backend → multi-frame intrinsics. Then the endgame: **delete `geom`, rename `geom2`→`geom`**.

Headless GL/Cycles smoke tooling kept: `scripts/sandbox-gl-smoke.sh`, `scripts/sandbox-gl-probe*.c*`
(`patch.sh` throw-away). sandbox writeup: `sandbox-harness-notes.md`.

---

**⏸️ BREAK POINT (end of Session 81).** On `further-restructuring-and-cleanup`, build +
**969/969** green throughout. The **geom→geom2 retirement is essentially complete** (only
camera-calibration remains, as a deliberate redesign); this session was mostly the
**camera-calibration redesign** (Phase A + start of Phase B). Build- + headless-checked only —
NO GL in this sandbox; real-display pass still owed on all geom2 apps.

### PBuffer port — LANDED (offscreen GL render from any thread)
The legacy `geom::Scene::PBuffer` pattern is ported as a **mode of `GLSceneCapture`**:
`GLSceneCapture(ownContext=true)` (`icl/geom2/SceneCapture.{h,cpp}`). It owns a
`QOpenGLContext` + `QOffscreenSurface` (shares lists with `globalShareContext()` for
textures), **lazily created inside the first `capture()`** so the context's thread affinity is
the *calling* (worker) thread, then `makeCurrent → Scene2::renderToImage → doneCurrent`.
Returns RGB+depth as `BVH::ImageResult` (just the shared `{Img8u image; Img32f depth;}` struct
— the GL path does NOT go through the raytracer; BVHSceneCapture is only the no-GL fallback).
`renderToImage` already saves/restores the caller's FBO+viewport, so borrowed-context mode
(default `ownContext=false`) still composes inside an on-screen draw callback. Build-checked
only (no GL in sandbox).

**⚠️ Context-sharing caveat that blocks the naive lab switch:** a `Renderer` uses **VAOs**,
which are **NOT shareable across GL contexts** (even with `AA_ShareOpenGLContexts`, which ICL
doesn't set anyway). So owned-context mode renders a scene whose `Renderer` lives ONLY in the
offscreen context. You CANNOT capture the *same* on-screen `Scene2` (whose VAOs were created in
the widget's context) through a second offscreen context. To finish the lab you must either:
(a) build a **dedicated capture Scene2** (mirror the board geometry + the interactive camera)
rendered solely through the owned-context capturer, or (b) drop the on-screen `Canvas3D` and
render BOTH the interactive view and the camera-0 view offscreen, displaying them as plain
`Canvas` images (then the mouse handler must drive the capture camera directly). (a) preserves
the interactive GL left pane; (b) is simpler but loses native GL navigation.

Then finish the lab switch: RIGHT pane = real offscreen render of camera 0 → apply the radial
lens-distortion (CPU) → detect → overlay; 2nd canvas = undistort with the (known) `k1,k2`.
Homography hack is to be removed. Add a lighting toggle so shading/shadows can be exercised.
**Eventual stretch:** also offer a **Cycles**-rendered camera view (`geom2::CyclesRenderer`)
for photoreal test images (real shadows/GI/material + lens effects) to stress the detector.

### Camera-calibration redesign — status (plan: `camera-calibration-redesign.md`)
Rethinking the drift-prone 3D joint-DLT pipeline instead of transliterating it. Decisions:
planar primary / 3D kept for the multi-cam one-click; registerable `CalibrationTarget` backend
(checkerboard NEW + marker-grid + ChArUco later); ICL-native intrinsics vs OpenCV compared;
harness-first. **Landed this session:**
- **`calibrate_extrinsic` bug fixed at the source.** Root cause: its linear SVD seed was
  non-robust (cheirality-flipped + ~3× mis-scaled → poisoned the LMA → the "divergence").
  Replaced with column-norm scale + cheirality + SVD-orthonormalisation. Now 5.8mm vs joint
  DLT 245mm at 3m. The reusable nearest-rotation piece was promoted to **`math::closest_rotation`**.
- **Phase-A harness** (`test-geom2-calibration-harness`): projection-based; reproduces the
  depth drift quantitatively (near 700mm: 2mm; far 3000mm: 351±265mm) + the decoupling fix.
- **Native ChESS checkerboard detector** (`cv::CheckerboardSaddleDetector`, OpenCV-free): ring
  2nd-harmonic saddle response + NMS + sub-pixel; per-corner orientation free; LOCAL → distortion
  robust. Tests (clean + barrel-distorted): clean 64/64 @ 0px, distorted 64/64 @ 0.49px. Emits
  `cv::CornerSeed{pos,score,orientation}`.
- **`markers::CalibrationTarget`** pluggable-backend interface scaffolded (detect→correspondences,
  generate→printable).
- **`icl-checkerboard-detection-lab`** app (geom2/apps): left interactive board, right
  distorted+detection / undistorted canvases. Currently homography-based (to be switched to the
  offscreen render above). Fixed a white-board bug (use scene `enable lighting`=false for a flat
  board, NOT emissive=white which the renderer ADDS → saturates).

**Detector architecture (decided):** HYBRID — region-quads (LocalThreshold→RegionDetector→
QuadDetector) own topology/`(row,col)`/origin-disambiguation + seed; ChESS owns precision +
splits the touching-quad merges. Generalise to a **seed-fusion/refinement framework** (pluggable
`CornerSeed` providers + refiners) once the 2nd provider exists. **Phase B next after the
offscreen render:** distortion-tolerant **grid recovery** (growth-on-seeds vs region-quad) →
`CheckerboardTarget` backend; then multi-frame intrinsic estimation (move board around scene).

### geom→geom2 retirement — DONE except one item (see `backlog.md`)
Only **camera-calibration + camera-calibration-planar** (markers) still touch the geom Scene
layer — that's the redesign above. Everything else is ported. CV-only tools (kinect-normals,
kinect-recorder, fix-kinect-calibration, show-extrinsic-calibration-grid, simplex-2D,
compute-relative-camera-transform, icp3d-test) stay in geom and don't block deletion. Endgame
after calibration: **delete `geom` → rename `geom2`→`geom`**.

(Earlier S79/S80 pipeline-era notes — scene→RGBD→point-cloud — kept below; unchanged.)

### Resume at: geom→geom2 retirement (active)
**Endgame:** port every keeper geom→geom2, then **delete `geom`**, then **rename geom2 →
geom**. Plan/worklist: `geom-retirement-worklist.md`; todo index: `backlog.md`.

**The full pipeline now exists (all on the RGBD-as-Image rails, no point-cloud-specific
transport):**
`-i scene` → `icl-point-cloud-pipe` *[box / sphere / near-far filter]* → `-o ws/file` →
`icl-point-cloud-viewer`. The depth **camera travels in image metadata** (operator<</>>);
the WS compressor preserves it (locked by a round-trip test). Filters are reusable
`PointCloud` methods, not app-local.

**Landed this session (10 commits, `82d2792b3`..`1fb04d569`):**
- **`SceneCapture`** — render a Scene2 through a camera → RGB+depth, two interchangeable
  backends: `BVHSceneCapture` (CPU raytrace, **headless** — the engine the sim source uses)
  and `GLSceneCapture` (GL offscreen `Scene2::renderToImage`). Shared `BVH::ImageResult`/
  `DepthMode` vocab. Demo: `scene-rgbd-capture`.
- **`-i scene`** source (`icl/geom2/detail/SceneSource.cpp`) — synthetic depth/RGBD/color
  camera over a built-in scene; registered into io (geom2 now links `icl_io_dep`).
- **`PointCloud::unprojectDepth`** (consumer side) + **`PointCloudSource`** helper (the
  ImageSource→cloud unit, shared by viewer+pipe) + **`filterBox`/`filterSphere`/
  `filterDepthRange`** (camera-based near/far needs the camera — cloud has no depth).
- **`icl-point-cloud-viewer`** + **`icl-point-cloud-pipe`** (geom2 apps). Retired the legacy
  geom targets `point-cloud-viewer`, `simple-point-cloud-viewer`, `point-cloud-pipe` (sources
  kept; geom2 apps claim the names).
- **Rendering:** `Material::setBaseColorMap` (version-bumped **real-time textures**; renderer
  re-uploads only changed materials, `glTexSubImage2D` fast path) + `scene-monitors` demo
  (4 cams + 4 in-scene monitors). `Scene2` **"show cameras"** flag → per-camera gizmo (RGB
  axes + stylized frustum + billboard label). **Thick lines** via a geometry shader (1px keeps
  built-in `GL_LINES`; >1px expands to screen-space quads — works on macOS where `glLineWidth`
  is capped at 1).
- **Bug fixes:** `BVH::raycastToImage` wrote baseColor [0,1] into an 8-bit image (everything
  black) → ×255; `GeometryNode::setPrimitiveVisible` didn't bump `geometryVersion` (visibility
  toggle silently ignored); the camera gizmo used [0,1] line colours but **MeshNode colours are
  0..255** (×1/255 → invisible) — this was the "lines never visible" mystery.

**Next concrete steps (pick up here):**
- **cylinder filter** — *declined by user* (box/sphere/near-far is enough; the cube/sphere/
  cylinder *primitive-filter* port with RSB/protobuf primitives stays deferred — "need
  `Primitive3D→node`").
- **point-cloud cluster leftovers:** `point-cloud-creator`, `point-cloud-define-world-frame`
  still legacy-geom; primitive-filter deferred.
- **kinect** demos (fuse the 2 segmenters), **surf-based-object-tracking** (clean app port),
  **marker-detection** (separate; 1-view↔n-view/source generalization), **superquadric**
  (new `SuperquadricNode`), **animated-grid**/**plot-widget-3D** (shader / widget rework),
  **markers** dep needs geom2 added.
- Each port: keep the demo/app character + the HSplit layout convention; build + headless-init
  only (no GL/hardware here → real-display verification deferred).

**Real-display testing (no hardware needed — run in a normal terminal, not the sandbox):**
`icl-point-cloud-viewer -i scene default` · `... -i scene default@format=rgbd` ·
`icl-point-cloud-pipe -i scene default@format=rgbd` (flip the filter combo) ·
`scene-rgbd-capture-demo` · `scene-monitors-demo`. Chain: `pipe -o ws 9000` →
`viewer -i ws 9000` (use `@compression.mode=raw` if a float stream complains).

---

**Earlier this session — Phase 6 (physics deletion).** **🎉 The legacy `icl/physics` module is
DELETED** — the physics-side of "get rid of old geom" (redesign-plan **Phase 6**) is done.
Three arcs:

1. **Driving-game M4** (the live integration test) + four tuning fixes — see the Session-78
   recap below. The cloth station is OUT (a threaded soft-body instability, deferred — memory
   `project_threaded_deformable_cloth_instability`), replaced with a static gateway arch.
2. **Ported the last two legacy physics demos to physics2:** `physics2-maze` (kinematic
   **compound** board + ghost **sensors** that follow the tilt; top-down camera) and
   `physics2-water-rocket` (compound bottle + thrust + draining mass + apogee + contact
   tip-separation). With these, **every** legacy demo had a physics2 equivalent.
   - **⚠️ `physics2-water-rocket` is MOSTLY BROKEN on a real display** — flight mechanics pass
     headless (`rocket_compound_thrust_to_apogee`) but the live demo misbehaves (thrust/mass
     unit scale, follow camera, tip separation, the faked parachute). **Undecided whether to
     keep it.** Header comment flags it. Don't rely on it.
   - `physics2-maze`: camera was a side 3/4 view → now **top-down** (a labyrinth is played from
     above); distance/focal framing is an unverified real-display estimate.
3. **Phase 6 deletion:** removed `icl/physics` entirely (81 files — module + `PhysicsScene` /
   `PhysicsScene2` bridge + the 5 legacy demos) and its meson wiring. **Zero external
   dependents**, so it was clean; `btSoftRigidDynamicsWorld` stays (it's in physics2). 124→119
   binaries, 943/943.

New reusable framework this needed (all tested): **compound bodies** (`shapeFromNode` turns a
`GroupNode` into a `btCompoundShape`; `deleteShape` frees them recursively),
`RigidBodyDriver::getLinearVelocity` / `setTransform` (teleport), `SensorDriver::setTransform`
(a moving ghost zone).

**Resume options (pick one):**
- **Front B — retire the legacy `geom::Scene` rendering layer** (the *other* half of "get rid
  of old geom"): **8 geom + 4 markers** demos/apps still render through `geom::Scene`; port
  each to geom2, then delete `Scene`/`SceneObject`/`GLRenderer`/`Primitive`/… (keep the geom
  CV/math core). Plan: `geom2-migration-plan.md` (Phase 3→4).
- **The threaded deformable-cloth instability** (blocks the driving-game cloth station + a real
  soft parachute for the water-rocket) — root-cause it (sim-thread soft capture vs UI `sync`
  race? deformable solver under many contacts at real-time cadence?). Memory
  `project_threaded_deformable_cloth_instability`.
- **Driving-game M5 polish** (HUD/speedometer, reset-on-flip) — small.

---

## Session 77 recap (was the break point) — Big session. On
`further-restructuring-and-cleanup`, build + **937/937** green throughout. Three arcs landed:

1. **geom2 pointer-ownership cleanup** — `deepCopy()` returns `NodePtr` (no raw *owning*
   pointers); one rule on `NodePtr`: **own with `shared_ptr`/`NodePtr`, observe with raw
   `Node*`**; new **weak cross-edge tier** (`getDriverPtr<T>`, `getNodePtr`/`getLightPtr`).
   The value-handle/Image-pattern rewrite was weighed and **declined** (not worth a
   module-wide rewrite of a typed hierarchy). See memory `feedback_polymorphic_over_registry`.
2. **physics2 Phase 4c — constraints** (`Constraint` + `SpringConstraint`; world factories
   `addHinge/addSlider/addBallSocket/addSixDOF/addSpring`). **Then a key correction:** the
   "constraints need a SoftRigid world" claim was a *misdiagnosis* — joints work in the
   default **Deformable** world, which is the unified **rigid + joints + cloth** world. So
   the planned **multi-world split is DROPPED** (one world does it all); `SoftRigid` survives
   only for the fold-aware paper. (`physics2-driving-plan.md` §1.)
3. **The driving game M1–M3** — a third-person car you can actually drive around a course.
   `VehicleDriver` (`btRaycastVehicle`), `qt::KeyboardHandler` (new ICLQt facility), chase
   camera, and a static playground course (ramps/jump/banked turn + chassis CCD).

**Also fixed a latent GUI bug:** `GUI::operator<<` **sliced** any *labeled* component to the
base type → "component type 'X' has no widget factory" at create — broke **every** labeled
component on a real display (only reachable with a GL context, so CI/sandbox missed it).
Fixed by cloning polymorphically; regression test added.

*(M4 — interactive stations — was the next step at the end of S77; it landed in Session 78,
see the break-point block above. Plan: `physics2-driving-plan.md`.)*

**Other open fronts** (unchanged): legacy-physics retirement still needs `physics-constraints`
(unblocked ✓), the **full maze** (compound bodies + ghost sensors), **water-rocket** (port),
then Phase 6 (delete the `geom::Scene` physics path + `PhysicsObject`/`PhysicsScene`
inheritance, KEEP `btSoftRigidDynamicsWorld` behind the flag). Also **DefaultScene step 2**,
**defaults policy → material database**. Full plan: `physics-geom2-redesign-plan.md`.

### ⏸️ DEFERRED — paper M3 polish + fold/crease bending regeneration

Pushed for possible future continuation. **Working assumption (user, Session 76):** we'll
likely develop a *new* physics paper on top of the **new deformable (btDeformable) stack**
rather than extend the current SoftRigid `PaperDriver` — in which case the items below are
superseded and shouldn't be invested in against the SoftRigid implementation. Revisit only
if we decide to keep evolving the SoftRigid paper.

- **M3 polish (best on a real display):** front/back **texture rendering** (needs geom2
  MeshNode texcoord/texture support — check first) and **hover + context-menu fold editing**
  (`adaptFoldStiffness` now edits the `Crease` primitive but is still UI-unexercised).
- **Fold/crease bending-constraint regeneration** (the 🔧 TODO that was here): after a
  screen-line fold the crease topology updates but the 2nd-order (bending) constraints are
  not reconciled — remove links crossing the crease, add constraints for inserted nodes, or
  (easiest) re-create ALL bending constraints from scratch with the crease-map taken into
  account, using **flat paper-space rest lengths** not deformed 3D distances. See memory
  `project_paper_fold_bending`.

All physics/composition is headless-tested (937/937).

After Phase 7 + (if pursued) paper: **DefaultScene step 2** (Landscape/Room + retire
`DemoScene2`), the **defaults policy → material database** (now that node-mass stiffness
derivation is half-built), Phase 4b raycast vehicle, then Phase 6. Also a natural next
experiment: the **multi-world** story (paper SoftRigid + cloth Deformable → one `Scene2`) —
the drivers are ready, only `PhysicsScene` owning a *list* of worlds is missing.
**Full plan + phase status: `physics-geom2-redesign-plan.md`.**

**Phase 6 is now nuanced:** we can NOT delete `btSoftRigidDynamicsWorld` — paper needs
it as a mode. Phase 6 = retire the dead `geom::Scene` physics path + the old
`PhysicsScene`/`PhysicsObject` inheritance, while KEEPING the legacy solver behind the
flag.

## Session 77 recap, part 2 (unified-world call + driving game M1–M3 + GUI slice fix) — committed

Continues part 1 below. Commits on `further-restructuring-and-cleanup`; suite grew 930 → 937.

- **The unified-world call (`30ca0375`).** Phase 4c had claimed constraints need a `SoftRigid`
  world — a **misdiagnosis** (confounded by an anchor-overlap bug + the Y-axis gimbal limit).
  Re-tested clean, **all joint types work in the default `Deformable` world**
  (`btDeformableMultiBodyDynamicsWorld`), which already hosts **rigid + 6DOF joints + stable
  cloth** in one solver. Decision: build everything on the single Deformable world; **drop
  the planned multi-world split**; `SoftRigid` kept only for paper (cluster self-collision).
  Tests now run joints in both modes; `Constraint.h` doc corrected (only real caveat: the
  6DOF middle/Y angular axis gimbal-limits — hinge about X or Z).
- **Driving game (`physics2-driving-plan.md`, M1–M3 LANDED):**
  - **M1 `VehicleDriver`** (`ddeae4c5`) — `btRaycastVehicle` as a geom2 Driver (chassis +
    4 raycast wheels, capture-hook sync, command-queue controls), `PhysicsScene::addVehicle`.
    Two non-obvious fixes: (1) the soft/deformable worlds override the step internals and
    **skip Bullet's `updateActions()`**, so `PhysicsWorld` ticks registered actions
    *manually* in `stepOnce` (→ vehicle works in any world); (2) **scale-aware suspension**
    (physics2 scales length not time/mass → Bullet gravity ~10×; stiffness/maxForce scaled
    by the gravity ratio, else the car bottoms out). 4 headless `stepOnce` tests.
  - **M2 controls + chase cam** (`c99a9fcb` + `3f5d20ef`) — **`qt::KeyboardHandler`** (the
    app-level keyboard input ICL lacked; mirrors `MouseHandler`, held-key set, installed via
    `widget`/`gui["draw"].install`; `ICLWidget` dispatches press/release + `setFocus()`s on
    install). WASD **and arrow keys**; a demo-local `ChaseCamera`. Real-display fixes: chase
    up-vector (ICL `m_up` points to image *bottom* → negate, like `Camera::lookAt`), keyboard
    focus, wheel orientation (CylinderNode axis is Z → rotate to the axle).
  - **M3 course + CCD** (`b2e645fe` + `980b1c6f`) — `VehicleDriver::setCcd`; a 16×16 m
    playground (boundary, climb ramp→platform, jump kicker, banked turn, blocks). Ramps
    **bury their low end** in the ground so the drive surface emerges at ground level (no
    floating edge). Headless `vehicle_climbs_ramp` test. Camera pulled well back.
- **Latent GUI bug fixed (`6900bbb1`).** `GUI::operator<<` **sliced** a *labeled* component to
  the base `GUIComponent` (`GUIComponent inner = component;`), dropping its `createWidget`
  override → "component type 'X' has no widget factory" at create. Hit **every** labeled
  component (e.g. a Scene2 Prop panel's `background color` → `ColorSelect`) but only on a real
  GL display (lazy widget creation never reached headless). Fixed by cloning polymorphically;
  `GUI::getComponent`/`getChild(Count)` made public; regression test
  `qt.gui.labeled_component_not_sliced`.

**Resume at driving-game M4** (interactive stations — the end-to-end integration test).

---

## Session 77 recap, part 1 (geom2 pointer-ownership cleanup + physics2 constraints) — committed

Branch `further-restructuring-and-cleanup`, 3 commits, build + **930/930** green at each step.
The session started as "scope the constraint drivers" and the constraint API kept tripping
over geom2's mixed `shared_ptr<Node>` / `Node*` usage — so we fixed geom2's ownership model
*first*, then built constraints on top of it.

- **`249847f7` — geom2 NodePtr ownership pass.** `Node::deepCopy()` returned a raw *owning*
  `Node*` (the one place ownership leaked into a bare pointer) → now returns `NodePtr`
  (= `shared_ptr<Node>`); all overrides build via `make_shared`. Introduced
  `NodePtr`/`ConstNodePtr`, threaded through the base-`Node` interfaces; added
  `Scene2::getNodePtr`. Documented the rule and tagged the raw getters
  (`getParent`/`getChild`/`getNode`/`Hit2::node`) as non-owning views. Rule-of-5 on node
  subclasses left intact (copy feeds `deepCopy`, move feeds `addNode(T&&)` — both load-bearing).
- **`ac0115bf` — geom2 weak cross-edge tier.** Named the third reference kind geom2 lacked.
  The tree edges were already clean (down = `shared`/owns, up = raw/observe-owner); the gap
  was the **cross-edge** — a long-lived reference to a peer you neither own nor are owned by.
  Three-tier model now documented on `NodePtr`: **down = `shared_ptr`, up = raw, cross =
  `weak_ptr`**. Added `Node::getDriverPtr<T>()` (the strong handle a cross-edge downgrades
  from), `Scene2::getLightPtr`. (Decision: did NOT do the value-handle/Image-pattern rewrite —
  see the deferred note at top + memory `feedback_polymorphic_over_registry`.)
- **`c6e7bf1a` — physics2 Phase 4c constraints.** `Constraint` + `SpringConstraint` (PIMPL,
  **no Bullet in the public surface**; ctors private + friend `PhysicsWorld`). The legacy
  `SixDOF/Slider/Hinge/BallSocket` tree is really one `btGeneric6DofConstraint` with limit
  presets; `Object2Point` → `SpringConstraint` (spring + phantom anchor body). World factories
  `addHinge/addSlider/addBallSocket/addSixDOF/addSpring` take `NodePtr`, return
  `shared_ptr<Constraint>`; `PhysicsScene` forwards. **Cross-edge lifetime:** each constraint
  holds `weak_ptr<RigidBodyDriver>`; `removeBody` drops dependents *first* (Bullet requires a
  joint to die before its bodies), the handle goes inert (never dangles). **Latent geom2 bug
  fixed:** `Node::~Node` now detaches drivers, so "destroy node → body leaves world" (the RAII
  promise the driver model rests on) is finally true. Added `RigidBodyDriver::setDamping`.
  7 headless tests; demo `physics2-constraints` (4 joint stations + grab + debug overlay).
- **Two Bullet gotchas (documented in `Constraint.h`, asserted by tests):** (1) joints need a
  **`SoftRigid`** world — the deformable multibody solver injects energy into 6DOF joints
  (door swung *up* past its start height); (2) `btGeneric6DofConstraint` gimbal-limits the
  **middle angular axis (Y)** — hinge about X or Z, not Y.

This unblocks `physics-constraints`, one of the four Phase-6 legacy-retirement demos.
**Full plan + phase status: `physics-geom2-redesign-plan.md` (Phase 4c marked LANDED).**

---

## Session 76 recap, part 2 (GUI builder re-engineered onto a polymorphic interface) — committed

After Phase 7 (below), the GUI builder was fully re-engineered per the user's call to
"use an actual polymorphic interface" (see memory `feedback_polymorphic_over_registry`).
8 commits, build + 923/923 green at each step. End state:

- **`GUIComponent` is the polymorphic interface:** virtual `createWidget(const CreateContext&)`
  + virtual `clone()`; CRTP `GUIComponentT<Self>` supplies clone(). Each component (Slider…
  Prop, geom `Plot3D`) is a `GUIComponentT<Self>` holding **typed fields**; the 9 layout
  containers share one `ContainerComponent` dispatched by a closed `Kind` enum. `GUI` stores
  `shared_ptr<GUIComponent>`; `create()` virtual-dispatches `getComponent()->createWidget(ctx)`.
- **PIMPL kept:** `createWidget` overrides are *declared* in public headers, *defined* in
  `GUI.cpp`/`PlotWidget3D.cpp` next to the private `*GUIWidget` classes. `CreateContext`
  {gui, parentLayout, parentProxy, parentWidget} replaces `GUIDefinition`.
- **Deleted:** the `register_widget_type` string registry + `create_widget` + `GUIDefinition`
  (.h/.cpp), `GUIComponent::m_params`/`toString()`/`form_args_*`, `encode_pointer`/
  `decode_pointer` (Prop now carries the `Configurable*` directly), the whole `detail::`
  factory layer, ui.h `applyCommon` + the `Component` concept, the dead `MultiDrawGUIWidget`.
- **Net win:** no string-keyed registry, no `GUIDefinition` round-trip, no comma-joined param
  channel — so a Label's text (commas included) is a typed field that can never be mangled.
  `tests/test-gui-definition.cpp` rewritten to assert this (typed payloads survive, clone()
  keeps the dynamic type, border-wrap) with no QApplication.

Remaining GUI polish (deferred): refresh stale `\code` doc-comments; the `ui.h` filename is
now a historical artifact (could be renamed/merged). `GUISyntaxErrorException` is likely dead.

---

## Session 76 recap, part 1 (ui-plan Phase 7 — GUI string round-trip retired + ui::→qt:: promotion) — committed

Full Phase 7 landed in one session (A→C→B), build + **926/926** green at each step
(919 prior + 7 new GUI tests). Branch `further-restructuring-and-cleanup`, **not yet
committed.** Memory `project_ui_namespace_endgame` + `ui-plan.md` Phase 7 marked done.

- **7A — killed the string round-trip (the bug fix).** Every component used to serialize
  to a `GUIDefinition` string (`label(TEXT)[@handle=..]`) that `GUI.cpp` re-parsed, so any
  free-text payload with `, ( ) @ =` threw `"Syntax Error … Widget could not be created"`
  at runtime (the `encode_pointer` hex fix was a point-patch of the same class). Now:
  `GUIDefinition` has a ctor straight from a `GUIComponent` (copies type / param-vector /
  handle / label / tooltip / sizes — no grammar); `GUI` carries a structured
  `shared_ptr<GUIComponent>` (`getComponent()`); `create()` + `to_string_recursive` use it,
  the `GUIDefinition(string)` parse path survives only for explicit legacy `GUI(string)`
  nodes. `operator<<(GUIComponent)` / `operator<<(GUI&)` do the label→border wrap
  structurally (`GUI::makeBorderComponent`, mirroring the old +1-cell border-size quirk).
  Headless regression test `tests/test-gui-definition.cpp` (metachar payloads + label-wrap
  via `createXMLDescription`, no QApplication/GL).
- **7C — retired legacy fluent usage.** The old `qt::Xxx` GUIComponent factories
  (`GUIComponents.h`, `ContainerGUIComponents.h`) moved to **`icl::qt::detail`** (still the
  wire-param encoders the public components delegate to via `toComponent()`). The last
  framework-internal fluent sites (`Quick.cpp`, `CamCfgWidget.cpp`, `ChromaGUI.cpp`)
  migrated via `scripts/ui-migrate.py`.
- **7B — promoted + stripped.** The designated-init structs moved `qt::ui::`→`icl::qt`;
  the `ui::` qualifier was scripted-stripped across **116 files**. Call sites now write
  plain `Slider(0,255,42,{.handle="x"})`. Name collision: the old `qt::ToggleButton`
  *widget* (a QPushButton) was renamed **`qt::ToggleButtonWidget`** (files
  `ToggleButtonWidget.{h,cpp}`) to free the component name. Non-`<<` fluent stragglers the
  chain-only script skipped (ternaries, `GUI x = Component(...).handle(...)` in
  flood-filler / depth-camera-simulator) hand-fixed to `Component(..., {...}).toComponent()`.

Deferred polish (non-blocking): retire the now-rarely-reached `operator<<(const string&)` +
`GUIDefinition(string)` parse path once no `GUI(string)` caller remains; fold `ui.h` into
`GUIComponents.h`; refresh old fluent snippets in `\code` doc-comments.

---

## Session 75 recap (mouse-handler chain redesign + paper fold/crease maturation) — committed

Committed across `e2c46f883`, `1af682cbb`, `fca8040a5`, `5b9788cdc` (+ the now-superseded
`794344adf`). 919/919 green; demo runs. Big arcs:

- **Mouse-handler framework redesign** (the structural one). `qt::MouseHandler::process()`
  now returns **`MouseResult { Processed, Forward }`**; `ICLWidget` keeps an **ordered
  handler vector** and dispatches front→back, stopping at the first `Processed` (was a Qt
  signal/slot broadcast). install order = priority; `AbstractPlotWidget` honours it too.
  **Camera nav (`Scene2MouseHandler`) is now an ordinary chain member installed last** —
  `PaperMouseHandler` / physics2 `PhysicsMouseHandler` / physics `PhysicsMouseHandler2`
  no longer *inherit* it; they return `Forward` to let the camera take over. Migrated all
  ~22 `process()` overrides (qt/geom/cv/physics) to the enum (legacy ones return `Forward`
  to preserve old broadcast). Demos install `[interaction, camera]`; `setSensitivities`
  moved to the camera handler.
- **`ui::StatusBar`** — bottom-docked thin strip (24px), built-in left-aligned `"status"`
  label (`gui["status"] = str(...)`), L/R margins for rounded corners. Inherits
  `ContainerGUIComponent` directly (no legacy `qt::StatusBar`). Demo `statusbar`.
- **Paper screen-line fold (works anywhere):** the camera centre + the press/release rays
  span a **cut plane**; `PaperDriver::projectScreenLine` returns the crease as that plane's
  intersection with the faces, so the drag may start/end off the sheet. `PaperMouseHandler`
  is modifier-based (Ctrl=fold, Shift=grab, Shift+Ctrl=sheet; never orbits while a modifier
  is held).
- **Creases as geometry primitives → `FoldMap` retired.** `PaperDriver::Crease {a,b,
  stiffness,memorized}` is the source of truth; `createBendingConstraints` reduces bending
  links that cross a crease via exact `segments_cross` (no raster → no missed long
  crossings; creases extended ~1cm past the edges to avoid edge tunneling). Flat
  paper-space rest lengths. `detail/FoldMap.{h,cpp}` deleted (legacy `physics/FoldMap.*`
  untouched). `adaptFoldStiffness` edits the primitive.
- **Debug viz:** `getDebugGeometry()` → faces (gray) / 1st-order (green) / 2nd-order
  (orange, crease-reduced hidden) / creases (yellow), toggled by demo checkboxes; the
  foldmap Display is replaced by a **2D `ui::Canvas` "pseudo paper"** drawing creases in
  A4-portrait paper space. Three driver Props share a **Tab** so the canvas fits.
- **Camera:** default-scene near plane dropped to `ext*0.005` (zoom right up to detail);
  paper demo starts the camera 2× closer to the sheet.
- **Test:** `physics2.paper_fold_reduces_bending`.

Open M3 follow-ups: front/back **texture rendering**, **hover + context-menu crease
editing** (adaptFoldStiffness UI). The 🔴 URGENT GUI-string-layer retirement still stands.

---

## Session 74 recap (Phase 3b — paper as composable drivers) — committed (Session 75)

Transplanted the crown-jewel `PhysicsPaper3` into physics2, **not** as a monolith but
split into a **substrate + behaviour drivers** on one `MeshNode` (the user's framing:
"test how drivers combine and dispatch"). 918/918 green; demo runs.

### ⏸️ OPEN ITEMS at break (resume here — physics2-paper demo polish)

All logic/tests green (918/918, 27 physics2); these are GUI-demo issues needing a real
display (the user is iterating live on `physics2-paper-demo`):

1. **Help label text not visible.** Added a bottom `ui::Label` (handle `help`, then
   `gui["help"] = std::string(...)` after `Show()` to dodge the GUI-string comma/paren
   parse bug). The label widget appears but shows no text. The working pattern is
   game-of-life `demos/game-of-life.cpp:344-345,456`: `ui::Label("---",{.handle="info2",
   .label="info2"})` + `gui["info2"] = std::string(...)`. **Diff to try:** mine has
   `.maxSize={100,2}` and **no `.label=`**; theirs has a `.label=` and smaller/no
   maxSize. Next: add `.label=`, drop the width-100 maxSize, confirm the handle is a
   LabelHandle. (Same file, `init()`.)
2. **Confirm Fold actually creases on screen.** Interaction is now a modifier-free
   **tool combo** (Camera/Fold/Grab/Sheet via `PaperMouseHandler::setTool`); left-drag
   in a tool is fully consumed (never orbits), right-drag/wheel always orbit/zoom. The
   earlier "fold works like camera" was off-paper presses delegating to the camera
   (fixed: consume). User reported "only grab works" pre-combo — needs a fresh on-display
   check that Fold now creases (cyan preview while dragging, yellow crease after; drop
   **bend range** ~0.3 to see it hinge). A diagnostic ERROR_LOG was added then removed.
3. **Overlay colors fixed** — `MeshNode::addVertex/addLine` take **0..255** colors
   (`*1/255` internally); were passed 0..1 → transparent-black (invisible). Demo overlay
   (crease yellow / preview cyan) + `PhysicsScene` debug wireframe now 0..255. *Should*
   be visible now; user hadn't confirmed at break.
4. **2D Canvas layer not ported (by choice).** Legacy `physics-paper3.cpp:161-163` drew
   the fold highlight in **2D** via `DrawHandle3D::draw(VisualizationDescription)`
   (screen-projected `getFoldLineHighlight`). The rework does crease/preview as **3D**
   overlay lines (`getCreaseSegments` + a render-on-top MeshNode). The 2D layer still
   works (`ui::Canvas3D` is a `DrawHandle3D`); decide if any 2D HUD/text is wanted.
5. **`PaperMouseHandler.cpp`** still `#include`s `icl/utils/Macros.h` (was for the
   removed ERROR_LOG) — harmless, can drop.

Architecture note (settled this session): there is ONE installed mouse handler;
`PaperMouseHandler : Scene2MouseHandler` IS the priority chain (paper logic first,
delegates to camera base only when it chooses not to consume). `Scene2`'s
`mouseHandlers` vector is only a sensitivity cache, not event-routed. A generic
N-handler registry was discussed but deemed unnecessary.

See also the 🔴 URGENT TODO above (retire the GUI-definition string layer — root cause
of the label parse crash + the earlier `encode_pointer` crash).

---


- **`PaperDriver`** (substrate, `PaperDriver.{h,cpp}`): the Bullet-level paper logic
  transplanted ~verbatim — manually-built **dual-mesh** `btSoftBody` (corner grid +
  per-cell centre vertices), `LinkState` fold metadata on `Link::m_tag`, `FoldMap`
  crease memory (copied to `physics2/detail/FoldMap.{h,cpp}`), the fold primitives
  (`addLink`/`addTriangle`/`addVertexOrReuseOldOne`/`replaceTriangle`),
  `createBendingConstraints`, paper-space picking (`hit`/`interpolatePosition`/
  `getLinkCoords`), Gaussian `dragPoint`, `wholeSheetMove`, `adaptFoldStiffness`.
  Legacy `icl2bullet*` macros → `physics2::Units`. Runs **SoftRigid only** (asserts).
- **Dynamic topology membrane:** new `PaperStateBuffer` (in `StateBuffer.h`) carries
  node positions + a **structure version** + a triangle-list snapshot. A fold (sim
  thread) bumps the version + republishes topology; `sync` (UI) copies positions every
  frame and **rebuilds the MeshNode topology only on a version change** — paper grows
  nodes/faces, unlike cloth's fixed mesh. This is the one genuinely new piece vs.
  `SoftBodyDriver`.
- **Composable behaviours:** `FoldDriver` + `PaperMoverDriver` own *no* physics — they
  attach to the same node, resolve the substrate via `node()->getDriver<PaperDriver>()`
  in `onAttach`, hold their own Configurable tunables (auto-extend / drag strength+
  radius), and forward to the substrate. `PaperMouseHandler` dispatches by modifier:
  **Ctrl=fold, Shift=soft-drag, Shift+Ctrl=whole-sheet move, plain=orbit**, routing all
  mutations through `world.enqueue` (closes the legacy `// TODO IMPLEMENT LOCKER`).
- **Thread-safety:** all fold/drag/move ops enqueue onto the sim thread; the const
  pickers lock the world (`std::scoped_lock<PhysicsWorld>`). The dual-mesh `appendNode`
  reallocations are safe — Bullet re-points link/face node ptrs.
- **Backend-aware props (M0, the user's other ask):** `SoftBodyDriver` now only
  exposes `position iterations` / `collision mode` in SoftRigid mode (the deformable
  solver ignores them) — `ui::Prop` shows only what applies. `PaperDriver` follows the
  same per-mode discipline.
- **`PhysicsScene::addPaper(cells, corners, …)`** factory; demo **`physics2-paper`**
  (one node, 3 drivers, 3 Prop panels, fold-map display, debug toggle).
- Tests: `paper_builds_and_drapes`, `paper_fold_grows_topology` (fold y=0.5 on an even
  grid is the degenerate fold-through-a-vertex case — use a diagonal), `paper_hit_and_
  interpolate`, `paper_composed_drivers_dispatch`, `cloth_props_are_backend_aware`.

- **New ICLQt feature `qt::ProgressContext`** (`ProgressContext.{h,cpp}`): an RAII
  modal progress dialog — `qt::ProgressContext p("title"); p = 42.f;` (percent), closes
  when it leaves scope. PIMPL (no Qt in the header); all Qt work marshals to the GUI
  thread via `ICLApplication::executeInGUIThread` (ctor/dtor block so lifetime matches
  the object; updates are async/FIFO); an **inert no-op when there's no GUI app**
  (headless/tests). Demo `progress-context` drives it from the ICLApp worker thread
  (the cross-thread case). Wired into the paper rebuild: `PaperDriver::
  setProgressCallback(fn(frac))` — the *property-driven* bending rebuild (bend range /
  fold softness / self collision) now runs **synchronously on the GUI thread under the
  world lock** and reports progress, so the demo drives a `ProgressContext` inline (no
  marshal → no deadlock); fast folds don't report. `ProgressContext` gained
  `minDurationMs` (quick rebuilds never flash) and a `modal` flag — **non-modal +
  WA_ShowWithoutActivating** for live-widget-driven ops (a modal bar stole focus mid
  slider-drag and killed the drag; the demo's rebuild bar is now non-modal). physics2
  stays qt-free — the callback is `std::function<void(float)>`, the qt bar lives in the demo.
- **Crash fixed (SIGSEGV in the paper demo):** scene `MeshNode`s were mutated off the
  `Scene2` lock — but `Scene2::sync()` and `render()` both hold its recursive mutex, so
  mutating a node outside it races the GL thread. Fixed the demo's per-frame overlay
  rebuild (`std::scoped_lock<Scene2>`; gather creases first since `getCreaseSegments`
  locks the *world*) AND the same latent bug in `PhysicsScene::sync`'s debug overlay.
- **Interaction is now a modifier-free TOOL COMBO** (final): keyboard modifiers proved
  unreliable here (Ctrl = camera speed; Alt silently dead on the user's setup → "only
  grab works"). Replaced with a `PaperMouseHandler::Tool` (Camera/Fold/Grab/Sheet) set
  from a demo `ui::Combo`; a LEFT-drag on the paper does the selected tool, right-drag +
  wheel always orbit/zoom. `ProgressContext` was removed from the paper demo (rebuild is
  fast — reverted to the async/enqueue rebuild for a smooth slider); the class + its
  standalone `progress-context` demo stay.
- **Interaction polish (earlier, superseded by the tool combo):** (a) **modifier conflicts** — the camera owns
  Left/Shift/Ctrl (orbit speeds) + Shift+Ctrl (place cursor), so fold moved to **Alt**,
  sheet-move to **Alt+Shift**; soft grab stays on **Shift** (intercepted on-paper). Alt
  is free in every `Scene2MouseHandler` mapping. (b) **drag now holds** — the soft drag
  was a one-shot velocity nudge (fell back the instant the mouse stopped); replaced with
  a **persistent kinematic grab** (`PaperDriver::beginGrab/updateGrab/endGrab`): pin the
  grabbed patch (inverse-mass 0, offsets preserved), follow the cursor, restore mass on
  release. Whole-sheet move (Alt+Shift) is the *same* grab with a radius covering every
  node (rigid carry, holds too) — `wholeSheetMove`'s one-shot translate had the same
  fall-back bug. Test `paper_composed_drivers_dispatch` asserts lift→hold→drop.
- **Picking-coordinate bug fixed (mouse interaction "taken by the camera"):** the
  mouse handlers cast rays with `cam.getViewRay(e.getPos())` — raw *widget* pixels —
  but `getViewRay`/`estimate3DPosition` expect *camera-resolution* pixels
  (`e.getRelPos() * cam.getResolution()`, per `Scene2MouseHandler::placeCursor`).
  They only coincide when widget size == camera res, so `paper->hit` always missed and
  every gesture fell through to camera orbit (orbit survived because it uses *deltas*).
  Fixed in `PaperMouseHandler` **and** `PhysicsMouseHandler` (the scene/tilt grab
  handler had the same latent bug — interactive grab was never exercised, only a
  manual-ray unit test). Also: accept Ctrl *or* Meta for the fold gesture (Qt remaps
  physical Ctrl→Meta on macOS). Paper demo slab resized to 1500×1500×30 (5× wide/long,
  1/10 height).
- **Startup-hang fixed (the "no window" report):** `createBendingConstraints` added
  bending links via `appendLink(checkExist=true)`, whose `checkLink` scans the whole
  growing link list → O(n⁴) over the all-pairs build. With the demo's dense grid +
  the legacy `maxLinkDist=0.5` (a disk covering ~78% of the sheet → hundreds of
  thousands of links) it spun for many seconds at `init()` (no window). Fix: dedup via
  a `std::set` (O(n²·log)) + `appendLink(checkExist=false)`. (Default `maxLinkDist` is
  **2.5** — fully connected = max stiffness, "papery", per the user; the set-dedup keeps
  the build feasible (~1.6s init for the 20×20 demo). The `bend range` slider lowers it
  live if a fold rebuild feels laggy — each fold re-runs `createBendingConstraints`.)
- **Framework bug fixed (found via the demo's 3 Prop panels):** `qt::encode_pointer`
  (the `ui::Prop(Configurable*)` pointer-smuggling) memcpy'd the raw 8 pointer bytes
  into the GUI definition string — if any byte collided with the grammar delimiters
  (`(` `)` `,` `@` `=`) the tokenizer threw "Syntax Error … Widget could not be
  created". ASLR-dependent → intermittent (one Prop usually lucky, three not). Fixed at
  the source: `encode_pointer`/`decode_pointer` now **hex**-encode (only safe chars).
  Benefits every `ui::Prop` user.

**Caveat:** GUI/texture/fold visuals unverifiable here (no GL); physics + composition
are deterministic-`stepOnce` tested. Build: `CCACHE_DISABLE=1 PATH=~/Qt/6.11.0/macos/
bin:$PATH ninja -C builddir -j 16`. Tests: `builddir/bin/icl-tests -f 'physics2.*' -j 1`.

---

## Session 73 recap (btDeformable soft-body world + cloth fixes) — committed `a6a0c4cd5`

**The flagged blocker is fixed.** `btSoftBody`-explodes-at-rest is gone: cloth now runs
in a `btDeformableMultiBodyDynamicsWorld` whose contact projection (split-impulse + ERP)
is stable at rest, where the legacy impulse contacts pumped energy. (The win is the
*contact solver*, not FEM — cloth still uses mass-spring.) Started from a standalone
Bullet spike (see memory `project_btdeformable_spike`); landed as:
- **`PhysicsWorld` `SoftBodyMode{Deformable,SoftRigid}` ctor flag, default Deformable.**
  Both worlds collapse to a common `btDiscreteDynamicsWorld*` so all rigid/step/sensor/
  debug/force-field machinery is solver-agnostic; only soft-body calls branch. Full
  deformable solver-info recipe is mandatory (without it the implicit CG solve freezes /
  contacts tunnel). `PhysicsScene(SoftBodyMode)` passthrough.
- **Per-cloth forces.** Bullet's `addForce` MERGES forces by type (one shared force across
  all cloths) → a per-cloth stiffness change freed a force still in use → SIGSEGV on slider
  change. Fix: manage the solver's force list directly (`getLagrangianForceArray()`),
  bypassing the merge — each cloth owns its mass-spring + gravity force.
- **`SoftBodyDriver` deformable path:** `SDF_RD|SDF_RDF` contacts, `appendDeformableAnchor`
  (legacy `appendAnchor` is ignored by the deformable solver), no clusters.
- **Node-mass-proportional stiffness** (`deformParams`, `kDeformBase=8000`): `k ∝ node
  mass` keeps `ω=√(k/m)` resolution-independent, so the slider is stable at any grid
  density (a fixed stiffness exploded the demo's 60×60 cloth while passing 16×16 tests).
- **Smooth shading actually works now:** the core `Renderer` honors
  `GeometryNode::getSmoothShading()` (true per-face normals when off; was ignored — only
  the legacy `SceneSynchronizer` read it), and the cloth wires triangle normal indices so
  its `createAutoNormals` array is used. `"smooth normals"` cloth property toggles it.
- **Demo tuning** (from the user's `defaults.json`): stiffness 0.85 / size 1.4 / 50×50 set
  per-cloth (0.85 is mass-2-specific, NOT a global default — it explodes light cloths);
  blue cloth 25% reflective.
- Tests: `cloth_stable_at_rest` (2500-step bounded), `dense_cloth_stable`,
  `live_stiffness_two_cloths` (the crash repro), `legacy_softrigid_mode`. **913/913 green.**

Also committed `ddbfcdb0b` — three plan-doc TODOs (material database, multi-world
`PhysicsScene`, coupled/"fused" worlds via cross-world penalty bridging).

**Caveat:** GUI render correctness is unverifiable in this sandbox (no GL context). The
deformable physics is validated via deterministic `stepOnce` + the tests; the smooth/flat
toggle, reflectivity, and drape *feel* need a real display (the user confirmed cloth looks
great + sim is stable). Build: `CCACHE_DISABLE=1 PATH=~/Qt/6.11.0/macos/bin:$PATH ninja -C builddir -j 16`
(ccache trips the sandbox). Tests: `builddir/bin/icl-tests -f 'physics2.*' -j 1`.

---

## Session 72 recap (`DefaultScene` + default physics scene + live-tunable soft bodies)

Commits `cec2b26e5` + `1fd0808ad`.
- **`geom2::DefaultScene : Scene2`** — self-furnishing scene, `SceneType{Void,
  Studio}`, up-axis aware (`setUpAxis`, Y viewer / Z physics). Live Configurable
  knobs (scene type / ground / coordinate frame / sky / shadows / SSR); toggles
  flip node visibility (no rebuild → camera preserved). Big checkerboard floor
  (GL_REPEAT tiling), lamp rig, camera aimed at content (not origin).
- **Renderer**: procedural **sky-as-background** pass (`setSkyEnabled/Up`); honor
  `enable lighting` + a `debug` viz menu (normals/albedo/…); **per-texel
  reflectivity map** on `geom::Material`. `Scene2MouseHandler` pan/dolly/wheel
  speeds retuned.
- **`PhysicsScene::setupDefault(SceneType, extent)`** — composes a `DefaultScene`
  + invisible static ground collider coincident with the drawn ground; migrated
  `physics2-scene/-tilt/-cloth`.
- **`SoftBodyDriver` is now `Configurable`** — live cloth params + reset button.
  (The S72 anti-explosion band-aid is now obsolete — superseded by the S73
  deformable world.)
- Minor still-open: `TextNode` aborts headless (eager Qt-font raster) —
  `CoordinateFrameNode` unusable GUI-less; defer its rasterization.

---

The notes below predate the physics2 work (Session 70 and earlier) and remain
valid background on the broader geom→geom2 migration.

**This session (69):** built `physics-water-rocket` (geom2-rendered water-rocket
+ Bullet soft-body parachute) and, in the process, fixed several framework bugs:
- `fa57da1c8` RigidConvexHullObject vertices-only ctor wasn't delegating (null body)
- `305bbdc03` Scene2 background-color type mismatch (crashed every geom2 render)
- `5622db449` PhysicsWorld soft-body air-density units (1000× too dense)
- `e11c56d8b` the demo; `590a1e5a6` next.md notes
Surfaced geom2 gaps (now noted below): `MeshNode` dynamic geometry doesn't
auto-invalidate the render cache; offscreen `Scene::render()` unimplemented in
the core-profile pipeline.

**Earlier:** io modernization DONE; qt::ui:: app/demo migration DONE (Session 68;
`ui-plan.md` Phases 1–6 landed) — converter dry-run `0 chains, 0 skips` across
all 111 GUI files. qt:: **framework internals** migration DONE too
(`ui-internals-plan.md`): `GUI.cpp`/`Widget.cpp` converted — 74 auto-chains +
the hand-converted parent-taking/decl sites; added a `QWidget*` parent-ctor
overload to the 7 `ui::` containers in `ui.h`. Only GUI.cpp:884
`CamPropertyWidget : public Tab` base-ctor left as `qt::Tab` (case 2c).
877/877 green; icl-viewer smoke (offscreen) builds the migrated ICLWidget menu
cleanly (only the GL context fails, as expected). **Phase 7 promotion
(make `ui::Xxx` the storage type, retire `toString()`/GUIDefinition parsing,
then promote `ui::Xxx` into `icl::qt` and strip the qualifier) is now the sole
ui:: remainder.**

Branch `further-restructuring-and-cleanup`; 877/877 tests green; build clean
(`CCACHE_DISABLE=1 PATH=~/Qt/6.11.0/macos/bin:$PATH ninja -C builddir -j 16`).
Note: SSH/git push is blocked in this sandbox — the user pushes themselves.
Note: the compiler was upgraded to Apple clang 21 mid-session, which forced a
full rebuild (stale PCHs); rebuild with `CCACHE_DISABLE=1` if ccache errors.

### TODO: phase out `geom` → `geom2` (new rendering pipeline)

`geom2` (clean scene-graph: `Scene2`/`Node`/`GroupNode`/`GeometryNode`/`MeshNode`
+ GL 4.1 Core `Renderer`) is now the canonical rendering pipeline. The legacy
`geom::Scene`/`SceneObject` GL path is effectively dead: `Application.cpp` forces
a Core-Profile default `QSurfaceFormat`, so `Scene::getGLCallback` always
dispatches to the (incomplete) `geom::GLRenderer` — legacy `renderScene` never
runs, and offscreen `Scene::render()` is unimplemented in the new pipeline.
Net effect: anything still rendering through `geom::Scene` shows only the sky
gradient (objects don't draw). Surfaced while building `physics-water-rocket`
(now rendered via `geom2`, mirroring Bullet poses into geom2 nodes each frame).

Work to do:
- **Port ICLPhysics visualization to geom2.** `PhysicsScene : geom::Scene +
  PhysicsWorld` renders through the dead `geom::Scene` path, so all physics demos
  currently draw blank. Provide a `geom2`-based render path for physics — either a
  new `Physics`-aware `Scene2` integration, or formalize the "simulate in
  `PhysicsWorld`, render in `Scene2`, sync each rigid body's Bullet pose into a
  geom2 node (and soft-body nodes into a `MeshNode`) per frame" bridge that
  `physics-water-rocket.cpp` demonstrates. Migrate the other physics demos onto it.
- Port the remaining `geom::Scene`-based modules/demos/apps to `geom2`
  (markers + geom demos still use `geom::Scene`).
- Decide the fate of `geom::Scene`/`SceneObject`/`GLRenderer`: either finish the
  core-profile `GLRenderer` or delete the legacy scene layer once `geom2` covers
  all call sites.
- Until ported, `geom::Scene` demos render blank — track which demos are affected.

geom2 gaps surfaced while building `physics-water-rocket`:
- **`MeshNode` dynamic geometry doesn't auto-invalidate the render cache.** The
  `Renderer` caches each `GeometryNode`'s GL buffers on first build and only
  rebuilds on a global `Renderer::invalidateCache()`. Mutating `MeshNode`
  vertices per frame (cloth, soft bodies, point clouds) changes the data but the
  canopy draws frozen until you call the global invalidate (which rebuilds *all*
  nodes). Add a per-node dirty flag / `MeshNode::markGeometryDirty()` (or bump a
  geometry version the cache compares) so dynamic meshes re-upload only themselves.
- **`Scene::render()` (offscreen pbuffer) is unimplemented in the core-profile
  pipeline** — port it onto `geom2`/`GLRenderer` so headless render-to-`Img`
  works again (needed for tests/CI and offscreen tools).

### Future: parachute / soft-body deployment in `physics-water-rocket`

The demo renders a real Bullet soft-body canopy (round, vented, soft-body shroud
lines) that billows and descends well — but it deploys **already open**. True
*airstream self-inflation from a packed bundle* is **not feasible with Bullet's
soft-body aero** (`V_TwoSided` is a crude per-face drag/lift approximation — no
pressure field, no air actually entering the canopy). Doing it properly needs
soft-body **self-collision** (so folds don't tunnel; expensive/unstable in
Bullet) or real **fluid-structure interaction / CFD coupling**. Pragmatic middle
ground if a deployment *animation* is ever wanted: scripted assist (rest-length
release or a packed→open position blend over ~1–2 s) — reads convincingly but is
not emergent. See the Session notes; the soft-body bridge pattern lives in
`icl/physics/demos/physics-water-rocket.cpp`.

### What landed in the ui:: migration (Session 68)

9 commits (`65ca7b04c`…`5e8d173c6`), ~**1005 fluent chains** converted:
- **Stage 0** (`65ca7b04c`): 4 hand-converted exemplars (compressor-playground,
  video-player, marker-detection, swiss-ranger) to harvest the rules.  Added
  **`ui::ToggleButton(untoggled, toggled, initiallyToggled, {opts})`** (replaces
  the awkward `ui::Button(..,{.toggledText=..})` form; the legacy `"!"`-prefix
  initially-toggled marker folds into the explicit bool).  Restored **primary
  args to positional** for `Fps`/`State`/`Ps`/`Canvas`/`Canvas3D` (timeWindow /
  maxLines / updateFPS / viewport) via the two-ctor pattern — only optional
  tuning belongs in the `{.foo=bar}` pack.  See `project_ui_namespace_endgame`.
- **Stage 1** (`325da3297`): `scripts/ui-migrate.py` — conservative,
  spec-driven converter.  Per-component positional arity + trailing-ctor-arg →
  Opts maps + struct-field order (avoids `-Wreorder-init-list`).  Default
  dry-run; `--apply` writes + adds the `ui.h` include.  Only `Plot` is
  intentionally unsupported (opts-only, two overloads).
- **Stage 2** (6 module batches, `2da8219d4`…`341ab1bbc`): qt 80, cv 186,
  geom+geom2 359, markers 124, filter+math+core 163, io+physics 75.
  Two gaps the rollout surfaced + fixed: `ui::Label` needed an opts-only ctor
  (`Label().handle("x")` → `ui::Label({.handle="x"})`); converter setter-scan
  now tolerates the access dot trailing a line (`x.label("..").`⏎`handle("..")`).
- **Plot finish** (`5e8d173c6`): the 18 `Plot` sites hand-converted (float
  ranges → `.minX/.maxX/.minY/.maxY`, all-zero range omitted, `gl`→`.openGL`).
  Two braced-init narrowing fixes (`pa(..).as<float>()`, `utils::Size(..)`).

### ui:: follow-ups (NOT done)

- **Framework internals still on legacy** — `qt/GUI.cpp` (~85) and
  `qt/Widget.cpp` (~48) build the GUI machinery the `ui::` layer sits on; the
  converter targets only `apps/`/`demos/`/`examples/`.  User said internals are
  in-scope eventually; they're the natural lead-in to Phase 7.
- **Phase 7 — string round-trip retirement** (separate arc, TODO.md "Rework
  GUIComponent internal representation").  Once `ui::Xxx` becomes the storage
  type and `qt::Xxx`/`toString()`/`GUIDefinition` parsing fall away, the
  endgame (memory `project_ui_namespace_endgame`) is to **promote `ui::Xxx`
  into `icl::qt`** and strip the `ui::` qualifier — call sites write plain
  `Slider(...)` again.  MUST come after legacy retirement (else name clash).
- **`ui::Plot` ergonomics** — opts-only today; if `Plot` gets used a lot,
  consider a positional `(minX,maxX,minY,maxY)` form, but four bare floats read
  worse than designators, so probably leave it.

Refs: `ui-plan.md` (Phases 1–6 LANDED), `scripts/ui-migrate.py`,
memory `project_ui_namespace_endgame`.

### What landed in the io modernization (Sessions 65–67)

- **ImageSource/ImageSink rework COMPLETE** (`image-source-sink-plan.md`):
  Grabber→ImageSource/ImageSink; concrete backends + both contracts
  (`SourceBackend`+`SinkBackend`) in `io/detail/`; registries +
  `DeviceDescription` stay public.  `-i list`/`-o list` harmonized through one
  shared printer (`io/detail/BackendListing.h`); descriptions live in the
  `PluginRegistry` Entry as `"paramHint~explanation"`.
- **io subdir rename**: `output/`→`sink/`, `detail/grabbers/`→`detail/sources/`,
  `DC/PylonGrabberThread`→`*SourceThread`; empty `grabber/` removed.
- **Full internal grabber→source token sweep** across io.  Protected: Basler
  Pylon vendor `IStreamGrabber`/`StreamGrabber`/… family + retired
  `SharedMemoryGrabber` proper noun.
- **icl-pipe `-fps` crash fixed** (declared `float=15.0`, was read `.as<int>()`).
- As-built summary: memory `reference_image_source_sink.md`.

### io follow-ups (NOT done — tracked in TODO.md, none blocking)

- **`icl-pipe` `-i list`** still needs a dummy 2nd sub-arg (ProgArg arity;
  `project_progarg_rework.md`).
- **`LibAVVideoWriter`→`LibAVSink`** — fold into the FFmpeg 6/7 rewrite
  (`project_ffmpeg.md`; unbuilt).
- **SDK-gated source backends** (pylon/openni/dc/kinect/optris/xi/sr/ps)
  renamed but compile-unverified here — build when a dep is enabled.
- **Generic `ImagePipeline`** (exploratory) — `source >> filter >> sink`
  stream-operator DAG (Display as a Sink?); BinaryOps → not strictly linear.

### Pre-existing io/ Next Step (Session 64, still open)

Session 64 finished the `io/detail/` audit (file-plugins,
compression-plugins, network, orphan grabbers).  Branch was 239 commits
ahead then; the orphan source backends (Optris/PixelSense/SwissRanger/Xi)
remain compile-unverified (no SDK deps) — build them when a dep is
enabled.  Latent `LibAVVideoWriter` FFmpeg 6/7 rewrite still pending
(`project_ffmpeg.md`).

### Remaining io subsystems (post Session-64)

In `icl/io/detail/`:

- **`detail/file-plugins/`** — DONE (Session 64).  PNG writer got a
  `png.compression-level` tunable; JPEG writer now delegates to
  JPEGEncoder (the 3-file JPEG split carries zero duplication now);
  ImageMagick writer deliberately left without a quality knob (no-op
  for ~all its formats).  Fixed the broken plugin-prefix wiring.
- **`detail/compression-plugins/`** — DONE (Session 64).  Audited,
  already modern (Configurable tunables + capabilities + codec-params,
  Image-typed); no changes needed.
- **`detail/network/`** — DONE (Session 64).  Clean apart from one dead
  `ImgBase.h` include in WSGrabber.cpp (removed).
- **`detail/grabbers/`** — orphan backends (Optris/PixelSense/
  SwissRanger/Xi) migrated to `Image acquireImage()` (Session 64), BUT
  these are NOT in meson.build (proprietary SDK deps unavailable), so
  the migration is mechanical/unverified-by-compile.  **First thing to
  do when any of these deps is enabled: build that backend and confirm
  the acquireImage() shallow-wrap idiom compiles.**
- **`detail/libav/`** — LibAVVideoWriter; per memory `project_ffmpeg.md`
  still needs the FFmpeg 6+/7+ API rewrite.  Not new, not yet touched.

### Image metadata persistence (reference — established Session 64)

ROI + timestamp round-trip is format-dependent:
- **ICL container formats** carry it: PNM/PGM/PPM/ICL (header `# ROI` /
  `# TimeStamp` comment lines, parsed back by the PNM reader); BICL/rle/
  jicl + the WS transport (the ImageCompressor wire envelope encodes
  full ImgParams incl. ROI + timestamp_us).
- **Foreign/lossy formats** store pixels only: JPEG, PNG, ImageMagick.
  JPEG never round-tripped ROI/time even before Session 64 — the writer's
  JPEG_COM markers were never read back (decoder's `jpeg_save_markers`
  was disabled).  To add JPEG metadata you'd re-enable `jpeg_save_markers`
  in JPEGDecoder AND re-add marker writes in JPEGEncoder — both halves.

### Session-62/63 carryovers (suggested order)

- **Namespace alignment for moved subdirs.**  Sessions 61–62 reorganised
  utils/math/core/filter/io into `<module>/<topic>/` subdirs but files
  keep their original `icl::utils` / `icl::math` / … namespace.
  `utils::cl/` and `utils::prop/` remain the only path/namespace-
  symmetric examples.  User wants this as a separate scripted/clang-
  rewrite pass.

- **Filter session housekeeping.**  Backend-split proposal in
  `project_filter_dispatch_arch.md`, IPP cross-check, BackendProxy
  `backends(Backend b)` shorthand.

- **Carryovers from Session 60.**  Metal-OpenGL completion crash
  verify, OSDGLButton drift audit on the other toggle buttons,
  qt::Prop UI integration of compression capability flags, icl-edit
  demo Op prerequisites.

### Build environment (Qt 6.11 + sandbox notes)

- **Qt is now `~/Qt/6.11.0/macos`**, not homebrew.  meson is configured
  via `PATH=~/Qt/6.11.0/macos/bin:$PATH PKG_CONFIG_PATH=~/Qt/6.11.0/
  macos/lib/pkgconfig CCACHE_DISABLE=1 meson setup builddir`.  The
  `-Wl,-rpath,<qt_libdir>` is baked into every link via the project's
  `add_project_link_arguments` so binaries find Qt without
  `DYLD_FRAMEWORK_PATH`.
- **Qt 6.11.0 `qyieldcpu.h` bug**: the header calls `__yield()` under
  `__has_builtin(__yield)` without including `<arm_acle.h>`.  meson.build
  force-includes `<arm_acle.h>` on `darwin/aarch64` to make the prototype
  visible.  Drop this when Qt 6.11.1+ lands a fix.
- **ccache is blocked in the sandbox** → always prefix `CCACHE_DISABLE=1`.
- **Stale-PCH SDK-mtime fatal errors** after a toolchain bump:
  `find builddir -name '*.pch' -delete` then rebuild.
- **Build options enabled this session**:
  `-Dtests=true -Dapps=true -Ddemos=true`.  Tests live at
  `builddir/tests/icl-tests` (not in `bin/`); run with `-j 1` per
  `project_test_parallel_flakiness`.

---

## Current State (Session 65 — ImageSource/ImageSink rework, Stages 1–2)

Started the io/ acquisition+output API rework (`image-source-sink-plan.md`):
rename the camera-jargon "Grabber" surface to symmetric ImageSource /
ImageSink, hide backends, fix the output property-access gap.  877/877
tests green throughout; 254 commits ahead of origin.

### Commits

- `41423a9b5` plan doc; `597458046` **Stage 1** — `SinkBackend` base +
  `ImageSink` (was GenericImageOutput): holds `shared_ptr<SinkBackend>`,
  inherits Configurable, forwards backend tunables (so
  `sink.setPropertyValue("compression.mode",…)` works — the
  std::function-based predecessor couldn't).  Backends derive SinkBackend;
  registry returns objects via `REGISTER_SINK_BACKEND`.
- `731a19d1a` **Stage 2a** — GenericGrabber→ImageSource,
  Grabber→SourceBackend, GrabberRegistry→SourceBackendRegistry,
  GrabberDeviceDescription→DeviceDescription, REGISTER_GRABBER→
  REGISTER_SOURCE_BACKEND, dir `io/grabber/`→`io/source/`.  ~196 files.
- `29051e1cd` **Stage 2b** — 18 concrete `*Grabber`→`*Source`.
- `8eadd53e0` **PIMPL** — ImageSource all-out-of-line, forward-declares
  SourceBackend; `getGrabber()`→`getBackend()` (+ sink symmetry); header
  doc points at `-i list` instead of a hand-synced backend list.
- `d10c9a651` `-i list`/`@info` `std::terminate()`→clean exit.
- `7825aad1a` TextTable: separator-aware wrap + header-only rule;
  `523c1879a` left-align; `61db15227` Unicode box-drawing borders.
- `7e63c05c5` **utils::exit()** app-aware exit hook — ICLApplication
  installs a handler (flush + std::_Exit) so `-i list` doesn't spew
  QThreadStorage teardown warnings; non-GUI keeps std::exit.
- `dced6b5b8` **priority-list collapse** — `init(device, spec)` single
  token + bare spec (no "TYPE=" re-tag); empty/"auto" scans all backends.
- `4c9661cf1` TODO: ProgArg framework rethink (per-app stringly-typed arg
  specs, arity baked in at ~51 sites — see `project_progarg_rework`).

### Conventions established

- **Master/backend/contract split**: user type `ImageSource`/`ImageSink`;
  contract `SourceBackend`/`SinkBackend`; concrete backends `*Source`/
  `*Sink` (mostly `detail/`).  `getBackend()` is the escape hatch.
- **Master is a PIMPL** forward-declaring its backend — keeps the public
  header thin and decoupled from the backend's full definition.
- **utils::exit()** for library code that terminates after a one-shot
  diagnostic; the app front-end intercepts it.

---

## Current State (Session 64 — io/detail audit completion)

Closed the four `io/detail/` subsystems the Session 63 Next Step
queued (file-plugins, compression-plugins, network, orphan grabbers).
6 commits, mostly small but each fixing a concrete issue.  876/876
tests green throughout (875 baseline + 1 new regression test).

### file-plugins (4 commits)

- `49d58851a` **PNG writer compression-level tunable + fix broken
  plugin-prefix wiring.**  Two things in one commit:
  - FileWriterPluginPNG becomes a Configurable singleton exposing
    `compression-level` (zlib 0-9, default 4), replacing the hardcoded
    `png_set_compression_level(writer, 4)` and its "later you will be
    able to select this" TODO.  Verified: level 0 → 2.26 MB, level 9 →
    787 KB on the parrot test image, roundtrips.
  - **Bug fix**: the plugin-prefix feature shipped *broken* in Session
    63.  `REGISTER_FILE_WRITER_CONFIG(jpeg, "jpeg", FACTORY)` expands to
    `registerPlugin("jpeg", FACTORY)` — so "jpeg" is the registry KEY
    and `description` defaults to empty.  But
    `FileWriter::attachPluginConfigurables` prefixed children with
    `e.description` (empty), so `jpeg.quality` / `csv.extend-file-name`
    never resolved — they were added unprefixed.  Fixed by reading
    `e.key`.  New regression test `FileWriter.plugin_prefix.tunables_resolve`.
- `e95cb3b60` **FileWriterPluginJPEG delegates to JPEGEncoder.**  The
  writer reimplemented ~140 lines of jpeg_compress logic that
  JPEGEncoder already does.  Delegate, so there's one JPEG-compress impl
  in the tree.  Safe because the writer's TimeStamp/ROI JPEG_COM markers
  were write-only dead bytes — JPEGDecoder never calls jpeg_save_markers
  (commented out), so marker_list is always empty on read.  Drops the
  writer's m_bufferImage/m_bufferMutex + JPEGHandle.h dependency.  The
  3-file JPEG split (Encoder/Decoder/Handle) now has zero duplication —
  the clean answer to "3 files for one codec is unusual".
- `6fc164459` **PNG bit-depth comment cleanup** — bit depth is correctly
  derived from image depth (16 for single-channel depth16s, else 8);
  dropped the misleading "later you will be able to select this".

ImageMagick writer: considered a `quality` tunable, declined — it would
be a no-op for ~all of IM's formats (lossless tiff/gif/bmp; libjpeg/
libpng win the lossy ones by priority).

### compression-plugins (audit only, no commit)

Already modern: CompressionPlugin base is Image-typed (compress takes
`const Image&`, decompress returns `Image`), inherits Configurable, has
`capabilities()` + codec-params string.  All 6 plugins (raw/rlen/jpeg/
1611/zstd + base) clean, no TODOs, no dead code.  Remaining `ImgBase*`
uses are legitimate raw-data access (JPEGDecoder borrow pattern in the
jpeg plugin; Zstd/Raw planar memcpy helpers), not legacy ownership.

### network (1 commit)

- `bd3700b0f` **WSGrabber — drop dead ImgBase.h include.**  WSGrabber is
  fully Image-typed; the include was its only ImgBase reference.
  WSImageOutput already clean.  The pair is symmetric.

### orphan grabbers (1 commit)

- `fe8c4da0a` **Optris/PixelSense/SwissRanger/Xi acquireImage → Image.**
  Session 63 updated their *headers* to `Image acquireImage()` but left
  the .cpp on `const ImgBase* acquireImage()` — a signature mismatch.
  Finished: .cpp return type → `core::Image`; the single return site
  wraps the backend-owned buffer via `Image(const ImgBase&)` (shallow,
  shares pixel data, valid until next acquireImage — the documented
  Grabber contract); `override` on decls; each .cpp now includes
  `<icl/core/Image.h>` (Grabber.h only forward-declares core::Image).
  **These are not in meson.build (proprietary SDK deps unavailable), so
  the change is mechanical and follows the built-backend idiom but could
  NOT be compile-verified — see Next Step.**

### Follow-up io/ cleanup (3 commits)

- `19274ba8b` **JPEGDecoder — delete dead TimeStamp/ROI marker loop.**
  The reader walked `marker_list`, but `jpeg_save_markers` was commented
  out so the list was always empty — dead since forever.  Removed the
  loop + the commented call + now-unused StrTok.h / `<charconv>`.  No
  behavioural change.  (See "Image metadata persistence" in Next Step.)
- `5706e50a2` **Move TestImages to `grabber/` + hide createImage_xxx().**
  TestImages is the shared test-image factory; CreateGrabber is its only
  grabber-shaped consumer, but qt::create() + benchmarks also use it and
  it's public API — so it stays public, just relocated out of the
  too-prominent module top into `io/grabber/` (alongside the grabber it
  feeds).  The six `createImage_{parrot,windows,flowers,lena,cameraman,
  mandril}()` free functions are no longer declared in any header; their
  defs in `detail/builtin-images/*.cpp` became `static` (only ever used
  in-TU as the REGISTER_TEST_IMAGE factory).  Public surface is now just
  `TestImages::create(name, ...)`.
- `c7221f5ce` **jpg2cpp codegen modernized + qt::show doc fix.**  The
  generator now emits the exact shape the hand-maintained builtins use
  (`namespace icl::io {`, JPEGDecoder::decode into a vector +
  `core::Image(raw)`, trailing REGISTER_TEST_IMAGE) instead of the old
  FileGrabber-temp-file-on-disk form — verified the generated .cpp
  compiles.  Also fixed qt::show's doc comment (referenced the removed
  TestImages::show; it's io::show / ExternalViewer now).

### Conventions reinforced

- **Plugin-prefix on a façade**: the registry KEY is the prefix.
  `attachPluginConfigurables` must `addChildConfigurable(cfg, e.key)`.
- **One codec impl**: file-writer plugins that have an in-memory codec
  twin (JPEG) should delegate to it, not re-inline the compress loop.
- **Image-returning grabber hooks**: `return Image(*backendOwnedImgBase)`
  — `Image(const ImgBase&)` shallow-copies (shares pixel data), giving
  the "view valid until next call" contract for free.
- **Hidden plugin factories**: a self-registering factory (REGISTER_*)
  used only in its own TU should be `static` + undeclared in any header.
  Users reach it through the façade (`TestImages::create`), not by name.

---

## Current State (Session 63 — io/ audit + Qt 6.11 env)

A deep audit + modernization sweep across the public io/ surface.
21 commits, mostly mechanical at call sites but each one fixing a
real concrete issue (dead surface area, ImgBase*→Image migration,
template gymnastics → named methods, broken setOption,
plugin-Configurable lifting, force-link → link_whole).
875/875 tests green throughout.

### Build environment work (2 commits)

- `267364b7c` (carried over from Session 62 — io subdir reorg, finally
  built+verified+committed after the Qt environment was reconfigured).
- `d1d8e7cb5` **meson: Qt 6.11 darwin arm64 — arm_acle force-include +
  Qt libdir rpath.**  See "Build environment" in Next Step.

### io top-level cleanup (6 commits)

- `58a4ea9a1` **`io/proto/` removed** — RSB long gone from deps.
- `387f16d94` **`reset-dc-bus` app retired** — `reset-bus -t dc` is the
  generic equivalent that already chains into `DCDevice::dc1394_reset_bus`.
- `35b3a5739` **GenericGrabber ctor pattern at fn scope** — 3 sites
  collapsed `GenericGrabber g; g.init(pa("-i"));` → `GenericGrabber
  g(pa("-i"));`.  Globals deliberately left alone (pa() not callable
  at static-init time).
- `b14cb2849` **`icl::io::save` / `icl::io::load` introduced via thin
  SaveLoad.h.**  Out-of-line so the header costs only `<icl/core/
  Image.h>` + `<string>`.  Retires the qt-side duplicates (`qt::save`
  deleted, `qt::load(string)` deleted; `qt::load(string, format)`
  stays — format-converting variant — and forwards to `icl::io::load`).
- `3f2b8ddc1` **TestImages modernized to Image; show/xv extracted to
  ExternalViewer.h/.cpp.**  Registry factory type `Img8u*()` →
  `core::Image()`; 6 built-in JPEG generators flipped to `Image
  cached` pattern; jpg2cpp codegen template updated.
- `cea487213` **Grabber static translate helpers (SteppingRange /
  DoubleVec / StringVec) deleted** — zero callers tree-wide.  Backend
  `\copydoc` refs pointing at the soon-to-be-private `grab(ImgBase**)`
  funnel redirected to `acquireImage()`.

### Grabber audit (5 commits)

- `43a7b4b9c` **Grabber desired-params templates retired.**  4 parallel
  function templates with explicit specializations outside the class
  body + `grabber_get_null<T>` sentinel template + `grabber_get_xxx
  _dummy` force-instantiate hack collapse to 3+3+3+3 named methods
  (`useDesired(format/depth/Size)`, `getDesiredFormat/Depth/Size`,
  `desiredFormatUsed/Depth/Size`, `ignoreDesired{Format,Depth,Size}`
  + the all-axes `ignoreDesired()`).  ~14 explicit-template call sites
  migrated; ~30 overload-resolution sites unchanged.  3 external
  callers of `setDesiredSizeInternal` routed through `useDesired(Size)`.
- `20ef3b388` **`Grabber::acquireDisplay` retired.**  Renamed to
  `acquireImage`, made pure-virtual.  16 backends + the Qt
  Camera/Video grabbers + an in-tree InputGrabber in vector-tracker
  demo migrated.  `REGISTER_CONFIGURABLE_DEFAULT(Grabber)` replaced
  with a `Grabber_VIRTUAL` dummy subclass per the Configurable docs.
- `9c4110800` **Dead image-callback chain deleted.**  `using callback`,
  `registerCallback(callback)`, `removeAllCallbacks`,
  `notifyNewImageAvailable`, the callbacks vector + mutex in Data,
  the GenericGrabber forwarders, and the "very new experimental
  feature" doc-chapter — all unused tree-wide.  ~50 LOC.
- `a136a3c9a` **Grabber funnel collapsed; ImgBase* gone from public
  surface.**  `acquireImage()` returns `Image` (view of a backend-
  owned buffer, valid until next acquireImage — same lifetime
  contract as the old `const ImgBase*`, just typed nicer).
  `grab(ImgBase**)` deleted.  `adaptGrabResult` made private + Image-
  typed.  `Data::image` warp-output ImgBase* member → `Image
  warpBuffer`.  Backends migrated to either `Image m_buffer` directly
  or the borrow-ptr/re-adopt pattern around APIs that still want
  `ImgBase**` (mat_to_img, JPEGDecoder, WarpOp).  FileGrabber's
  vector<ImgBase*> bufferImages cache → vector<Image>.
- `ce04bc8ad` **`grabImage()` renamed to `grab()`.**  The name is free
  again now that the legacy ImgBase** overload is gone, and reads
  more naturally.  74 call sites migrated.

### Registry extraction (1 commit)

- `ebf0ac6da` **GrabberRegistry extracted to its own header/source
  pair.**  Was 60 lines glued onto the end of Grabber.h + 70 lines of
  impl on Grabber.cpp.  GrabberRegistry has no class-level coupling
  to Grabber (only a forward-declared `Grabber*` in CreateFn), and the
  split drops `<functional>`/`<set>`/PluginRegistry.h from Grabber.h's
  include footprint.  Grabber.h re-#includes GrabberRegistry.h at the
  bottom for backward compat.

### Output / FileWriter / FileGrabber (3 commits)

- `7a96c3a25` **GenericImageOutput**: `std::terminate()` after `-o list`
  → `std::exit(0)` (clean exit); BACK doc-comment refreshed to drop the
  stale "v4l" backend claim (V4L2LoopBackOutput.cpp doesn't actually
  invoke REGISTER_IMAGE_OUTPUT).
- `946684e3f` **FileWriter modernization** — three things at once:
  - FileWriter inherits `Configurable`, with each tunable plugin
    (FileWriterPluginJPEG, FileWriterPluginCSV) becoming a
    Configurable singleton and self-registering via the new
    `REGISTER_FILE_WRITER_CONFIG` macro.  FileWriter ctor walks
    `fileWriterConfigRegistry()` and addChildConfigurable for each
    under a named prefix.  Callers can now do
    `writer.setPropertyValue("jpeg.quality", 85)` — the broken
    `setOption("jpg:quality", ...)` (guarded on a never-defined
    `WITH_JPEG_SUPPORT` macro) is gone, along with the unused
    `send(Image)` shim and `operator<<(ImgBase*)`.
  - `write(const ImgBase*)` → `write(const Image&)`.  ~12 caller sites
    migrated.
  - Plugin singletons (function-local statics) — formerly the
    per-class statics `s_iQuality` / `s_bExtendFileName` /
    `s_oBufferImage` collapse to instance members.
- `eeac78474` **FileGrabber cleanup + `Configurable::
  setPropertyValueSilently`** — four things:
  - New utils API: `Configurable::setPropertyValueSilently` and
    `setPropertyValueTypedSilently` skip firing change callbacks.
    Solves the recursion-guard antipattern.
  - FileGrabber `updateProperties` uses the silent setter for the 6
    derived Info props; recursion-guard members
    (m_propertyMutex + m_updatingProperties) and the early-return
    in processPropertyChange dropped.
  - `frame-index` property setter had 3 latent bugs (read prop.as<int>
    twice, modulo against size-1 instead of size making the last file
    unreachable, a bare `Thread::sleep(0.2)` with no comment) —
    all fixed.
  - `forcePluginType(suffix)` had zero callers — deleted along with the
    `forcedPluginType` member and the conditional in `find_plugin`.

### ImageCompressor (1 commit)

- `ec1314cd9` **ImageCompressor — setCompression bug fix +
  link_whole + dead state.**  Three things:
  - **Bug fix**: `setCompression(spec)` was installing the codec
    twice.  The first install applied spec.quality; writing
    `prop("mode").value = spec.mode` fired the mode-change callback,
    which re-entered `installPlugin(mode, "")` — clobbering the
    codec-specific params with empty.  Net effect was that
    `setCompression({jpeg, "85"})` silently dropped to jpeg's default
    quality.  Fix uses the new `setPropertyValueSilently`.
  - **Force-link block removed (~30 LOC)**.  The
    `extern "C" { void iclRegisterCompressionPlugin_…(); }` block plus
    the `iclForceLinkCompressionPlugins[]` reference array existed
    solely to prevent macOS dyld from dead-stripping self-registering
    plugin .o files.  Move the 6 plugin TUs into a dedicated
    `static_library` (`icl-io-compression-plugins`) linked into
    libicl-io via meson's `link_whole`, which forces every .o to be
    retained regardless of external references.  Adding a new
    compression plugin no longer requires editing this file.
  - **`Data::decoded` dead member** dropped.  Comment claimed "kept
    alive for caller's pointer stability" but uncompress returns
    Image by value and nothing reads decoded back.

### Conventions / patterns established this session

- **Configurable plugin tunables on facades**: when a facade class
  (FileWriter, ImageCompressor) wraps a set of plugins that have
  per-plugin tunables, each plugin singleton inherits Configurable,
  self-registers a factory in a sibling registry, and the facade ctor
  iterates the registry to `addChildConfigurable` each under a named
  prefix.  See FileWriter.cpp for the pattern.
- **`setPropertyValueSilently`** for "I'm publishing a derived/Info
  value, don't re-enter the user's callback" cases.  Replaces the old
  m_updatingProperties / m_propertyMutex recursion-guard idiom.
- **`link_whole` for self-registering plugin .o's** — preferred over
  the force-link extern "C" boilerplate.  Use when adding a new
  always-built plugin family.
- **Rung 1 lifetime contract for `Image`-returning hooks**: the
  returned Image shallow-shares a backend-owned buffer, valid until
  the next call to the same hook.  Callers who need to retain it
  longer deepCopy explicitly.  Matches the documented contract
  Grabber::acquireImage() has now.

---

## Current State (Session 62 — io/ subdir reorg)

The `icl/io/` public surface was clustered into topical subdirs,
matching the core/filter protocol from Session 61.  Committed early
in Session 63 (`267364b7c`) after the Qt environment was
reconfigured against `~/Qt/6.11.0` and the build verified clean.

### What moved (via `git mv`, history preserved)

  io/grabber/   GenericGrabber.{h,cpp}, Grabber.{h,cpp},
                GrabberDeviceDescription.h
  io/output/    GenericImageOutput.{h,cpp}
  io/file/      FileGrabber.{h,cpp}, FileWriter.{h,cpp},
                FileList.{h,cpp}, FilenameGenerator.{h,cpp}
  io/compress/  ImageCompressor.{h,cpp}

Stays at module-top: `IO.h` (umbrella) + `TestImages.{h,cpp}`
(standalone utility).  `io/detail/` was already organized into
per-backend subdirs (dc/, file-plugins/, grabbers/, kinect/, pylon/,
opencv/, openni/, network/, v4l2/, libav/, compression-plugins/,
builtin-images/) and was left as-is — those are whole backends, not
the `*_<backend>.cpp` dispatch variants that the flat-detail rule
targets.

### Mechanics done

- ~139 include sites rewritten tree-wide via a per-file `perl`
  loop (`#include <icl/io/X.h>` → `#include <icl/io/<sub>/X.h>`).
  NOTE: bulk `perl -pi -e $FILES` and `grep -rlZ | xargs -0 perl`
  BOTH failed in this sandbox (whole file-list arrived as one
  ENAMETOOLONG arg).  The reliable pattern was
  `while IFS= read -r f; do perl -pi -e '…' "$f"; done < list`.
- `icl/io/meson.build` updated: split `io_public_headers` into
  `io_{grabber,output,file,compress}_headers` file() lists, updated
  `io_sources` paths, added per-subdir `install_headers(...,
  install_dir: .../icl/io/<sub>)` blocks (mirrors filter/meson.build).
- libicl-io.dylib compiles clean; all io-consuming TUs across qt /
  geom / cv / markers / physics / tests compile clean.  The only 4
  FAILED objects in a full `-k 0` build are the 3 QtMultimedia files
  (ICLVideoSurface.cpp + its moc, QtCameraGrabber.cpp,
  QtVideoGrabber.cpp) — pure missing-dependency, unrelated to io.

### Pending in this arc

All landed in Session 63 — Qt env reconfigured against `~/Qt/6.11.0`,
875/875 tests green, io reorg committed (`267364b7c`), io/proto/
removed (`58a4ea9a1`).

---

## Current State (Session 61 — utils/math/core/filter audit + subdir reorg)

A multi-week-equivalent reorg sweep across the four foundation
modules.  No single arc; instead a connected programme of audit
cleanup + topical clustering + backend-split consolidation.  Output
is dramatically cleaner module trees (filter top-level: 55 → 1
header; math top-level: 35 → 3; utils top-level: 43 → 26; core
top-level: 36 → 17), zero dead code in the audited modules, and one
consistent layout convention.

### utils/ (5 subdirs, 5 commits)

  utils/thread/    Thread, Lockable
  utils/plugin/    PluginRegistry, BackendDispatching, EnumDispatch
  utils/time/      Time, Timer, FPSEstimator, FPSLimiter, StackTimer
  utils/config/    Configurable, ConfigFile  (ConfigurableProxy
                   was retired in an earlier session)
  utils/dispatch/  AnyMap, Assign, AssignRegistry, AutoParse, ParamMap

Stays at top level: BasicTypes, CompatMacros, Macros, Exception,
Point, Size, Rect, Range, SteppingRange, Random, ClippedCast, File,
StringUtils, StrTok, TextTable, ConsoleProgress, VisualizationDescription,
FixedArray, Array2D, ProgArg, ProcessMonitor, SignalHandler,
UncopiedInstance, Yaml, Xml, Utils.h umbrella.

### math/ (6 subdirs, 7 commits + 1 cross-module relocation)

  math/fft/        FFTException, FFTUtils
  math/tree/       KDTree, Octree, QuadTree, PCLKdtree, PCLOctree
  math/transform/  HomogeneousMath, Homography2D, LinearTransform1D,
                   Projective4PointTransform, StraightLine2D,
                   ConvexHull (relocated from core/)
  math/fit/        LeastSquareModelFitting/2D, LevenbergMarquardtFitter,
                   PolynomialRegression, PolynomialSolver, RansacFitter,
                   SimplexOptimizer, StochasticOptimizer
  math/ml/         LLM, SOM, SOM2D, KMeans, GraphCutter
  math/la/         DynMatrix, DynMatrixBase, DynMatrixUtils, DynVector,
                   FixedMatrix, FixedVector, MatrixSubRectIterator

Top level: Math.h, MathFunctions.h, SimdCompat.h.

ConvexHull was a cross-module move with namespace change
(`icl::core::convexHull` → `icl::math::convexHull`); 4 consumer
sites + 2 `using namespace math;` adds.

### core/ (7 subdirs, ~10 commits, plus audit cleanup)

#### Audit cleanups (4 commits)

  - Retired `ImageRenderer.{h,cpp}` (zero consumers anywhere)
  - Retired `ImageSerializer.{h,cpp}` (BICL plugins use ImageCompressor)
  - Privatized `ImgBuffer.{h,cpp}` to `core/detail/` (only consumer:
    `qt/Quick.cpp`; iclquick-plan.md flags eventual replacement)
  - Privatized `CCLUT.{h,cpp}` to `core/detail/` (only consumer:
    sibling `CCFunctions.cpp`)

#### Subdir reorg

  core/cc/         CCFunctions, Color, Parable, ChromaClassifier,
                   ChromaAndRGBClassifier, BayerConverter
  core/convert/    Converter, FixedConverter
  core/line/       Line, LineSampler  (Line templated as LineT<T>;
                   Line32f.{h,cpp} retired)
  core/compat/     OpenCV (cv::Mat ↔ ICL interop)
  core/dispatch/   ImgOps, ImageBackendDispatching
  core/detail/     CCLUT, ImgBuffer (privatized), Img backend cpps
                   (Img_Cpp, Img_Ipp, Img_Accelerate)
  core/prop/       (existing) Constraints

Top level: Image, Img, ImgBase, ImgBorder→retired, ImgIterator,
ImgParams, Channel, PixelRef, Types, Visitors, VisitorsN,
DataSegment, DataSegmentBase, CoreFunctions (now also hosts the
former PixelOps API after merge), Core.h umbrella.

`DoxygenMainPage.h` relocated to `icl/` root (it documents the whole
framework, not core specifically).

#### Other core landmarks

  - `Line` templatized as `LineT<T>` (mirrors PointT/SizeT/RectT
    pattern from Session 48); `Line = LineT<int>`,
    `Line32f = LineT<float>`.  Line32f.{h,cpp} retired.
  - `ImgIterator.h` polish: deleted dead `inRegionSubROI`, trimmed
    ~200 lines of Core-2-Duo-era benchmark doxygen.
  - `OpenCV<4` support dropped: hard `#error` on cv<4, retired
    OpenSurfLib + LensUndistortionCalibrator + 1 app, modernized
    OpenCVCamCalib to the OpenCV 4 C++ API (cv::Mat / FileStorage).
    Net −3462 LOC.
  - `PixelOps.h/.cpp` merged into CoreFunctions (~270 lines of SSE2
    convert specializations + `copy`/`convert` templates).
  - `ImgBorder.{h,cpp}` retired; `Img<T>::fillBorder()` (existing
    4-overload API) now routes through ImgOps dispatch so the
    IPP-optimized `ipp_replicateBorder` path activates automatically
    for all callers (previously reachable only via the deleted
    facade).  `cpp_replicateBorder` lives in `detail/img/Img_Cpp.cpp`.

### filter/ (13 subdirs + flat detail/, ~17 commits)

Largest module so far — 55 top-level headers reduced to **just
`Filter.h` umbrella**.

  filter/morph/     MorphologicalOp, GradientOp, ChamferOp, MedianOp
  filter/arith/     {Unary,Binary,Inplace}ArithmeticalOp
  filter/logical/   {Unary,Binary,Inplace}LogicalOp
  filter/compare/   {Unary,Binary}CompareOp
  filter/affine/    AffineOp, RotateOp, ScaleOp, TranslateOp,
                    MirrorOp, WarpOp, ImageRectification,
                    ImageUndistortion, AffineWrappers.cpp
  filter/conv/      ConvolutionOp, ConvolutionKernel,
                    DynamicConvolutionOp, GaborOp, ProximityOp
                    (relocated — correlation = convolution)
  filter/fft/       FFTOp, IFFTOp, BaseFFTOp
  filter/threshold/ ThresholdOp, LocalThresholdOp,
                    LocalThresholdOpHelpers
  filter/color/     ColorDistanceOp, ColorSegmentationOp,
                    PseudoColorOp, DitheringOp (color-reduction op)
  filter/lut/       LUTOp, LUTOp3Channel, LUT2D, FixedConvertOp
  filter/advanced/  BilateralFilterOp, WienerOp,
                    MotionSensitiveTemporalSmoothing, CannyOp
  filter/base/      UnaryOp, BinaryOp, InplaceOp, NeighborhoodOp,
                    BaseAffineOp, OpROIHandler, UnaryOpPipe,
                    ImageSplitter, IntegralImgOp
  filter/channel/   WeightChannelsOp, WeightedSumOp
  filter/detail/    ALL backend cpps (~40 `*_Cpp/Simd/Ipp/Accelerate
                    /OpenCL.cpp` files), 10 `LocalThresholdOpHelpers
                    _<depth>_<bool>.cpp` template specialization
                    splits, `BilateralFilterOp.cl` + matching
                    `BilateralFilterOpKernel.h` (former `OpenCL/`
                    subdir absorbed)

The filter pass had three mid-stream fixup commits during the
threshold/ migration (a typo'd Edit parameter name caused a
meson-build incomplete edit; perl regex pass missed a few backend
cpps that included the public ThresholdOp / LocalThresholdOp
headers).  Final state clean.

### Convention recorded

  - Each module's public headers cluster under topical subdirs.
  - Backend dispatch impls (`*_<backend>.cpp`) all live under flat
    `<module>/detail/` (no hierarchy).
  - When a public header has a `*_<topic>` impl that's NOT a backend
    variant (e.g. `AffineWrappers.cpp` registers Configurable
    metadata), it stays alongside the public header in its subdir.
  - Cross-module relocations (e.g. ConvexHull, OpenCV interop) update
    the namespace to match the new owning module.
  - Namespace updates for the new subdirs themselves (e.g.
    `utils::time::Time`) are deferred — to be done in a follow-up
    pass with automated tooling.

875/875 tests green at HEAD.  48 commits in the session.

---

## Current State (Session 60 — codec caps + GL post-free + rlen rewrite + OSD scale-range)

A hardening session, no single arc.  Three threads ran in parallel:
the user's CONTINUE.md "next step" list (capability flags + OSD
button), a memory-safety chase that started inside the GL renderer
and ended inside rlen, and finally a clean template rewrite of the
RLE plugin.

### ICLWidget OSD scale-range button (`3a37f9184`)

Long-standing "button does nothing visible" bug.  Three layered
fixes in `icl/qt/Widget.cpp`:

1. **Root cause: `rebufferImageInternal` ignored `m_data->rm`.**
   The function pushed the manual BCI sliders into `GLImageRenderer`
   unconditionally, so flipping `rm` between `rmOff`/`rmAuto`/`rmOn`
   only took effect on the *next* `setImage()` call, not the current
   frame.  Now mirrors the rm-aware branch from `setImage()`.
2. **`OSDGLButton::stateFn` callback** (new field).  When set, the
   icon (toggled vs untoggled) is derived live from the callback
   each paint, instead of caching an internal `toggled` flag.  The
   scale-range button reads `m_data->rm != rmOff` so its icon stays
   in sync regardless of which UI surface (OSD button or `bci-mode`
   combo) last touched the mode.  `update_mouse_press` routes
   `bcb(!stateFn())` so external state ownership works without drift.
3. **`setRangeModeNormalOrScaled` round-trips through `rmOff`** via
   a new `Data::rmLastNonOff` field (default `rmAuto`).  Click off →
   `rmOff` and remember; click on → restore the previous non-off
   mode, preserving manual-BCI ("custom"/`rmOn`) configuration.
   `bciModeChanged` keeps `rmLastNonOff` updated when the user picks
   a mode via the combo.  Dropped the `create_menu` +
   double-`showHideMenu()` hack that flashed the menu on first OSD
   click.

Same commit also fixes:

- **Graceful init failure** — `ICLApplication::exec` wraps each
  user-supplied `init()` callback in a try/catch.  `icl-viewer -i
  crate cameraman` (typo'd backend) now prints a clean stderr message
  and exits 2 instead of letting `GenericGrabber::init`'s
  `ICLException` escape Qt's half-built event loop and SIGSEGV during
  teardown.
- **`create_menu` rebuild guard** — snapshot `bciAuto` from the old
  menu BEFORE reassigning `data->menu`, then drop both
  `bciUpdateAuto` / `channelUpdateAuto` to safe-stub lambdas during
  the build window.  Prevents `KeyNotFoundException: bci-update-mode`
  on `setMenuEmbedded` recreate.
- **CLAUDE.md** — replaced the obsolete CMake/ctest section with the
  actual meson/ninja workflow + `bin/icl-tests`, and added a
  permanent note about `QT_QPA_PLATFORM=offscreen` for non-interactive
  Qt-app testing in this sandbox.

### Codec capability flags (`d99753476`)

Closes the carryover from Sessions 55–56.  Each
`CompressionPlugin` declares its accepted shapes via a virtual
`capabilities()` returning a `Capabilities` struct with three
whitelists (each empty = no constraint):

```cpp
struct Capabilities {
  std::vector<core::depth>  depths;
  int                       minChannels = 0;   // both 0 = any
  int                       maxChannels = 0;
  std::vector<core::format> formats;

  bool        accepts(core::depth, int channels, core::format) const;
  std::string describe() const;
};
```

Codec declarations:

| codec | depths        | channels | formats |
|-------|---------------|----------|---------|
| raw   | any           | any      | any     |
| zstd  | any           | any      | any     |
| rlen  | depth8u       | any      | any     |
| jpeg  | depth8u       | 1 or 3   | any     |
| 1611  | depth16s      | exactly 1| any     |

`ImageCompressor::compress()` validates against the active plugin's
capabilities and throws a uniform error before delegating:

```
ImageCompressor: codec '1611' does not accept this image
(codec accepts: depth depth16s, 1ch)
```

Plugin-internal throws remain as defense-in-depth.  Two new tests
(`capabilities.1611_rejects_rgb`, `capabilities.raw_accepts_any`).

UI greying for incompatible codecs in `qt::Prop` is the obvious
follow-up — left as a separate item.

### GLImageRenderer post-free SIGBUS (`a2f446392`)

User reported a Mac crash dump from compressor-playground with the
rlen codec: `SIGBUS` at exactly `0x0000000200000000` inside
`agxsTwiddleAddressCommon` on the `com.Metal.CompletionQueueDispatch`
queue.

Root cause: `GLImageRenderer::uploadTexture()` built the planar→RGBA8
staging buffer as a stack-local `std::vector<icl8u> rgba(w*h*4)` and
passed `rgba.data()` to `glTexSubImage2D`.  On Apple Silicon, OpenGL
is implemented atop Metal as a *deferred* backend — `texSubImage2D`
queues the upload onto a Metal command buffer that completes on a
separate dispatch queue, **after** `uploadTexture()` has already
returned.  The stack-local vector destructed before the GPU consumed
the bytes; the allocator returned the page to the kernel by the time
the GPU twiddled the source pointer.

Fix: promote `rgba` to a `Data` member so its storage outlives any
individual upload, plus an explicit `glFlush()` at the end of
`uploadTexture()` to push the queued command into the driver before
the next frame can re-resize the buffer.  Affects all codecs, not
just rlen — rlen just had the most reliable timing window.

### rlen rewrite arc (`4c6654416` → `462100e87`)

Four commits, each peeled a layer off a stack of memory-safety bugs
in the RLE plugin that were corrupting the heap and surfacing as
seemingly-random crashes (sometimes inside `compress`, sometimes
much later inside Metal's command-buffer machinery).

1. **`4c6654416` — quality-flip heap overflow.**  `m_quality` was
   read twice in `compress()`: once for the worst-case buffer sizing
   (`q == 8 ? 2 : 1` bytes/pixel), once per `encodeChannel` call.
   The Configurable property callback writes `m_quality` from the
   GUI thread (qt::Prop), `compress()` reads it on a worker thread,
   nothing serialised them — a 6→8 flip between reads sized the
   buffer for `dim` bytes but encoded at q=8's `2*dim` worst case.
   Snapshot `m_quality` once at the top of each call into a local.
2. **`c36ed6215` — end-of-buffer OOB read** in q=4/6/8.  Each case
   re-seeded `currVal = *imageData` at the END of every iteration
   to prep the next run; with `imageData == imageDataEnd` after the
   last run, that's a 1-byte read past the channel buffer.  Heap
   padding masked it for small allocations, but a tight chunk on a
   page boundary (large channels, certain malloc paths) tripped a
   bus error straight inside `RlenPlugin::compress`.  Restructured
   all four cases to read the run's seed AFTER the bounds check.
   Two regression tests added (`rlen.roundtrip_all_qualities` on a
   640×480×3 image to stress the allocator into mmap territory,
   `rlen.quality_flip_mid_session` to exercise every transition).

   **My own infinite loop** also fixed in this commit: when
   restructuring q=1 to remove the OOB-at-tail, the seed
   (`!!*imageData`) and `find_first_not_binarized`'s threshold
   (`>=127`) disagreed for bytes in `[1, 126]` — same byte
   re-seeded the same wrong boolean, find returned the same position,
   0-length run, no progress, infinite loop.  Fixed by using
   `(*imageData >= 127) ? 1 : 0` to match find's threshold.
3. **`92ca9afe4` — template rewrite.**  The four quality levels
   share one pattern: each pixel reduced to a "compare key" (mask
   for q=4/6/8, threshold for q=1), runs of equal key collapsed,
   token packs (key, length-1) into one byte (q≤6) or two (q=8).
   Collapsed to a single `RlenCodec<VAL_BITS>` policy struct + two
   templated loops (`encodeChannelT` / `decodeChannelT`).  Per-quality
   differences live in tiny `if constexpr` branches inside
   `quantize` / `emit` / `decodeOne`.  The four switch cases that
   were the source of every bug above are gone — each is now a
   single line in a quality-keyed dispatch.
4. **`462100e87` — q=2.**  With the template, adding a fifth
   quality level was three lines (one extra dispatch case in
   each of encode/decode, one extra entry in the property menu).
   Fills the natural gap between q=1 (binary) and q=4 (16-level):
   2 value bits, 6 length bits, MAX_LEN=64.  Useful for coarse
   4-level masks/icons.

After all four commits the algorithmic core of rlen is roughly
**~25 LOC of templated loops** + the ~50 LOC `RlenCodec` policy
(comments included), down from ~80 LOC of switch cases.  Five
quality levels supported (1/2/4/6/8) instead of four.

### Landmarks

- 875/875 tests green at HEAD (873 baseline + 4 new tests:
  `capabilities.1611_rejects_rgb`, `capabilities.raw_accepts_any`,
  `rlen.roundtrip_all_qualities`, `rlen.quality_flip_mid_session`).
- Three TODO items closed: capability flags, OSD scale-range
  button, capability-flag codec classification (the same item under
  two TODO sections).
- Crash class eliminated from rlen by construction (no OOB reads,
  no run-tail re-seeds, single source of truth for `m_quality`).
- Q=2 rlen quality available for coarse 4-level encode.

---

## Current State (Session 59 — qt::ui:: designated-init syntax)

Single arc across 5 commits: introduce a modern `qt::ui::` namespace
that gives every GUI component a mixed positional+designated-init
shape, on top of the existing widget factory.  The legacy
`qt::Slider(0,255,42).handle(...)` builder is unchanged — both
routes converge.

### Design decisions (documented in `ui-plan.md`)

1. **Mixed shape.**  Primary data args positional (min/max/val for a
   slider, text for a button), trailing `XxxOpts{}` pack for
   everything else via designated init.  Avoids the C++ no-mix-rule
   inside one call: the designators all live inside the Opts literal.
2. **Per-component Opts** hoisted to namespace scope (`ui::SliderOpts`,
   not `ui::Slider::Opts`).  Nested types' default member initializers
   aren't visible from the enclosing class's own default argument, so
   `Slider(..., SliderOpts={})` only works with Opts at namespace scope.
3. **Shared-metadata block duplicated** across every XxxOpts (8 lines
   of handle/label/tooltip/size/minSize/maxSize/hide).  Rejected
   inheritance (wrecks flat designated-init — caller would need
   `{.CommonOpts={...}}` spelling) and macros (magic for little win).
4. **`applyCommon` uses `if constexpr(requires{o.field;})`** so each
   Opts can freely add or drop metadata fields.  Containers skip
   `tooltip`/`hide`, scrolls add `margin`/`spacing`.
5. **Leaves are plain structs with `toComponent()`; containers
   inherit from their legacy `qt::` counterparts.**  Leaves are data;
   containers are accumulators.  Inheriting means container
   `<<`-chaining of children works through the existing
   `ContainerGUIComponent::operator<<` plumbing for free, and
   top-level `gui << ui::HBox({...})` routes through the existing
   `GUI::operator<<(const GUI&)` overload — no new dispatch.
6. **Free `operator<<(GUI&, T)` + `operator<<(GUI&&, T)`** templates
   in `icl::qt`, guarded on a `ui::Component` concept (requires a
   `toComponent()` member).  Rvalue overload mirrors the
   const-member pattern on `ContainerGUIComponent::operator<<` so
   `ui::HBox({...}) << ui::Slider(...)` chains from a temporary.

### Commits

- `103e9316f` — **Phase 1 spike.**  `icl/qt/ui.h` with `applyCommon`,
  `Component` concept, lvalue+rvalue stream overloads, `ui::Slider` +
  `ui::SliderOpts`.  `ui-syntax-demo` showing legacy + ui:: side by
  side.  meson wiring.  Validates the design before expanding.
- `fbafa28a9` — **Phase 2.**  12 leaves: FSlider, Int, Float, Spinner,
  String, Label, State, Button, CheckBox, ButtonGroup, Combo.
  `ui::Label` case-study validates the mixed-syntax decision —
  positional text + `.label` for border disambiguates cleanly.
  `ui::Button` toggle semantics match legacy (empty `.toggledText`
  → push, non-empty → toggle).
- `b3ccf13d8` — **Phase 3.**  10 components: Display, Canvas, Canvas3D,
  Disp, Plot, Fps, ColorSelect, CamCfg, Ps, Prop.  `Prop` preserves
  the dual-ctor shape (live `Configurable*`/`&` via pointer-encoding
  trick, plus string ID).  `Plot` adopts a flat four-float form
  rather than Range32f wrappers — more readable under designated init.
- `8891274e2` — **Phase 4+5.**  7 containers (HBox/VBox/HScroll/
  VScroll/HSplit/VSplit/Tab) + 3 finalizers (Show/Create/Dummy).
  Single `BoxOpts` shared by all box-style containers (margin,
  spacing + common metadata).  `Border` dropped — qt::Border's ctor is
  friend-only; any container's `.label` gives the same visual effect.
  Demo converted to 100% ui:: end-to-end.
- `9ffa376c7` — **`ui-plan.md`** roadmap at repo root.  Inventory of
  all 33 components grouped by positional-arg shape, design
  decisions, 6 phases with checkboxes.

### Landmarks

- 33/33 components covered.
- Interactively verified: `ui-syntax-demo` fires callbacks on all
  component types (slider, fslider, int, float, spin, text, run, pp,
  chk, radio, color).
- 871/871 tests green throughout every commit.
- String round-trip inside GUIComponent preserved as-is — Phase 7 of
  `ui-plan.md` (separate multi-session arc) will retire it.
- `TODO.md` "Designated-init GUI component syntax" item checked off.

---

## Current State (Session 58 — token-based callback unregister)

Single arc: closed the long-standing `removedCallback` gap on both
Configurable callback channels.

- **`Configurable::CallbackToken`** (opaque `std::uint64_t`, `0` reserved
  as "invalid / never registered").  Monotonic counter per instance;
  single ID space across property-change and child-set channels so
  callers can't accidentally cross-unregister (the two `remove*` methods
  only look at their own vector).
- **`registerCallback(Callback)` → `CallbackToken`** (was
  `void registerCallback(const Callback &)`).  Taking by value lets the
  impl `std::move` into the storage pair; existing call sites that pass
  lambdas are unaffected.  Discard the return when subscribing for life.
- **`onChildSetChanged(ChildSetCallback)` → `CallbackToken`** — same
  shape.
- **`removeCallback(CallbackToken)` / `removeChildSetCallback(CallbackToken)`**
  — linear-scan erase; `token == 0` and not-found are both no-ops so
  default-init tokens + double-remove are safe.
- Storage flipped from `vector<Callback>` to
  `vector<pair<CallbackToken, Callback>>`; fire sites iterate `.second`.
- Dead `removedCallback` stub deleted (was retained "for ABI
  compatibility" with an empty body since std::function has no `operator==`
  — unreachable now that we have a real token API).
- **`UnaryOp::registerCallback`** and **`Grabber::registerCallback`** —
  both widened to match the new base signature (`CallbackToken` return,
  `Callback` by value).  Their `using Configurable::registerCallback;`
  clauses are removed: now that the shadow and the base share a signature,
  the using-declaration is redundant (and would collide).  Callers that
  want the unwrapped base version still reach it via `Configurable::
  registerCallback(...)` qualified-name.  Also removed the dead
  `using utils::Configurable::registerCallback;` in `GenericGrabber.h`.
- **`qt::Prop`'s ConfigurableGUIWidget** — two new `CallbackToken`
  members (`propChangeToken`, `childSetToken`), return values captured
  at registration, new dtor unregisters both.  Closes the documented
  segfault window where a Configurable outliving the widget fires into
  a dangling `this`.
- 5 new tests in `tests/test-utils.cpp` — fire, distinct-nonzero
  tokens, unregister-stops-firing, zero + stale + double-remove no-op,
  `onChildSetChanged` add+remove round-trip.  871/871 green (-j 1).

Files touched: `icl/utils/Configurable.{h,cpp}`, `icl/filter/UnaryOp.{h,cpp}`,
`icl/io/Grabber.{h,cpp}`, `icl/io/GenericGrabber.h`, `icl/qt/GUI.cpp`,
`tests/test-utils.cpp`.

---

## Current State (Session 57 — pugi retirement + in-house XML library)

### Session 57 Summary

9 commits.  Single arc: the in-house XML library, both consumer
migrations, pugi deletion, and follow-up perf work.  Milestone: the
entire pugixml dependency surface is gone from the ICL source tree.

#### Phase 1 — XML library

- `786e71c05` **utils: `icl::utils::xml` — parser + DOM + emitter +
  XPath subset.**  Single-pass recursive-descent parser modelled on the
  YAML lib's shape.  Zero-copy for element names, attribute names, and
  raw attribute / text content (views into the source buffer); entity-
  decoded values materialise lazily into the Document's string arena.
  DOM is linked-list children + linked-list attributes (head + tail
  pointers), all allocated from a page-backed `NodeArena`.  Emitter is
  deterministic and escape-aware with a round-trip fixpoint test.
  XPath subset covers absolute/relative paths, wildcard step, descendant
  axis, self-predicate + attr-eq + attr-exists + index + or/and inside
  predicates, attribute-axis terminal step — ~900 LOC vs pugi's ~5000-LOC
  XPath engine.  42 tests (parse shapes, attrs, entities / CDATA /
  comments / PI / DOCTYPE / BOM, error line:col, emit round-trip,
  mutation, XPath axes + predicate combos + the exact Primitive3DFilter
  and Optris queries).  Plan document at repo root: `xml-config-plan.md`.

#### Phase 2 — consumer migrations

- `d9d99d622` **io/geom: migrate Primitive3DFilter + OptrisGrabber from
  pugixml to icl::utils::xml.**  Primitive3DFilter's XPath
  `doc.select_nodes("/pointcloudfilter/*[self::remove or self::setpos
  or ...]")` maps 1-to-1 onto `doc.root().selectAll(...)`; everything
  else is a mechanical `s/pugi::/xml::/` with `.next_sibling` →
  `.nextSibling`, `.as_float` → `.asFloat`, and `std::string(a.value())`
  at sites that consumed pugi's `const char*`.  OptrisGrabber's single
  XPath collapses to `doc.root().selectOne("/CaliData/Temperature/
  Optics/OpticsDef/FOV")`.

#### Phase 3 — benchmarks + pugi retirement

- `1051ca9e2` **benchmarks: bench-xml vs pugi; pin numbers in Xml.h.**
  Shape mirrors `bench-yaml.cpp`: parse small / parse large / traverse /
  xpath / emit, plus matching `utils.xml.pugi.*` probes running the same
  inputs through the vendored pugi for side-by-side.  Numbers recorded
  in `Xml.h`'s `\file` doc comment.
- `aef6466ab` **utils: retire vendored pugixml (−15,218 LOC).**  Once
  benchmark numbers were pinned, `icl/utils/detail/pugi/` was deleted
  wholesale; `detail/pugi/PugiXML.cpp` dropped from meson, the
  "pugixml lives under detail/pugi/" comment block removed, and the
  `utils.xml.pugi.*` benchmark probes deleted.  Cycles' own vendored
  pugixml under `3rdparty/cycles/.../pugixml/` is untouched (that's
  for the raytracing engine, not ICL).

#### Phase 4 — XML parser perf work

The remaining four commits progressively closed the gap to pugi on raw
parse throughput: 174 µs → 167 → 108 → 101 → 97 µs (parse_large, median
of 50, Apple-Silicon arm64 -O3).  Cumulative −44%; gap vs pugi narrowed
from 3.5× to 1.96×.

- `8c0b906e2` **utils: SIMD content-text scan in XmlParser (+4%).**
  The `while(peek() != '<') advance()` in `parseElement`'s content loop
  becomes a 16-byte SIMD scan via sse2neon: one `_mm_cmpeq_epi8` against
  `'<'` + a parallel `'\n'` compare for bulk line/col bookkeeping.
  Also tried SIMD-ifying `skipWs` and the attribute-value scan — both
  regressed (whitespace runs and attr values are typically <16 bytes,
  SIMD setup dominates).  Reverted those; kept only the content scan.
- `9c5419cd4` **utils: page-backed NodeArena (−35%).**  Swap
  `std::deque<ElementNode>` + per-element `std::vector<AttributeNode>`
  for a page-backed bump allocator (64 KB pages) with attributes as a
  singly-linked list (head + tail pointers).  Pages are never reclaimed
  during Document life; both node types are trivially destructible, so
  release is page-wholesale.  This was the single biggest win — node
  allocation was by far the dominant overhead, not any parsing inner
  loop.
- `50a297afe` **utils: raw-ptr parser cursor + tag dispatch (−10%).**
  Parser cursor reshape: `std::string_view m_src` + `std::size_t m_pos`
  become `const char *m_begin / m_cur / m_end`.  Hot inner loops walk
  `m_cur` as a raw pointer in a register instead of indirecting through
  a string_view member per byte.  Content-loop tag dispatch: four
  sequential `startsWith` / `memcmp` probes (`<![CDATA[`, `<!--`, `<?`,
  `</`) become a single switch on `m_cur[1]` after checking
  `*m_cur == '<'`.
- `badadf48d` **utils: extract page-backed Arena<> into shared
  detail/Arena.h.**  Generalise the ad-hoc `xml::NodeArena` into a
  header-only template `icl::utils::detail::Arena<PageBytes>`; `alloc<T>()`
  placement-news a default-constructed T (trivially destructible,
  static_asserted).  `xml::Document` swaps to `Arena<> m_nodeArena`
  with `alloc<detail::ElementNode>()` / `alloc<detail::AttributeNode>()`
  call sites.  No behavioural change.
- `fcfc2a4f6` **utils: extend Arena<> with byte-pool methods; YAML intern
  experiment (no win).**  Added `allocBytes(n)` + `internString(sv)` to
  the shared Arena template.  Tried swapping `yaml::Document::m_arena`
  from `std::deque<std::string>` to `Arena<>` + the new byte-pool API:
  measured essentially no change on parse_large (~276 µs either way,
  within noise).  Two reasons documented inline in Yaml.h: (1) `intern`
  is rarely hit for plain-scalar configs — the fast path returns zero-
  copy views; (2) when intern IS called, the arena's memcpy is extra
  work vs the deque's in-place `emplace_back(move(s))`.  Takeaway:
  arenas win on data models with many small pointer-linked allocations
  (XML's ElementNode / AttributeNode), not on by-value containers with
  SSO.  The byte-pool primitive stays in place for future consumers.

### Session 57 landmarks

- 866/866 tests green throughout every commit.
- `icl/utils/detail/pugi/` wholly deleted from the tree (15,218 LOC).
  Only remaining pugi presence is Cycles' own vendored copy under
  `3rdparty/cycles/…/pugixml/include` — third-party raytracing engine,
  not ICL.
- `xml-config-plan.md` at repo root lists all phases with checkboxes
  (all ticked post-session) + the deferred "optional SIMD perf pass"
  item kept for the record.
- `project_xml_config.md` memory updated to "ALL PHASES LANDED".
- `TODO.md` Phase 3 entry (YAML-arc follow-up) checked off with
  pointers to the xml-config-plan + memory.

---

## Current State (Session 56 — YAML library + ConfigFile migration)

### Session 56 Summary

13 commits.  Single arc: the in-house YAML library and its immediate
consumer (ConfigFile).  Milestone-heavy: the entire pugixml dependency
surface for ConfigFile is gone.

#### Phase 1 — YAML library

- `2ffb43439` **utils: `AutoParse<std::string_view>` backend + `from_chars`
  `parse<T>(sv)` fast paths.**  Third specialization alongside the
  string / any backends.  Trim-and-from_chars for integral family; no
  allocation.  Base-10 defers to from_chars native signed handling so
  edge values like INT_MIN still parse.  14 new autoparse tests.
- `1aea0ec36` **utils: slim YAML parser — `icl::utils::yaml`, Phase 1.**
  Single-pass recursive-descent, indentation-aware, scalar views into
  source.  Node = `variant<monostate, ScalarData, Sequence, Mapping>`;
  `Document::view(sv)` / `::own(string)` / `::file(path)` / `::empty()`.
  `AutoParse<string_view>` drives `Node::as<T>()`.  Strict-mode kind
  matrix.  `detail::yaml/Yaml{Parser,Emitter,Scalar}.{h,cpp}` private.
  Corpus harness loads 22 curated yaml-test-suite cases + 95 JSON y_*.json
  files at runtime via `-DICL_TEST_DATA_DIR`.  SSO lifetime hazard found
  and fixed during corpus work — `Document::m_source` is a
  `unique_ptr<std::string>` so moves don't relocate parsed view bytes.
- `6432e6705` **utils: YAML subset expansion — chomping, tags, hex/oct,
  sortKeys, same-indent seqs.**  Six A-features in one commit: `|-` /
  `|+` / `>-` / `>+` chomping indicators, `0x1F` / `0o17` parse paths
  via from_chars with base, `.inf` / `-.inf` / `.nan` in float parse,
  `EmitOptions::sortKeys` actually honored, same-indent block sequences
  as mapping values (`key:\n- a\n- b`), and explicit `!!str` / `!!int` /
  `!!bool` / `!!float` / `!!null` tag prefixes with emitter round-trip.
- `bc5c96393` **utils: `Node::operator=` for scalar-ish types — crisper
  programmatic building.**  `ScalarData` gains optional `owned` string
  for self-contained scalars.  Overloads for const char*, string,
  string_view (copies), bool, plus a SFINAE template via `str(T)` for
  arithmetic / ICL geometry types.  `scalarView()` / emitter /
  kind-resolver all use `effectiveView(sd)` so move-safety is preserved.
- `e8c040985` **utils: init-list + container assignment; `Mapping` gets
  a spill arena.**  `Mapping` is no longer a bare `vector<pair<sv,
  Node>>` — it's a struct with `entries` + `ownedKeys` deque spill for
  programmatic keys.  Parser keeps zero-copy `emplace_back(sv, Node)`;
  `operator[]`, `operator=(initializer_list<...>)`, container assigns use
  `emplaceOwned(string, Node)`.  Custom copy-ctor rebinds owned-key
  views via an old→new data-ptr remap.  Operator= adds init-list
  sequence, init-list mapping, `vector<T>`, `map<K,V>`, `unordered_map<K,V>`.
- `b59335891` **utils: crisp int indexing + mapping init-list ctor.**
  Template `operator[](I)` over integrals so `n[2]` works without
  `size_t(...)`.  Ctor-level `Node n = {{"k", 1}, {"j", 2}}` mapping
  init-list works; sequence init-list ctor deliberately not added
  (would be ambiguous).
- `fb8bb1d2b` **tests: drop `size_t(N)` casts in YAML indexing.**
  17-site mechanical cleanup now that `n[int]` works.
- `b93ed7611` / `dc31ba2df` **`yaml::sequence` / `yaml::mapping` tag-type
  helpers → renamed `yaml::seq` / `yaml::map`.**  Sidesteps the ctor
  init-list ambiguity for sequences via distinct type signature +
  implicit `operator Node()` conversion.  Both tags copy contents into
  owned storage so temporaries are safe.  Required two SFINAE touch-ups
  (converting-ctor and generic-scalar-assign exclusion) to route calls
  through the implicit conversion rather than recursive operator=.
- `78d8c371b` / `73a25bbd9` **benchmarks + pin numbers in Yaml.h.**
  Five `utils.yaml.*` benchmarks covering parse_small / parse_large /
  emit / build / traverse.  Initial run included yaml-cpp and rapidyaml
  competitors (via probe-and-conditional meson); measured numbers
  recorded in the `\file` doc block at the top of `Yaml.h`, competitor
  deps removed from the build surface to keep the benchmark
  dependency-free.  Results: **~20× faster than yaml-cpp, tied with
  rapidyaml** on typical inputs.

#### Phase 2 — ConfigFile migration

- `80ae880ac` **utils: ConfigFile migrated from pugixml to
  `icl::utils::yaml`.**  Aggressive cleanup per user directive: `type=`
  / `range=` / `values=` wire attributes gone, entire RTTI /
  register_type machinery deleted, `Impl` PIMPL removed (yaml is our own
  type so no hiding needed), restrictions demoted to in-memory-only.
  Deleted `StaticConfigFileTypeRegistering` from `FixedMatrix.cpp` as
  dead code.  `m_entries` stays as canonical flat store;
  `yaml::Document` is only an IO transient at `load`/`save`.  Dotted-
  path keys map to nested YAML.  Header shrank 568 → 243, source 424 →
  206.  +10 new `utils.configfile.*` tests covering basic I/O,
  registered types, file round-trip, nested-YAML emission, prefix
  semantics, Data-proxy usage, in-memory restriction handling.

---

## Session 55 recap (dynamic child-configurable arc)

Session 55 closed the dynamic child-configurable arc and swept the
residual polish items from Session 54's "Next Step" list.  Highlights:

- `Configurable::onChildSetChanged` observer API + qt::Prop rebuild
  (landed).  `ConfigurableGUIWidget::buildFromConf()` extracted from
  the ctor; rebuild coalesces via `QMetaObject::invokeMethod(Qt::QueuedConnection)`.
  Two real-world consumer demos land alongside: `icl-compressor-playground-demo`
  (ImageCompressor codec swap) and `icl-grabber-backend-swap-demo`
  (GenericGrabber::init re-entry).
- `Configurable::Handle::as<T>()` facade-read fix — reads now forward
  via `getPropertyValue`, matching the write-path protocol.  Latent
  bug; every `Prop(...)` on a Configurable with children was hitting it.
- `ImageCompressor` compress/uncompress/installPlugin now serialized
  by a recursive_mutex.  Pre-existing race surfaced by the new demo
  (GUI-thread `mode` flip vs. worker `compress`).
- `configurable-gui-demo` thread finally stops cleanly on exit
  (`~B() { stop(); }` + `while(running())`).  Pre-existing exit crash.
- Float-setter / int-property audit (5 Ops), `core/Image.h` pulls
  `Img.h` transitively, CTAD sweep for `lock_guard` / `scoped_lock` /
  `unique_lock` (~278 sites, 71 files).  All mechanical.

Concrete work items remaining (in suggested order):

- **`removeChildSetCallback` API.**  Subscribers can't unregister
  today — if a ConfigurableGUIWidget dies while its Configurable
  survives, the captured `this` is dangling and the next
  add/removeChildConfigurable segfaults.  Same shape as the
  long-standing `removedCallback` gap on the property-change
  channel.  Either ID-based registration on both, or a token-
  returning pattern.

- **Compression-codec capability flags.**  Surfaced by the
  compressor demo: `1611` only handles single-channel icl16s and
  throws on RGB input.  Today the demo try/catches and reports
  in the ratio label; proper fix is to put a capability matrix
  (format / depth / channels) on `CompressionPlugin` and gray out
  incompatible choices in the UI.  Dovetails with the Session 48
  "auto codec" deferral.

- **ICLWidget OSD scale-range button misbehaves** (2026-04-21
  report).  User-visible; needs a proper repro session.

- **Designated-init GUI component syntax** — `gui << Slider{.min=0,
  .max=255}`.  Bigger arc; pairs with retiring `GUIComponent::toString()`'s
  string round-trip.

- **Fun: icl-edit image editor demo.**  Needs new filter Ops
  (`BrillianceOp`, `VibranceOp`, `ClarityOp`, `CurvesOp`, interactive
  crop/rotate).  Good exercise of the push-callback refresh +
  filter chaining + ConfigFile round-trip.

---

## Current State (Session 55 — dynamic child-configurable + polish)

### Session 55 Summary

15 commits.  Two logical arcs: (1) clear the Session 54 "Next Step"
polish list (float-setters, Image.h include, CTAD sweep); (2) land
the dynamic child-configurable refresh and two real-world consumer
demos.

#### Polish items (mechanical)

- `177624b10` **core: `Image.h` pulls in `Img.h` transitively.**
  `Image::as<T>()`'s inline `static_cast<Img<T>*>` needs the
  derivation visible under Clang 21; every consumer now gets it
  automatically.
- `268e36225` **filter+geom: align Op property constraint value_types
  with setter args.**  Five Ops (ThresholdOp, CannyOp,
  UnaryArithmeticalOp, UnaryCompareOp, RansacBasedPoseEstimator).
  Range<int> widened to Range<float> where the setter was float/icl32f;
  icl64f setters cast explicitly to float at the write site (only
  Range<int>/Range<float> adapters are registered); Ransac's
  integer-count setter was narrowed from float to int instead.
- `11b5eeb7c` **tree-wide: CTAD sweep for lock_guard / scoped_lock /
  unique_lock.**  ~278 sites, 71 files.  `lock_guard<std::mutex>` →
  `scoped_lock` (and similarly for `recursive_mutex`); `scoped_lock`
  is the C++17+ default and handles multi-mutex for free.
- `289db2d2e` **utils: drop stale dead code in AssignRegistry.h /
  ConfigFile.h.**  #if-0 blocks + stale transitional comments.

#### Dynamic child-configurable refresh

- `8532471f7` **Handle::as<T>() forwards through getPropertyValue.**
  Latent facade-read bug: facades carry a constraint but no
  `typed_value` (owning storage lives on the child).  Writes already
  forwarded; reads now match.
- `be71b2b54` **Configurable::onChildSetChanged** — observer API.
  Fires from the end of `addChildConfigurable` / `removeChildConfigurable`.
- `51b2ec0a6` **qt: ConfigurableGUIWidget rebuilds on child-set
  change.**  Constructor body extracted into `buildFromConf()`;
  `clearWidgets()` tears the Qt child tree down; `rebuild()` =
  clear + rebuild; `enqueueRebuild()` coalesces via
  `QMetaObject::invokeMethod(Qt::QueuedConnection)` + a
  rebuildScheduled flag (same pattern as the property-change push
  channel from Session 54).
- `2491c3aa4` **qt: dynamic-child-props demo.**  Host with
  add/remove Command buttons — simplest visual proof that the
  mechanism works.
- `23b637b2d` **configurable-gui demo stops thread cleanly on exit.**
  Pre-existing `while(true)` + no `stop()` led to "recursive_mutex
  lock failed" or "Property not supported" on exit depending on
  destruction-order roulette.  Two-line fix.

#### Real-world consumers

- `87b70f544` **io: ImageCompressor serializes
  compress/uncompress/installPlugin.**  Latent race: GUI-thread
  `mode` flip destroys the old plugin while a worker thread is
  partway through `compress()`; worker reads the new plugin's
  `name()` after encoding with the old plugin → envelope lies
  about the payload → `uncompress` aborts with codec-specific
  errors ("Not a JPEG file: starts with 0xff 0xd9").  recursive_mutex
  in the pimpl; pair with the onChildSetChanged demo work since
  the demo is what surfaced it.
- `614d4fbdd` **qt: compressor-playground demo.**  Grabber → compress
  → uncompress side-by-side.  Flipping `mode` swaps the codec child,
  the widget rebuilds, codec-specific knobs (`quality`, `level`)
  appear/disappear.  Catches incompatible combos (e.g. `1611` on
  RGB) and surfaces them in the ratio label.
- `403f2a897` **qt: grabber-backend-swap demo.**  Four buttons drive
  `GenericGrabber::init()` with different (type, id) pairs;
  Prop on the grabber rebuilds as the backend child swaps.
  Documents the init() params shape gotcha (`"type=id"`, not
  bare id) in the `swapBackend` helper.

#### Docs + follow-ups

- `8fedcd7f5` / `deb8035df` — TODO entries for the polish checkoffs
  + the three follow-ups (`removeChildSetCallback`, ImageCompressor
  demo, GenericGrabber demo — the latter two now checked off in
  this session).

#### Verification

Full test suite: 659/659 passing at every commit under
`meson compile -j16` (CCACHE_DISABLE=1 in this sandbox).

---

## Current State (Session 53 — Configurable typed-storage migration + proxy)

### Session 53 Summary

23 commits across a single multi-step arc.  Closed out the typed-
std::any migration for Configurable property storage, queued at
end of Session 52.  End state: every ICL property has a structured
`prop::*` constraint + typed value; `Property::value` (std::string)
retired; `prop("x").value = v` preserved as an ergonomic syntax via
a write-through proxy that routes through a new typed setter.

#### 1. `prop::` constraint framework (commits 12ea853e0 .. 4d27d8453)

New `icl/utils/prop/Constraints.h` introduces first-class C++ types
that replace the legacy stringly-typed `addProperty(name, type,
info, value)` grammar:

    prop::Range<T>{.min, .max, .step, .ui}   // Slider / Spinbox
    prop::Menu<T>{choices...}                // categorical pick-one
    prop::Flag{}                             // bool
    prop::Command{}                          // button, no value
    prop::Info{}                             // read-only string
    prop::Text{.maxLength}                   // free-form string
    prop::menuFromCsv(csv)                   // runtime-parsed Menu<string>

Plus `core::prop::Color` (RGB triplet) and `core::prop::ImageView`
(volatile image readback) in `icl/core/prop/` for constraints that
need core-module types.

Each constraint has a `ConstraintAdapter` (toString / fromString /
typeId / infoString function table) enrolled at static-init time in
a `PluginRegistry<std::type_index, ConstraintAdapter>`.  Adapters
synthesize the legacy type + info + value strings for backward
compat — ConfigFile save and qt::Prop's string-matching dispatcher
keep working through the migration.  Apple Clang 21 (Xcode CLT 26.4)
aggregate CTAD + designated-init supports
`prop::Range{.min=0.f, .max=500.f}` without spelling the template
argument.

#### 2. Typed addProperty<C> + migration (commits 69767fa82 .. 434a0f0a3)

Configurable gains a templated `addProperty<C>` overload:

    addProperty("gain",    prop::Range{.min=0.f, .max=500.f},  250.f);
    addProperty("mode",    prop::Menu{"fast","slow","auto"},   "fast");
    addProperty("enabled", prop::Flag{},                       true);
    addProperty("save",    prop::Command{});
    addProperty("bg",      core::prop::Color{},                core::Color(0,0,0));

`Property::constraint` (std::any) + `Property::typed_value` (std::any
of declared C++ type) added alongside existing `value` (std::string)
for transitional coexistence.

Step-6 bulk migration: ~500 `addProperty` call sites across
filter/io/cv/qt/geom/geom2/markers/physics migrated from the legacy
string form to typed overloads via `scripts/migrate-addProperty.py`
— 80-90% automated, manual cleanup for concat'd info strings,
menu-with-named-constant, and a few dynamic-registration sites.

#### 3. Storage + API flip (commit 8c40415c7)

`Configurable::getPropertyValue` return type flipped from
`AutoParse<std::string>` to `AutoParse<std::any>`.  Callers writing
`T x = c.getPropertyValue(name)` continue working via the cascade;
string-specific sites use `.str()` / `.as<std::string>()`.

#### 4. `payload` folded into typed_value (commits f3bb77ae6 .. 0f8c54c05)

`Property::payload` (std::any, used only for "image") retires.
`typed_value` absorbs its role.  qt::Prop's `VolatileImageUpdater`
reads through `getPropertyValue` (typed fast path) before falling
back to `getPropertyPayload` for unmigrated callers.  GaborOp's
"kernel preview" migrates to `core::prop::ImageView`; Scene/Scene2
background color migrate to `core::prop::Color`.

#### 5. Regression fix (commit 83d7155de)

Direct `prop("x").value = v` writes bypassed `setPropertyValue` →
stale `typed_value` → broken sliders (LocalThresholdOp et al).
Migrated 13 such sites to `setPropertyValue`.  Also caught an
infinite recursion in `UnaryOp::setClipToROI`: its override of
`setPropertyValue` re-dispatches to `setClipToROI`; changing
`setClipToROI` to call `setPropertyValue` looped.  Fixed by
explicit `Configurable::setPropertyValue(...)` (non-virtual
dispatch).

#### 6. Step 9 — retire Property::value, add proxy (commits f789b934e .. 533ec1cd7)

Six commits:

  1. **Unify legacy addProperty dispatch** — the string-taking
     `addProperty(name, "type", "info", value)` overload internally
     builds a typed constraint via `buildConstraintFromLegacy` and
     populates `constraint` + `typed_value` just like the typed
     overload.  After this, every property has both fields.
  2. **Property::as<T>()** — typed read helper off typed_value
     through `AutoParse<any>`'s cascade.
  3. **Bulk-migrate ~155 callback reads** — `parse<T>(prop.value)`
     → `prop.as<T>()`, `prop.value == "x"` → `prop.as<std::string>() == "x"`.
     Perl-scripted across filter/io/cv/geom/markers/physics.
  4. **Retire `Property::value` field** — `typed_value` becomes
     sole storage.  getPropertyValue always wraps typed_value;
     setPropertyValue parses into typed_value via constraint adapter.
  5. **`setPropertyValueTyped(name, std::any)`** — new public
     setter that writes typed_value directly.  No stringify.
  6. **`PropertyHandle` + `PropertyValueRef` proxy** —
     `Configurable::prop(name)` returns a short-lived handle with a
     `PropertyValueRef value` proxy (operator= → setPropertyValueTyped)
     + reference members for name/type/info/constraint/typed_value/
     volatileness/etc.  Ergonomic end state:

         prop("gain").value = 0.5f;        // typed, no string
         float g = prop("gain").value;     // typed read, no parse
         if (prop("enabled").value == "on") { ... }
         std::any_cast<const prop::Range<float>&>(prop("x").constraint);

  Internal storage accessor renamed to `prop_storage(name)` — used
  by Configurable.cpp setters/getters and the inline
  `addProperty<C>` template that pokes Property fields directly.

#### 7. Verification

- `tests/icl-tests -j 1`: 658/658 pass at every commit of this
  session (626 pre-session + 32 new `utils.prop.*` + `core.prop.*`
  cases).
- Full tree builds clean at every commit under `meson compile -j16`
  (CCACHE_DISABLE=1 in this sandbox — ~/.ccache isn't in the allow
  list; unrelated to the session).

#### 8. Toolchain bump

Apple Clang 15 → 21 (Xcode CLT 26.4).  Enabled C++20 aggregate CTAD
with designated initializers (P2082R1) — core ergonomic win for the
`prop::Range{.min=..., .max=...}` call-site style.  No code change
required; `sudo xcode-select -s /Library/Developer/CommandLineTools`
picked up the new toolchain.

#### 9. Documentation artifacts

- `scripts/migrate-addProperty.py` — reusable tool for migrating
  legacy addProperty call sites in future Configurable subclasses.
  Handles flag / info / command / string / menu / range{,:slider,
  :spinbox} / float / int / value-list; `:`-as-separator oddity;
  `str()` wrapper stripping for numeric value args.
- TODO entries added: step-9 follow-ups (proxy-or-retire for
  Property::value — closed by this session), designated-init GUI
  component syntax, lock_guard/scoped_lock CTAD cleanup,
  `core/Image.h` include-Img.h fix.

#### 10. What's deferred

- **Step 5** — qt::Prop variant-visit dispatch on constraint type.
  See "Next Step" above.
- **Step 7** — retire legacy string-taking `addProperty` overload.
  Blocked on dynamic-registration sites (PylonCameraOptions etc.)
  that supply type/info as runtime strings; need a typed
  equivalent or keep the overload as the dynamic-registration
  entry point.
- **Step 8** — synthesize `getPropertyType`/`getPropertyInfo` from
  constraint on demand; drop `Property::type` and `Property::info`
  fields.  Pattern matches step-9's `Property::value` retirement.
- **Step 10** — this file.  Done.

---

## Previous State (Session 52 — `utils::Any` retired; AutoParse<Backend> landed; plugin APIs split into overloads)

### Session 52 Summary

7 commits along a single coherent arc.  Closed out the long-
standing `utils::Any` vs `std::any` confusion by introducing a
properly-named, properly-scoped conversion proxy
(`utils::AutoParse<Backend>`), migrating every consumer onto it,
and deleting `utils::Any.h` outright.  Net: ICL's own source no
longer contains a general-purpose container called "Any" — the
name collision with `std::any` is gone, and every former role of
`utils::Any` is served by an appropriately-specific tool.

#### 1. `utils::AutoParse<Backend>` — new conversion proxy (commit 56ad2eebc)

New `icl/utils/AutoParse.h`, a lightweight by-value conversion
proxy meant for index-style accessors like `ProgArg::operator[]`
and `DataStore::operator[]`.  Two backends:

- **`AutoParse<std::string>`** — publicly inherits `std::string`
  (same trick `utils::Any` used; safe here because AutoParse is
  disciplined-proxy-only, never stored).  Normal string operations
  (concat with `+`, comparison, stream-insertion, passing to
  `const std::string&` APIs) work without per-operator overloads.
  Templated `operator T()` parses into any stream-extractable T
  via `parse<T>`, SFINAE-excluding char-like types reachable
  through the base to avoid ambiguity.  Templated constructor
  accepts any streamable T (via `str(t)`) so
  `setPropertyValue("gain", 500)` keeps working.

- **`AutoParse<std::any>`** — cascade on extraction: exact
  `std::any_cast<T>` → numeric widening for arithmetic T → parse
  the stored value if it is a `std::string` → stringify the stored
  value if T is `std::string` and the stored value is a numeric.
  Used internally by `DataStore::Slot::operator T()` as a fast-path
  before falling through to the existing AssignRegistry cross-type
  dispatch — adds numeric-widening paths that previously required
  a string round-trip.

Discipline rule documented in the header: *AutoParse is a
conversion proxy, not a storage type.*  Never stored, never a
function-parameter type, never in a container — consume
immediately as `T x = ap` or `ap.as<T>()`.

Initial adopters: `ProgArg::operator[](int)` (return type flip
from `utils::Any` to `AutoParse<std::string>`) and
`DataStore::Slot::operator T()` (delegation to
`AutoParse<std::any>` on the fast path).

19 unit tests in `tests/test-auto-parse.cpp` covering both
backends.

#### 2. Configurable migration (commit 27a0cc5ea)

The biggest former consumer.  Flipped
`addProperty` / `setPropertyValue` / `getPropertyValue` / ditto on
`ConfigurableProxy` + all subclass overrides (`UnaryOp`,
`CornerDetectorCSS`, `MultiCamFiducialDetector::property_callback`)
from `utils::Any` to `AutoParse<std::string>`.  Internal property
storage untouched (still `std::string`), config-file
serialization untouched — this was an API-surface rename.

Semantics preserved exactly: both types inherit `std::string`,
both accept arbitrary T in the constructor via `str(t)`, both
parse on extraction via `parse<T>`.  Call sites unchanged.

#### 3. `ParamMap` moved out of `Any.h` (commit d23d60298)

`utils::ParamMap` — the `std::map<string, Any, std::less<>>` used
across plugin APIs — moved to its own header
`icl/utils/ParamMap.h` and retyped as
`std::map<string, AutoParse<std::string>, std::less<>>`.
`Any.h` kept the include transitively for continuity.

#### 4. Plugin-code `Any()` sentinel cleanup (commit 6a8077d4c)

7 residual `Any()` / `Any(value)` wrappers in
`FileGrabber.cpp`, `DCDeviceFeatures.cpp`, `V4L2Grabber.cpp`
replaced with plain `""` / the value itself — the wrapper was
only there to pacify the old `const Any&` param type.

#### 5. `FiducialDetector` overloads (commit c92621de7)

The cleanest-design part of the arc.  Deleted the single
`const Any &which` parameter throughout the FiducialDetector
family and replaced with three purpose-built overloads reflecting
what callers actually pass:

```
loadMarkers(int)                 → single numeric ID (BCH / ICL1)
loadMarkers(const vector<int> &) → typed list (no string parse)
loadMarkers(const string &)      → range "[a,b]", list "{a,b,c}",
                                    or file path/glob (ART, Amoeba)
```

`unloadMarkers`, `createMarker`, and the constructor's
`markersToLoad` parameter got the same treatment.  Int and
vector-int overloads delegate internally to the string form so
plugins still see a single string spec — plugin impls changed only
by virtue of signature (`const std::string &` instead of
`const Any &`), and the implicit Any→int conversions in three
plugin create-marker bodies became explicit `parse<int>(...)`.

18 files touched; `create-marker-grid-svg.cpp` was the only app
call site needing an update (`Any()` dropped from the ctor call).

#### 6. Delete `utils::Any.h` (commit 4e4c55f19)

Final retirement.  Remaining consumers:

- **`ImageRegion::{set,get}MetaData`** + `ImageRegionData::meta`
  and **`DefineRectanglesMouseHandler`'s meta bag**: genuine
  type-erased metadata stores, migrated to `std::any`.  Only the
  visualization path needed a tiny guard — `w.text(meta, ...)`
  became `if (auto *s = std::any_cast<std::string>(&meta)) w.text(*s, ...)`,
  which preserves the previous implicit "Any→string via is-a"
  behaviour.

- **`VisualizationDescription::Part::content`** (stringly-typed
  payload for draw primitives): migrated to
  `AutoParse<std::string>`.  Callers passed `std::vector<float>` /
  `<int>` for polygon/point lists; old Any had a binary packing
  specialization.  Replaced with text CSV via new
  `str(vector<T>)` + `parse<vector<T>>` specializations in
  `StringUtils.h` for float / int / double.  Cheaper to reason
  about than the binary packing; configs are cold-path anyway.

- **`Any::ptr<T>` pointer encoding** (the only non-string-round-trip
  use of Any): replaced by two small free functions
  `qt::encode_pointer<T>` / `decode_pointer<T>` in
  `qt/GUIComponents.h`.  Used only by the Prop GUI component to
  smuggle a `Configurable*` through the stringly-typed GUI
  parameter channel; will become moot when the GUIComponent
  stringification rework lands.

- **`ConfigFile.cpp`**: dropped `REGISTER_CONFIG_FILE_TYPE(Any)` —
  plain `std::string` already covers that role.

Headers that transitively pulled in `Any.h` rewired to include
`ParamMap.h` (FiducialDetector, FiducialDetectorPlugin,
Marker*Detector).  `Configurable.h` drops the Any include
outright.

Net delete: `icl/utils/Any.h` (211 LOC) + `examples/any.cpp`
(43 LOC benchmark of the binary packing).

#### 7. YAML config TODO (commit da12ae6f0)

User flagged "I don't like XML anymore".  Added a TODO entry
under a new `Configuration format` section in `TODO.md` + a
`project_yaml_config.md` memory.  Migrate `ConfigFile` on-disk
format from pugixml to YAML (yaml-cpp likely, rapidyaml if cold
path turns out to matter); keep the public API stable; auto-detect
existing `.xml` files on load during transition.  Leaves the
pugi dependency deletable afterward.

#### 8. Verification

- `tests/icl-tests -j 1`: 626/626 pass at every commit (607 from
  Session 51 + 19 new `utils.autoparse.*` cases).
- Full tree builds clean at every commit (`meson compile -j16`).
- No runtime verification of the FiducialDetector overloads (Qt
  GUI ctor still crashes in this sandboxed-macOS environment per
  the Session 51 note); API-surface change only, confirmed via
  build + unit tests.

#### 9. What's deferred

- **AutoParse template flip**.  The decision to keep `AutoParse`
  as `template<class Backend>` with explicit specializations for
  `std::string` and `std::any` is held open for future extension.
  No current consumer needs a third backend.
- **Typed `std::any` internal storage for Configurable properties**.
  Discussed during scoping; deferred.  Would require changes to
  config-file format semantics (today all round-trips through
  strings).  Separate future modernization pass.
- **GUIComponent stringification rework** (TODO item still open):
  will eliminate the `@pointer@:` encoding, making
  `qt::encode_pointer` / `decode_pointer` deletable.

---

## Previous State (Session 51 — thread-safe handle reads, `.out()` retired, DataStore on AnyMap, Event smuggling gone)

### Session 51 Summary

20 commits along a single coherent arc.  Closed out everything on
the Session 50 "DataStore / Assign" punch list plus the larger
`.out()` retirement that the atomic-cache work unblocked.  Net:
the `void* + RTTI-name` dispatch that had been the main source of
hacky type erasure in the GUI path is gone, the DataStore is
std::any-backed end-to-end, and every `Data::*` verb
(`render` / `install` / `link` / `registerCallback` / `enable` /
`disable` / `removeCallbacks`) is a one-line macro expansion over
a direct `std::any_cast<H>` type-cascade.

#### 1. Thread-safe handle reads (6 commits)

Problem: every value-carrying handle's `getValue()` reached into
the wrapped Qt widget from the application thread (e.g.
`SliderHandle::getValue()` called `QSlider::value()`).  Qt doesn't
document widgets as thread-safe for reads outside the GUI thread.

Pattern landed in two forms:

- **Slider / FSliderHandle** — the pre-existing
  `ThreadedUpdatableSlider` widget subclass gained
  `std::atomic<int> m_atomicValue`, seeded in both ctors and
  published from `collectValueChanged` (the GUI-thread slot wired
  to `valueChanged(int)` in the widget ctor — always connected
  before any external `connect` call).  Handle `getValue()` reads
  via `atomicValue()`.

- **Remaining 9 handles** (Spinner / Int / Float / CheckBox /
  Button / ButtonGroup / Combo / String / Color) — Option H
  chosen over Option W (no new widget subclasses): each handle
  owns a `std::shared_ptr<std::atomic<T>>` (or a mutex-guarded
  struct for string / color / combo index+text).  The primary
  ctor installs a `QObject::connect(widget, signal, widget,
  lambda)` with the widget as the connection's context object;
  the lambda captures the `shared_ptr` by value so the cache
  outlives every handle copy, and the connection dies with the
  widget.

- **Signal-ordering bug** discovered during testing: GUI.cpp
  connects `widget→ioSlot` before `allocValue<Handle>()` installs
  the cache-update lambda, so user callbacks (dispatched from
  ioSlot) ran before the cache updated — visible in
  `icl-filter-playground` as a "one selection behind" Combo lag.
  Fixed by reordering four widget ctors (Combo, Spinner, CheckBox,
  ButtonGroup) so the handle is allocated first.

- **`ButtonHandle::m_triggered`** upgraded from `shared_ptr<bool>`
  to `shared_ptr<std::atomic<bool>>` with `exchange(false)` read-
  and-clear semantics.

#### 2. `.out()` retirement (4 commits, 38 files migrated)

After atomic caches, `.out("name")` — which allocated a raw `int`
/ `bool` / `float` / `std::string` in the DataStore and updated
it via a Qt signal slot — no longer served any purpose; reading
via `gui["handle-name"]` dispatched through the registry and hit
the thread-safe cache.

- New `scripts/migrate-out-to-handle.py` mechanically rewrote
  consumer sites per the rule table (dead handle → drop; both
  named → keep reader's name; single `.out()` → promote to
  `.handle()`).  287 edits across 38 files; 1 ambiguous case
  (`video-player.cpp` reading both names) fixed manually.  3
  sites with dynamic-string keys (`"enable-obj-"+str(c)`) fixed by
  hand — the regex was intentionally conservative.

- User-API surface deleted: the `.out()` method and
  `GUIComponentWithOutput` subclass.  Every
  `GUIComponentWithOutput` inheritor (Button, CheckBox, Slider,
  FSlider, Int, Float, Spinner, ColorSelect, ButtonGroup)
  collapsed onto `GUIComponent` directly; the `Options::out` field
  and `@out=X` serialization removed.

- Producer-side removal: 11 `allocValue<primitive>(def.output(0),
  ...)` branches in `GUI.cpp` gutted, each with its cache field
  (`m_piValue` / `m_pfValue` / `m_stateRef` / `m_psOutput` /
  `m_uiIdx` / `m_psCurrentText`) and the processIO slot that used
  to mirror widget state into that primitive.  ProcessIOs became
  empty or reduced to the remaining side effect (LCD display
  update for sliders).

- Handle API cleanup: `CheckBoxHandle::m_stateRef` and
  `StringHandle::m_str` dropped from handle ctors (no longer need
  the external pointer); `CheckBoxHandle::check/uncheck` no longer
  write to the stateRef, they just call setCheckState and let the
  signal update the atomic cache.

- `ColorSelectGUIWidget` fix: was leaking a `Color4D` (allocated
  at the top of the ctor, then m_color reassigned to point into
  DataStore).  Replaced with inline `Color4D m_color;` member.

- Fallout cleanup: `gui.get<primitive>("key")` sites (27 across
  11 apps/demos) rewritten as `T v = gui["key"]` — snapshot reads
  via the handle rather than live references to a DataStore
  primitive that no longer exists.  `pipe.cpp`'s
  `bool *ppEnabled` indirection replaced with
  `std::function<bool()>`; `Widget.cpp`'s bciUpdateAuto /
  channelUpdateAuto got the same treatment.

#### 3. DataStore/Assign finalization (7 commits)

Picking up from Session 50's infrastructure (AnyMap, Assign trait,
AssignRegistry with compile-time + std::any runtime dispatch):

- **Flip the dispatch.**  `DataStore::Data::assign()` routes to
  `AssignRegistry::dispatch()`.  The ~830-line in-DataStore
  `AssignSpecial<>` / `create_assign_map` / `INST_NUM_TYPES` /
  `ADD_T_TO_T` / `register_assignment_rule` machinery deleted in
  one pass.  Identity enrollments (`enroll_identity<H>()`) added
  to every migrated handle's constructor + a consolidated
  `HandleIdentityEnrollments.cpp` for the remaining ones (Border,
  Box, Disp, FPS, Plot, Splitter, State).  `Event → H` rules
  re-enrolled under AssignRegistry via a concept-driven
  `Assign<H, Event>` specialization (later retired — see #4).

- **AnyMap → std::map.**  Swap the internal container from
  `std::unordered_map` to `std::map` for pointer-stability, so
  GUI code like `m_poHandle = &allocValue<H>(...)` keeps working.

- **Compose AnyMap inside DataStore.**  DataStore stops
  inheriting `MultiTypeMap`; instead composes
  `shared_ptr<AnyMap>` + `shared_ptr<recursive_mutex>` (preserving
  the shared-backing semantics GUI copy-assign relies on).
  `Data::m_entry` changes from `DataArray*` to `std::any*`;
  `operator=(T)` / `as<T>()` pack and unpack via
  `std::in_place_type<T>` (required to disambiguate
  std::any's templated ctor for types like `utils::Any` that
  publicly inherit `std::string`).  `AssignRegistry`'s transitional
  `void*` / name-table paths all deleted — one dispatch form
  (`dispatch(std::any&, std::any&)`) remains.  `MultiTypeMap.{h,cpp}`
  deleted (~312 lines) along with its `friend class` in
  `GUIHandleBase.h`.  The vestigial `GUI::release<T>` wrapper
  goes with it.

- **GUI::registerCallback/removeCallbacks fix.**  These used to
  `reinterpret_cast` through `getValue<GUIHandleBase>(key, false)`
  — safe only under `MultiTypeMap`'s void* storage, UB under
  `std::any`.  Rewritten to route through the `Slot` proxy's own
  `registerCallback` / `removeCallbacks` methods.

- **Retire the `Event` smuggling.**  `Data::render() /
  install() / link() / registerCallback() / enable() / disable() /
  removeCallbacks()` used to build an `Event` struct and push it
  through AssignRegistry to a per-handle `Assign<H, Event>`
  specialization.  Now that the store is `std::any`-backed,
  dispatch goes directly: new `qt/HandleVerbDispatch.cpp` owns a
  single `visitHandle()` type-cascade over a
  `std::tuple<AllHandles...>`, with per-verb capability concepts
  (`HasRender`, `HasInstallMouse`, etc.) gating the call via
  `if constexpr`.  Each `Slot` method collapses to a one-line
  `ICL_SLOT_VERB(Cap, action)` macro expansion — verb name comes
  from `__func__` so the log label can't drift from the method
  name.  `Event` struct + `Assign<H, Event>` specialization +
  `HandleEventEnrollments.cpp` deleted.

- **Kill void* on the Data API.**  `Data::install(void*)` →
  `Data::install(MouseHandler*)`; `Data::link(void*)` →
  `Data::link(GLCallback*)`.  `GLCallback` pulled out of its
  nested `ICLDrawWidget3D::GLCallback` scope into a top-level
  `icl::qt::GLCallback` class (new 40-line `qt/GLCallback.h`), with
  a backward-compat `using GLCallback = icl::qt::GLCallback;`
  alias inside the widget class preserving all 89 existing
  spellings.  One knock-on: `Scene2::getGLCallback` returned
  `shared_ptr<Scene2::GLCallback>` where the inner type was
  forward-declared — now returns `shared_ptr<qt::GLCallback>`
  (base) with the impl class moved to an anonymous namespace as
  `SceneGLCallback`.  One file-local name clash fixed in
  `camera-calibration-planar.cpp` (`struct GLCallback` renamed to
  `MultiViewGLCallback`).

- **Rename `DataStore::Data` → `DataStore::Slot`.**  Historical
  "Data" name was an artifact of `MultiTypeMap`'s "data arrays";
  `Slot` is the modern description of a one-entry view over the
  keyed store.  Purely cosmetic — same template API as before,
  `int v = gui["k"]` already worked without `.as<int>()`.

#### 4. Verification & test status

- `tests/icl-tests -j 1` — 607/607 pass at every commit.
- Full build of all 240 targets clean at every commit.
- GUI runtime surfaced a few fallouts handled as they appeared
  (`Any = FSliderHandle` assign rule missing; signal-ordering
  lag; `any_cast<GUIHandleBase&>` exact-match failure).  All
  captured in the history.

#### 5. Bugs surfaced; not fixed (deferred to TODO.md)

- **AffineOp: translation component ignored** (Filter section).
  Rotate and scale work; translate-x / translate-y sliders in
  `icl-filter-playground` don't shift the output.
- **Scale-range OSD button in ICLWidget behaves strangely** —
  symptom uncharacterized.
- **Qt6 `QOpenGLWidget` ctor crashes in sandboxed/macOS-26
  terminals** — Qt / Cocoa issue, not ICL.  Blocks runtime GUI
  verification in this environment.

#### 6. What's deferred

TODO.md's "DataStore / Assign migration" section is effectively
closed.  Remaining items are the surfaced bugs above + speculative
ones (core-type identity enrollments — not needed unless a
non-handle ends up in DataStore).  The GUIComponent
stringification rework the user flagged is in a new
"Qt GUI component plumbing" section, unrelated to this arc.

---

## Previous State (Session 50 — AnyMap + Assign<Dst,Src> infrastructure; 15 qt handles migrated)

### Session 50 Summary

One extended design conversation that landed a fresh, orthogonal
replacement for the `MultiTypeMap` / `DataStore::Assign` machinery and
then started mechanical per-handle migration onto it.  Key properties
of the new design: compile-time and runtime dispatch paths that share
one source of truth; no implicit conversion operators on handles
(they're a known overload-resolution foot-gun); explicit `as<T>()`
extraction for readback; `<Dst, Src>` template-arg order matching
`dst = src` and `std::is_assignable`.  Legacy `DataStore` untouched
throughout — both dispatch tables coexist.

#### 1. Infrastructure — `icl/utils/` (commit 9fdd01a6b)

Three new public headers, all header-only or tiny:

- **`AnyMap.h`** — thin typed wrapper around
  `std::unordered_map<std::string, std::any>`.  `set<T>`, `get<T>`
  (throws on miss/mismatch), `tryGet<T>` (nullptr), `contains` /
  `containsAs<T>`, `typeOf`, `erase`, `clear`, iteration.  Replaces
  `MultiTypeMap`'s `void* + RTTI-string + static T _NULL + len-bit-hack`
  machinery with plain `std::any`.  Not wired into DataStore yet;
  orthogonal for now.

- **`Assign.h`** — `Assign<Dst, Src>` trait collapsed to one template
  with two `apply()` overloads on disjoint constraints:

  ```cpp
  template<typename Dst, typename Src>
  concept DirectlyAssignable = std::is_assignable_v<Dst&, Src>;

  template<typename Dst, typename Src>
  concept ExtractableAs = requires(Dst &d, Src &s) {
    d = s.template as<Dst>();
  };

  template<typename Dst, typename Src>
  struct Assign : std::bool_constant<
      DirectlyAssignable<Dst, Src> || ExtractableAs<Dst, Src>>
  {
    static void apply(Dst &dst, Src &src)
      requires DirectlyAssignable<Dst, Src> { dst = src; }
    static void apply(Dst &dst, Src &src)
      requires (!DirectlyAssignable<Dst, Src> && ExtractableAs<Dst, Src>)
      { dst = src.template as<Dst>(); }
  };
  ```

  No per-pair specializations.  Direct path wins when both are viable.
  Class-side convention: `operator=(Src)` for incoming, `as<T>()`
  member template (constrained with `requires`) for outgoing.

- **`AssignRegistry.{h,cpp}`** — runtime type-erased dispatcher.
  **Fully static** public API (no `.instance().` noise at call sites;
  the singleton is private-internal).  Map stores bare function
  pointers via `+[](std::any&, std::any&){…}`, no `std::function`
  overhead.  API:

  ```cpp
  AssignRegistry::enroll<Dst, Src>();               // static_asserts
  AssignRegistry::enroll_symmetric<A, Bs...>();     // A↔B pairs
  AssignRegistry::enroll_receiver<Dst, Srcs...>();  // Dst = each Src
  AssignRegistry::enroll_provider<Src, Dsts...>();  // each Dst = Src
  AssignRegistry::dispatch(any &dst, any &src);
  AssignRegistry::has(type_index, type_index);
  AssignRegistry::size();
  ```

  `enroll` static-asserts `is_assignable_v<Dst, Src>` so runtime
  registration and compile-time trait stay in sync by construction.

#### 2. Per-handle migrations — `icl/qt/`

**Idiom** each migrated handle's `.cpp` picks up:

```cpp
namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::XxxHandle;
  __attribute__((constructor))
  static void icl_register_xxx_handle_assignments() {
    AssignRegistry::enroll_symmetric<XxxHandle, int, float, double, std::string>();
  }
}
```

— one-line enroll call per handle.  Symmetric/receiver/provider
variants handle the asymmetric cases.  **15 handles** landed across
6 commits:

| Commit | Handles | Shape |
|---|---|---|
| 30452ca07 | SliderHandle | symmetric value (pilot) |
| 063b4b8fa | IntHandle, FSliderHandle, SpinnerHandle, FloatHandle | symmetric value |
| 83c79a4b0 | *(infra: fully static API + enroll_symmetric)* | — |
| 7e0364dc6 | CheckBoxHandle, StringHandle, ButtonHandle, ButtonGroupHandle, TabHandle | mixed (+ `enroll_receiver`/`enroll_provider`) |
| cac200f0f | ComboHandle, LabelHandle | symmetric + receiver |
| 398c01f19 | ImageHandle, DrawHandle, DrawHandle3D, ColorHandle | receiver (images) + symmetric (Color/Color4D) |

Notable per-handle details:

- **ComboHandle**: dropped the legacy `operator int() const` and
  `operator std::string() const` implicit conversion operators — they
  were the exact kind of landmine we're avoiding.  One caller
  (`filter/apps/local-thresh.cpp`) had to be updated from
  `(int)comboHandle` to `comboHandle.getSelectedIndex()`.  Zero other
  callers in tree depended on the implicit operators.
- **StringHandle**: templated `operator=(T)` for arithmetic writes via
  `utils::str`; templated `as<T>()` for arithmetic reads via
  `utils::parse`.
- **DrawHandle3D**: was missing `operator=(const ImgBase&)` — legacy
  FROM_IMG used `setImage(&src)`.  Added it.
- **Image handles** (ImageHandle, DrawHandle, DrawHandle3D): each
  enrolls 18 image-type pairs (5 `Img<T>` by value, 12 pointer
  variants, `core::Image`) via a single `enroll_receiver<H, …>()`
  call.  All Img<T> paths fall through to
  `operator=(const ImgBase&)` via derived-to-base conversion.
- **Smuggled-command pairs retired**: `Event → handle` (for
  render/install/link) and `Range → SliderHandle` (setRange) do **not**
  map through the new trait.  Those idioms become direct method calls
  (`handle.render()`, `slider.setRange(a, b)`) in the new model.
- **Color↔Color4D cross-conversions** deliberately not ported into
  qt — they belong on the types themselves in core, not leaked into
  qt registration.

#### 3. Verification

- Clean builds after every commit (full target set + tests + demos).
- `tests/icl-tests -j 1` → **607/607 pass** throughout the migration.
- New test coverage: 16 `utils.assign.*` tests (compile-time trait,
  direct path, extract path, direct-wins priority, runtime dispatch
  for both paths, parity, enroll_symmetric variadic expansion,
  idempotent re-enrollment) + 24 `utils.anymap.*` tests.

#### 4. What's deferred

See `project_assign_migration.md` for the full punch list.  Highlights:

- **Identity enrollments** (`enroll<H, H>()`) for every migrated handle.
  Old DataStore had `ADD_T_TO_T` for each pair; new system doesn't yet.
  Needed before flipping DataStore — a consumer writing
  `gui["a"] = gui["b"]` with same-type handles needs this entry.
- **Core-type identity** (Rect, Size, Point, Image, std::string, Any)
  — same concern for non-handle types.  Needs a home outside qt.
- **Event-only handles** (PlotHandle, FPSHandle, BoxHandle, DispHandle,
  StateHandle, SplitterHandle, MultiDrawHandle) — had only `Event → H`
  + identity in old DataStore; not yet migrated.  Decide whether to
  skip entirely or add identity-only enrollment.
- **`Slot` proxy** — the `gui["key"]` return type.  Needs
  implicit-template `operator=(T)` and `operator T() const` that route
  through `AssignRegistry::dispatch`.  This is what makes
  `int v = gui["key"]` work without users writing `.as<int>()`.
- **`DataStore::Data::assign()` flip** — replace the legacy
  `AssignSpecial<>` map lookup with `AssignRegistry::dispatch()`;
  retire `DataStore::Assign`, `create_assign_map()`,
  `register_assignment_rule()`.
- **`MultiTypeMap` replacement** — swap DataStore's inherited
  `MultiTypeMap` for composed `AnyMap`.  Bigger API break.

---

## Previous State (Session 49 — math detail/ reorg + pugi privatization + DynamicGUI retirement)

### Session 49 Summary

Three landings, one theme: **strict `detail/` invariant**. Under the strict
rule (locked this session per user directive), anything under a module's
`detail/` subdirectory must never be reachable by an installed public
header. Previously `utils/detail/pugi/` was installed-anyway because
`utils/XML.h` pulled in pugi types — that was the anti-pattern. This
session fixes it, along with a related math reorg and a dead-code cull.

See `feedback_detail_strict_rule.md` for the rule itself.

#### 1. `icl/math/` detail/ reorg

Backend-dispatch facade files moved under per-family subdirs:

```
icl/math/detail/
  blas/     BlasOps.{h,cpp} + BlasOps_{Accelerate,Cpp,Mkl}.cpp
  fft/      FFTOps.{h,cpp}  + FFTOps_{Accelerate,Cpp,Mkl}.cpp
  lapack/   LapackOps.{h,cpp} + LapackOps_{Accelerate,Cpp,Eigen,Mkl}.cpp
  mathops/  MathOps.{h,cpp} + MathOps_Cpp.cpp
```

**19 files moved.** All 4 facade headers dropped from the `math_headers`
install list — they're genuinely private (zero external consumers; the
22 `#include`s of them are all from `.cpp`s inside `icl/math/` itself).
Facade types (`utils::BackendDispatching<>` subclasses named `BlasOps`,
`LapackOps`, `FFTOps`, `MathOps`) never appeared in any other public
header, so no API break. Top-level `icl/math/` shrinks 74 → 55 files.

Meson updates: per-subdir source paths + removal from `math_headers`.
Internal `#include`s rewritten: `<icl/math/BlasOps.h>` →
`<icl/math/detail/blas/BlasOps.h>`, etc. 22 include sites, all inside
`icl/math/`.

#### 2. Pugi privatization

The trigger: `utils/meson.build` had this anti-pattern block —

```
# Vendored pugixml lives under detail/pugi/ — it's only reached via
# icl/utils/XML.h (which aliases pugi:: types as icl::utils::XML*).
# The header must still be installed so XML.h is self-contained.
install_headers(files('detail/pugi/PugiXML.h', 'detail/pugi/pugiconfig.hpp'), …)
```

"detail/ but installed because a public header needs it" is exactly the
state the strict rule forbids. Fixed by making pugi actually private:

**`utils/XML.h` — deleted.** Contained 21 `using` aliases (`XMLDocument =
pugi::xml_document`, etc). Audit showed **zero** ICL public headers
consumed any of those aliases, and only 4 in-tree .cpps used them —
trivially switched to `pugi::` directly. Dropped from `utils_headers`,
removed from `Utils.h` umbrella include, deleted.

**`utils/ConfigFile.h` → pugi-free via PIMPL.** Four pugi references in
the public header, all now gone:

| Old | New |
|---|---|
| `ConfigFile(pugi::xml_document *handle)` ctor | **Deleted.** 2 external callers (ImageUndistortion, Camera) switched to existing `ConfigFile(std::istream&)` ctor. |
| `const pugi::xml_document *getHandle()` inline | **Deleted.** Zero callers. |
| `static void add_to_doc(pugi::xml_document&, …)` static private | Moved to anonymous namespace free function in `ConfigFile.cpp`. |
| `mutable std::shared_ptr<pugi::xml_document> m_doc` member | **PIMPL**: `struct Impl { pugi::xml_document doc; }; mutable std::shared_ptr<Impl> m_impl;` |
| `namespace pugi { class xml_document; }` forward decl | **Deleted.** |

The PIMPL member is `m_impl` (renamed from `m_doc` per user preference
— name reflects it's the opaque pimpl, not just the document).

**`utils/ConfigFile.cpp`** — swapped the 21 XML-alias references for
direct `pugi::xml_document` / `pugi::xml_node` / `pugi::xml_attribute` /
`pugi::xpath_query` / `pugi::xpath_node_set` / `pugi::xpath_node`.
`is_text_node`, `add_to_doc`, `get_id_path` moved into an anonymous
namespace. `#include <icl/utils/XML.h>` replaced by direct
`#include <icl/utils/detail/pugi/PugiXML.h>` (privately — the header is
no longer installed, but `.cpp`s can reach into `detail/` freely during
in-tree builds).

**`filter/ImageUndistortion.cpp`, `geom/Camera.cpp`** — dropped the
`new XMLDocument + doc->load(is)` pattern; now `ConfigFile(is)` directly.
Cleaner anyway; the pattern existed only because the `ConfigFile(pugi::…*)`
ctor used to take ownership of a raw pugi doc, which is no more.

**`io/detail/grabbers/OptrisGrabber.cpp`** — uses pugi directly now:
`pugi::xml_document`, `pugi::xpath_node`. Include path updated to
`<icl/utils/detail/pugi/PugiXML.h>` (private — this .cpp is itself under
`io/detail/`, so it's in-tree-only anyway).

**`geom/Primitive3DFilter.cpp`** — was already pugi-direct; no change.

**`utils/meson.build`** — dropped `install_headers(files('detail/pugi/PugiXML.h', 'detail/pugi/pugiconfig.hpp'), …)`. Pugi is now genuinely private: compiled into libicl-utils, not shipped in the install tree.

#### 3. DynamicGUI retirement

`icl/qt/DynamicGUI.{h,cpp}` surfaced during the pugi work — three private
methods had `pugi::xml_node` in their signatures (already forward-declared,
so the header compiled without pugi, but still leaked pugi names).

Audit: **zero in-tree consumers.** The only `#include <icl/qt/DynamicGUI.h>`
in the entire tree is `DynamicGUI.cpp` itself. No apps, demos, examples,
tests, other modules. Framework-DSL-for-out-of-tree-users or dead code —
the user confirmed it was dead ("cannot remember needing this at any
time"). Retired:

- `icl/qt/DynamicGUI.h` — deleted
- `icl/qt/DynamicGUI.cpp` — deleted
- `icl/qt/meson.build` — entries removed

(A brief intermediate state had DynamicGUI's pugi refs moved to anonymous-
namespace helpers in the .cpp — rendered moot by the subsequent deletion.
Not worth preserving in git history.)

#### Verification

- Full build clean (124 targets linked post-retirement).
- `tests/icl-tests -j 1` → **567/567 pass**, unchanged.
- `grep 'pugi::' icl/**/*.h` → zero hits outside vendored `detail/pugi/PugiXML.h` itself. Rule holds strictly.
- `grep '#include.*detail/' icl/**/*.h` → zero installed-header hits into `detail/`. Audit clean.

#### Memory writes this session

- `feedback_detail_strict_rule.md` — new. Captures the strict invariant
  "`detail/` ⟺ not installed; no installed header may reach into detail/".
- `project_module_subdirs.md` — updated: math now DONE; utils now fully
  DONE (pugi genuinely private).
- `project_utils_subdirs.md` — updated: XML.h deletion noted; pugi's
  not-installed status noted; cross-link to `feedback_detail_strict_rule.md`.
- `project_test_parallel_flakiness.md` — new, surfaced during verification.
  `tests/icl-tests` default-parallel loses 5-10 tests; `-j 1` passes all
  567; shared static/global somewhere in `Quick2` path.

#### What's deferred

- **The other module reorgs** (`icl/core/`, `icl/filter/`). `filter/`
  pairs with `project_filter_dispatch_arch.md` (splitting legacy Ops
  into `X.cpp / X_Cpp.cpp / X_Ipp.cpp / X_SSE.cpp`). Core is smaller.
- **Parallel-test flakiness root cause** — likely `QuickContext` pool
  (see `project_memorypool.md`) or a filter Op scratch buffer. Debug
  with `TEST_FOREACH` around a flaky test to reproduce reliably.
- All Session 48 deferrals still open: browser viewer for WSGrabber,
  `wss://`, additional codecs (`webp`, `jxl`, `lz4`), `Configurable`
  events on child-set change, capability-flag codec classification.

---

## Previous State (Session 48 — plugin-registry unification + ZMQ retirement)

### Session 48 Summary

Nine landings over one session, consolidated into two git commits
(`ce7b47aa7` + `4d464b558`). Every plugin-registration mechanism in the
codebase now runs on a single primitive (`utils::PluginRegistry<Key,
Payload, Context>`); the remaining façades are either thin
free-function accessors or one class (`GrabberRegistry`) that carries
real domain-specific side maps. Three plugin base classes gone, two
more façades gone, one network transport retired.

See `plugin-registry-plan.md` at repo root for the full phase-by-phase
intent; what's below is what actually shipped.

#### 1. The primitive (`utils::PluginRegistry<Key, Payload, Context>`)

New header `icl/utils/PluginRegistry.h` (~220 lines). Keyed-entry
registry with five orthogonal capabilities the various ICL registries
used to reimplement inconsistently:

- **Payload-agnostic**: Payload is a template parameter. Callers pick
  between `std::function<Sig>` (callable-plugin) or
  `std::function<std::unique_ptr<T>(Args...)>` (class-plugin).
- **Priority**: each entry has a `priority` int. `KeepHighestPriority`
  policy resolves conflicts deterministically across TU static-init
  orderings (replaces ImageMagick's previous dead `overrideExisting`
  bool — libpng at prio 0 now beats ImageMagick at prio -10 for `.png`
  by construction, not by linker luck).
- **Applicability predicate**: optional `ApplicabilityFn<Context>`.
  Used by `BackendSelector` for context-aware backend picking (depth,
  size, IPP-applicability, …). Classics leave it empty.
- **Forced-key override**: `setForced(key)` / `clearForced()` —
  testing affordance used by `BackendDispatching::forceAll` to
  cross-validate filter backends.
- **OnDuplicate policies**: `Throw` / `KeepFirst` / `Replace` /
  `KeepHighestPriority`.

Public API: `registerPlugin`, `get`, `getOrThrow`, `resolve`,
`resolveOrThrow`, `has`, `keys`, `entries`, `setForced`,
`unregisterPlugin`, `clear`.

Two canonical aliases:

```cpp
template <class Sig>
using FunctionPluginRegistry =
    PluginRegistry<std::string, std::function<Sig>>;

template <class T, class... CtorArgs>
using ClassPluginRegistry =
    PluginRegistry<std::string, std::function<std::unique_ptr<T>(CtorArgs...)>>;
```

One registration macro `ICL_REGISTER_PLUGIN(registry_expr, tag, ...)`
using `__attribute__((constructor, used))` — the only macOS-portable
mechanism that survives dead-stripping (established in Session 47 for
compression plugins; now applied universally).

18 unit tests in `tests/test-plugin-registry.cpp` covering both
aliases, all four policies, priority+applicability resolution, forcing,
exact-match vs predicate-based lookup, threading stress (8 threads ×
200 concurrent ops).

#### 2. `BackendSelector` refactored onto the primitive

The inner `impls` vector + manual sort + manual `setImpl` gone.
`BackendSelector<Context, Sig>` now holds a `PluginRegistry<Backend,
shared_ptr<ImplBase>, Context>` with `OnDuplicate::Replace`. Priority
= `static_cast<int>(backend)` (matches pre-refactor "higher enum value
= preferred" ordering).

`ImplBase` slimmed: description + applicability hoisted to the
registry's `Entry`; `cloneFn` stays for stateful backends.
`BackendSelectorBase::forcedBackend` field → virtual `force/unforce/
forcedBackend()` methods delegating to the registry's
`setForced/clearForced/forcedKey`.

`BackendDispatching<Context>` outer shell untouched (heterogeneous
multi-Sig container + enum-indexed selector array + clone ctor +
`forceAll`/`unforceAll` + `allBackendCombinations` + `BackendProxy`
fluent API — all stay, they address a distinct "outer" dimension that
the primitive doesn't).

Zero behavioural change; all filter tests pass. `BackendDispatching.h`
shrunk 378 → 327 lines.

#### 3. Classic registries migrated

| Old mechanism | Outcome |
|---|---|
| `utils::PluginRegister<T>` | **Deleted.** Replaced by domain-specific `pointCloudGrabberRegistry()` + `pointCloudOutputRegistry()` free-function accessors. `REGISTER_PLUGIN(TYPE, NAME, ...)` macro → domain-specific `REGISTER_POINT_CLOUD_GRABBER` / `REGISTER_POINT_CLOUD_OUTPUT`. TextTable rendering (`getRegisteredInstanceDescription`) moved to `pointCloudGrabberInfoTable()` / `pointCloudOutputInfoTable()` free functions. `creationSyntax` lives in a free-function-accessed side map. |
| `CompressionRegister` | **Demolished.** Replaced by free-function `compressionRegistry()` returning `ClassPluginRegistry<CompressionPlugin>&`. `REGISTER_COMPRESSION_PLUGIN` macro is now a one-line alias over `ICL_REGISTER_PLUGIN`. 5 codec .cpps + WSImageOutput.cpp swapped include to `<icl/io/CompressionRegistry.h>`. ImageCompressor's 3 call sites rewritten to `compressionRegistry().getOrThrow(mode).payload()` + `.keys()` + sort. |
| `FileWriterPluginRegister` | **Demolished.** Replaced by `fileWriterRegistry()` free function. `REGISTER_FILE_WRITER_PLUGIN` is a one-line alias. ImageMagick's priority-based loop calls `fileWriterRegistry().registerPlugin(...)` directly. |
| `FileGrabberPluginRegister` | **Demolished.** Symmetric. Plus: `HeaderInfo` struct hoisted from nested `FileGrabberPlugin::HeaderInfo` to namespace-scope `icl::io::HeaderInfo` (consumers: `JPEGDecoder.cpp`, `FileGrabberPluginPNM.cpp`, `FileGrabberPluginCSV.cpp`). |
| `GrabberRegister` | **Renamed to `GrabberRegistry`** — stays a class because it carries three orthogonal side maps (device-list per backend, bus-reset function per backend, per-backend description strings) that don't fit the primitive's `Entry`. Factory map now uses `PluginRegistry<string, CreateFn>` internally; side maps stay as plain `std::map`s on the class. `REGISTER_GRABBER` and `REGISTER_GRABBER_BUS_RESET_FUNCTION` macros upgraded to `__attribute__((constructor, used))` (drops the file-scope-static-struct idiom). Zero changes to the 19 grabber backend .cpp files. |
| `GenericImageOutput` (was hardcoded `#ifdef` switch) | **Flipped to registry-based dispatch.** `imageOutputRegistry()` accessor; each backend (`WSImageOutput`, `LibAVVideoWriter`, `OpenCVVideoWriter`, `V4L2LoopBackOutput`, plus built-in `null`/`file`) registers a `params → sender-callable` factory via `REGISTER_IMAGE_OUTPUT`. `init()` shrunk from ~140 lines of `#ifdef` chain to ~20 lines of registry lookup. `-o list` affordance auto-populated from registry entries. |

#### 4. Function-plugin conversions (3 base classes retired)

- **`ImageOutput` base class deleted.** 7 backends drop inheritance.
  WSImageOutput keeps `utils::Configurable` (its compression settings
  + port/clients/bytes properties are still exposed); its ImageOutput
  ancestry is just replaced by the class being directly-instantiable
  and registering itself with `imageOutputRegistry()`. `GenericImageOutput`
  itself no longer inherits anything; its `impl` is now
  `std::function<void(const Image&)>` instead of `shared_ptr<ImageOutput>`.

- **`FileWriterPlugin` base class deleted.** 6 plugins (PNG, JPEG, CSV,
  PNM, BICL, ImageMagick) drop inheritance. Each plugin's registration
  lambda uses a **per-macro-type function-local static** instance for
  state, avoiding the dyld-init-time construction that caused the
  Session 47 BICL/CompressionRegister ordering bug. BICL's multiple
  variants (rle1/4/6/8, jicl) each get their own distinct lambda type
  → their own static instance with distinct ctor args.

- **`FileGrabberPlugin` base class deleted.** Symmetric conversion.
  `HeaderInfo` struct hoisted to namespace scope (see above).

#### 5. ZMQ backend retired (~260 LOC)

Deleted `ZmqGrabber.{h,cpp}`, `ZmqImageOutput.{h,cpp}`, the
`libzmq`/`cppzmq` dep detection in root `meson.build`, the `zmq`
option in `meson.options`, the `libzmq3-dev` apt install and
`BUILD_WITH_ZMQ=ON` CMake flag in CI, and all doc references.

Why retire: `ws` (Qt6 WebSockets, added Session 46) covers the same
pub/sub-over-network use case with auto-reconnect resilience, browser
compatibility, and lower build-config weight. ZMQ's additional value
(cross-language interop) has no active consumer in the ICL ecosystem.
Strong circumstantial evidence of disuse: `ZmqImageOutput::send` had
been missing its class qualifier (`void send(...)` instead of `void
ZmqImageOutput::send(...)`) — a latent undefined-virtual that would've
been a runtime crash or linker error for anyone actually using it.
Parallels the SharedMemory retirement in Session 47.

Pre-existing bugfix discovered in passing: the same missing-qualifier
bug also affected `V4L2LoopBackOutput::send`. Fixed.

#### 6. Priority-based conflict resolution (replaces dead `overrideExisting`)

FileWriter / FileGrabber registries now use
`OnDuplicate::KeepHighestPriority`. Register signature:
`registerExtension(ext, factory, int priority = 0)`. ImageMagick
registers all its extensions at `priority = -10` (fallback); libpng /
libjpeg register at default priority 0 and win for .png/.jpg/.jpeg
deterministically. Extensions ImageMagick uniquely handles (tiff,
gif, bmp, svg, …) resolve to ImageMagick unopposed.

The dead `overrideExisting = true` flag — nobody actually passed it —
is gone.

#### 7. `V4L2LoopBackOutput::send` + `ZmqImageOutput::send` bugfix

See §5 above. Latent undefined-virtuals caused by missing
`ClassName::` qualifier on the method definitions. ZMQ version fixed
then deleted; V4L2 version kept (the V4L2 backend is still present).

#### Final layout

| Entity | Kind |
|---|---|
| `utils::PluginRegistry<Key, Payload, Context>` | primitive template |
| `utils::FunctionPluginRegistry<Sig>` | alias (callable payload) |
| `utils::ClassPluginRegistry<T, CtorArgs...>` | alias (factory-producing-unique_ptr payload) |
| `ICL_REGISTER_PLUGIN(registry_expr, tag, ...)` | macro (attribute-constructor) |
| `compressionRegistry()` | free-fn accessor |
| `fileWriterRegistry()` | free-fn accessor |
| `fileGrabberRegistry()` | free-fn accessor |
| `imageOutputRegistry()` | free-fn accessor |
| `pointCloudGrabberRegistry()` / `pointCloudOutputRegistry()` | free-fn accessors (with side-map + TextTable helpers) |
| `GrabberRegistry` (class) | façade with side maps for device-list / bus-reset / descriptions |
| `BackendSelector<Context, Sig>` (class) | Sig-specialization over the primitive, for filter backend dispatch |
| `BackendDispatching<Context>` (class) | outer container over N BackendSelectors, for filter Op prototypes |

Every `REGISTER_*` macro in the codebase now expands to
`__attribute__((constructor, used))`. Every plugin-registration
mechanism is backed by one primitive.

#### Verification

- Full build clean throughout (per phase)
- 551 → 567 tests (18 new `utils.plugin-registry.*` + 1 `get_or_throw`
  + 1 `policy.keep_highest_priority`). Zero regressions.
- End-to-end smoke: `icl-pipe -i create lena -o ws PORT` + `-i ws PORT
  -o file '/tmp/###.png'` round-trip. PNG write/read. BICL write.
  `-o list` + `-i list` affordances auto-populated from registries.

#### Memory writes this session

- `project_plugin_registry_unification.md` — rewritten with the locked
  design decisions (function-plugin vs class-plugin split, `Register`
  → `Registry` rename intent, façade-demolition map, open questions
  for future cleanup).
- `project_module_subdirs.md` — new. TODO: consider `detail/` subdirs
  per module (top-level = public API only; implementation-only files
  like per-backend grabbers and per-extension plugins go under
  `module/detail/<group>/`; second-order groupings like
  `detail/pylon/`, `detail/video/`, `detail/network/`,
  `detail/file-plugins/`, `detail/compression-plugins/`). Separate
  session — best done after the plugin-registry work stabilizes
  (done now).
- `feedback_sed_sandbox.md` — updated. Use `perl -pi -e '...'` (not
  `sed -i`) for bulk in-place substitutions in Agent sessions.

#### What's deferred (post-Session-48)

- **Directory reorganization** — `module/detail/<group>/` structure
  per `project_module_subdirs.md`. Biggest cleanup still pending;
  touches the entire source tree.
- **Capability-flag codec classification** (lossy/lossless +
  supported depths) — enables `auto` codec mode via the primitive's
  `applicability` machinery. Currently deferred.
- **Additional codecs**: `webp`, `jxl`, `lz4`, `deflate`/`zlib`.
- **`Configurable` events on child-set change** (`qt::Prop`
  auto-rebuild when `ImageCompressor` swaps codec) — surfaced in
  Session 47, unblocked by this session but not done.
- **Browser viewer for WSGrabber** (JS-side envelope parser).
- **`wss://` TLS** for WS transport.
- **WSGrabber server mode** (push-source workflow).
- **Path-based multi-stream on one WS server** (`/cam0`, `/cam1`).

---

## Previous State (Session 47 — SharedMemory retired + ImageCompressor → plugin framework + FileWriter/Grabber factory-ized)

### Session 47 Summary

Three landings, related by a common theme (uniform plugin-registration
across ICLIO).

#### 1. Retired the SharedMemory backend (1274 LOC)

- Deleted `SharedMemoryGrabber.{h,cpp}`, `SharedMemoryPublisher.{h,cpp}`,
  `SharedMemorySegment.{h,cpp}`. Removed from meson, GenericGrabber.h,
  GenericImageOutput.{h,cpp}, IO.h, V4L2LoopBackOutput.h (dead include),
  ZmqGrabber.h (stale comment).
- Migrated 4 consumers off `sm`: `SceneMultiCamCapturer.cpp` and
  `physics-paper-SceneMultiCamCapturer.cpp` now use `ws=PORT`;
  `Widget.cpp`'s auto-cap Combo now offers `ws` instead of `sm` (and
  finally consumes the `FILE/VIDEO/XCFP/SM`-style "active" markers it
  was preparing all along — that was dead code).
- WSImageOutput/Grabber doxygen note that they replace SM.
- Why: WS loopback covers the same loose-coupling-between-processes use
  case with auto-reconnect resilience and cross-host as a free bonus,
  for ~100 µs more latency (invisible at typical 30 fps workloads).

#### 2. ImageCompressor → plugin framework

`ImageCompressor` is now a thin **facade** over a process-wide
`CompressionRegister`. Built-in codecs (`raw`, `rlen`, `jpeg`, `1611`)
are now plugin classes that self-register at static init via
`REGISTER_COMPRESSION_PLUGIN`; new codecs drop in as a single .cpp.
**`zstd`** added as a proof-of-extensibility (optional dep on libzstd —
`brew install zstd` on macOS, gated by `ICL_HAVE_ZSTD`).

Wire format **broken vs the pre-Session-47 `Header::Params` POD** (per
explicit user OK — "no backwards compat required"). New envelope is a
46-byte fixed prefix + variable-length codec-name / codec-params /
image-meta / payload. Codec name has no length cap (so codecs longer
than 4 chars work, e.g. `deflate`).

Each plugin inherits `Configurable` so per-codec tunables auto-surface.
`ImageCompressor` itself inherits `Configurable` and exposes a `mode`
property (menu populated from the registry) plus the **active plugin
as a child Configurable** with empty prefix — its `quality`/`level`/etc.
appear as siblings of `mode` and switch when the codec changes. When
`ImageCompressor` is itself added as a child of (e.g.) `WSImageOutput`
under prefix `compression`, the user sees `compression.mode` plus
`compression.quality` / `compression.level` / etc.

**Implemented `Configurable::removeChildConfigurable`** along the way
(previously a stub that threw `"is not yet implemented"`) so codec
swaps actually work.

`ImageOutput`'s historic `protected ImageCompressor` inheritance dropped
— the legacy `setCompression`/`getCompression` re-exports were the only
users; consumers that need compression now own an `ImageCompressor`
explicitly and expose it as a child Configurable (clean diamond-free
inheritance hierarchy).

#### 3. FileWriter + FileGrabber → factory-based plugin map

The static initialization order trap: `FileWriter::s_mapPlugins[".bicl"]
= new FileWriterPluginBICL` ran during dyld init, eagerly constructing
an `ImageCompressor`, which queried the (still-empty) `CompressionRegister`
and threw — aborting the rest of dyld's init pass and leaving the
process with a permanently empty registry.

Fix: make both file-format plugin maps factory-based, mirroring
`REGISTER_GRABBER` and `REGISTER_COMPRESSION_PLUGIN`:

- `icl/io/FileWriter.h`: new `FileWriterPluginRegister` singleton +
  `REGISTER_FILE_WRITER_PLUGIN(tag, ext, factory)` macro using
  `__attribute__((constructor, used))`.
- `icl/io/FileGrabber.h`: symmetric `FileGrabberPluginRegister` +
  `REGISTER_FILE_GRABBER_PLUGIN`.
- Each `FileWriterPluginXxx.cpp` / `FileGrabberPluginXxx.cpp` adds its
  registrations at the bottom (one line per extension; ImageMagick uses
  a custom `__attribute__((constructor))` for its long extension list).
- The old `FileWriterPluginMapInitializer` static class and the
  function-local-static `find_plugin` map are gone.
- Plugin instances are built lazily on first lookup and cached for the
  lifetime of the process (semantically identical to the previous
  shared-instance model, but no static-init-time construction).

With both file-format plugin systems factory-based, **no `ImageCompressor`
is ever constructed before main**, so its constructor can again be eager
(install the active plugin immediately). Consumers don't need to know
about any laziness — `ImageCompressor` "just works".

**Static-init lesson learned (worth recording):**
`__attribute__((constructor))` on a free function is the only macOS-portable
way to guarantee dyld-time invocation. Anonymous-namespace static-storage
objects with non-trivial constructors *can* be dead-stripped at the .o
level even though their `__GLOBAL__sub_I_*` symbol survives in `nm`.
LLVM/Clang/GoogleTest/Boost-Test all use the constructor-attribute idiom
for this reason.

#### Verification

- Full build clean.
- 551/551 tests pass sequentially (was 547; +4 new compression-framework
  tests: `CompressionRegister.builtins_registered`,
  `ImageCompressor.raw.roundtrip`, `ImageCompressor.auto_detect_codec`,
  `ImageCompressor.zstd.roundtrip`).
- End-to-end smoke: `icl-pipe -i create lena -o ws 9999 -no-gui` +
  `icl-pipe -i ws 9999 -o file '/tmp/v4_###.png' -no-gui` produces
  24 PNGs/2s. WSGrabber introspection (`icl-camera-param-io -i ws PORT
  -l`) shows the full property tree.

#### Memory writes this session

- `reference_websocket.md` — extended with the SM-replacement note (was
  written in Session 46, this session validates and extends).
- `project_dynamic_child_configurables.md` — TODO: `qt::Prop` doesn't
  auto-rebuild when child Configurables are added/removed at runtime
  (surfaced when ImageCompressor swaps codec plugins).
- `project_plugin_registry_unification.md` — TODO: collapse the now-4
  plugin-registration patterns (GrabberRegister, CompressionRegister,
  FileWriterPluginRegister, FileGrabberPluginRegister, plus the filter
  backend dispatch idiom) into one generic `utils::PluginRegistry<T>`
  template + one macro family. Estimated ~4-6 hours.

#### What's deferred

- **Generic plugin registry in ICLUtils** (per `project_plugin_registry_unification.md`)
- **Capability-flag-based codec classification** (lossy vs. lossless +
  supported depths) — surfaces in the v2 `auto` codec mode + future
  `auto-but-non-lossy` mode hint
- **`auto` codec mode** that picks per-frame based on entropy/sparsity
- **Additional codecs**: `webp`, `jxl`, `lz4`, `deflate`/`zlib`
- **Browser viewer** for WSGrabber (would need a JS-side envelope parser)
- **`Configurable` events on child set change** so `qt::Prop` can
  rebuild when codec swaps

---

## Previous State (Session 46 — WebSocket image I/O via Qt6 WebSockets)

### Session 46 Summary

Added a WebSocket-based image transfer pair to ICLIO for loose coupling
between ICL processes (and potentially browser viewers later).

#### Files added

- `icl/io/WSImageOutput.{h,cpp}` — server side (`QWebSocketServer`),
  broadcasts every `send()`-ed image to all connected clients. PIMPL,
  Configurable.
- `icl/io/WSGrabber.{h,cpp}` — client side (`QWebSocket`) with
  auto-reconnect state machine (Disconnected → Connecting → Connected),
  exponential backoff (250 ms → 5 s capped), bounded frame queue
  (drop-oldest), block-with-timeout + replay-last semantics in
  `acquireImage()`.
- `tests/test-quick-io.cpp` — three new tests:
  `WS.loopback.roundtrip`, `WS.multi_client.broadcast`,
  `WS.client_survives_server_restart`.
- Memory: `reference_websocket.md`.

#### Wiring

- meson detects `Qt6 WebSockets` (`brew install qtwebsockets` on macOS),
  defines `ICL_HAVE_QT_WEBSOCKETS`, gates the new sources in
  `icl/io/meson.build`. Build-clean either way.
- `WSGrabber` registers itself with the GenericGrabber plugin map via
  `REGISTER_GRABBER(ws, ...)` — `-i ws ws://host:port` works through any
  GenericGrabber-driven app.
- `WSImageOutput` plugged into `GenericImageOutput.cpp`'s switch — `-o
  ws PORT` (or `-o ws BIND:PORT`) works through `icl-pipe` etc.

#### URL form

```
-o ws PORT                # server: bind 0.0.0.0:PORT, broadcast (LAN-open)
-o ws BIND:PORT           # server: bind a specific interface
-i ws ws://host:port      # client: connect & receive (auto-reconnect)
```

Server bind defaults to `0.0.0.0` per the loose-coupling-between-machines
use case. Server-mode WSGrabber (`-i ws server:PORT`, push-source) is
deferred to v2.

#### Wire format

`ImageCompressor::Header` + compressed payload (one binary WS frame per
image). Default mode = `none` (raw bytes, lossless). The `compression`
property on WSImageOutput exposes ImageCompressor's full menu (`none`,
`rlen`, `jpeg`, `1611`).

#### Resilience

The WSGrabber's reconnect state machine on a private QThread papers over
server outages: on disconnect, schedules a `QTimer::singleShot` retry
with exponential backoff. `acquireImage()` blocks for up to `block
timeout ms` (default 1000), then either replays the last successfully
decoded frame (default true) or returns null. The application loop never
observes a dead state.

#### Threading + bootstrap

QWebSocket(Server) needs an event loop. Each WS class owns its own
QThread that runs the local event loop — no main-thread `exec()`
required. But `QObject` machinery requires *some* `QCoreApplication` to
exist in the process. ICL apps using `ICLApp::exec()` already create a
`QApplication`. Headless callers (`icl-pipe -no-gui`, library users
embedding ICL without Qt main loops) are covered by a static
`ensureQCoreApplication()` lazy bootstrap inside both WS classes' ctors.
Tests get a `QCoreApplication` from `tests/icl-tests.cpp main()` via a
new `#ifdef ICL_HAVE_QT` block.

#### Verification

- Full build clean (118 binaries, all module libs, including
  `WSImageOutput.cpp.o` and `WSGrabber.cpp.o`).
- 546/546 tests pass sequentially (was 543; +3 WS tests).
- End-to-end smoke: `icl-pipe -i create lena -o ws 19090 -no-gui`
  publisher + `icl-pipe -i ws ws://127.0.0.1:19090 -o file
  '/tmp/wspipe_###.png' -no-gui` receiver wrote 24 PNG frames in 2 s.
- Reconnect path proven by `WS.client_survives_server_restart`: the same
  grabber receives frames from a brand-new server brought up on the
  same port post-disconnect.

#### Doxygen / discoverability

- `GenericGrabber.h` backend-list comment now includes `ws` and `zmq`.
- `GenericImageOutput.h` backend-list comment now includes `ws`.

#### What's deferred (per `reference_websocket.md`)

- `wss://` (TLS)
- WSGrabber server mode (push-source workflow)
- Path-based multi-stream on one server (`/cam0`, `/cam1`)
- Generic `CompressionPlugin` interface + `auto` codec mode (drops
  zstd/webp/jxl/lz4/etc. into the same envelope without touching
  `ImageCompressor.cpp`)
- Promote `ImageOutput` base to `Configurable`
- Browser viewer

---

## Previous State (Session 45 — ICLIO demos retired, apps modernized)

### Session 45 Summary

Audited ICLIO module (12 apps + 3 demos) following the playbook from
Session 43 (Filter audit). Net: 3 demos + 2 apps retired, 1 demo
converted to a test, 4 latent bugs fixed, dead code stripped from
2 apps, two apps modernized, framework callback type upgraded to
`std::function`.

#### A. Demos folder is now empty

All three IO demos retired:
- **`png_write_test`** → already covered by `Quick2.IO.save.load.png`
- **`undistortion`** → subsumed by the `@udist=file.xml` qualifier on
  every GenericGrabber-driven app (`GenericGrabber.cpp:273` calls
  `enableUndistortion(propVal)` directly during init)
- **`depth_img_endcoding_test`** → converted into two real tests in
  `tests/test-quick-io.cpp`:
  - `ImageCompressor.1611.lossless_in_range` (lossless 11-bit pack
    with quality="1")
  - `ImageCompressor.1611.clamps_above_11bit` (overflow → mask 2047,
    not modulo wrap)
  - Discovered while writing the tests: `"1611"` quality `"0"` is the
    *lossy* depth-mapping variant (`pack16to11_2`/`unpack11to16_2`,
    applies the Kinect Z formula); quality `"1"` is the lossless
    bit-pack. The original demo used quality `"0"` so its visual
    "roundtrip" was actually lossy.

#### B. Apps retired (2)

- **`icl-k2`** — bypassed ICL's own `Kinect2Grabber` to talk to
  libfreenect2 directly (~110 lines, with pointless GLFW init). Use
  `icl-pipe -i kinect2 0` instead.
- **`icl-dcdeviceinfo`** — trivial `DCGrabber::getDCDeviceList()`
  wrapper. Subsumed by `-i dc 0@info` on any GenericGrabber-driven app.

Remaining IO apps (8 + 2 dc1394-only): camera-param-io, convert,
create, jpg2cpp, multi-viewer, pipe, reset-bus, video-player +
dcclearisochannels, reset-dc-bus.

#### C. Real bugs fixed (4)

1. **`icl-convert`** — every typed flag (`-size`, `-format`, `-scale`,
   `-depth`) crashed with `parse<T>: type is not stream-extractable`.
   Root cause: `parse<X>(pa("-y"))` doesn't compile cleanly for ProgArg
   inputs; should be `pa("-y").as<X>()`. Same file already used the
   correct form in one place.
2. **`icl-convert`** `-scalemode` was silently ignored — inner
   `std::string sm = pa("-scalemode").as<std::string>();` shadowed the
   outer `scalemode sm`, then `sm = interpolateNN;` assigned an enum
   into the *string* variable (compiles via implicit char conversion!),
   never updating the actual scalemode used. Fixed by renaming the
   local string `s`.
3. **`icl-camera-param-io`** had two duplicate `pa_explain` entries:
   `-p` was described twice (first wrong, "grabber type"; second
   correct, "parameter file"), and `-g` was described twice (second
   actually for `-go`/`-grab-once`). Help output dropped the first
   in each pair. Fixed: each flag described once with the right
   meaning.
4. **`icl-jpg2cpp`** with a path-containing input filename emitted
   invalid C++ identifiers (`aauc_Data_/tmp/lena[NROWS][NCOLS]`).
   Documented caveat, but trivially fixed with
   `std::filesystem::path::stem()`.

Plus the `icl-dcclearisochannels` source had unqualified `vector<>`
(would not compile even with libdc); fixed.

#### D. Dead code stripped

- **`icl-pipe`**: `pthread.h` include + retired
  `EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK` comment, commented-out
  `setIgnoreDesiredParams` block, ~14-line commented-out
  `-reinterpret-input-format` block + matching `-dist` arg, "interactive
  clip mode is not yet implemented" branch + corresponding help text.
- **`icl-video-player`**: dead globals `disableNextUpdate`,
  `mouseInWindow`. Old name `ICLApplication` → `ICLApp`.

#### E. Apps modernized (3)

1. **`icl-pipe`** — the `-pp <name>` filter ladder used to be a hard-coded
   string menu (`gauss/gauss5/median/median5`); now also accepts any
   UnaryOp class registered with `REGISTER_CONFIGURABLE` (CannyOp, FFTOp,
   BilateralFilterOp, GaborOp, ConvolutionOp, MedianOp, ... — all 29
   from Session 43). The fallback uses `Configurable::create_configurable`
   and a `dynamic_cast<UnaryOp*>`; raw `UnaryOp*` static became
   `unique_ptr`. Help text updated; bad names get a friendly error
   instead of an uncaught exception.
2. **`icl-convert`** — one-shot CLI no longer uses static buffers for
   intermediate ImgBase pointers. Each stage (convert / flip / rotate /
   crop) now produces an `Image` (shared_ptr-backed) which auto-releases
   on reassignment. Added missing `return 0;` and second `return -1;`
   on the bad-crop-rect error path.
3. **`icl-multi-viewer`** — collapsed `template<int N> void run()` plus
   8 explicit `if(nInputs > N) app.addThread(run<N>);` blocks (lines
   213-220) into a single `void run(int n)` registered via
   `app.addThread([i]{ run(i); })`. Required upgrading the framework
   callback type (see F).

#### F. Framework: ICLApplication callback type → std::function

`icl::qt::ICLApplication::callback` was `void(*)(void)` (raw function
pointer, 2006-style). Upgraded to `std::function<void()>` so capturing
lambdas work. Function pointers still convert implicitly so every
existing `app.addThread(my_run_fn)` call site is unchanged. Internal
`ExecThread` ctor also updated to take callback by value + move. Net
cost: one indirection per thread tick (invisible at typical 30-fps
loops). Net win: lambdas become first-class throughout the framework.

#### G. Verification

- Full build clean (118 binaries linked, down from 121 — 3 demos +
  2 apps retired, 1 net new test pair added).
- 543/543 tests pass sequentially. Parallel-run flakes (4 in Quick2
  Math/Filter/Compose) reproduce on master, unrelated.
- Smoke-tested the modernized apps: `icl-pipe -pp gauss5` (legacy
  mnemonic), `icl-pipe -pp CannyOp` (registry fallback), `icl-pipe -pp
  NotAnOp` (friendly error), `icl-convert -size … -scalemode NN`,
  `icl-convert -rotate 30`, `icl-convert -flip both`, `icl-convert
  -c x y w h`, `icl-multi-viewer` in both sync and async modes.

#### H. What's next

Filter playbook step matched on IO. Possible next targets within IO:
- ImageMagick 7 PixelPacket → Quantum (per `project_imagemagick7.md`)
- LibAVVideoWriter for FFmpeg 6/7 (per `project_ffmpeg.md`)
- Qt6 multimedia grabbers (per `project_qt6_multimedia.md`)
- Or move on to **ICLCV** (next module in the dependency chain).

---

## Previous State (Session 44 — io::Grabber mutex pattern + entry into ICLIO audit)

### Session 44 Summary

Ported the `UnaryOp::m_applyMutex` pattern (Session 43, see
`project_configurable_op_threadsafety.md`) into `io::Grabber` as a
preventative against the analogous race (GUI/control thread firing
`processPropertyChange` mid-`acquireImage()`). Same shape, same rationale —
a property mutation that rebuilds backend state (size, format, depth,
exposure, gain, mode menus, etc.) must not interleave with the grab
thread's read of that state.

#### Grabber base changes
- `protected: mutable std::recursive_mutex m_grabMutex;` added.
- `void registerCallback(const Configurable::Callback &cb);` overload added,
  wrapping `cb` with a `scoped_lock(m_grabMutex)` before dispatch. Mirror
  of `UnaryOp::registerCallback`. `using Configurable::registerCallback;`
  re-exposes the base overload list.
- `Grabber::grab(ImgBase**)` acquires `m_grabMutex` at the top — single
  funnel for every backend's `acquireImage()` + `adaptGrabResult` + warp
  undistortion. No subclass changes required for reader-side coverage.

#### Subclass migrations (writer-side)
All 16 Grabber subclass ctors swapped from
`Configurable::registerCallback([this](Property &p){processPropertyChange(p);})`
→ unqualified `registerCallback(...)` so the wrap takes effect:
- always-built: CreateGrabber, FileGrabber, DemoGrabber, SharedMemoryGrabber,
  GenericGrabber, OpenCVCamGrabber, OpenCVVideoGrabber
- platform-conditional (mechanical rename, can't local-build): DCGrabber,
  V4L2Grabber, KinectGrabber, Kinect2Grabber, OpenNIGrabber,
  SwissRangerGrabber, XiGrabber

OptrisGrabber and PylonGrabber don't register property callbacks at the
Grabber level so they didn't need migration. Helpers that are Configurables
but not Grabbers (DCDeviceFeatures, OpenNIUtils generator-options,
PylonCameraOptions) intentionally still call `Configurable::registerCallback`
— they're outside the Grabber inheritance chain and have their own locking
strategies (often a per-class `m_propertyMutex` already).

#### Verification
- Full build clean (240/240 targets) on macOS with the always-built
  backends.
- Test suite: 541/541 pass when run sequentially (`-j 1`). Parallel runs
  (`-j 16`, the harness default) show 4 pre-existing flakes
  (Quick2.Math.scalar.add, Quick2.Math.chained, Quick2.Filter.filter.chain,
  Quick2.Compose.vconcat.triple) — confirmed identical behaviour against
  the unmodified tree, unrelated to this change.

#### What's next — ICLIO audit (after Filter)
Module dependency order is `…→Filter→IO→CV→Qt→…`, so IO is the natural
next module to audit. Suggested entry points:
- Run each grabber backend's demo/app under `icl-camviewer -i <backend>`
  to confirm the property-callback path is exercised post-migration.
- Look at `icl-pipe`, `icl-stream-server`, `icl-create`, `icl-image-viewer`,
  `icl-rtsp-streamer`, `icl-recorder`, `icl-video-recorder` etc. — same
  Configurable-driven UI pattern as the filter audit suggests these may
  benefit from `qt::Prop(&grabber)` to auto-render device controls
  (would need a brief look at how CamCfgWidget already does it).
- ImageOutput counterpart: spot-checked — ImageOutput is a minimal
  `send(Image)` interface, NOT a Configurable, no property callbacks. No
  race surface analogous to Grabber. Skip.

---

## Previous State (Session 43 — ICLFilter Configurable migration + filter-playground)

### Session 43 Summary

Completed Phase 1 of the ICLFilter migration plan (see
`project_filter_playground.md` memory): 29 UnaryOps ported from hand-rolled
setters/getters to `utils::Configurable` properties, a unified
`icl-filter-playground` app built that auto-generates the UI from any Op's
properties, and 9 redundant per-op demos + the `UnaryOp::fromString`
string-registry deleted (~970 lines net). Several latent framework bugs
found and fixed along the way. Four Configurable-level extensions landed
so the migration could avoid per-Op boilerplate.

Final op count by family:

- **Arithmetic/logic/compare:** UnaryArithmeticalOp, UnaryCompareOp, UnaryLogicalOp
- **Thresholding:** ThresholdOp, LocalThresholdOp
- **Neighborhood:** ConvolutionOp, MedianOp, MorphologicalOp, WienerOp
- **Affine:** AffineOp + RotateOp/ScaleOp/TranslateOp (inherit AffineOp with
  irrelevant knobs hidden via `deactivateProperty` regex filters) +
  MirrorOp (BaseAffineOp direct)
- **Derivative/gradient:** CannyOp, GradientOp (new — supersedes the
  non-UnaryOp `GradientImage` class which was retired)
- **Color/LUT:** LUTOp, PseudoColorOp, DitheringOp
- **Bank/feature:** GaborOp (with live kernel-preview image property),
  BilateralFilterOp, MotionSensitiveTemporalSmoothing (MSTS)
- **Transform/rescale:** FFTOp, WarpOp, FixedConvertOp, IntegralImgOp,
  ChamferOp, WeightChannelsOp, WeightedSumOp

Skipped: ProximityOp (BinaryOp + apply currently unimplemented),
ImageRectification (needs 4-point quadrangle input, not a straightforward
UnaryOp).

#### A. Configurable framework extensions

1. **"image" property type + type-erased `Property::payload`** —
   `utils::Configurable::Property` gained a `std::any payload` field and
   two new virtuals (`setPropertyPayload` / `getPropertyPayload`). The Qt
   `Prop` widget now renders an embedded `Display` for `type == "image"`,
   polled on the property's volatileness timer by a new
   `VolatileImageUpdater`. `core::Image` is ref-counted via shared_ptr so
   the handoff is cheap. First consumer: GaborOp's kernel preview —
   visible in the playground without any bespoke GUI plumbing. Design
   captured and then resolved in `project_configurable_image_type.md`.

2. **UnaryOp-level apply/callback mutex** — `UnaryOp` now owns a
   protected `mutable std::recursive_mutex m_applyMutex` and overrides
   `registerCallback` to auto-wrap every callback with a lock on that
   mutex. Subclasses only need one line (`std::scoped_lock lock(m_applyMutex);`
   at the top of their `apply()`). Hit after 3 open-coded consumers
   (filter-swap UAF, GaborOp vector race, WienerOp mid-apply mask race).
   Design captured and resolved in `project_configurable_op_threadsafety.md`.

3. **GUIComponents `String` now escapes commas** — initText with literal
   commas (e.g. CSV defaults like `"0.299,0.587,0.114"` for
   WeightChannelsOp) was tripping the GUI definition parser's
   comma-split. Now backslash-escapes commas and backslashes before
   concatenation; the parser's StrTok (configured with `\\` as escape
   char) unescapes on split. Latent bug, affected anyone with commas in
   `String` defaults.

4. **Collapsed setter boilerplate** — all migration sites went from
   `prop("X").value = str(v); call_callbacks("X", this);` to
   `setPropertyValue("X", v);` (existing Configurable API — just wasn't
   being used by the pre-existing LocalThresholdOp pattern that everyone
   was copying).

#### B. filter-playground (`icl/filter/apps/filter-playground.cpp`)

A single unified app exposing every migrated Op through
introspection-driven UI. Users pick a filter from a combo; the properties
panel is rebuilt via `Prop(&currentOp)` using the CamCfgWidget-style
BoxHandle-swap pattern. The playground subsumes 9 retired single-op
demos (canny-op, convolution-op, dither-op, fft, temporal-smoothing,
warp-op, bilateral-filter-op, gabor-op, filter-array).

Polish landed this session:

- **Source controls** — size / depth / format combos feeding `useDesired`
  on the grabber, plus a source-ROI mode combo with 7 presets
  (none/UL/UR/LL/LR/center/interactive).
- **Interactive ROI** — left-click-drag on the source canvas defines a
  rubber-banded (transparent blue) rect; release commits it (red
  outline); right-click resets.
- **Dynamic Prop panel** — full filter swap on combo change, serialized
  against the exec thread's `apply()` via `opMutex`.
- **Auto-range result display** — `ImageHandle::setRangeMode(rmAuto)` on
  the result so 16s/32f filter outputs render over their actual dynamic
  range (gradients, FFT magnitude, etc. stop looking mostly-black).
- **Timing + status** — apply-time label, fps counter, status label
  (shows "ok" or the exception text).

#### C. Framework bug fixes exposed by the playground

1. **`CannyOp::followEdge` stack overflow** — recursive 8-connected flood
   fill blew the thread stack when low threshold ≈ 0 made most pixels
   weak edges. Rewrote as iterative with a heap-backed work stack;
   identical semantics. Latent for years, trivially reached once the
   playground exposed the default `lowT=0`.
2. **`ConvolutionOp` black-output-on-depth-flip** — when reverting
   float→int, `m_kernel.toInt(true)` truncates normalized gauss/sobel
   kernels to all-zero. Fix: if the kernel has a known `fixedType`,
   rebuild from the lookup table instead of truncating. Custom float
   kernels still warn+truncate as before.
3. **`GaborOp`** — three bugs: (a) float-Gabor-kernel applied to int src
   degraded to toInt zeros + Img32f append type mismatch (now converts
   non-float src once via an internal `m_src32fBuffer`); (b) default ctor
   produced empty kernel bank → black first frame (now seeds from
   property defaults); (c) property-driven updateKernels raced the exec
   thread's vector iteration (initially fixed with a per-op mutex, then
   folded into the framework-level `UnaryOp::m_applyMutex`).
4. **`WienerOp` had no C++ fallback** — implemented one: classic
   adaptive Wiener (output = μ + max(0, σ²−noise)/max(σ², noise) ·
   (src − μ)) over 8u/16s/32f via an integral-image optimization
   (allocates once per apply, reused across channels, gives O(W·H)
   independent of mask size). Playground default seeded at 5×5 /
   noise=100 so the filter produces visible output at load time
   (`noise=0` is a mathematical no-op — textbook Wiener with "no noise
   expected" returns src unchanged). Mid-apply mask race (same pattern
   as GaborOp) fixed via the UnaryOp mutex. 3 new tests.
5. **GradientImage retired** — fully replaced by GradientOp
   (x/y/intensity/angle mode menu + normalize flag, ConvolutionOp-backed).

#### D. Phase 2 deferred items

- ImageRectification → UnaryOp (needs a 4-point quadrangle GUI picker;
  playground would grow a point-picker mode)
- `project_configurable_op_threadsafety.md` pattern potentially applies
  to `io::Grabber`/similar Configurables outside ICLFilter; revisit if
  those hit the same races

#### E. Memory writes this session

- `project_filter_playground.md` — the overall plan (already approved)
- `project_configurable_image_type.md` — resolved (first consumer GaborOp)
- `project_configurable_op_threadsafety.md` — resolved (UnaryOp mutex)
- `feedback_img_channel_access.md` — prefer `ImgChannel<T> c = img[0];
  c(x,y)` over `img(x,y,c)` in pixel loops (cached pointer vs. vector
  re-lookup)

---

## Previous State (Session 42 — rectify-image fix, AffineOp backends, Quick2 pool)

### Session 42 Summary

Finished the filter module audit (9/10 demos runtime-verified, warp-op deferred).
Major framework work driven by a broken `icl-rectify-image`:
- Hartley-normalized homography estimator (fixes ~128 px fitting errors)
- AffineOp Accelerate backend fixes (rotation direction, ROI respect, centering)
- Quick2 pool accounting workaround (trackedBytes per PooledBuffer)
- DataStore typed `install(MouseHandler*)` overload for MI-safe pointer offsets

Also: Common2.h now transitively exposes FPSLimiter. Two pre-existing
gotchas flagged (parse<T>(pa()) trap, vImage matrix convention).

#### A. rectify-image fix trail

Root cause was a chain of four independent issues, each found by attempting
to reproduce the reported "can't drag corners, bad output" failure mode:

1. **Mouse install broken** — `gui["draw"].install(mouse)` didn't compile
   (non-copyable AffineOp). Changed to `install(&mouse)`. Added a typed
   `DataStore::Data::install(MouseHandler*)` overload
   (icl/qt/DataStore.h:137-139) that performs the derived→base conversion at
   the call site, so the stored `void*` actually points to the MouseHandler
   subobject — matters for multiply-inherited handlers like
   DefineQuadrangleMouseHandler. Forward-declared `MouseHandler` in DataStore.h.

2. **Handles unclickable in small window** — DefineQuadrangleMouseHandler's
   click-tolerance was 8 *image pixels*. On a FullHD camera shown in a
   compact window the handles became sub-pixel-sized. Rewrote process() at
   icl/qt/DefineQuadrangleMouseHandler.cpp:105-166 to use *widget-pixel*
   distance via `e.getWidgetPos()` + widget-image scale, `max(projected,
   10px)` threshold. Rendering unchanged (still in image coords).

3. **Homography residuals of ~128 px** — unnormalized DLT is numerically
   unstable. Added Hartley normalization in
   icl/math/Homography2D.cpp:17-63 (translate centroid to origin, scale to
   mean-distance √2, fit there, un-normalize with `H = Ty^-1 · H̃ · Tx`).
   Both algorithms previously failed on rotated quads; now < 1 px residual.
   **Deleted the `Simple` algorithm** — nobody in the codebase passed it,
   it was structurally wrong for any perspective mapping. Also removed
   `advanedAlgorithm` param from ImageRectification::apply. 3 new tests
   in tests/test-math.cpp:1174-1221.

4. **Opaque error messages** — ImageRectification's "at least one edge…
   outside the source image rectangle" throw now names the offending
   corner, its mapped (x,y) value, the src rect, and the source quad
   corner for debugging (icl/filter/ImageRectification.cpp:106-115).

#### B. AffineOp backend rework (macOS/Accelerate)

Working on affine-op-demo revealed three distinct bugs in the Accelerate
LIN path. All in icl/filter/AffineOp_Accelerate.cpp.

1. **Rotation direction flipped vs NN** — vImage's matrix convention is
   empirically not what the docs imply. Guesswork (invert + row-vector,
   invert + column-vector) all broke centering. Working fix: decompose
   the forward matrix as an isotropic similarity (scale + angle + user
   translation), negate the angle, recompute the bbox-centering
   translation, feed through the original (known-centered) struct layout.
   Documented in the project_affineop_vimage.md memory.

2. **Lost user translation** — the decomposition was rebuilding the
   translation purely from ROI bbox, throwing away `op.translate(1, 0)`
   calls and breaking Filter.AffineOp.translate test. Fixed by separating
   user-translation from bbox-centering adjustment
   (AffineOp_Accelerate.cpp:71-75).

3. **ROI not respected** — vImage_Buffer described the full image, so
   clipToROI silently bled pixels outside the ROI. Fixed by pointing
   srcBuf at ROI origin (`s.getData(c) + roi.y*W + roi.x`), using ROI
   dims, and adjusting the translation by `A·(roi.x, roi.y) + t`
   (column-vector form). Initial sign error of `t − (roi.x, roi.y)` only
   worked for identity A — corrected to proper formula.

Separate cleanup: **AffineOp::apply** (icl/filter/AffineOp.cpp:91-95)
now consults `getClipToROI()` to choose the bbox region
(`src.getROI()` vs `src.getImageRect()`). `m_adaptResultImage` stays
orthogonal (used by TranslateOp + tests, unchanged).

#### C. Quick2 / QuickContext pool accounting

The pool's `currentUsage` (a `size_t`) was underflowing to ~2^44 during
icl-tests. Diagnosed with a new `setTracing(bool)` emitting per-event
alloc/resize/evict/unpooled traces to stderr, plus an `ERROR_LOG` +
clamp-to-0 guard that caught the underflow. Root cause: pool tracked
sizes via live `Image::memoryUsage()` queries at mutation time, which
desyncs if callers externally resize a handed-out buffer.

Workaround (not the real fix): **PooledBuffer{Image, size_t trackedBytes}**
pairs (icl/qt/QuickContext.cpp:62-64). Decrements use `trackedBytes`, not
live queries. Underflow vanishes.

Also added **`setThrowOnCapExceeded(bool)`** for strict-mode test assertions
— keeps stderr clean in icl-tests' `Quick2.Context.memoryCap` test.

**Real fix deferred**: replace the Image-holding pool with a generic
`utils::MemoryPool` of raw byte chunks + `shared_ptr` custom-deleter
release (captured in project_memorypool.md memory). Green-field refactor,
not worth interleaving with other work.

#### D. Filter demo audit (runtime verification)

All 10 filter demos from Session 41 had been *header-swap Done*. Session 42
ran each one end-to-end:

- ✅ affine-op (modernized), bilateral-filter-op, canny-op, convolution-op,
  dither-op, fft, gabor-op (modernized + parse<T>(pa()) fix),
  pseudo-color, temporal-smoothing
- ⏸ warp-op — needs camera-distortion pipeline to produce warp tables

Modernization touched affine-op.cpp (Img8u global → Image, static AffineOps
with explicit reset(), FPSLimiter, two orthogonal checkboxes + QuickDraw
ROI overlay) and gabor-op.cpp (std::vector<float>{x} over vec1 helper,
useDesired<T>(pa()) over parse<T>(pa())).

Minor cleanups: **canny-op** and **temporal-smoothing** had a useless
`update()` indirection (run() just called update()). Folded into run().

#### E. Common2.h + pre-existing gotchas

- **Common2.h** now `#include`s FPSLimiter.h transitively, matching the
  old Common.h convention. Stripped the redundant explicit include from
  25 demo/app files.
- **parse<T>(pa("…"))** throws at runtime — ProgArg's templated
  `operator T()` matches `operator std::string_view()` which recurses
  into `parse<std::string_view>` (not stream-extractable). Use
  `pa(...).as<T>()` or `useDesired<T>(pa(...))` instead. Present in
  ~20 files, worth a sweep later. Documented inline in gabor-op.cpp
  and in this guide.
- **vImage matrix convention** stays opaque; the Accelerate backend
  works for isotropic similarity transforms (ICL's overwhelmingly common
  case). Documented in project_affineop_vimage.md memory.

---

## Session 41 (Filter module audit + framework bug fixes)

### Session 41 Summary

Filter module audit complete. Three critical framework bugs found and fixed.
Color-segmentation app ported to geom2. All compiler warnings eliminated.

#### A. Filter demos/apps audit (all 10 demos + 4 apps checked)

- **Modernized** 8 files: affine-op, bilateral-filter-op, convolution-op,
  gabor-op, temporal-smoothing, filter-array, local-thresh, rectify-image
  (Image-based apply, smart pointers, removed raw ImgBase* buffers)
- **dither-op, fft, pseudo-color, canny-op** — already clean, no changes
- **warp-op** — fixed gui key bug (`gui["lin"]` → `gui["interpolation"]`)
- **bilateral-filter-op** — major cleanup: removed unused geom includes,
  eliminated template dispatch, simplified to Image pipeline
- **color-segmentation** — ported from geom::Scene to geom2::Scene2
  (GroupNode + CuboidNode children, MeshNode for axes, TextNode for labels,
  3 separate mouse handlers instead of widget-dispatch pattern)
- **local-thresh** — split into GUI app + headless `local-thresh-batch` CLI;
  fixed ROI clamping to image bounds, skip filter when ROI too small for mask
- **rectify-image** — fixed. Added typed `DataStore::Data::install(MouseHandler*)`
  overload that forwards to the `void*` version after implicit derived-to-base
  conversion — ensures the stored void* points to the MouseHandler subobject for
  multiply-inherited handlers (DefineQuadrangleMouseHandler etc.). Call site
  now passes `&mouse` (was `install(mouse)` which didn't compile).

#### B. clipped_cast UB fix (ClippedCast.h) — CRITICAL

`static_cast<icl8u>(-FLT_MAX)` is undefined behavior. On ARM/Apple Clang
with optimizations, this caused `clipped_cast<icl8u,icl32f>` to always
return `-FLT_MAX`, breaking ALL SSE-path color conversions framework-wide.
Rewritten with `if constexpr` dispatch: int→float just casts, float→int
clamps, int→int uses wide intermediary. Added 5 tests.

#### C. sse_for pointer underflow fix (SSEUtils.h)

`dstEnd - (step - 1)` wraps when image dim < step (16 pixels), causing
the SSE loop to process garbage. Fixed all 64 occurrences with safe guard:
`(dstEnd - dst0 >= step) ? dstEnd - (step - 1) : dst0`.

#### D. ColorSegmentationOp::lutEntry fix

- Loop start `a-rA` goes negative when `a=0`, step 256 skips all valid bins.
  Fixed with `std::max(0, ...)` clamping + precomputed end values.
- Missing `return` on the `fmt == m_segFormat` early path (fell through to
  double-conversion).
- `pow(2, shift)` → `1 << shift`.

#### E. Compiler warnings eliminated

- Added `override` to 30 methods in PointCloudObject.h, PCLPointCloudObject.h,
  PointCloudObjectBase.h
- Fixed `class`/`struct` ViewRay mismatch in Scene2.h, BVH.h, RayCastOctree.h
- Removed unused variables in SceneSynchronizer.cpp
- Debug build now produces zero warnings from ICL code

#### F. cc() color conversion tests

Added 3 tests: 1x1 image conversion, RGB→YUV→RGB roundtrip, large image
spot-check. Verified SSE path matches scalar path within ±1.

#### G. Build system

- Disabled ccache (was causing spurious rebuilds + stale .ninja_deps)
- Fixed corrupted .ninja_deps by deleting and rebuilding

### Session 40 Summary

Module-by-module audit of all demos/apps. Completed utils, core, math.
Major framework improvements along the way.

#### A. DataStore improvements

- `operator string_view()` — thread-local buffer for `parse<T>(gui["key"])`
- `operator T()` fallback — stream-extractable types try string→parse<T>
  when direct assignment fails (e.g. `cc(image, gui["fmt"])`)

#### B. PseudoColorConverter → PseudoColorOp

- Moved from core to filter as proper UnaryOp. `pseudo()` Quick2 function.
- Updated all 7 consumers (kinect demos, OptrisGrabber, etc.)

#### C. AbstractCanvas removed (unused outside its own demo)

#### D. Quick2 additions

- `create()` accepts `optional<depth>` for direct depth conversion
- `pseudo(image, stops, maxValue)` in QuickFilter
- `roi(Image&, Rect)` and `copyroi(Image&, Rect)` overloads
- AbstractPlotWidget default background → white

#### E. Bug fixes

- **DrawWidget::customPaintEvent** null PaintEngine crash on macOS Core Profile
- **QuadTree nn()**: rewrote with double-precision distance math; fixed AABB
  degeneration bug (integer halfSize/2 reaches 0 after ~9 levels); fixed
  queryAll() inner loop bound + missing root points
- **Renderer::invalidateCache()** — deferred GL cleanup to render thread
  (was calling glDelete* from run thread → crash)

#### F. Scene2 enhancements

- Inherits Configurable: background color, wireframe, enable lighting,
  point size, info properties. OSD button on Canvas3D.
- Thread safety: lock()/unlock() with recursive mutex, render() auto-locks
- **BVH::raycastToImage()** — CPU raycast to Img8u + Img32f depth buffer
- **RayCastOctree** (new, geom2) — Octree with rayCast/rayCastSort methods
- **DemoScene2::setupNatureScene()** — green ground, rocks, trees
- **raycast-octree demo** — BVH→pointcloud→octree, mouse probe highlights

#### G. Module audit results

- **utils**: done — configurable-info checked (no changes needed)
- **core**: done — colorspace modernized, pseudo-color moved to filter,
  canvas removed
- **math**: done — all 6 demos checked:
  - k-means: done (header swap)
  - llm-1D: cleaned up (constexpr, brace init)
  - llm-2D: fully ported from ImgQ to Image + planarToInterleaved
  - octree: removed → replaced by geom2/raycast-octree demo
  - quad-tree: cleaned up, QuadTree NN bug fixed
  - polynomial-regression: split into 3 demos (1D plot, 2D surface
    with Scene2, image approximation), all ported to Quick2/geom2

#### H. PCL integration

- Meson: manual include path extraction for Homebrew
  (meson strips Cellar paths as "system")
- PCL includes propagated through icl_geom_dep

### What's next (after Session 42)

**Module audit** — continue through remaining modules:
- [x] filter (demos: 9/10; apps: 4+1 batch) — warp-op deferred
- [ ] io (demos: 3; apps: 12)
- [ ] cv (demos: 12; apps: 6) — `lens-undistortion-calibration` also unlocks warp-op
- [ ] qt (demos: 8; apps: 7; examples: 2)
- [ ] geom (demos: 23; apps: 16)
- [ ] geom2 (demos: 6)
- [ ] markers (demos: 2; apps: 8)
- [ ] physics (demos: 8)

**Deferred framework cleanups** (all non-blocking, documented in memory):
- `utils::MemoryPool` refactor → QuickContext migrates to raw-byte chunks
  with shared_ptr custom-deleter release. Current `(Image, trackedBytes)`
  workaround fixes the observed underflow without restructuring anything.
  See `project_memorypool.md`.
- vImage matrix convention investigation — needs empirical pixel-diff
  testing against C++ fallback. Current backend's "decompose & negate"
  approach works for isotropic similarity transforms. See
  `project_affineop_vimage.md`.
- Sweep `parse<T>(pa("..."))` usages (~20 files) and convert to
  `pa(...).as<T>()` / `useDesired<T>(pa(...))`. They all throw at runtime
  but most haven't been exercised yet.
- CPP LIN backend in `AffineOp_Cpp.cpp` samples from outside ROI when
  bilinear neighborhood is within full-image bounds — falls back to
  ROI-respecting NN only at the edges. Pre-existing, inconsistent with
  NN behavior. Not the same path as the Accelerate backend so doesn't
  affect macOS users.

For each: compile, run, check if still useful/valid, modernize API usage,
remove if obsolete. See `porting-progress.md` for per-file status.

**Quick2 Phase 2 remaining** (ImgQ pixel access rewrites):
- signature-extraction demo — deleted (was the only demo left on ImgQ)
- 3 library files: Scene.cpp, FiducialDetectorPluginICL1.cpp, DrawWidget.h
- ~16 umbrella headers — trivial swap once library code is done
- Final: delete Quick.h/Quick.cpp, retire Common.h

**Scene2 open items**:
- Wire remaining Configurable properties to Renderer: "enable lighting"
  (needs Renderer toggle), "point size" (default override for all point clouds)
- Consider adding more properties: shadows toggle, SSR toggle, debug viz mode,
  exposure, ambient level

**Quick2 open items**:
- MorphologicalOp opening/closing crash via Quick2 `filter()` — pre-existing
- Pool byte-accounting desync — *workaround landed* Session 42:
  `(Image, trackedBytes)` per buffer + ERROR_LOG underflow clamp +
  `setTracing()` for diagnosis + `setThrowOnCapExceeded()` for tests.
  Real fix (raw-byte MemoryPool) still deferred — see `project_memorypool.md`.
- `ImgROI2`: store target ROI separately instead of modifying image (deferred)
- `.out("name")` on `Int`/`Float`/`String` GUI components only updates on
  Enter/returnPressed, not on every keystroke. The `.handle("name")` path
  reads the live widget value via `getValue()`. Consider removing `.out()`
  entirely or making it sync on every change.
- QuadTree: SF template parameter is dead weight with double math — remove

---

## Session 38 (Quick2 framework)

### Session 38 Summary (12 commits)

Complete implementation of Quick2 — the Image-based replacement for
Quick.h's float-only ImgQ API. Quick.h stays untouched for incremental
migration (Phase 2).

#### A. Quick2 Architecture (8 sub-files + multi-includer)

- **QuickContext**: memory-capped buffer pool (default 256 MB) with eviction,
  thread-local activation via `QuickScope` RAII, drawing state (color/fill/font)
  with push/pop stack, grabber cache, `applyOp(UnaryOp&/&&)` and
  `applyOp(BinaryOp&/&&)` for one-liner pool-backed op application
- **QuickCreate**: zeros, ones, load, create, grab — all return `Image` at
  native depth (no forced float conversion)
- **QuickFilter**: 18 functions (filter, blur, cc, rgb/hls/lab/gray, scale,
  channel, levels, thresh, copy, copyroi, norm, rotate, flipx, flipy) via
  `applyOp` + `poolCopy`/`poolConvert`
- **QuickMath**: arithmetic (+,-,*,/), math (exp,ln,sqr,sqrt,abs), logical
  (||,&&), bitwise (binOR/XOR/AND) — all via `BinaryArithmeticalOp`/
  `UnaryArithmeticalOp` + `applyOp`
- **QuickCompose**: concatenation (,/%/|) with depth promotion, `ImgROI2`
  with `shallowCopy()` for side-effect-free ROI operations
- **QuickDraw**: `DrawTarget<T,NC>` template with compile-time channel count,
  cached raw channel pointers, channel-outer loops for cache-friendly bulk ops.
  `withDrawTarget(image, lambda)` dispatches 5 depths × 3 NC = 15 variants
- **QuickIO**: save, show, print
- **Quick2.h**: multi-includer entry point

#### B. Framework additions

- `Image::memoryUsage()` — pool tracking
- `Image::shallowCopy()` — new ImgBase, shared pixels, independent metadata
- `Image::isExclusivelyOwned()` — `shared_ptr::use_count() == 1`, for pool
  safety. Documents two levels of sharing (ImgBase handle vs channel data)
- `UnaryOp::getDestinationParams()` / `BinaryOp::getDestinationParams()` —
  virtual, returns `pair<depth, ImgParams>`. `NeighborhoodOp` overrides to
  subtract mask margin. `prepare()` refactored to non-virtual, delegates to
  `getDestinationParams()`
- `LineSampler::forEach(a, b, callback)` — zero-allocation Bresenham with
  fully-inlined callback (engine moved to header `detail` namespace)

#### C. Critical bug fix — pool aliasing

`Image` copies share the same `ImgBase` via `shared_ptr`. The pool's
`isIndependent()` check tested channel-level sharing (SmartPtr use counts),
but both the pool entry and the returned Image share the same ImgBase object
— so channel SmartPtrs have use_count=1 even when the buffer is held externally.
Fix: `isExclusivelyOwned()` checks `shared_ptr::use_count() == 1`.

#### D. Test suite

528 tests in single `icl-tests` binary (384 existing + 144 Quick2).
7 test files: context, create, filter, math, compose, draw, io.
3 multithreaded stress tests (10 parallel workers each): arithmetic+drawing,
concurrent image drawing, pool isolation with 4MB cap.

#### E. Misc

- `CLDeviceContext` startup message changed from `std::cout` to `DEBUG_LOG`
- TODO: `BackendDispatching::addStateful` eagerly calls factory at static init
- Removed stale `Testing/` CTest artifact directory
- Plan document: `iclquick-plan.md`
- MorphologicalOp opening/closing crash via Quick2 `filter()` — pre-existing bug
- Consider `localThresh()` convenience function
- Pool memory accounting may drift over time (currentUsage counter) — add
  periodic reconciliation or compute from scratch

---

## Session 37 — Shadow mapping + soft shadows + renderer fixes

### Session 37 Summary (4 commits)

Ported shadow mapping from geom GLRenderer to geom2 Renderer, added
per-light soft shadows with Poisson disk PCF, fixed several rendering issues.

#### A. Shadow mapping pipeline

Shadow depth pass ported from geom `GLRenderer` (4× 2048² FBOs,
`sampler2DShadow` with `GL_COMPARE_REF_TO_TEXTURE` for hardware PCF).
Light VP matrix auto-computed from LightNode world position (lookAt toward
origin + 90° perspective). Per-light shadow slot mapping in PBR shader via
`uLightShadowSlot[8]` → `uShadowMatrix[4]` → `sampleShadow()`.

#### B. Per-light soft shadows

`LightNode::setSoftShadowRadius(float texels)` — 0 = hard (default),
\>0 = 16-sample Poisson disk PCF. Radius stored per shadow slot as
`uShadowSoftness[slot]` (converted from texels to UV space). Hard shadows
take the fast single-sample path with zero overhead.

DemoScene2: key light gets soft shadows (radius 3), top light hard shadows.
Overlay viewer has interactive shadow softness slider (0-20).

#### C. Rendering fixes

- **Scene2::render()**: letterbox viewport to preserve camera aspect ratio
  (was stretching when widget AR ≠ camera AR)
- **Billboard text Y-flip**: negate local Y column of billboard matrix
  (ICL projection uses Y-down image convention)
- **Billboard text unlit**: added `uUnlit` flag to PBR shader — when set,
  outputs baseColorMap directly without lighting (keeps alpha for transparency)
- **TextNodes excluded from shadow depth pass** (no box shadows from labels)
- **Light color auto-detection**: `collectLights()` handles both 0-1 and
  0-255 GeomColor ranges (LightNode defaults are 0-1, old demos use 0-255)
- **Renderer destructor**: properly cleans up GL resources (programs, FBOs, textures)

### Session 36 Summary (4 commits)

Complete SSR rewrite from broken world-space/texture-space approaches to
working view-space ray march. Added reflectivity uniform, improved demo
scene, fixed Cycles smooth shading bug.

#### A. SSR rewrite — view-space ray march

**Three failed approaches** before finding the working one:
1. World-space stepping + projected depth comparison — banding, wrong depths
2. Screen-space DDA + linear depth interpolation — depth is non-linear for
   rays toward camera (floor reflecting objects above), fundamentally broken
3. Texture-space linear march (ported from 3rdparty/SSR) — self-intersection
   from previous-frame depth mismatch, tight threshold issues

**Working approach**: view-space ray march with per-step projection:
- Transform fragment to previous frame's view space via `uPrevView`
- Step along reflection ray in view space (linear Z — no non-linear artifacts)
- At each step: project to screen via `uPrevProjection`, read depth buffer,
  reconstruct view-space Z via `uPrevInvProjection`, compare in linear space
- 8-iteration binary search refinement on sign change
- **4x supersampled**: traces 4 rays with stratified jitter offsets (0, 0.25,
  0.5, 0.75 of one step), confidence-weighted average eliminates Moiré ring
  artifacts on curved surfaces
- 256 steps, ray distance 4x camera depth, screen-edge + roughness fade
- SSR skipped for non-reflective surfaces (`reflectivity < 0.01 && !metallic`)

**New uniforms**: `uPrevView`, `uPrevProjection`, `uPrevInvProjection`
(uploaded alongside existing `uPrevVP`). Inverse projection computed per
frame via `Mat::inv()`.

**Depth buffer**: upgraded from `GL_DEPTH_COMPONENT24` to
`GL_DEPTH_COMPONENT32F` for better precision.

#### B. Reflectivity uniform

`uReflectivity` wired from `Material::reflectivity` to PBR shader. Controls
reflection strength: `reflFactor = max(envFresnel, vec3(uReflectivity))`.
At reflectivity=0, only Fresnel contributes; at 1.0, full mirror. Previously
the reflectivity field was ignored by the GL renderer.

#### C. Demo scene overhaul

**SSR test scene** (default when no `-scene` files given):
- RGB wireframe cube: 104 CuboidNode voxels forming 12 thick edges, corner
  colors from RGB cube (x,y,z → R,G,B), `smoothShading=false` for crisp edges
- Red sphere (90% reflective, roughness 0.15) — mirror-like SSR test
- Gold metallic sphere (metallic 0.9, roughness 0.35, reflectivity 0.3)
- Brown/black checkerboard ground (2x bigger, 50% reflective)
- Checkerboard back wall
- `-no-checkerboard` flag: flat black ground, no wall
- Camera near/far set tight to scene (20/6400, ratio 320:1) for depth precision

#### D. Cycles smooth shading fix

`SceneSynchronizer.cpp`: smooth shading now based on `Material::smoothShading`
only, not on presence of normals. Previously any geometry with normals
(including flat-faced cuboids) got smooth shading → rounded/glassy appearance.

### What's next

**SSR polish**:
- Step aliasing still visible at extreme close-up (could increase to 512 steps)
- Temporal accumulation: ping-pong feedback should naturally denoise over frames
- Consider Hi-Z acceleration for performance (hierarchical depth mip chain)

**Quick2 framework** (Phase 1 complete, Phase 2 next):
- Quick2.h fully implemented: QuickContext (memory-capped pool, thread-local
  activation, applyOp), QuickCreate, QuickFilter, QuickMath, QuickCompose,
  QuickDraw (DrawTarget<T,NC> optimized), QuickIO. 144 tests passing.
- Phase 2: migrate consumers from Quick.h → Quick2.h one file at a time
  (demos first, then library code). See `iclquick-plan.md` for full list.
- Key architectural additions: Image::memoryUsage(), Image::shallowCopy(),
  Image::isExclusivelyOwned(), UnaryOp/BinaryOp::getDestinationParams(),
  LineSampler::forEach(), QuickContext::applyOp(UnaryOp&&/BinaryOp&&).
- Pool bug found+fixed: isIndependent() vs isExclusivelyOwned() — two levels
  of Image sharing (ImgBase handle vs channel pixel data).
- MorphologicalOp opening/closing crash via Quick2 filter() — pre-existing bug.

**BackendDispatching TODO**:
- `addStateful` eagerly calls factory() at static init (registration time),
  triggering CLDeviceContext/CLProgram construction before main(). For OpenCL
  backends this causes heavyweight GPU driver calls during library loading.
  Fix: defer first factory() call to first resolve/apply. See TODO in
  `icl/utils/BackendDispatching.h:addStateful`.
- CLDeviceContext should be a per-thread singleton instead of created per-
  CLProgram. Currently ~30 CLDeviceContext instances created during test runs.

**Signature extraction demo** (`icl/cv/demos/signature-extraction.cpp`):
- Uses RotateOp, LocalThresholdOp, RegionDetector, blur, Quick.h functions
- Currently crashes — needs debugging (likely RegionDetector on filtered
  binary image, or ImgQ channel mismatch in the `scaled | blurred` concat)
- GUI: rotation, local threshold (mask size + global offset), min region
  size filter, alpha blur, scale-down factor, save PNG with transparency
- Uses `executeInGUIThread()` for save dialog (blocking, from worker thread)

**Shadow maps** (DONE — Session 37):
- Full pipeline: depth FBOs, shadow depth pass, PBR integration, soft PCF
- See Session 37 summary above for details
- TODO: shadow camera direction/FOV controls on LightNode,
  directional light ortho projection, PCSS (variable penumbra),
  debug visualization mode for shadow maps

**geom2 API cleanup**:
- Scene2 getters: return references or shared_ptrs instead of raw pointers
- `getGLCallback()`: return raw ptr instead of shared_ptr (avoid `.get()`)
- CoordinateFrameSceneObject: add PIMPL

**Other work**:
- CI update — meson in GitHub Actions
- ImageMagick 7 / FFmpeg 7+ rewrites
- ConvolutionOp IPP mixed-depth

---

## Previous State (Session 34 — Matrix migration + Cycles geom2 integration)

### Session 34 Summary (8 commits)

Two major areas: completed the matrix (row,col) convention migration across the
entire codebase, and wired Cycles renderer into geom2 with demos.

#### A. Matrix (row,col) migration — COMPLETE

**Full migration**: `operator()(col,row)` → `operator()(row,col)` across 67
files, ~1200 call sites. Three-phase approach:
1. Remove `operator()`, add `index_yx(row,col)` to both `FixedMatrix` and
   `DynMatrixBase`. Automated via `scripts/fix-matrix-indexing.py` (column-based)
   and new `scripts/fix-matrix-indexing2.py` (regex inside-out, handles nested
   calls, deref patterns, chained calls). Iterated build→fix→rebuild until clean.
2. Migrate `at(col,row)` → `at(row,col)` (5 files, ~60 sites).
3. Rename `index_yx` back to `operator()` — clean `M(row, col)` syntax restored.

**False positives caught**: `pow.index_yx`, `atan2.index_yx`, `std::swap.index_yx`,
`.mult.index_yx`, `setSamplingResolution.index_yx`, `Point32f.index_yx`,
`ICL_TEST_EQ.index_yx` — all fixed. Two semantic bugs caught in geom module
review (Camera.cpp principal point, Posit.cpp atan2 arg order).

**384/384 tests pass** after migration.

#### B. Cycles renderer wired into geom2

**Meson build wiring** (`icl/geom2/meson.build`): `CyclesRenderer.cpp` and
`SceneSynchronizer.cpp` now compiled when `cycles_found`. Reuses include paths,
compile flags, and link libraries from geom module's meson config.

**SceneSynchronizer fixes** for current Cycles 4.x API:
- `unique_ptr<ShaderGraph>` for `set_graph()`
- Removed `graph->add()` (nodes auto-added via `create_node`)
- `PointLight` instead of generic `Light`
- Camera FOV uses `getSamplingResolutionY()` + `compute_auto_viewplane()`
- Light color normalized from 0-255 to 0-1 before applying physical intensity
- Gradient sky background added (zenith/horizon/ground blend)
- Analytic sphere path disabled (known offset bug), always tessellates

---

## Previous State (Session 32 — Material refactoring + geom2 scene graph)

### Session 32 Summary (23 commits)

Two major areas of work:

## Previous State (Session 33 — PointCloud, BVH, textures, text, mouse interaction)

Session 33 added PointCloud, PointCloudNode, BVH raytracer, texture/text
rendering, mouse interaction to geom2. Started matrix indexing migration
(added `index_yx`, migrated math module). 5 commits.

### Session 32 Summary (23 commits)

#### A. Material & deprecated warning cleanup (geom module, 7 commits)

**Material restructured** with lazy sub-structs:
- `TextureMaps` behind `shared_ptr` (null for untextured objects, saves ~80 bytes)
- `TransmissionParams` behind `shared_ptr` (null for opaque objects)
- New fields: `lineColor`, `pointColor`, `pointSize`, `lineWidth`
- Non-copyable with explicit `deepCopy()` returning `shared_ptr<Material>`
- New factory: `fromColors(faceColor, wireColor)` for mixed face+wire colors

**SceneObject copy semantics fixed:**
- `operator=` deleted, `copy()` renamed to `deepCopy()`
- Protected copy ctor preserved for subclass deepCopy()
- Old `operator=` was buggy (missing material, reflectivity, emission)
- All PointCloud subclasses updated: `copy()` → `deepCopy()` with override

**GLRenderer line/point rendering:**
- New unlit shader (GLSL 410) for GL_LINES + GL_POINTS
- Separate VAOs in GLGeometryCache for lines and points
- Lines and points now visible in GLRenderer (were silently dropped)

**All 77 deprecated setColor/setShininess calls migrated** across 26 files.

**Bugs fixed:**
- Analytic sphere overlay offset: DemoScene auto-scale modified vertices
  but not `m_sphereCenter`/`m_sphereRadius`. Fixed by updating sphere params
  after transforms + adding `setSphereParams()` setter.

#### B. geom2 — Clean scene graph module from scratch (16 commits)

**Why:** SceneObject was a 940-line monolith (geometry + scene graph + materials +
transforms + rendering + physics base). Every incremental refactor tangled in
backward compatibility. geom2 is a greenfield design in `icl/geom2/` namespace
`icl::geom2`, independent of ICLGeom.

**Node hierarchy (clean separation of concerns):**
```
Node (abstract)              ← transform + visibility + name + locking. PIMPL'd.
├── GroupNode                ← has children. No geometry. Pure container.
├── GeometryNode (abstract)  ← read-only geometry + material (renderer interface)
│   ├── MeshNode             ← mutable geometry leaf (addVertex, getVertices()&)
│   ├── SphereNode           ← parametric sphere (no mutable vertices)
│   ├── CuboidNode           ← parametric box
│   ├── CylinderNode         ← parametric cylinder
│   └── ConeNode             ← parametric cone
├── LightNode                ← point/directional/spot, position from transform
└── CoordinateFrameNode      ← GroupNode with 3 CuboidNode axes (R=X, G=Y, B=Z)
```

**Key design rules:**
- GroupNode has children. Leaf nodes (MeshNode, SphereNode, LightNode) do not.
- GeometryNode provides read-only const access for renderers.
- MeshNode adds mutable access (for physics/dynamic geometry).
- Parametric shapes inherit GeometryNode (NOT MeshNode) — no mutable vertex access.
- All nodes PIMPL'd with `unique_ptr<Data>`, Rule of 5 (copy+move).
- No raw pointer ownership. `shared_ptr` everywhere.
- No display lists. No legacy GL. Core profile only.
- `MeshNode::ingest(MeshData{...})` for zero-copy bulk geometry loading.

**Components implemented:**
- `Node.h/.cpp` — abstract base (~120 lines)
- `GroupNode.h/.cpp` — children container (~80 lines)
- `GeometryNode.h/.cpp` — read-only geometry + material (~230 lines)
- `MeshNode.h/.cpp` — mutable geometry + `ingest()` (~120 lines)
- `Primitive.h` — Line/Triangle/Quad as plain structs (~50 lines)
- `SphereNode`, `CuboidNode`, `CylinderNode`, `ConeNode` — parametric shapes
- `CoordinateFrameNode` — ported from geom's CoordinateFrameSceneObject
- `LightNode` — point/directional/spot with color+intensity
- `Renderer.h/.cpp` — GL 4.1 Core, PBR shader + unlit shader, multi-light (~530 lines)
- `Scene2.h/.cpp` — scene manager with cameras, shared_ptr ownership, GL callback
- `Loader.h/.cpp` — .obj + .glb/.gltf file loading via `ingest()`
- `Raytracer.h`, `CyclesRenderer.h`, `SceneSynchronizer.h/.cpp` — Cycles scaffolding
  (headers ready, .cpp adapted but not compiled — needs Cycles build wiring)
- `PORTING.md` — geom → geom2 migration guide
- `demos/geom2-hello.cpp` — working demo with all shape types + lighting

**Total: 24 files, ~4800 lines, zero warnings.**

**Material is shared** between geom and geom2 (`icl::geom::Material` used by both).

### What's next for geom2

Remaining items from PORTING.md:
- **PointCloudNode** — replaces PointCloudObjectBase (important for CV visualization)
- **Mouse interaction / picking** — hit testing against the scene graph
- **Text primitives / billboard text** — needed for labels and debug overlays
- **Texture primitives** — textured quads
- **Cycles build wiring** — connect CyclesRenderer.cpp + SceneSynchronizer.cpp to Cycles
  libraries in geom2/meson.build (headers and adapted .cpp are ready)
- **ComplexCoordinateFrameNode** — cones + cylinders + text labels
- **More demos** — file loading demo, interactive mouse demo

### Build

```bash
meson setup build --buildtype=debug --wipe
CCACHE_DISABLE=1 meson compile -C build -j16
bin/geom2-hello-demo   # first geom2 visual demo
```

---

## Previous State (Session 31 — Meson build system, directory restructure)

### Session 31 Summary

**Build system migrated from CMake to Meson+Ninja:**

The entire project structure was overhauled in one session. 9 commits.

**Directory restructure:**
- `ICLGeom/src/ICLGeom/Scene.h` → `icl/geom/Scene.h` (matches `icl::geom` namespace)
- All 10 modules renamed: `ICLUtils→icl/utils`, `ICLMath→icl/math`, etc.
- `src/` wrapper dropped — headers and sources coexist directly
- Include paths: `#include <icl/geom/Scene.h>` (was `<ICLGeom/Scene.h>`)
- Library names: `libicl-geom.dylib` (was `libICLGeom.dylib`)

**Demos/apps flattened:**
- 120+ single-file subdirectories eliminated (each had its own CMakeLists.txt)
- Now: `icl/filter/demos/canny-op.cpp` (was `ICLFilter/demos/canny-op/canny-op.cpp`)
- Multi-file targets use naming convention: `camera-calibration-CalibrationGrid.cpp`

**Meson build system:**
- `meson.build` + `meson.options` replace ~170 CMakeLists.txt + 29 cmake modules
- 10 module `meson.build` + 10 target `meson.build` files
- All dependencies detected: Qt6, OpenCV, Eigen3, OpenCL, ImageMagick, FFmpeg,
  Bullet, Accelerate, Cycles (full integration with 21 compile defs, 20+ include
  dirs, 14 static + 18 shared libs)
- Config headers generated without `.in` templates (Meson `configure_file()`)
- Qt MOC via `qt6.compile_moc()` — trimmed to 11 headers with actual Q_OBJECT
- PCH headers in `icl/_pch/` (required: macOS case-insensitive FS shadows
  system `<time.h>` with our `Time.h` — fixed via `implicit_include_directories: false`)
- Post-build hook symlinks all executables into `build/bin/`

**Build result:** 122 targets (10 libs + 68 demos + 35 apps + 8 examples + 1 test)
all compile and link. Tests pass.

**Dead code removed:**
- `VideoGrabber.cpp/.h` — xine dependency no longer exists
- Old CMake system: 518 files deleted (45K lines)
- Old `ICL*` module directories removed
- `ICLExperimental/` removed (Cycles demos moved to `icl/geom/demos/`)

**Scenes and assets:** moved to `icl/geom/scenes/` (9 glb/obj files),
doc images moved into `icl/*/doc/`.

**Key Meson commands:**
```bash
meson setup build                              # configure
meson setup build -Ddemos=true -Dapps=true     # with demos+apps
meson compile -C build -j16                    # build
meson test -C build                            # run tests
meson configure build -Dtests=true             # reconfigure option
```

**Known disabled targets (pre-existing code issues, not build system):**
- 3 OpenCV legacy C API files (`OpenCVCamCalib`, `LensUndistortionCalibrator`,
  `OpenSurfLib`) — need rewrite for OpenCV 4+
- `TemplateTracker` — depends on IPP-only `ProximityOp`
- `corner-detection-css` demo — uses removed `DebugInformation` API
- `heart-rate-detector` demo — needs `ICL_OPENCV_INSTALL_PATH` define
- `octree` demo — needs PCL
- `point-cloud-primitive-filter` app — needs dead RSB dependency

### Next Steps

**Immediate:**
- **CI update** — `.github/workflows/ci.yaml` needs `pip install meson ninja`
  and `meson setup/compile/test` instead of cmake
- **CLAUDE.md update** — build instructions reference cmake, need meson equivalents
- **Docker scripts** — `scripts/docker/` needs meson adaptation

**Build system polish:**
- OpenCL kernel header generation (`scripts/cl2header.py` + `custom_target` in
  `icl/filter/meson.build`) — currently no `.cl` kernels are being compiled
- Fix disabled targets: rewrite OpenCV C API files for OpenCV 4+
- Add `subprojects/*.wrap` for Windows builds (zlib, libpng, libjpeg, gtest)

**Other work (unchanged from Session 30):**
- **ConvolutionOp IPP: mixed-depth support**
- **Benchmark IPP backends**
- **ImageMagick 7** — rewrite for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API

## Previous State (Session 30 — Bilinear fix, LAPACK helpers, IPP backend implementations)

### Session 30 Summary

**C++ bilinear scaling bug fixed (`Img_Cpp.cpp`):**
- Root cause: formula used `(src-1)/dst` instead of `(src-1)/(dst-1)` for the
  bilinear mapping. For identity scale (8→8), this gave `fSX = 7/8 = 0.875`
  instead of `1.0`, producing wrong pixel values (e.g., 8.75 instead of 10).
- Also fixed edge clamping: the old code clamped the base index `xll`, which
  gave the wrong pixel at the last row/column. Now clamps the +1 neighbor
  index (`x1 = min(x0+1, maxX)`) while keeping fractional weights correct.
- Test `Img.scaledCopy_identity` now passes on all platforms.

**LAPACK transpose helpers migrated to Accelerate and MKL backends:**
- Replaced 28 manual transpose loops in `LapackOps_Accelerate.cpp` and
  `LapackOps_Mkl.cpp` with `lapack_row_to_col` / `lapack_col_to_row` calls.
- Added vector-returning overload `auto AT = lapack_row_to_col(A, M, N, lda)`
  that allocates and returns the transposed buffer. Eliminates manual sizing.
- GELSD backward transpose kept manual (BT stride `mx` ≠ row count `N`).
- Net -78 lines across both backend files.

**5 IPP backends implemented (Docker-verified, 384/384 tests pass with IPP+MKL):**

| File | IPP Functions | Depths | Notes |
|------|--------------|--------|-------|
| UnaryArithmeticalOp_Ipp.cpp | ippiAddC/SubC/MulC/DivC + ippiSqr/Sqrt/Ln/Exp/Abs | 8u/16s/32f | withVal + noVal ops |
| LUTOp_Ipp.cpp | ippiReduceBits (modern signature with buffer param) | 8u | noise=0, ippDitherNone |
| MedianOp_Ipp.cpp | ippiFilterMedianBorder | 8u/16s | Fixed + generic sizes |
| ConvolutionOp_Ipp.cpp | ippiFilterBorder (spec-based) | 8u/32f | Odd kernels only; mixed-depth/even delegates to C++ |
| AffineOp_Ipp.cpp | ippiWarpAffineNearest/Linear (spec-based) | 8u/32f | NN + bilinear interpolation |

**MorphologicalOp_Ipp.cpp remains a stub:** IPP 2022.3 removed the general
morphology border API (ippiMorphologyBorderGetSize/Init, ippiDilateBorder,
ippiErodeBorder). Only `ippiDilate3x3_64f_C1R` / `ippiErode3x3_64f_C1R`
remain. The C++ backend handles all morphology; Accelerate provides
vImageDilate/Erode for 8u/32f on macOS.

**ConvolutionOp IPP limitations:** IPP's `ippiFilterBorder` uses center-anchored
convolution, which doesn't match ICL's configurable anchor for even-sized kernels.
Also, mixed-depth cases (8u→16s for Sobel) require `ippiFilterBorder_8u16s_C1R`
which isn't implemented yet. The IPP backend checks for odd kernel + same depth
and delegates all other cases to the C++ backend via explicit `get(Backend::Cpp)`.

**Docker Dockerfile updated:** pinned `intel-oneapi-mkl-devel-2022.2.1` to match
IPP 2022.3 era (avoids potential symbol conflicts between MKL 2025 and IPP 2022).
Confirmed: IPP+MKL work correctly together — earlier "MKL crash" was stale build
artifacts from bisection testing, not a real incompatibility.

**Build: 384/384 tests pass on macOS and Docker Linux (IPP+MKL).**

### Next Steps

**Immediate:**
- **ConvolutionOp IPP: mixed-depth support** — implement `ippiFilterBorder_8u16s_C1R`
  for 8u→16s (Sobel, Laplace) and even-kernel anchor alignment
- **Benchmark IPP backends** — measure speedup vs C++ for the 5 new IPP ops

**Experimental — Raytracing:**
- Real-time raytracer in `ICLExperimental/Raytracing/` — see
  [continue-raytracing.md](ICLExperimental/Raytracing/continue-raytracing.md)
  for full details. CPU backend working (BVH + OpenMP + reflections). Next: Metal RT
  backend for hardware-accelerated raytracing on Apple Silicon.

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers

## Previous State (Session 29 — IPP/MKL Docker verification, LAPACK bug fixes)

### Session 29 Summary

**Docker IPP+MKL build infrastructure:**
- Added `intel-oneapi-ipp-devel` to Dockerfile alongside MKL
- Enabled `-DBUILD_WITH_IPP=ON -DBUILD_WITH_MKL=ON` in docker-test.sh
- Fixed CMake IPP detection: removed dead `iomp5` (Intel OpenMP) and `ippm`
  (deprecated matrix lib) from required libs. IPP doesn't need Intel OpenMP;
  system OpenMP works fine.
- MKL: switched from `mkl_intel_thread`+`iomp5` to `mkl_sequential` (avoids
  Intel OpenMP dependency; ICL's own OpenMP handles parallelism)
- Added oneAPI search paths for MKL
- Fixed `--cpus` in docker-test.sh to use Docker's available CPU count
- Fixed PugiXML.cpp: `#include "pugixml.hpp"` → `"PugiXML.h"` (pugixml.hpp
  was never in git; PugiXML.h IS the header, renamed to ICL convention)

**All 6 real _Ipp.cpp files compile against oneAPI IPP — zero API changes needed.**
APIs verified available in modern oneAPI: ippiMirror, ippiSet, ippiLUTPalette,
ippiMax/Min/Mean, ippiMulC/AddC, ippiCopyReplicateBorder, ippiCopy P↔C,
ippiThreshold, ippiCompareC, ippiAndC/OrC/XorC/Not, ippiRemap, ippiFilterWiener.

**Bug found: cpp_getri (C++ LU inverse) produced completely wrong results.**
- The in-place U-inversion algorithm used `A[i][i]` which had already been
  overwritten to `1/U[i][i]` in a prior iteration. Errors were 20-500x, not
  precision issues.
- Hidden because on macOS the Accelerate backend (higher priority) was always
  selected — the C++ fallback was never exercised.
- Discovered by running tests in Docker where only the C++ backend is available.
- Fix: rewrote to solve `A*x=e_j` column-by-column using the LU factorization
  directly (forward-substitute with L, then back-substitute with U).

**Bug found: MKL geqrf/orgqr/getrf/getri row-major handling was wrong.**
- MKL backend used a dimension-swap trick (passing swapped M,N to LAPACK) to
  handle row-major → column-major. This works for symmetric operations (SVD,
  eigenvalue) but NOT for QR: QR(A^T) ≠ QR(A).
- Caused Docker test `math.dyn.qr_reconstruct` to fail with errors of 17+
  and subsequent heap corruption from wrong Q/R propagating.
- Fix: all four functions now use explicit transposition (matching the
  Accelerate backend pattern). `gelsd` already had this correct.

**Transpose helpers added to LapackOps:**
- `lapack_row_to_col(A, M, N, lda, AT)` and `lapack_col_to_row(AT, M, N, A, lda)`
  centralize the row-major ↔ column-major pattern used by all LAPACK backends.
- Defined in LapackOps.cpp, declared in LapackOps.h, instantiated for float/double.
- Backend callers to be migrated to use helpers in a future pass.

**C++ QR fallback (cpp_geqrf, cpp_orgqr) verified correct.** Standalone testing
with the exact Wikipedia 3×3 matrix from the test suite, plus 4×3, 5×5, and
double precision — all produce correct Q*R=A reconstruction within float epsilon.
Also verified cpp_gelsd (SVD least-squares solve) is correct.

**Pre-existing bug: C++ bilinear scaling produces wrong results for identity scale.**
- Test `Img.scaledCopy_identity` fails in Docker (383/384 pass without MKL).
- Scales 8×6 → 8×6 with `interpolateLIN`, gets `8.75` for pixel value `10`.
- The value `8.75 = 0.875 * 10` suggests `fSX = 7/8 = 0.875` (wrong) instead
  of `fSX = 7/7 = 1.0` (correct for identity scale with the `(src-1)/(dst-1)` formula).
- Investigation in `Img_Cpp.cpp` shows the bilinear code at line 394 computes
  `fSX = (srcSize.width - 1) / (dstSize.width - 1) = 7/7 = 1.0` — which is correct.
- The `scaledCopy` call path passes `getSize() → srcSize` and `poDst->getSize() →
  dstSize`, both `(8,6)` — confirmed correct.
- Cannot reproduce on macOS because Accelerate backend (vImageScale, Lanczos)
  takes priority. Need Docker debug session with print statements in
  `cpp_scaledCopyChannel` to see actual argument values at runtime.
- Hypotheses: (1) some other code path is being called that computes fSX
  differently, (2) the ImgOps dispatch wraps sizes differently, (3) ROI
  handling mutates the sizes before the backend sees them.
- TODO: Add `fprintf(stderr, ...)` debug prints to `cpp_scaledCopyChannel` in
  a Docker shell session to trace actual argument values.

**Docker test results (IPP only, no MKL): 383/384 pass.**
Remaining failure is the bilinear scaling bug above. With MKL: QR and
inverse tests now pass after the fixes, but MKL+Rosetta causes heap
corruption in some LAPACK paths (genuinely Rosetta — the MKL code is
now correct per our analysis). Need real x86 Linux for full MKL verification.

**Build: 100% clean (zero warnings), tests: 384/384 pass on macOS.**

### Next Steps

**Immediate:**
- **Fix C++ bilinear scaling bug** — Docker debug session needed
- **Migrate LAPACK backends to transpose helpers** — replace manual loops
  in Accelerate, MKL, Eigen backends with `lapack_row_to_col`/`lapack_col_to_row`

**IPP stubs (6 files, incremental performance):**
- AffineOp_Ipp.cpp — rewrite with modern `ippiWarpAffineNearest`/`Linear` + spec init
- ConvolutionOp_Ipp.cpp — rewrite with `ippiFilterBorder_*` (spec-based API)
- MorphologicalOp_Ipp.cpp — rewrite with `ippiMorphInit_*` + spec-based API
- MedianOp_Ipp.cpp — rewrite with `ippiFilterMedianBorder_*`
- LUTOp_Ipp.cpp — update `ippiReduceBits` signature (added noise param)
- UnaryArithmeticalOp_Ipp.cpp — write registrations for `ippiAddC/MulC/SubC/DivC`

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers or MKL verification

## Previous State (Session 28 — C++17 Phases 2-6: complete modernization)

### Session 28 Summary

**C++17 Phases 2-6 — mechanical and structural modernization across 183 files:**

Completed the entire C++17 modernization plan (phases 2-6) in a single session.
All changes are zero-semantic-impact except where noted (string_view API, transparent
comparators). Build clean (zero warnings), tests: 384/384 pass throughout.

**Phase 2 — scoped_lock, structured bindings, if-init, [[maybe_unused]] (118 files):**

- `std::scoped_lock` (75 files, 281 replacements): all `std::lock_guard` replaced.
  GradientImage.cpp and DC.cpp manual lock()/unlock() converted to RAII scoped_lock.
- Structured bindings (10 files, ~20 conversions): map iteration loops converted to
  `auto [key, value]` syntax. Pair unpacking in GenericGrabber (`split_at_first`).
  Files: ConfigFile, UnaryOp, GUI, Widget, PointCloudObjectBase,
  PhysicsPaper3ContextMenu, ManipulatablePaper, CCFunctions, GenericGrabber,
  ProcessMonitor.
- If-with-initializer (25 files, ~40 conversions): `find()`+`if` patterns converted to
  `if(auto it = find(); it != end())`. Files: Configurable, ConfigFile, Size,
  UnaryOp, Grabber, FileGrabber, GenericGrabber, Scene, MarkerGridDetector,
  PointCloudObjectBase, Benchmark, SignalHandler, and more.
- `[[maybe_unused]]` (33 files, ~52 conversions): `(void)var` casts replaced with
  `[[maybe_unused]]` on declarations/parameters. Includes 14 ICLFilter Op files
  (static init pattern), function parameters, catch variables, conditional vars.

**Phase 3 — std::filesystem (2 files):**

- `File.cpp`: `break_apart()` rewritten with `fs::path` methods (parent_path, stem,
  extension). Fixes pre-existing bug where `.gz` double-extension handling truncated
  the last character of basename (`substr(0,p-1)` → correct via `fs::path::stem`).
  `file_exists()` → `fs::exists()`, `file_is_dir()` → `fs::is_directory()`,
  `erase()` → `fs::remove()`. Removed `<sys/stat.h>` and `DIR_SEPERATOR`.
- `FileList.cpp`: eliminated all three platform-specific glob implementations
  (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replaced with
  `fs::directory_iterator` + `std::regex_match`. Added tilde expansion via `$HOME`,
  deterministic sort, no-wildcard fast path. Removed `<wordexp.h>`, `<glob.h>`,
  `<windows.h>`, `<cstring>`.

**Phase 4 — std::from_chars / std::to_chars (7 files):**

- Integer parsing: `atoi()` → `std::from_chars` in IntHandle, GUIDefinition,
  FileGrabberPluginCSV, TestImages, JPEGDecoder.
- Float parsing: `atof()` / `istringstream` → `std::strtof` / `std::strtod` in
  FloatHandle, GUIDefinition, FileGrabberPluginCSV, StringUtils (parse_icl32f/64f).
  Apple Clang libc++ lacks `from_chars` for floats; `strtof`/`strtod` are
  locale-independent and avoid istringstream allocation overhead.
- Integer formatting: `snprintf` → `std::to_chars` in StringUtils (i2str, time2str).
  Buffer sizes use `std::numeric_limits<T>::digits10 + 3` for portability.

**Phase 5 — if constexpr cleanup (3 files):**

- `SimdCompat.h`: added non-Apple scalar fallbacks for `add`/`sub`/`smul` in
  `simd_compat` namespace (compiler auto-vectorizes at -O3).
- `FixedMatrix.h`: removed all 9 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in element-wise
  operators (*, /, +, -, negate). Each replaced with a direct `simd_compat::` call
  that works on all platforms — Apple SIMD for 4x4/2x2, scalar fallback otherwise.
- `BackendDispatching.h`: 6 `enable_if` SFINAE constraints replaced with
  `static_assert` (cleaner error messages, `_v` trait aliases).

**Phase 6 — std::string_view + transparent map comparators (52 files):**

- `StringUtils.h/cpp`: `tok`, `toLower`, `toUpper`, `parse<T>`, `startsWith`,
  `endsWith`, `match`, `skipWhitespaces`, `analyseHashes`, `to8u/16s/32s/32f/64f`
  all take `std::string_view` instead of `const std::string&`. Deleted dangerous
  `parse<const char*>` specialization. Functions needing null-terminated strings
  (`strtof`, `regcomp`) construct `std::string` internally.
- `StrTok.h/cpp`: constructor takes `string_view`, internal tokenization updated.
- Transparent map comparators (`std::less<>`) added to all `std::map<std::string,...>`
  declarations across 48 files, enabling heterogeneous lookup with `string_view` keys
  without allocating temporary `std::string`. Modules: ICLUtils (Configurable,
  ConfigFile, ParamList, PluginRegister, MultiTypeMap, Benchmark, IppInterface),
  ICLCore (Color), ICLFilter (UnaryOp), ICLIO (Grabber, FileWriter, GenericGrabber,
  DCDeviceFeatures, SharedMemorySegment, OpenNIUtils, V4L2Grabber), ICLGeom
  (PointCloudObjectBase, PointCloudSerializer, Primitive3DFilter, PCDFileGrabber,
  Scene, DepthCameraPointCloudGrabber), ICLQt (Widget, GUI, Quick, MultiDrawHandle,
  DefineRectanglesMouseHandler, IconFactory), ICLPhysics (ManipulatablePaper,
  PhysicsPaper3ContextMenu).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Modernization — Final Status

| Phase | What | Files | Status |
|---|---|---|---|
| 1 | Nested namespaces, std::clamp, [[nodiscard]] | 821 | Done (session 27) |
| 2 | scoped_lock, structured bindings, if-init, [[maybe_unused]] | 118 | Done (session 28) |
| 3 | std::filesystem (File.cpp, FileList.cpp) | 2 | Done (session 28) |
| 4 | std::from_chars / std::to_chars | 7 | Done (session 28) |
| 5 | if constexpr (SimdCompat, BackendDispatching) | 3 | Done (session 28) |
| 6 | std::string_view + transparent map comparators | 52 | Done (session 28) |
| 7 | std::optional | ~10 | Deferred — do opportunistically |

Phase 7 (std::optional) is low-priority. The guide recommends doing it when touching
the relevant APIs for other reasons (MarkerGridDetector::getPos, ImgBase::getMax/Min,
FilenameGenerator error returns).

### Next Steps

**Remaining non-C++17 work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers. Run on CI or cloud x86 VM.

## Previous State (Session 27 — C++17 Phase 1: namespaces, std::clamp, [[nodiscard]])

### Session 27 Summary

**C++17 Phase 1 — mechanical modernization across 821+ files:**

Three categories of changes, all purely mechanical with zero semantic impact:

**1. Nested namespace fusion (821 files):**
- Fused `namespace icl { namespace X {` → `namespace icl::X {` across the entire
  codebase using `scripts/fuse-namespaces.py`. Script handles the standard case
  (outer `namespace icl {` on one line, inner `namespace X {` on the next).
- 11 files with non-standard patterns fixed manually:
  - Sibling namespaces (`namespace icl { namespace qt {...} namespace geom {...} }`)
    in Scene.h, ShaderUtil.h, SceneLight.h, PhysicsWorld.h, PhysicsUtils.h, Quick.h,
    ConvexHull.cpp, DCDeviceFeatures.cpp — split into separate `namespace icl::X` blocks.
  - Multi-block files (SignalHandler.cpp — two `#ifdef` blocks with separate namespaces).
  - Forward-declaration blocks (Grabber.h, JPEGHandle.h).
- Closing braces updated: `} // namespace X` → `} // namespace icl::X`, outer `}` removed.
- Content dedented by 2 spaces (`scripts/dedent-namespaces.py`) to restore standard
  2-space indent for namespace content (664 files, purely whitespace).

**2. std::clamp (3 files):**
- `ICLUtils/ClippedCast.h` — `clip()` body replaced with `std::clamp(tX, tMin, tMax)`.
  All ~60 call sites across the codebase get std::clamp behavior automatically.
- `ICLFilter/LocalThresholdOp.cpp` — `myclip()` replaced with `std::clamp` + cast.
- `ICLPhysics/demos/physics-maze/physics-maze.cpp` — `std::max(std::min(...))` →
  `std::clamp(val*5, -1., 1.)`.

**3. [[nodiscard]] (3 files):**
- `ICLCore/Image.h` — deepCopy, convert, scaledCopy, selectChannel, selectChannels,
  mirrored (all return new Image values that should never be silently discarded).
- `ICLFilter/UnaryOp.h` — single-arg apply() and operator()().
- `ICLUtils/BackendDispatching.h` — registeredBackends, bestBackendFor,
  applicableBackendsFor (pure query methods).
- `tests/test-filter.cpp` — 19 call sites suppressed with `(void)` where apply() is
  called for side effects (feeding frames into MotionSensitiveTemporalSmoothing).

**New utility scripts:**
- `scripts/fuse-namespaces.py` — fuses two-level `namespace icl { namespace X {` into
  `namespace icl::X {`. Uses last-zero-indent-brace strategy for closing brace removal.
  Skips one-line forward declarations. Does NOT handle sibling-namespace files (those
  need manual fixup).
- `scripts/dedent-namespaces.py` — removes 2 leading spaces from all lines inside
  fused namespace blocks. Skips blank lines and preprocessor directives.
- `scripts/fix-orphan-braces.py` — earlier iteration, superseded by the fuse script
  improvements (can be deleted).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phases 2-6 — COMPLETED (session 28)

See session 28 summary above for details.

#### Phase 7 — std::optional (deferred)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 26 — FixedMatrix compile-time SIMD acceleration)

### Session 26 Summary

**Compile-time SIMD acceleration for FixedMatrix:**

FixedMatrix (`ICLMath/FixedMatrix.h`) handles fixed-size matrices (2x2, 3x3, 4x4)
used in hot paths (camera projection, scene graph transforms, rotation composition).
Previously only 4x4 float multiply had SIMD (SSE2/sse2neon). All other sizes and
double precision fell back to generic `std::inner_product` with strided column
iterators — no vectorization. The runtime BlasOps dispatch mechanism was too costly
for these tiny matrices.

**Solution: compile-time `#ifdef` selection with zero dispatch overhead.**

**Apple SIMD (`<simd/simd.h>`) on macOS:**
- All functions are inline `SIMD_CFUNC` — zero call overhead, native NEON
- Row-major/column-major compatibility solved with zero overhead:
  - 4x4/2x2: `memcpy` reinterpret (same byte size), swap multiply args
  - Matrix-vector: `simd_mul(v, A_cm)` = `A_rm * v`
  - det/inv: transpose-invariant, reinterpret works directly
- Covers: mult, inv for 4x4/2x2 float/double; det for 4x4/2x2; matvec for 4x4
- Element-wise ops (add, sub, scalar mul, negate, div) for 4x4/2x2
- inv checks determinant and throws SingularMatrixException (matching C++ semantics)
- Replaces existing SSE2/sse2neon specializations on macOS

**SSE2/sse2neon (non-macOS fallback):**
- Existing specializations for 4x4 float multiply + matvec
- On Linux x86, clang -O3 auto-vectorizes remaining C++ loops to SSE/AVX

**Benchmark results (Apple M-series, -O3, batch=128 independent ops):**

| Operation | float SIMD | float C++ | Speedup | double Speedup |
|---|---|---|---|---|
| 4x4 multiply | 1.8 ns | 8.6 ns | **4.8x** | **3.2x** |
| 4x4 * vec4 | 0.8 ns | 1.4 ns | **1.6x** | 1.0x |
| 4x4 inverse | 5.4 ns | 20.1 ns | **3.7x** | **2.1x** |
| 4x4 det | 2.0 ns | 2.2 ns | ~1x | ~1x |
| Full pipeline | 2.6 ns | 6.5 ns | **2.5x** | **1.7x** |

**Key findings from benchmarking:**
- 3x3 Apple SIMD is **10x slower** than C++ due to `simd_float3` padding overhead
  (48 vs 36 bytes for 3x3). Excluded — uses C++ closed-form instead.
- cblas/MKL is **25x slower** than C++ for 4x4 (~100ns call overhead for a 4ns op).
  Removed from FixedMatrix — only useful for DynMatrix via BlasOps.
- Element-wise ops (add, smul, det) show ~1x because clang -O3 auto-vectorizes
  the C++ loops to equivalent NEON/SSE code.
- `memcpy` for load/store compiles to identical assembly as `reinterpret_cast`
  at -O2. Kept for correctness (alignment not guaranteed by FixedArray).

**New files:**
- `ICLMath/src/ICLMath/SimdCompat.h` — Apple SIMD load/store helpers + element-wise
  ops (add, sub, smul) for 4x4/2x2 float/double
- `benchmarks/bench-fixedmatrix.cpp` — standalone benchmark (no ICL deps), supports
  Apple SIMD, SSE2 intrinsics, MKL cblas, and plain C++ backends
- `scripts/docker/` — Docker infrastructure for Linux testing:
  `docker-build.sh`, `docker-shell.sh`, `docker-test.sh`, `docker-bench-fixedmatrix.sh`
  Uses rsync + named volumes for fast incremental builds.

**Modified files:**
- `ICLMath/src/ICLMath/FixedMatrix.h`:
  - `#include <ICLMath/SimdCompat.h>`, `<type_traits>`, `<initializer_list>`
  - Apple SIMD specializations: mult (4), matvec (2), inv (4), det (4),
    element-wise ops in operator+/-/*/negate bodies
  - SSE2 block restructured: `#elif defined(ICL_HAVE_SSE2)` (non-macOS fallback)
  - New `std::initializer_list<T>` constructor
  - 3x3 inv/det always use C++ closed-form (even on macOS)
- `tests/test-math.cpp`: 12 new cross-validation tests

**ICP.cpp inner loop heap allocations eliminated:**
- Transform loop did `new DynMatrix`+`delete` per point per iteration — replaced
  with a single stack-allocated temp buffer reused across all points.
- `error()` function simplified to avoid DynMatrix temporaries (direct element access).

**Codebase scan for BlasOps/SIMD wiring — completed:**
Systematic scan of all modules found that remaining hand-written loops (VectorTracker
`eucl_dist`, MathFunctions `euclidian`, KMeans `dist`) are NOT worth wiring through
BlasOps: they operate on small runtime-sized vectors (2-10 elements) where BlasOps
dispatch overhead (~2ns) exceeds the computation itself. The compiler auto-vectorizes
these short loops at -O3. BlasOps is already wired where it matters (DynMatrix),
and SimdCompat/Apple SIMD is already wired where it matters (FixedMatrix 4x4/2x2).

**Image scaling dispatched via ImgOps with Accelerate backend:**
- `scaledCopyChannelROI` (the single worker for all image scaling) added to ImgOps
  as `Op::scaledCopy` with C++ and Accelerate backends.
- C++ implementation moved from Img.cpp to Img_Cpp.cpp (NN, bilinear, region-average).
  Also replaced 8x `new[]`/`delete[]` in RA mode with `std::vector`.
- New `Img_Accelerate.cpp`: vImageScale for icl8u (Planar8), icl16s (Planar16S),
  icl32f (PlanarF). Uses Lanczos resampling (superior to bilinear/RA).
  NN mode uses inline integer indexing. icl32s/icl64f fall back to C++.
- CMakeLists.txt: `_Accelerate.cpp` exclusion added for ICLCore.
- Benchmark results (640x480 single channel, -O3):

  | Operation | Accelerate | C++ | Speedup |
  |---|---|---|---|
  | NN downscale 8u | 46 us | 134 us | **2.9x** |
  | LIN downscale 8u | 47 us | 146 us | **3.1x** |
  | RA downscale 8u | 49 us | 180 us | **3.7x** |
  | LIN downscale 32f | 140 us | 167 us | **1.2x** |
  | LIN upscale 8u | 52 us | 583 us | **11.2x** |

**Tests: 384/384 pass (5 new scaling tests).** Build clean on macOS.

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phase 2 — Structured bindings, if-init, scoped_lock (~40 files, low risk)

**Structured bindings** (~40+ locations with `.first`/`.second`):
- `ICLIO/GenericGrabber.cpp:247-280` — `split_at_first()` result unpacking
- `ICLMarkers/FiducialDetectorPluginICL1.cpp:89-90` — pair unpacking
- `ICLPhysics/PhysicsPaper3.cpp:105-116,1656,1720` — map lookups, coords
- All map iterations using `it->first`/`it->second`

**If-with-initializer** (~25+ map-find-then-check patterns):
- `ICLMarkers/MarkerGridDetector.cpp:42-44` — `auto it = find(); if(it != end())`
- `ICLCore/Color.cpp:43-45` — color map lookup
- `ICLIO/FileGrabber.cpp:154-156` — plugin map lookup
- `ICLIO/DCDeviceFeatures.cpp:302-304` — feature map lookup
- `ICLGeom/PointCloudObjectBase.cpp:340-343` — metadata lookup
- `ICLPhysics/PhysicsPaper3ContextMenu.cpp:35,45` — action map
- `ICLQt/MultiDrawHandle.cpp:113-115` — name→index map
- `ICLUtils/SignalHandler.cpp:151,165,180` — handler maps

**std::scoped_lock** (replaces lock_guard, fixes manual lock/unlock):
- `ICLGeom/PointCloudCreator.cpp:125,294` — `lock_guard<recursive_mutex>` → `scoped_lock`
- `ICLQt/AbstractPlotWidget.cpp:110` — same
- `ICLFilter/GradientImage.cpp:29,42` — **manual lock()/unlock()** → RAII (bug risk)

**[[maybe_unused]]** (replaces `(void)variable` casts):
- `ICLGeom/Scene.cpp`, `ICLQt/GLImg.cpp`, `ICLGeom/PointCloudObjectBase.cpp`,
  `ICLGeom/CoplanarPointPoseEstimator.cpp`, `ICLIO/SharedMemoryGrabber.cpp`, etc.

#### Phase 3 — std::filesystem (~15 files, medium risk)

Biggest single win. Eliminates platform `#ifdef` blocks in FileList.cpp entirely.

**Core targets:**
- `ICLUtils/File.cpp:34-62` — `break_apart()` does manual `rfind('/')`,
  `rfind('.')`, `substr()` for dir/basename/suffix/filename. Replace with
  `path::parent_path()`, `stem()`, `extension()`. Special `.gz` double-extension
  handling needs care.
- `ICLUtils/File.cpp:110-119` — `file_exists()` and `file_is_dir()` use
  `struct stat`. Direct replacement with `std::filesystem::exists()`/`is_directory()`.
- `ICLUtils/File.cpp:590` — `File::erase()` uses C `remove()` →
  `std::filesystem::remove()`.
- `ICLIO/FileList.cpp:40-126` — Three platform-specific directory listing
  impls (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replace with
  `std::filesystem::directory_iterator`. Note: glob pattern matching still needed
  on top of directory_iterator (std::filesystem doesn't do glob natively).
- `ICLIO/V4L2Grabber.cpp:91-95,147-150` — string concatenation with `"/"`
  for device paths → `operator/`; `stat()` for device existence → `fs::exists()`.
- `ICLQt/Widget.cpp:703-705` — `QDir("/").mkpath()` →
  `std::filesystem::create_directories()`.
- `ICLIO/V4L2Grabber.cpp:434-435` — hardcoded `/tmp/` →
  `std::filesystem::temp_directory_path()`.

#### Phase 4 — std::from_chars / std::to_chars (~10 files, low risk)

Faster, locale-independent number parsing/formatting:
- `ICLQt/IntHandle.cpp:17` — `atoi()`
- `ICLQt/FloatHandle.cpp:17` — `atof()`
- `ICLQt/GUIDefinition.cpp` — multiple `atoi`/`atof`
- `ICLIO/FileGrabberPluginCSV.cpp:26,110,118` — CSV float parsing with `atof`
- `ICLIO/TestImages.cpp:130,424-427` — XPM parsing with `atoi`
- `ICLIO/JPEGDecoder.cpp:140,142` — timestamp/ROI parsing
- `ICLUtils/StringUtils.cpp:22-42` — `snprintf` for number→string

#### Phase 5 — if constexpr cleanup (~5 files, medium risk)

- `ICLUtils/BackendDispatching.h:268-327` — 6 `enable_if` SFINAE overloads for
  enum/integral key types → single template with `if constexpr(is_enum_v<K>)`
- `ICLMath/FixedMatrix.h:300-336` — 4 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in
  operator bodies → `if constexpr` with a constexpr flag
- Depth switch statements (~240 instances) — many could leverage the existing
  `dispatchEnum` fold-expression helper from EnumDispatch.h more broadly

#### Phase 6 — std::string_view (20+ files, higher risk — API change)

Changes function signatures. Start with leaf utilities (no virtual overrides):

**Leaf utilities (safe to change first):**
- `StringUtils.h` — `tok()`, `toLower()`, `toUpper()`, `parse<T>()`,
  `startsWith()`, `endsWith()`, `match()`
- `File.h` — `read_file()`, `read_lines()`, `write_file()`, constructors

**Map-lookup APIs (key parameter only reads):**
- `Configurable.h` — `prop()`, `addProperty()`, `getPropertyType/Info/Tooltip()`,
  `supportsProperty()`, `deactivateProperty()`
- `MultiTypeMap.h` — `allocValue()`, `getValue()`, `contains()`, `getType()`
- `ConfigFile.h` — `load()`, `save()`, `set()`, `check_type()`

**Caveat:** Functions that call `.c_str()` internally (fopen, etc.) need the string
materialized — string_view doesn't guarantee null termination.

#### Phase 7 — std::optional (10+ files, highest risk — API change)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 25 — NeighborhoodOp fix, dead IPP cleanup, Accelerate mapping)

### Session 25 Summary

**NeighborhoodOp even-mask IPP workaround removed:**
- `computeROI()` unconditionally shrunk output ROI by 1px for even-sized masks —
  this was an IPP-specific workaround (`#ifdef ICL_HAVE_IPP`) that had been
  applied unconditionally in the rewritten "NEW Code" path. The C++ backend
  has no anchor bug, so the shrink was incorrect (lost a valid row/column).
- Removed the workaround and cleaned up dead commented-out old code block.
- Added 3 even-kernel tests: 4x4 identity (verifies 7x7 output from 10x10),
  2x2 sum (verifies 7x7 from 8x8), 4x4 cross-validate across backends.

**Dead CONFIGURE_GTEST macro removed:**
- Removed the unused `CONFIGURE_GTEST` function from ICLHelperMacros.cmake
  (~67 lines). No module called it — tests migrated to centralized `tests/`
  directory in session 19. The per-module test stubs no longer exist.

**Dead `#if 0` IPP blocks removed (~1,000 lines):**
- 5 _Ipp.cpp files cleaned to TODO stubs with modern API + Accelerate notes:
  AffineOp_Ipp.cpp (57→16 lines), ConvolutionOp_Ipp.cpp (261→17),
  LUTOp_Ipp.cpp (35→14), MedianOp_Ipp.cpp (110→16), MorphologicalOp_Ipp.cpp (229→19)
- CannyOp.cpp: removed 22-line `#if 0` IPP block, kept C++ fallback
- IntegralImgOp.cpp: removed 40-line `ICL_HAVE_IPP_DEACTIVATED` block
  (C++ loop-unrolled version was already faster)
- CoreFunctions.cpp: removed 448-line SSE block (moved to PixelOps.cpp)
  and 45-line dead IPP histogram block

**NeighborhoodOp.h cleanup:**
- Removed stale `TODO:: check!!` and `TODO: later private` comments
- Fixed typos in class documentation (shrinked→shrunk, adaptROI→computeROI)

**Accelerate-IPP mapping reference created:**
- `claude.insights/accelerate-ipp-mapping.md` documents which IPP operations
  have Apple Accelerate (vImage) equivalents, with function names and limitations
- Good coverage: affine warp, convolution, histogram, basic morphology
- Gaps: no median filter, no Canny, no integral image in vImage

**Accelerate filter backends — ConvolutionOp, MorphologicalOp, AffineOp:**
- `ConvolutionOp_Accelerate.cpp` using `vImageConvolve_PlanarF` for icl32f
  - Handles even-sized kernels by padding to odd dimensions (vImage requires odd)
- `MorphologicalOp_Accelerate.cpp` using `vImageDilate/Erode_Planar8/PlanarF`
  for icl8u and icl32f (all 11 optypes: basic, 3x3, borderReplicate, composites)
  - Planar8: uses ICL's binary mask directly (unsigned char, nonzero=include)
  - PlanarF: converts binary mask to float kernel (0.0=include, -INF=exclude)
    because vImage PlanarF morphology uses additive structuring elements
  - Composite ops (open/close/tophat/blackhat/gradient) create sub-ops that
    dispatch through Accelerate for inner dilate/erode
- `AffineOp_Accelerate.cpp` using `vImageAffineWarp_Planar8/PlanarF` for
  icl8u and icl32f with bilinear interpolation; NN falls back to C++ inverse map
  - Maps ICL's 2x3 forward matrix to `vImage_AffineTransform` struct
  - Background fill with 0 for out-of-bounds pixels
- All registered as `Backend::Accelerate` (priority 6, above C++ fallback)
- Accelerate header included before ICL to avoid macOS Point/Size name conflicts
- ICLFilter CMakeLists.txt: `_Accelerate.cpp` excluded when `NOT ACCELERATE_FOUND`;
  Accelerate framework linked via 3RDPARTY_LIBS
- All cross-validated against C++ backend across all test cases

**Benchmark results (640x480 images, 50 iterations):**

| Benchmark | Accelerate | C++ | Speedup |
|---|---|---|---|
| convolution gauss3x3 32f | 235 us | 314 us | 1.3x |
| convolution gauss5x5 32f | 339 us | 9904 us | **29x** |
| morphology dilate3x3 8u | 243 us | 6477 us | **27x** |

**DynMatrixBase<bool> for non-float/double users:**
- Replaced `DynMatrix<bool>` → `DynMatrixBase<bool>` across 18 files (85 occurrences)
- DynMatrix<T> method bodies are out-of-line, only instantiated for float/double;
  DynMatrix<bool> only "worked" because callers used inherited DynMatrixBase methods
- DynMatrixBase<bool> is the correct type: fully header-only, all operations available
- Files: GraphCutter.h/.cpp, 12 ICLGeom segmentation files, PhysicsWorld.cpp

**SmartArray<T> replaced with std::shared_ptr<T[]>:**
- C++17 `shared_ptr<T[]>` provides native array semantics (operator[], delete[])
- Replaced all 38 SmartArray usage sites across 10 files
- Owning: `shared_ptr<T[]>(ptr)`. Non-owning: `shared_ptr<T[]>(ptr, [](T*){})`
- PlotWidget::Buffer changed from inheritance to composition (can't inherit shared_ptr)
- `SmartArray.h` deleted — zero remaining consumers
- Key files: Img.h/cpp (channel storage), Array2D.h, SOM.h, Scene.h, PlotWidget/LowLevelPlotWidget

**Debug-mode warnings fixed (zero remaining):**
- AffineOp.cpp: member initializer order
- BinaryArithmeticalOp_Simd.cpp: unused lambda capture
- DynVector.cpp: struct/class mismatch in explicit instantiation

**Accelerate vDSP arithmetic backends (UnaryArithmeticalOp + BinaryArithmeticalOp):**
- `UnaryArithmeticalOp_Accelerate.cpp`: vDSP_vsadd/vsmul (add/sub/mul/div),
  vDSP_vsq (sqr), vvsqrtf/vvlogf/vvexpf (sqrt/ln/exp), vDSP_vabs — for icl32f
- `BinaryArithmeticalOp_Accelerate.cpp`: vDSP_vadd/vsub/vmul/vdiv + vabs — for icl32f
- Scalar multiply benchmark: **5.7x speedup** over C++

**Full BLAS Layer (Level 1/2/3) in BlasOps:**
- Extended BlasOps with complete BLAS coverage via backend dispatch:
  - Level 3: gemm (existing)
  - Level 2: gemv (matrix-vector multiply) — new
  - Level 1: vadd/vsub/vmul/vdiv/vsadd/vsmul (existing), dot/nrm2/asum/axpy/scal (new)
- C++ fallbacks in BlasOps_Cpp.cpp, Accelerate backends (cblas + vDSP) in
  BlasOps_Accelerate.cpp — both float and double
- Cached inline dispatch: `BlasOps<T>::dot(a, b, n)` — function-local static
  resolves backend on first call, ~1-2ns overhead thereafter
- DynMatrix element-wise ops (operator+/-/*, elementwise_mult/div) wired through
  BlasOps dispatch instead of std::transform

**Accelerate-IPP mapping updated:**
- Deep-dive of all Accelerate APIs (vImage, vDSP, vForce, BNNS, simd)
- Tier 1-3 opportunities ranked by impact
- See `claude.insights/accelerate-ipp-mapping.md`

**Tests: 367/367 pass.** Build clean, zero warnings on macOS.

### Next Steps

- **Wire BlasOps through codebase** — grep for hand-written dot products, norm
  calculations, axpy-style accumulations, and matrix-vector multiplies across
  ICLMath, ICLGeom, ICLCV, ICLFilter; replace with `BlasOps<T>::dot/nrm2/axpy/gemv`
- **Image scaling Accelerate backend** — `vImageScale_Planar8/PlanarF` for
  `scaledCopyChannelROI()` in Img.cpp (needs backend dispatch added first)
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **C++17 source pass** — std::filesystem, std::string_view, structured bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs

## Previous State (Session 24 — LapackOps expansion, API cleanup, DynMatrixBase split)

### Session 24 Summary

**New LapackOps (3 new ops, 2 gap-fills):**
- `geqrf` + `orgqr` (QR factorization via Householder reflectors) — all 4 backends
- `gelsd` (SVD least-squares solve) — all 4 backends
- `getrf`/`getri` added to MKL and Eigen backends (were missing)

**Accelerate backend row-major fix:**
- getrf, getri, geqrf, orgqr, gelsd now transpose explicitly to/from column-major
  before calling LAPACK. Ensures packed output (L/U, Householder reflectors)
  matches C++ backend convention. gesdd/syev keep dimension-swap trick.

**Consumer wiring:**
- `decompose_QR()` → geqrf + orgqr (was hand-written Gram-Schmidt)
- `decompose_LU()` → getrf + unpack (was hand-written partial pivoting)
- `solve()` → gelsd directly (was string-based method dispatch with "lu"/"svd"/"qr"/"inv")
- `pinv()` → always reduced SVD + BLAS gemm (was bool useSVD with QR fallback)
- `mult()` → BLAS gemm for float/double (was hand-written inner_product loop)
- `matrix_mult_t()` → gemm transpose flags for float/double (no temp copies)

**API simplification:**
- Removed `big_matrix_pinv()` — merged into `pinv()`
- Removed `big_matrix_mult_t()` — merged into `matrix_mult_t()`
- Removed `pinv(bool useSVD)` parameter — always SVD
- Removed `solve(b, string method)` parameter — always gelsd
- Removed `solve_upper_triangular()` / `solve_lower_triangular()` — dead code
- Removed `PolynomialRegression::apply()` useSVD parameter
- Removed dead `#if 0` block in DynMatrixUtils.h (old svd_cpp_64f)
- Removed dead EigenICLConverter (zero callers)

**DynMatrixBase<T> refactor — compilation time reduction:**
- New `DynMatrixBase<T>` (header-only, ~290 lines): storage, element access,
  flat iterators, properties, operator<</>>
- `DynMatrix<T>` inherits DynMatrixBase, adds col/row iterators, DynMatrixColumn,
  all arithmetic and linalg — declarations only in header (~200 lines)
- All method bodies in `DynMatrix.cpp` (~700 lines), whole-class instantiation
  for float/double: `template class ICLMath_API DynMatrix<float/double>;`
- `DynVector.h` declarations only, `DynVector.cpp` with whole-class instantiation
- DynMatrix.h includes only `<iterator>` beyond ICL headers (was `<numeric>`,
  `<functional>`, `<vector>`, `<cmath>`, `<algorithm>`)
- Concatenation operators (operator,/%) moved out of line

**License header cleanup:**
- Replaced 29-line decorative headers with 3-line SPDX across 1038 files (~27,600 lines removed)
- Format: `SPDX-License-Identifier: LGPL-3.0-or-later` / `ICL - Image Component Library (URL)` / `Copyright (original authors)`
- New top-level LICENSE file with project info and EXC 277 acknowledgment

**Tests: 364/364 pass (15 new).** Build clean, zero warnings on macOS.

### LapackOps Summary (8 operations, 4 backends)

| Op | Signature | C++ | Accelerate | MKL | Eigen |
|---|---|---|---|---|---|
| gesdd | SVD divide-and-conquer | ✓ | ✓ | ✓ | ✓ |
| syev | Symmetric eigenvalue | ✓ | ✓ | ✓ | ✓ |
| getrf | LU factorization | ✓ | ✓ | ✓ | ✓ |
| getri | LU inverse | ✓ | ✓ | ✓ | ✓ |
| geqrf | QR factorization | ✓ | ✓ | ✓ | ✓ |
| orgqr | Form Q from reflectors | ✓ | ✓ | ✓ | ✓ |
| gelsd | SVD least-squares solve | ✓ | ✓ | ✓ | ✓ |
| gemm | Matrix multiply (BlasOps) | ✓ | ✓ | ✓ | — |

### DynMatrix File Layout

```
DynMatrixBase.h   — storage, element access, streaming (header-only, any type)
DynMatrix.h       — col/row iterators, arithmetic/linalg declarations (~200 lines)
DynMatrix.cpp     — all method bodies + whole-class instantiation float/double
DynVector.h       — DynColVector/DynRowVector declarations
DynVector.cpp     — all method bodies + whole-class instantiation float/double
```

## Previous State (Session 23 — Full Backend Dispatch Architecture)

### Session 23 Summary

**FFTOps_Cpp.cpp segfault fixed:**
- Root cause: bug in `fft2D_cpp` — `buf(src.rows(),src.cols())` was element access
  (operator()) on a null pointer, not resize. Fix: one-liner → `buf.setBounds(...)`.
- FFTOps_Cpp.cpp keeps non-owning wrappers for src/dst (zero-copy, no overhead).

**FFTUtils.cpp fully wired to FFTOps — 21 MKL blocks eliminated:**
- Generic `fft2D`/`ifft2D` templates dispatch through FFTOps with type conversion.
- FFTDispatching.h/.cpp deleted. All explicit MKL specializations removed.

**SVD deduplicated — svd_dyn dispatches through LapackOps:**
- Single generic `svd_dyn<T>` template, no float/double specializations.
- Handles LAPACK Vt → ICL V convention via post-dispatch transpose.

**GLImg.cpp — last active IPP block removed.**

**Dead code cleanup — ~1,600 lines removed total:**
- `#if 0` blocks in FFTUtils.cpp, DynMatrixUtils.cpp, DynMatrix.cpp deleted.
- PThreadFix.h stripped to empty header.
- `jacobi_iterate_vtk` + `find_eigenvectors` removed from DynMatrix.cpp (moved to
  LapackOps_Cpp.cpp as `cpp_syev`).
- `get_minor_matrix` removed (inv() no longer uses cofactor expansion).

**Apple Accelerate backend:**
- CMake auto-detects on macOS via `find_library(Accelerate)`.
- `BlasOps_Accelerate.cpp`: cblas_sgemm/dgemm.
- `FFTOps_Accelerate.cpp`: vDSP_DFT row-column decomposition (arbitrary sizes).
- `LapackOps_Accelerate.cpp`: sgesdd/dgesdd, ssyev/dsyev, sgetrf/dgetrf, sgetri/dgetri.

**LapackOps dispatcher — clean BLAS/LAPACK separation:**
- `LapackOps.h`: `enum LapackOp { gesdd, syev, getrf, getri }`.
- `LapackOps.cpp`: constructor, instance(), toString().
- gesdd moved from BlasOps to LapackOps (all backends updated, all consumers rewired).
- BlasOps now contains only GEMM.

**LapackOps backends:**
- `LapackOps_Cpp.cpp`: gesdd (Golub-Kahan), syev (Jacobi), getrf (LU), getri (LU inverse).
- `LapackOps_Accelerate.cpp`: gesdd, syev, getrf, getri via Apple LAPACK.
- `LapackOps_Mkl.cpp`: gesdd, syev via MKL LAPACK.
- `LapackOps_Eigen.cpp`: gesdd (JacobiSVD), syev (SelfAdjointEigenSolver) via Eigen3.

**Backend enum expanded:**
- Added `Backend::Eigen = 3` between OpenBlas and Ipp.
- Priority order: Cpp(0) < Simd(1) < OpenBlas(2) < Eigen(3) < Ipp(4) < FFTW(5)
  < Accelerate(6) < Mkl(7) < OpenCL(8).

**Consumer wiring:**
- `svd_dyn()` → `LapackOps<T>::gesdd`
- `big_matrix_pinv()` → `LapackOps<T>::gesdd` + `BlasOps<T>::gemm`
- `DynMatrix::eigen()` → `LapackOps<T>::syev` (resolveOrThrow, C++ fallback always available)
- `DynMatrix::inv()` → `LapackOps<T>::getrf` + `getri` (O(n³) LU, replaces O(n!) cofactor)
- `DynMatrix::det()` for n>4 → `LapackOps<T>::getrf` (product of U diagonal, replaces O(n!) cofactor)

**C++ fallback convention enforced:**
- All C++ backend registrations now in `_Cpp.cpp` files.
- MathOps C++ backends moved from DynMatrixUtils.cpp → `MathOps_Cpp.cpp`.
- ImgOps channelMean moved from CoreFunctions.cpp → `Img_Cpp.cpp`.
- Only exception: ImgBorder.cpp (depends on file-local `_copy_border`, documented).

**Design decisions:**
- BlasOps for GEMM only; element-wise ops stay inlined (bandwidth-bound).
- FixedMatrix (4x4): keep inlined, IPP ippm dropped, compiler auto-vectorization sufficient.
- det() keeps special cases for 1x1–4x4 (zero overhead); n>4 uses LU.

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Add more LapackOps** — geqrf (QR factorization) to accelerate decompose_QR(), solve()
- **Investigate legacy test stubs** — Per-module test executables compile to 0 tests
- **NeighborhoodOp.cpp** — Two `#ifdef ICL_HAVE_IPP` workaround blocks (anchor bug)

## Previous State (Session 22 — BlasOps/FFTOps + BackendDispatching Refactor + BLAS Consumer Wiring)

### Session 22 Summary (continued)

**New BLAS/FFT abstraction layer + BackendDispatching refactor.**

**BackendDispatching refactored: array → sorted vector:**
- `std::array<ImplPtr, NUM_BACKENDS>` replaced with sorted `std::vector<Entry>`
- Backends sorted by priority descending (highest first); only registered backends occupy space
- `NUM_BACKENDS` constant removed entirely — enum can grow freely
- New `resolve()` / `resolveOrThrow()` no-arg overloads for dummy-context singletons
- New `Entry` struct: `{ Backend backend; shared_ptr<ImplBase> impl; }`
- `setImpl()` private helper maintains sorted order on insertion

**Backend enum expanded (values encode priority):**
```
Cpp=0, Simd=1, OpenBlas=2, Ipp=3, FFTW=4, Accelerate=5, Mkl=6, OpenCL=7
```

**BlasOps\<T\> — BLAS/LAPACK abstraction (new):**
- `BlasOps.h` — singleton template, `enum class BlasOp { gemm, gesdd }`
- `BlasOps.cpp` — constructor (addSelector), instance(), toString()
- `BlasOps_Cpp.cpp` — C++ fallback: naive GEMM + SVD via existing `svd_dyn`
- `BlasOps_Mkl.cpp` — MKL backend: `cblas_sgemm/dgemm` + `sgesdd/dgesdd`
- Raw pointer interface (no DynMatrix dependency in signatures)
- Consumer code will use `resolveOrThrow()` — C++ fallback always available

**FFTOps\<T\> — FFT abstraction (new, replaces FFTDispatching):**
- `FFTOps.h` — singleton template, `enum class FFTOp { r2c, c2c, inv_c2c }`
- `FFTOps.cpp` — constructor, instance(), toString()
- `FFTOps_Cpp.cpp` — C++ fallback via existing `fft2D_cpp` / `ifft2D_cpp`
- `FFTOps_Mkl.cpp` — MKL DFTI backend (forward/inverse, real/complex)
- Raw pointer interface; DynMatrix wrapping stays in consumers
- `FFTOps<float>` + `FFTOps<double>` — separate singletons per precision

**FFTDispatching transitional rename:**
- `FFTOp` enum → `LegacyFFTOp` to avoid conflict with FFTOps
- FFTDispatching struct kept temporarily until FFTUtils.cpp is rewired
- Will be deleted once FFTUtils.cpp uses FFTOps directly

**Convention: C++ fallbacks in `_Cpp.cpp` files:**
- All backend implementations (including C++ fallback) go in `_<Backend>.cpp` files
- The `.cpp` file (e.g., `BlasOps.cpp`) only contains constructor, instance(), toString()
- Pattern: `BlasOps_Cpp.cpp`, `BlasOps_Mkl.cpp`, `FFTOps_Cpp.cpp`, `FFTOps_Mkl.cpp`

**CMake: new exclusion patterns in ICLMath/CMakeLists.txt:**
- `_FFTW.cpp` excluded when `!FFTW_FOUND`
- `_Accelerate.cpp` excluded when `!ACCELERATE_FOUND`
- `_OpenBlas.cpp` excluded when `!OPENBLAS_FOUND`

**BLAS consumers wired (DynMatrix + DynMatrixUtils):**
- `DynMatrix.cpp`: `big_matrix_pinv` now uses `BlasOps<T>::gesdd` + `gemm` via
  `resolveOrThrow()`. No fallback logic — C++ backend always available.
- `DynMatrixUtils.cpp`: `big_matrix_mult_t` now uses `BlasOps<T>::gemm` via
  `resolveOrThrow()`. MKL explicit specializations removed.
- All `#ifdef ICL_HAVE_MKL` removed from DynMatrix.h, DynMatrix.cpp, DynMatrixUtils.cpp (6 blocks).
- `BlasOps_Cpp.cpp` GEMM optimized: compile-time dispatch of transA/transB/alpha/beta
  via template specialization + `if constexpr` (eliminates inner-loop conditionals).
  Epsilon-based `isZero()`/`isOne()` for float-safe alpha/beta comparison.
- `BlasOps_Cpp.cpp` SVD: full Golub-Kahan bidiagonalization (`svd_bidiag`) moved here
  from DynMatrixUtils.cpp. C++ GESDD backend always works (no more stub returning -1).

**FFT consumer wiring attempted but reverted (segfault):**
- Rewrote `fft2D<T1,T2>` and `ifft2D<T1,T2>` generic templates to use FFTOps dispatch.
  All 19 explicit specializations with `#ifdef ICL_HAVE_MKL` eliminated (replaced by
  template instantiations). The generic template handles type conversion + dispatch.
- **Segfault in multi-threaded tests.** Root cause: `FFTOps_Cpp.cpp` wraps raw pointers
  in non-owning `DynMatrix(cols, rows, ptr, false)` and passes to `fft2D_cpp`. The FFT
  row-column decomposition writes to `dst` with transposed dimensions — the non-owning
  wrapper's buffer may be too small or have wrong stride. Fix: use owned DynMatrix
  intermediates in the C++ backend, copy result back to raw pointer at the end.
- **Changes reverted** to keep branch green. FFTUtils.cpp still has 21 MKL blocks.

**Remaining `ICL_HAVE_MKL` references:**
- FFTUtils.cpp: 21 blocks (2 wrapper blocks + 19 specialization blocks, all still active)
- Camera.cpp: 1 (dead comment)
- PThreadFix.h: 1 (dead, inside `#if 0`)
- ICLConfig.h.in / doxyfile.in: build system definitions (must stay)

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Fix FFTOps_Cpp.cpp segfault** — Use owned DynMatrix in C++ FFT backend instead of
  non-owning wrappers. The issue is `fft2D_cpp`'s row-column decomposition writes `dst`
  with transposed buffer layout. Either: (a) allocate an owned DynMatrix, call fft2D_cpp,
  copy result to raw pointer; or (b) rewrite the C++ FFT backend to work directly on
  raw pointers without going through fft2D_cpp.
- **Wire FFTUtils.cpp to FFTOps** — The generic template rewrite is ready (tested
  single-threaded), just needs the C++ backend fix above. Once fixed, 21 MKL blocks
  disappear and FFTDispatching.h/.cpp can be deleted.
- **Deduplicate SVD** — `svd_internal` now lives in both `BlasOps_Cpp.cpp` and
  `DynMatrixUtils.cpp`. Wire `svd_dyn()` to dispatch through `BlasOps<T>::gesdd`,
  then remove the old copy from DynMatrixUtils.cpp.
- **Camera.cpp** — Remove dead MKL comment (trivial)
- **GLImg.cpp** — Last active `#ifdef ICL_HAVE_IPP` block (min/max, could use ImgOps)
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Consider LapackOps** — Separate dispatch for LAPACK operations (eigenvalue
  decomposition, Cholesky, LU, etc.) beyond just GESDD
- **FixedMatrix BLAS** — For small matrices (commonly 4x4), BlasOps dispatch overhead
  is too high. Keep inlined implementations. IPP's `ippm` module (small matrix ops) was
  dropped from modern IPP. Best choice for small matrices: compiler auto-vectorization
  of the inlined code, or hand-written SIMD for the hot 4x4 case.

## Previous State (Session 21 — A2 + B + C/IPP Complete)

### Session 21 Summary

**A2 complete, B complete, C/IPP complete for all modules.** All active
`#ifdef ICL_HAVE_IPP` operational code across ICLUtils/ICLCore/ICLMath/
ICLFilter/ICLIO has been migrated to dispatch or removed with TODOs.

**A2 — all three phases:**

**Phase 1 — Global string registry removed:**
- Deleted `detail::RegistryEntry`, `detail::globalRegistry()`, `detail::addToRegistry()`
- Deleted `registerBackend()`, `registerStatefulBackend()` static methods
- Deleted `loadFromRegistry()` private method
- Deleted `BackendDispatching.cpp` (only contained the registry impl)
- Removed `m_selectorByName` map and `m_prefix` string member
- Removed `initDispatching()` — all 15 filters, ImgOps, FFTDispatching updated
- Removed string-keyed `addSelector(string)`, `getSelector(string)`, `selectorByName(string)`
- Enum-keyed `addSelector(K)` is now standalone (no delegation to string version)
- Added `selector(K)` returning `BackendSelectorBase*` (replaces `selectorByName` for tests)
- Fixed `ThresholdOp_Simd.cpp` — switched from global registry to prototype direct registration

**Phase 2 — Stateful backend cloning (factory pattern):**
- `ImplBase` now has `std::function<shared_ptr<ImplBase>()> cloneFn` (null for stateless)
- `BackendSelector::clone()`: calls `cloneFn()` for stateful backends, shares `shared_ptr` for stateless
- `BackendSelector::addStateful(b, factory, applicability, desc)` — factory called per clone
- `BackendDispatching::addStatefulBackend<Sig>(key, b, factory, app, desc)` — convenience
- Migrated 4 stateful backends:
  - `WienerOp_Ipp.cpp` — IPP scratch buffer now per-instance
  - `WarpOp_OpenCL.cpp` — CLWarpState (GPU buffers/kernels) now per-instance
  - `BilateralFilterOp_OpenCL.cpp` — CLBilateralState now per-instance
  - `MorphologicalOp_Ipp.cpp` — MorphIppState (IPP state objects) now per-instance

**Phase 3 — AffineOp test bug fixed:**
- `AffineOp_Cpp.cpp`: bilinear interpolation bounds check now correctly validates
  the full 2x2 neighborhood (`x2 < width-1, y2 < height-1`)
- Edge pixels outside the safe bilinear zone fall back to nearest-neighbor
- **349/349 tests pass both single-threaded AND multi-threaded** (SIGTRAP eliminated)

**FFTDispatching migrated to enum pattern:**
- Added `enum class FFTOp : int { fwd32f, inv32f, fwd32fc }` with `toString()`
- Selector setup moved to `FFTDispatching()` constructor
- `FFTUtils.cpp` now uses `getSelector<Sig>(FFTOp::xxx)` (enum-keyed O(1))

**Test changes:**
- All `selectorByName("name")` calls replaced with `selector(FilterOp::Op::name)`

**B: ICLCore IPP blocks — all active blocks migrated:**
- `CCFunctions.cpp`: `planarToInterleaved`/`interleavedToPlanar` IPP specializations
  extracted to `Img_Ipp.cpp` as ImgOps selectors. Generic template now dispatches
  through ImgOps for same-type cases (S==D), falling back to `_Generic` if no backend.
  The `#ifdef` block + conditional instantiations removed. Two new `ImgOps::Op` values added.
- `BayerConverter.h/.cpp`: Removed dead IPP code. `nnInterpolationIpp()` was defined but
  never called (the `apply()` method always calls `nnInterpolation()` instead).
  `m_IppBayerPattern` member and constructor switch statements removed.
- Only `Types.h` retains compile-time `ICL_HAVE_IPP` guards (enum definitions, stays).

**BackendProxy for terser registration:**
- Added `BackendProxy` struct + `backends(Backend b)` method on `BackendDispatching`
- All ~65 `addBackend`/`addStatefulBackend` call sites across _Cpp/_Ipp/_Simd/_OpenCL files
  migrated to use the proxy: `auto cpp = proto.backends(Backend::Cpp); cpp.add<Sig>(...);`
- Removed `addBackend`/`addStatefulBackend` as public API — proxy calls getSelector().add() directly

**C/ICLMath — MathOps dispatch framework + all IPP removed:**
- Created `MathOps<T>` template singletons (`MathOps<float>`, `MathOps<double>`)
  using `BackendDispatching<int>` (dummy context, no applicability checks)
- `enum class MathOp` with 11 selectors: mean, var, meanvar, min, max, minmax,
  unaryInplace, unaryCopy, unaryConstInplace, unaryConstCopy, binaryCopy
- Sub-operation enums: `UnaryMathFunc` (12 ops), `UnaryConstFunc` (5 ops), `BinaryMathFunc` (6 ops)
- `DynMatrixUtils.cpp`: entire `#ifdef ICL_HAVE_IPP` block (lines 80-548) replaced with
  MathOps dispatch + C++ backend registration
- `DynMatrix.h`: removed IPP specializations (sqrDistanceTo, distanceTo, elementwise_div,
  mult-by-scalar, norm), added TODO
- `MathFunctions.h`: removed IPP mean specializations, added TODO
- `FixedMatrix.h`: removed unused `#include <ipp.h>`
- `CMakeLists.txt`: added `_Ipp.cpp`/`_Mkl.cpp` exclusion patterns

**C/ICLIO — all IPP removed:**
- `DC.cpp`: removed `ippiRGBToGray_8u_C3C1R`, always use weighted-sum loop
- `ColorFormatDecoder.cpp`: removed `ippiYUVToRGB_8u_C3R` + `ippiCbYCr422ToRGB_8u_C2C3R`
- `PylonColorConverter.h/.cpp`: removed `Yuv422ToRgb8Icl` and `Yuv422YUYVToRgb8Icl`
  IPP-only classes, always use PylonColorToRgb fallback
- TODOs added at every removal site

### Remaining `ICL_HAVE_IPP` References

Only these remain in the codebase:

| Category | Files | Status |
|---|---|---|
| Type definitions | BasicTypes.h, Size.h, Point.h, Rect.h, Types.h | Compile-time aliases, must stay |
| `#if 0` dead code | 5 *_Ipp.cpp files, FFTUtils (2), CannyOp, ImageRectification, CoreFunctions.cpp (histogramEven), DynMatrixUtils.cpp (ippm matrix ops) | Disabled, contains code for future re-enablement |
| IPP bug workaround | NeighborhoodOp.cpp (2 blocks) | Minor, stays |
| Qt GPU upload | GLImg.cpp (1 block) | ICLQt, deferred |
| Comments | CCFunctions.cpp (2 `/* */` blocks), DynMatrix.cpp (`#if 1 // was:`), DynMatrixUtils.cpp (SVD comment), IntegralImgOp.cpp (`ICL_HAVE_IPP_DEACTIVATED_...`) | Dead |
| CMake / config | CMakeLists.txt, ICLConfig.h.in, doxyfile.in | Build system definitions |
| Dead (nested `#if 0`) | PThreadFix.h | Inside outer `#if 0`, unreachable |

### Future IPP Optimization Opportunities (TODOs in code)

| Location | IPP Function | What It Accelerates |
|---|---|---|
| BayerConverter.cpp | `ippiCFAToRGB_8u_C1C3R` | Bayer → RGB nearest-neighbor |
| DynMatrix.h | `ippsNormDiff_L2`, `ippsDiv`, `ippsMulC`, `ippsNorm_L1/L2` | Matrix distance, element-wise div, scalar mult, norms |
| MathFunctions.h | `ippsMean_32f/64f` | Scalar mean over float/double arrays |
| DynMatrixUtils.cpp | All math ops already dispatched | Re-enable via `DynMatrixUtils_Ipp.cpp` |
| DC.cpp | `ippiRGBToGray_8u_C3C1R` | RGB → grayscale conversion |
| ColorFormatDecoder.cpp | `ippiYUVToRGB_8u_C3R`, `ippiCbYCr422ToRGB_8u_C2C3R` | YUV → RGB color conversion |
| PylonColorConverter.cpp | `ippiCbYCr422ToRGB_8u_C2C3R`, `ippiYCbCr422ToRGB_8u_C2C3R` | Pylon YUV422 → RGB |
| ConvolutionOp_Ipp.cpp | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | 34 Sobel/Gauss/Laplace specializations |
| MorphologicalOp_Ipp.cpp | `ippiDilate/Erode_*_C1R_L` + spec | Modern morphology with border |
| AffineOp_Ipp.cpp | `ippiWarpAffineNearest/Linear_*` + spec | Affine warp |
| MedianOp_Ipp.cpp | `ippiFilterMedianBorder_*_C1R` | Median filter with border |
| LUTOp_Ipp.cpp | `ippiReduceBits` (new signature) | Bit reduction |
| CannyOp.cpp | Modern `ippiCanny` with border spec | Canny edge detection |

### Next Steps

- **Wire consumers to BlasOps/FFTOps** — Replace 27 `#ifdef ICL_HAVE_MKL` blocks
  in DynMatrix.cpp, DynMatrixUtils.cpp, FFTUtils.cpp with BlasOps/FFTOps dispatch.
  Remove MKL includes, delete FFTDispatching.h/.cpp and LegacyFFTOp.
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Re-enable disabled IPP backends** — update to modern oneAPI APIs (see table above)
- **GLImg.cpp** — 1 remaining `ICL_HAVE_IPP` block in ICLQt
- **NeighborhoodOp.cpp** — 2 IPP bug workaround blocks (minor)
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD vs MKL comparison

### Key Files (Updated)

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework (sorted vector storage)
ICLMath/src/ICLMath/BlasOps.h                  — BLAS/LAPACK dispatch singleton
ICLMath/src/ICLMath/BlasOps.cpp                — constructor, instance, toString
ICLMath/src/ICLMath/BlasOps_Cpp.cpp            — C++ fallback (naive GEMM + svd_dyn SVD)
ICLMath/src/ICLMath/BlasOps_Mkl.cpp            — MKL backend (cblas_xgemm + xgesdd)
ICLMath/src/ICLMath/FFTOps.h                   — FFT dispatch singleton
ICLMath/src/ICLMath/FFTOps.cpp                 — constructor, instance, toString
ICLMath/src/ICLMath/FFTOps_Cpp.cpp             — C++ fallback (row-column FFT)
ICLMath/src/ICLMath/FFTOps_Mkl.cpp             — MKL DFTI backend
ICLMath/src/ICLMath/FFTDispatching.h           — TRANSITIONAL (LegacyFFTOp, delete after wiring)
ICLMath/src/ICLMath/FFTDispatching.cpp         — TRANSITIONAL (delete after wiring)
```

## Previous State (Session 20 — All Filters Migrated to Prototype+Clone)

### Session 20 Summary

**All 15 filters now use the prototype+clone pattern.** The global string
registry is no longer needed for filter dispatch.

Migrated the remaining 14 filters (ThresholdOp was already done in session 19):

| Filter | Selectors | Backend Files | Notes |
|---|---|---|---|
| WienerOp | apply | _Cpp, _Ipp | Now always built (removed IPP-only exclusion from CMakeLists) |
| LUTOp | reduceBits | _Cpp, _Ipp | Two constructors, both clone from prototype |
| AffineOp | apply | _Cpp, _Ipp | C++ uses FixedMatrix for inverse transform |
| ConvolutionOp | apply | _Cpp, _Ipp | Large dispatch chain moved to _Cpp.cpp |
| MorphologicalOp | apply | _Cpp, _Ipp | Buffer accessors added for _Cpp.cpp access |
| BilateralFilterOp | apply | _Cpp, _OpenCL | C++ bilateral filter (all depths) |
| BinaryLogicalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryArithmeticalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryCompareOp | compare, compareEqTol | _Cpp, _Simd | 2 selectors |
| WarpOp | warp | _Cpp, _Ipp, _OpenCL | 3 backend files |
| MedianOp | fixed, generic | _Cpp, _Ipp, _Simd | Sorting networks + Huang median in _Cpp |
| UnaryArithmeticalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryLogicalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryCompareOp | compare, compareEqTol | _Cpp, _Ipp, _Simd | 2 selectors |

**Pattern for each migrated filter:**
1. **Header** — `enum class Op : int { ... }`, `static prototype()`,
   `toString(Op)` declaration with `ICLFilter_API`
2. **_Cpp.cpp** — C++ backend implementations, registers via
   `prototype().addBackend<Sig>(Op::x, Backend::Cpp, fn, desc)`
3. **_Ipp/_Simd/_OpenCL.cpp** — same registration pattern into prototype
4. **.cpp** — constructor is `ImageBackendDispatching(prototype())`,
   `apply()` uses `getSelector<Sig>(Op::x)` (enum-indexed, O(1))
5. `toString(Op)` free function for ADL (used by `addSelector(K)`)

**CMake change:** WienerOp is no longer excluded when `!IPP_FOUND`. Its C++
backend throws an exception (IPP required), but the class is always available.

**MorphologicalOp special handling:** The C++ backend accesses private
composite-operation buffers. Added public buffer accessors
(`openingBuffer()`, `gradientBuffer1()`, `gradientBuffer2()`) so the free
function in `_Cpp.cpp` can use them via the `MorphologicalOp&` parameter.

**14 new _Cpp.cpp files created:**
```
ICLFilter/src/ICLFilter/WienerOp_Cpp.cpp
ICLFilter/src/ICLFilter/LUTOp_Cpp.cpp
ICLFilter/src/ICLFilter/AffineOp_Cpp.cpp
ICLFilter/src/ICLFilter/ConvolutionOp_Cpp.cpp
ICLFilter/src/ICLFilter/MorphologicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BilateralFilterOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryCompareOp_Cpp.cpp
ICLFilter/src/ICLFilter/WarpOp_Cpp.cpp
ICLFilter/src/ICLFilter/MedianOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryCompareOp_Cpp.cpp
```

**Test results:** 349/349 pass (single-threaded). Multi-threaded test crash
is a **pre-existing bug** in the AffineOp test: heap-buffer-overflow in
`Img<icl8u>::subPixelLIN()` when bilinear-interpolating a 2×2 image
(ASAN confirmed at `AffineOp_Cpp.cpp:45`, `test-filter.cpp:745`).

### Known Issue: Stateful Backend Sharing

With the prototype+clone pattern, `ImplBase` objects are shared across all
instances via `shared_ptr`. This is correct for stateless backends (free
functions), but **stateful backends share mutable state** across instances:

| Backend | Shared State | Impact |
|---|---|---|
| WienerOp_Ipp.cpp | IPP scratch buffer (`vector<icl8u>`) | Concurrent calls corrupt buffer |
| WarpOp_OpenCL.cpp | `CLWarpState` (GPU buffers/kernels) | Concurrent calls corrupt GPU state |
| BilateralFilterOp_OpenCL.cpp | `CLBilateralState` | Same |
| MorphologicalOp_Ipp.cpp | `MorphIppState` (IPP state objects) | Same |

**Fix needed:** Add a factory/creator pattern to `ImplBase` so that stateful
backends can create fresh state per clone. The `BackendSelector::clone()`
should call `impl->cloneImpl()` instead of copying the `shared_ptr`:

```
Option A: virtual cloneImpl() on ImplBase
  - Stateless impls: return shared_from_this() (share as before)
  - Stateful impls: call factory, return new Impl with fresh state

Option B: Store optional factory lambda on ImplBase
  - add() takes optional Factory parameter
  - clone() calls factory if present, else shares shared_ptr
  - Backends register: proto.addStatefulBackend<Sig>(key, backend,
      factory, applicability, desc)
  - The factory lambda creates fresh state each time
```

**Important:** If the state contains `Image` or `Img<T>` members, the factory
must `deepCopy()` them — ICL images use shallow copy by default.

## Previous State (Session 19 — Full ImgOps Dispatch Migration)

### Session 19 Summary

**Img.cpp + Img.h now have zero `#ifdef ICL_HAVE_IPP` blocks.**

Migrated all 7 remaining IPP-guarded operations in Img.cpp/Img.h to the
ImgOps BackendDispatch framework (Img_Cpp.cpp / Img_Ipp.cpp):

| Operation | IPP Functions | Depths |
|---|---|---|
| clearChannelROI | `ippiSet_*_C1R` | 8u, 16s, 32s, 32f |
| lut | `ippiLUTPalette_8u_C1R` | 8u |
| getMax | `ippiMax/MaxIndx_*_C1R` | 8u, 16s, 32f |
| getMin | `ippiMin/MinIndx_*_C1R` | 8u, 16s, 32f |
| getMinMax | `ippiMinMax/MinMaxIndx_*_C1R` | 8u, 32f |
| normalize | `ippiMulC/AddC_32f_C1IR` | 32f |
| flippedCopyChannelROI | `ippiMirror_*_C1R` | 8u, 32f |

**Key design details:**
- **Type-erased dispatch signatures** — operations that return typed values
  (getMax, getMin) use `icl64f` return through dispatch, cast back at call site
- **const methods** — `getMax`/`getMin`/`getMinMax`/`lut` use `const_cast` to
  pass `const ImgBase*` through `ImgBase*` dispatch context (safe: resolve
  only checks depth, backends don't modify source)
- **clearChannelROI** — dispatch at `Img<T>::clear()` level, not in the header
  template itself (avoids circular include: Img.h→ImgOps.h→Image.h→Img.h).
  Direct callers of `clearChannelROI<T>()` get the C++ path.
- **Mirror helpers moved to Img_Cpp.cpp** — `getPointerOffset`,
  `getMirrorPointerOffsets`, `getMirrorPointers`, plus the per-channel
  `Img<T>::mirror(axis, int, Point, Size)` definition (with explicit
  instantiations). These were only used by mirror and flippedCopy.
- **scaledCopyChannelROI** `#if 0` dead code cleaned up — just uses
  `ICL_INSTANTIATE_ALL_DEPTHS` now (IPP APIs deprecated, TODO for future)
- **`getStartIndex`/`getEndIndex`** are protected on `Img<T>` — backends
  inline the logic: `startC = ch < 0 ? 0 : ch`

**New dispatch signatures in ImgOps:**
```
ClearChannelROISig  = void(ImgBase&, int ch, icl64f val, const Point& offs, const Size& size)
LutSig             = void(ImgBase& src, const void* lut, ImgBase& dst, int bits)
GetMaxSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinMaxSig       = void(ImgBase&, int ch, icl64f* minVal, icl64f* maxVal, Point* minCoords, Point* maxCoords)
NormalizeSig       = void(ImgBase&, int ch, icl64f srcMin, icl64f srcMax, icl64f dstMin, icl64f dstMax)
FlippedCopySig     = void(axis, ImgBase& src, int srcC, const Point& srcOffs, const Size& srcSize,
                          ImgBase& dst, int dstC, const Point& dstOffs, const Size& dstSize)
```

**BackendDispatching rewrite (also this session):**
- `map<Backend,ImplPtr>` → `array<shared_ptr<ImplBase>, 4>` (fixed array, shared for cloning)
- Removed `backendPriority[]` — reverse iteration over enum values
- `BackendSelector` un-nested from `BackendDispatching` (standalone template)
- `resolve`/`resolveOrThrow`/`get` now const
- Added `virtual clone()` + clone constructor for per-instance dispatch
- Enum-keyed `addSelector(K)` with index assertion + ADL `toString(K)`
- Enum-keyed `getSelector<Sig>(K)` — O(1) vector index
- ImgOps backends register directly into singleton (no global registry)
- ThresholdOp migrated as proof of concept for prototype+clone pattern:
  all backends in `_Cpp.cpp` / `_Ipp.cpp`, constructor just clones prototype
- Removed dead code (`callWith`, `qualifiedName`, `backendPriority[]`)
- CoreFunctions `channel_mean` + ImgBorder `replicateBorder` migrated to ImgOps

**CMake note:** `FILE(GLOB)` is evaluated at configure time. After adding new
`_Cpp.cpp` / `_Ipp.cpp` files, re-run `cmake ..` to pick them up.

### Previous Session Summary (Session 18)

**Docker IPP build — now green:**
- Fixed `ContourDetector.cpp` missing `#include <cstring>`
- Fixed `CornerDetectorCSS.cpp` removed `ippsConv_32f` (deprecated, use C++ fallback)
- Fixed `CV.cpp` removed `ippiCrossCorrValid_Norm` / `ippiSqrDistanceValid_Norm` (deprecated)
- Fixed `TemplateTracker.h` missing `#include <ICLUtils/Point32f.h>`
- Fixed `DataSegment.h` missing `#include <cstring>`
- Fixed `ICLMarkers/CMakeLists.txt` spurious Qt PCH headers
- Fixed `FiducialDetectorPluginART.cpp` dead `Quick.h` include
- Fixed `FiducialDetectorPluginICL1.cpp` guarded `Quick.h`, added proper includes
- Fixed `ProximityOp.cpp` — provided stub implementations (was entirely inside `#if 0`)
- All modules compile and tests pass on Linux/IPP (Docker) and macOS

**Incremental Docker builds:**
- `build-and-test.sh` now uses `rsync` (preserves timestamps) instead of `cp -a`
- Use named Docker volume for persistent build cache
- Dockerfile adds `rsync` package

**ICLFilter IPP migration — complete:**
- Extracted `WarpOp` inline IPP (`ippiRemap_8u/32f_C1R`) to `WarpOp_Ipp.cpp` as `Backend::Ipp`
- Removed `#ifdef ICL_HAVE_IPP` from NeighborhoodOp.cpp (anchor workaround now always-on)
- Removed `#ifdef ICL_HAVE_IPP` from LocalThresholdOp.cpp (C++ path is both faster and higher quality)
- Removed `#ifdef ICL_HAVE_IPP` from UnaryOp.cpp (Canny creator — works without IPP now)
- Removed redundant `#ifdef ICL_HAVE_IPP` guards from all `_Ipp.cpp` files (CMake already excludes them)
- ICLFilter now has zero active `#ifdef ICL_HAVE_IPP` in non-backend files

**ImgOps singleton + ImgBaseBackendDispatching (new this session):**

Established the pattern for migrating `Img<T>` utility functions to dispatched backends.
First operation migrated: **mirror**.

Key design decisions:
- **ImgBaseBackendDispatching** (`BackendDispatching<ImgBase*>`) — new dispatch context for
  `Img<T>` methods, so they can dispatch via `this` without constructing an `Image` wrapper
- **ImgOps singleton** — inherits from `ImgBaseBackendDispatching`, owns `BackendSelector`s
  for Img utility operations (mirror, and later: min/max, lut, normalize, etc.)
- **ALL implementations in separate files** — `Img_Cpp.cpp` has the C++ fallback,
  `Img_Ipp.cpp` has the IPP backend. The `Img<T>` method itself is dispatch-only.
  This ensures dispatch is always used regardless of call path (`Image::mirror()` or
  `Img<T>::mirror()` directly).
- **`applicableToBase<Ts...>()`** — applicability helper for `ImgBase*` context (checks depth)
- **`resolveOrThrow()`** — safe dispatch that throws `std::logic_error` with selector name
  instead of returning nullptr

**Mirror migration details:**

```
Call chain:
  Image::mirror(axis)
    → ImgBase::mirror(axis, bool)  [virtual]
      → Img<T>::mirror(axis, bool)  [dispatch-only, calls resolveOrThrow]
        → ImgOps::instance().getSelector<MirrorSig>("mirror").resolveOrThrow(this)
          → Backend::Ipp: Img_Ipp.cpp — ippiMirror_8u/16u/32s_C1IR (4 depths)
          → Backend::Cpp: Img_Cpp.cpp — calls Img<T>::mirror(axis, ch, offset, size)

The per-channel Img<T>::mirror(axis, int, Point, Size) is the raw C++ swap
implementation. It never dispatches — backends call it directly.
```

Files created:
- `ICLCore/src/ICLCore/ImgOps.h` — singleton header, dispatch signatures
- `ICLCore/src/ICLCore/ImgOps.cpp` — singleton impl, creates selectors
- `ICLCore/src/ICLCore/Img_Cpp.cpp` — C++ backend (mirror)
- `ICLCore/src/ICLCore/Img_Ipp.cpp` — IPP backend (mirror)
- `ICLCore/src/ICLCore/ImageBackendDispatching.h` — added `ImgBaseBackendDispatching` + `applicableToBase<>`

Changes:
- `ICLCore/CMakeLists.txt` — added `_Ipp.cpp` exclusion pattern (same as ICLFilter)
- `ICLCore/src/ICLCore/Img.cpp` — `Img<T>::mirror(axis, bool)` is now dispatch-only
- `ICLCore/src/ICLCore/Img.h` — made per-channel mirror/normalize public (backends need access)
- `ICLUtils/src/ICLUtils/BackendDispatching.h` — added `resolveOrThrow()`, `#include <stdexcept>`

### Important Rules (Learned This Session)

1. **Never delete IPP/MKL code** — extract to `_Ipp.cpp`/`_Mkl.cpp` backend files.
   IPP specializations have real performance value (BLAS, image ops, etc.).

2. **No `#ifdef ICL_HAVE_IPP` in `_Ipp.cpp` files** — CMake excludes them via
   `list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")` when `!IPP_FOUND`.

3. **All implementations in backend files** — both `_Cpp.cpp` AND `_Ipp.cpp`.
   The main code is dispatch-only. This ensures dispatch works regardless of call path.

4. **MKL follows the same pattern** — `_Mkl.cpp` files, `Backend::Mkl` enum (to be added).

### Previous Session Summary (Session 17)

**BackendDispatching refactoring:**
- Nested `BackendSelectorBase`, `BackendSelector<Sig>`, `ApplicabilityFn` inside `BackendDispatching<Context>`
- `ImageBackendDispatching` is now just `using = BackendDispatching<Image>` (removed `Dispatching` alias)
- All 15 filter headers updated from `core::Dispatching` to `core::ImageBackendDispatching`
- `dispatchEnum` applied to BinaryOp SIMD backends to eliminate inner-loop branching

**Cross-validation tests (20 new, 349 total):**
- Added `crossValidateBackends()` template helper (forces C++ ref, iterates all backend combos)
- All 15 BackendDispatch filters now have cross-validation tests
- Tests cover per-depth validation for all applicable depths

**Benchmarks (25 filter benchmarks):**
- All benchmarks use 1000x1000 (1M pixels) baseline
- Backend parameter: `-p backend=cpp/simd/ipp/auto` for direct comparison

### IPP APIs — What's Active vs Disabled

**ACTIVE (compiles with modern oneAPI IPP 2022+):**

| Backend File | IPP Functions | Filter |
|---|---|---|
| ThresholdOp_Ipp.cpp | `ippiThreshold_LTVal/GTVal_*` | ThresholdOp |
| UnaryCompareOp_Ipp.cpp | `ippiCompareC_*` | UnaryCompareOp |
| UnaryLogicalOp_Ipp.cpp | `ippiAndC/OrC/XorC_*` | UnaryLogicalOp |
| WienerOp_Ipp.cpp | `ippiFilterWiener_*` | WienerOp |
| WarpOp_Ipp.cpp | `ippiRemap_*` | WarpOp |
| Img_Ipp.cpp | `ippiMirror_*` | Img::mirror (new) |
| Img.cpp (inline) | `ippiLUTPalette_*`, `ippiMin/Max*`, `ippiMulC/AddC_*` | Img utilities (TODO) |
| CoreFunctions.cpp (inline) | `ippiMean_*` | channel mean (TODO) |
| DynMatrixUtils.cpp (inline) | `ippsMean_*`, `ippsStdDev_*`, `ippsMeanStdDev_*` | matrix stats (TODO) |
| DynMatrix.h (inline) | `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`, `ippsNormDiff_*` | matrix ops (TODO) |
| MathFunctions.h (inline) | `ippsMean_*` | math mean (TODO) |

**DISABLED (deprecated/removed APIs — TODO re-add via BackendDispatch):**

| Location | Deprecated API | Modern Replacement | Priority |
|---|---|---|---|
| `ConvolutionOp_Ipp.cpp` | `ippiFilterSobelHoriz/Vert/Laplace/Gauss_*` | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | HIGH — 34 specializations |
| `MorphologicalOp_Ipp.cpp` | `ippiMorphologyInitAlloc_*`, `ippiDilate/Erode_*_C1R` | `ippiDilate/Erode_*_C1R_L` + spec buffers | HIGH |
| `AffineOp_Ipp.cpp` | `ippiWarpAffine_*_C1R` | `ippiWarpAffineNearest/Linear_*` + spec | MEDIUM |
| `MedianOp_Ipp.cpp` | `ippiFilterMedian_*_C1R` | `ippiFilterMedianBorder_*_C1R` | MEDIUM |
| `LUTOp_Ipp.cpp` | `ippiReduceBits_8u_C1R` | Modern `ippiReduceBits` (added noise param) | LOW |
| `CannyOp.cpp` (inline) | `ippiCanny_32f8u/16s8u_C1R` | Modern `ippiCanny` with border spec | MEDIUM |
| `ProximityOp.cpp` | `ippiSqrDistance/CrossCorr Full/Same/Valid_Norm_*` | `ippiSqrDistanceNorm_*` | LOW |
| `Img.cpp` (inline) | `ippiResizeSqrPixel_*` | `ippiResizeLinear/Nearest_*` | MEDIUM |
| `CoreFunctions.cpp` (inline) | `ippiHistogramEven_*` | `ippiHistogram_*` (new API) | LOW |
| `FFTUtils.cpp` (inline) | `ippiFFTInitAlloc_*` | `ippiFFTInit_*` + manual buffers | MEDIUM (or use MKL) |
| `DynMatrix.h/.cpp` (inline) | `ippmMul_mm/Invert/Det/Eigen_*` | MKL BLAS/LAPACK | MEDIUM (ippm module dropped entirely) |
| `DynMatrixUtils.cpp` (inline) | `ippmAdd/Sub/Mul_mm/tm/tt_*` | MKL BLAS | MEDIUM |

### Backend Dispatch Framework

```
BackendDispatching<Context>           — ICLUtils (header-only, no .cpp)
  BackendSelectorBase<Context>        — abstract per-selector base
  BackendSelector<Context, Sig>       — typed dispatch table
    .add(b, f, applicability, desc)   — register stateless backend
    .addStateful(b, factory, app, d)  — register stateful backend (factory per clone)
    .resolve(ctx) → ImplBase*         — returns nullptr if no match
    .resolveOrThrow(ctx) → ImplBase*  — throws logic_error if no match
    .clone()                          — stateful: calls cloneFn(); stateless: shares shared_ptr
  ApplicabilityFn<Context>            — std::function<bool(const Context&)>
  ImplBase::cloneFn                   — optional factory for stateful backends

API on BackendDispatching<Context>:
  addSelector<Sig>(K key)             — enum-keyed only (no string overloads)
  getSelector<Sig>(K key)             — O(1) vector index
  selector(K key)                     — returns BackendSelectorBase* (introspection/tests)
  addBackend<Sig>(K, b, f, app, desc) — convenience for getSelector().add()
  addStatefulBackend<Sig>(K, b, factory, app, desc) — convenience for getSelector().addStateful()

Two context types:
  ImageBackendDispatching             — BackendDispatching<Image>
  ImgBaseBackendDispatching           — BackendDispatching<ImgBase*>

ImgOps singleton                      — ICLCore (enum class Op, 10 selectors)
FFTDispatching singleton              — ICLMath (enum class FFTOp, 3 selectors)

Filter prototype+clone pattern        — all 15 ICLFilter ops
  Static prototype() holds selectors + ImplBase objects
  Constructor clones: ImageBackendDispatching(prototype())
  Stateful backends get fresh state per instance via factory cloneFn
  _Cpp.cpp / _Ipp.cpp / _Simd.cpp / _OpenCL.cpp register into prototype()

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
       _Cpp.cpp always built
```

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks to Migrate

**ICLCore — Img.cpp and Img.h are DONE (zero `#ifdef ICL_HAVE_IPP`).**

Remaining ICLCore files:
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions (compile-time, may stay)

**ICLMath — needs own dispatch singleton (similar pattern):**
- `DynMatrix.h` — `ippsNormDiff_L2_*`, `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`
- `MathFunctions.h` — `ippsMean_*`
- `DynMatrixUtils.cpp` — mean/stddev/meanstddev (3 blocks), unary math functions (large block)

**ICLIO — needs own dispatch or extend ImgOps:**
- `DC.cpp` — `ippiRGBToGray_8u_C3C1R`
- `ColorFormatDecoder.cpp` — `ippiYUVToRGB_8u_C3R`
- `PylonColorConverter.cpp/.h` — YUV conversion classes

### Docker Build Commands

```bash
# First run (full build with persistent volume):
docker build --platform linux/amd64 -t icl-ipp packaging/docker/noble-ipp
docker run --platform linux/amd64 --rm -e JOBS=16 -e BUILD_DIR=/build-cache \
  -v $(pwd):/src:ro -v icl-ipp-build:/build-cache \
  icl-ipp bash /src/packaging/docker/noble-ipp/build-and-test.sh

# Subsequent runs (incremental — only recompiles changed files):
# Same command — volume "icl-ipp-build" persists CMake state + object files
```

### Key Files

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework template (header-only, no .cpp)
ICLUtils/src/ICLUtils/EnumDispatch.h           — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h  — Image + ImgBase* typedefs
ICLCore/src/ICLCore/ImgOps.h                   — singleton header, dispatch signatures
ICLCore/src/ICLCore/ImgOps.cpp                 — singleton impl, creates selectors
ICLCore/src/ICLCore/Img_Cpp.cpp                — C++ backends (8 ops + mirror helpers)
ICLCore/src/ICLCore/Img_Ipp.cpp                — IPP backends (8 ops)
ICLMath/src/ICLMath/FFTDispatching.h           — FFTOp enum, FFTDispatching singleton
ICLMath/src/ICLMath/FFTDispatching.cpp         — FFT C++ backends
ICLFilter/src/ICLFilter/*_Cpp.cpp              — 15 C++ backend files (one per filter)
ICLFilter/src/ICLFilter/*_Ipp.cpp              — IPP backends (excluded when !IPP_FOUND)
ICLFilter/src/ICLFilter/*_Simd.cpp             — SIMD backends (always built)
ICLFilter/src/ICLFilter/*_OpenCL.cpp           — OpenCL backends (excluded when !OPENCL_FOUND)
tests/test-filter.cpp                          — 349 tests
benchmarks/bench-filter.cpp                    — 25 filter benchmarks
packaging/docker/noble-ipp/                    — Docker IPP build
.github/workflows/ci.yaml                     — CI with IPP job
```

### Next Steps

#### A. ~~Migrate all 14 remaining filters to prototype+clone pattern~~ **DONE** (Session 20)

All 15 filters now use prototype+clone. See Session 20 summary above.

#### A2. ~~Remove global string registry + add stateful backend cloning~~ **DONE** (Session 21)

See Session 21 summary above. All three phases complete.

#### B. ~~Remaining ICLCore IPP blocks~~ **DONE** (Session 21)

- ~~CoreFunctions.cpp — channel_mean~~ **DONE** (Session 19)
- ~~ImgBorder.cpp — border replication~~ **DONE** (Session 19)
- ~~CCFunctions.cpp — planarToInterleaved/interleavedToPlanar~~ **DONE** (Session 21, added to ImgOps)
- ~~BayerConverter.h/.cpp~~ **DONE** (Session 21, removed dead IPP code — `nnInterpolationIpp` was never called)
- Types.h — enum value definitions (compile-time, stays as-is)

#### C. Other modules

- ~~**ICLMath IPP**~~ **DONE** (Session 21) — MathOps<T> singletons, all `ICL_HAVE_IPP` removed
  from headers + DynMatrixUtils.cpp. CMake `_Ipp.cpp`/`_Mkl.cpp` exclusion added.
- **ICLMath MKL** — 27+ `#ifdef ICL_HAVE_MKL` blocks remain (DynMatrix, DynMatrixUtils,
  FFTUtils). Need `_Mkl.cpp` files + `Backend::Mkl` enum value. Deferred.
- ~~**ICLIO**~~ **DONE** (Session 21) — DC.cpp, ColorFormatDecoder.cpp, PylonColorConverter
  IPP guards removed, C++ fallbacks always used, TODOs added
- **Update disabled IPP backends** to modern oneAPI APIs
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
