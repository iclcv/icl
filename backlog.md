# Backlog

Terse running TODO index (1–5 words each) so nothing gets forgotten. Detail lives in the
linked plans; this is just the index. Check off / prune as done.

## ✅ DONE — matrix (row,col) migration complete (S94)
- [x] **`DynMatrix` dim-order flip — DONE.** Flipped `DynMatrixBase` dim ctors to `(rows,cols)` (matching the `(row,col)` accessors), migrated all ~186 sites via the private-ctor + `create()`/`fromData()` forcing recipe (compiler pinpointed every site; Python scripts `flip_dynmatrix.py`/`flip_decls.py` did the bulk swap), then restored a public `(rows,cols)` ctor (create/fromData kept as self-documenting aliases). KEY BUG the process caught: constructing a `DynMatrix` base from a `fromData(...,false)` **shallow** temporary via the copy ctor silently deep-copied → broke write-through views (LMA `y_est` went stale → calibrate_extrinsic regressed 5.8→70px); fixed by adding a **move ctor** to `DynMatrixBase`/`DynMatrix` (transfers the buffer, preserving shallow wraps) + restored copy-assign (no move-assign, so `view = rvalue` still deep-copies into place). 1010/1010.
- [x] **`FixedMatrix<T,COLS,ROWS>` → `<T,ROWS,COLS>` flip — DONE (S94).** Made `FixedColVector`/`FixedRowVector` **alias templates** first (they only added ctors) → the ~94 vector sites absorb the flip via the 2 alias lines (fallout: dropped a duplicate `SimplexOptimizer` instantiation; converted FixedMatrix's unused templated range ctor `(begin,end)` to a named `fromRange()` factory since it hijacked two-value calls like `Pos(0,0)`). Phase A: spelled ~77 raw `<T,1,N>`/`<T,N,1>` sites as FixedCol/RowVector. Phase B: reorder ONLY the primary class template-param decl (the one edit that flips external meaning; bodies use COLS/ROWS by name), swap arg2↔arg3 of every explicit `FixedMatrix<...>` spelling (behaviour-preserving under the reordered convention), flip the aliases, swap the ~17 genuine non-square (Camera 3×4/4×3, PoseEstimator NUM_POINTS×N, gramSchmidtOrtho). Completeness by tree-wide grep of every non-square/symbolic spelling (static_assert pass unnecessary). `PixelRef`/`DrawWidget` shape-agnostic helpers left as-is. 1010/1010.

## geom → geom2 retirement (active arc)
Plan + per-app detail: `geom-retirement-worklist.md`. Keep demos/apps split (CLAUDE.md).
- [ ] **`geom2::PlotWidget3D` coord-box tics/labels off** (S97) — `makeAxis()` in
  `geom2/PlotWidget3D.cpp`: inverted (Y) axis places labels at `-r` while the tic mesh
  stays at `+r` → labels mirrored off their ticks; tics also draw as L-brackets (two
  segments). Needs a real display to verify label↔tick alignment across all 3 rotated
  axes. Surfaced by `icl-model-fitting-playground`'s superquadric tab.
- [x] Port scene-object, simplex-3D, generic-texture-coords, texture-cube,
      scene-shadows, scene-graph, superquadric (+`SuperquadricNode`),
      offscreen-rendering (demos)
- [x] Port surf-based-object-tracking, rotate-image-3D, depth-camera-simulator (apps)
- [x] `ray-cast-octree`, point-cloud viewer+pipe, show-scene→scene-viewer (earlier)
- [x] marker-detection + multi-cam-marker-demo (markers; geom2 dep added per-target)
- [ ] **camera-calibration REDESIGN** (multi-session, ACTIVE) — plan + findings:
      `camera-calibration-redesign.md`. Not a transliteration: rethinking the drift-prone
      3D joint-DLT pipeline. Decisions: planar primary / 3D kept for multi-cam one-click;
      registerable `CalibrationTarget` backend (detect+generate; checkerboard NEW + marker-grid);
      ICL-native intrinsics vs OpenCV comparison; harness-first.
  - [x] **Phase A.1** projection harness (`test-geom2-calibration-harness`) — reproduces the
        depth drift quantitatively (near 700mm: 2mm; far 3000mm: 351±265mm).
  - [x] **geom2 offscreen GL render from a worker thread** (PBuffer port LANDED). Legacy
        `geom::Scene::PBuffer` ported as `GLSceneCapture(ownContext=true)` — owns a
        `QOpenGLContext`+`QOffscreenSurface` (shares lists w/ globalShareContext), lazily
        created in (and thread-bound to) the calling thread, makeCurrent → `renderToImage` →
        doneCurrent. Renders a Scene2 to RGB+depth `Image` from any (worker) thread.
        **CAVEAT:** a `Renderer` uses VAOs (NOT context-shareable), so owned-context mode is for
        scenes rendered ONLY offscreen — you cannot capture the same on-screen `Scene2` through
        both the widget context and the offscreen one. Wiring the lab/depth-sim therefore needs a
        dedicated capture scene (or drop on-screen `Canvas3D`, show captured images) — a design
        step beyond the raw port.
  - [x] **ChESS checkerboard detector** (`cv::CheckerboardSaddleDetector`) + `CalibrationTarget`
        scaffold + `icl-checkerboard-detection-lab`. **Lab now on REAL offscreen render** (option
        (a)): a dedicated `capScene` mirrors the board + tracks the interactive camera, rendered
        from `run()` via `GLSceneCapture(ownContext=true)` → `captureRGB` → CPU `k1,k2` distortion
        → detect → overlay; 2nd canvas undistorts. Homography hack removed; lighting toggle added.
        Build-checked only (no GL in sandbox) — real-display pass owed.
  - [x] **lab: Cycles-rendered targets (LANDED).** The checkerboard-detection-lab has an
        `offscreen` renderer Combo (`GL (fast)` / `Cycles (photoreal)`), guarded by
        `#ifdef ICL_HAVE_CYCLES` (no meson change — `CyclesRenderer` is in the geom2 lib). Cycles
        is GL-free → renders `capScene` from `run()` (lazy, Preview/32 spp).
        **GOTCHA (crashed first cut):** `CyclesRenderer` has TWO mutually-exclusive drive models —
        `start()`+autonomous management thread, OR poll-driven `render()`. Calling BOTH (as
        cycles-scene-viewer does) races on a non-atomic `initialized` flag and on the shared ICL
        scene → use-after-free. Use `render()` ONLY (single driver thread); never `start()` when
        the app also mutates the scene from `run()`, and never `renderBlocking()` (its
        `session->wait()` freezes the loop through first-time kernel compile). Real-display
        verification still owed.
  - [ ] Phase A.2 rendering harness (geom2 offscreen + synthetic light/noise → detect → calibrate).
  - [x] **investigated** calibrate_extrinsic divergence: broken linear SVD seed (cheirality/scale); LMA itself is correct + wide basin; fix = homography/PnP seed (Phase B). Regression locked (4.4mm vs 249mm).
  - [x] **detector ~13× faster** (`CheckerboardSaddleDetector`: precomputed weights, squared gate, OpenMP; OpenCL tried + dropped as slower).
  - [x] **grid recovery v1** (`cv::CheckerboardGrid` + growth) + **guided growth** (edge-evidence axes; fixes the strong-foreshortening diagonal trap) + **edge validation pass** (`scoreCheckerboardGridEdges`, perpendicular-spacing probe; lab colours edges by it).
  - [x] **`markers::CheckerboardTarget`** — first concrete `CalibrationTarget` (detector+recovery → correspondences).
  - [x] **`cv::CheckerboardDetector` technique interface** — swappable seam under `CheckerboardTarget` (`detect(Img8u,Hints)→Result{vector<CheckerboardGrid>}`; Hints carry board dims + a distortion-estimate feedback channel). Backends: `NativeCheckerboardDetector` (ChESS+growth) + `OpenCVCheckerboardDetector` (former `cv::CheckerboardDetector`, now a backend; findChessboardCorners → complete grid). = comparison-harness substrate.
  - [x] **false-positive suppression** — `refineCheckerboardGrid()`: robust homography fit → Hungarian seed↔node re-association (gated, rejects via dummies) → boundary trim of weakly-supported/contrastless ranks. Opt-in `NativeCheckerboardDetector::setCleanup()` (OFF by default: a plain homography mispredicts lens-distorted borders until a distortion estimate is fed back via Hints). NOTE: catches irregular/partial + geometrically-inconsistent phantoms; a phantom rank on the *extended regular lattice* would survive (edge-score trim is conservative — through-square edges still score ~0.69).
  - [x] **`markers::MarkerGridTarget`** — 2nd concrete `CalibrationTarget`, wraps `AdvancedMarkerGridDetector`. `detect()` → each found marker's 4 corner correspondences (grid-space mm ↔ image px); partial/occluded grid still calibrates + absolutely labelled (markers self-ID). `generate()` renders BCH markers via `FiducialDetector::createMarker`. PIMPL hides the non-const detector. Roundtrip test: 4×3 grid, 48 corrs, sub-px affine residual.
  - [x] **comparison harness (Phase B)** — `test-markers-calibration-comparison`: render perspective view (warp generate()+noise) → detect() → completeness + homography residual, BOTH targets × every backend × {frontal,keystone,noisy}. Numbers: checkerboard native ~0.6px vs **opencv ~0.01px** (native has NO sub-pixel polish); native-graph fails under noise; marker none/edge/pattern 0.52/0.39/0.56 (pattern's win is exposure, not clean px).
  - [x] **native-checkerboard sub-pixel corner polish (LANDED)** — `cv::refineCheckerboardCornersSubPix()` (OpenCV cornerSubPix-style gradient orthogonality: Σw·gg^T q = Σw·gg^T p over a Gaussian window, window capped to 0.4× cell spacing). Wired default-ON into all three native backends (`setSubPixel`) after cleanup, + lab `subpixel` toggle. Phase-B now checkerboard native **0.002/0.014/0.012px** (frontal/keystone/noisy) — matches/beats opencv 0.005/0.017/0.015. Added a **ground-truth accuracy metric** to Phase-B (analytic gen corners → exact gen→view homography → nearest-corner RMS, frame-invariant): native g=0.007/0.015/0.016 vs opencv 0.009/0.020/0.018 — confirms it's absolutely accurate, not just self-consistent. Test `cv.checkergrid.subpixel_refine_improves_corners` (0.56→0.08px on a keystone synthetic; render supersampling is the floor). 1003/1003.
  - [x] **marker sub-pixel corner refiners** — `cv::SubPixelCornerRefiner` (edge-line fit, 2× on markers) + `markers::MarkerPatternRefiner` (BCH interior edges, exposure-bias-robust: 0.78% vs 1.8% drift). `MarkerGridTarget::RefineMode{None,Edge,Pattern}`.
  - [x] **two-solution planar pose (IPPE)** — `CoplanarPointPoseEstimator::getPoses` closed-form (Collins-Bartoli), both flip solutions + errors, deterministic ~1e-5px. Demo `single-marker-pose`. TODO: wire into Fiducial pose getters + temporal disambiguation; grid-based pose.
  - [x] **calib-target-detection-lab** — generalized the checkerboard lab (markers/apps) over `CalibrationTarget` (both targets), context-sensitive controls, dynamic detector `Prop`.
  - [ ] **iterative undistortion bootstrap** (`CalibrationSession` above `IntrinsicCalibrator`) — detect raw / calibrate raw, rectify only to reach border boards (predict-and-refine, not full-frame inverse warp — diverges k1≳0.2); accept param update iff held-out reproj error improves; coverage map + re-process stored frames. Makes the opt-in cleanup safe-by-default.
  - [x] **lab: backend switch + LAP cleanup toggle** wired into `icl-checkerboard-detection-lab`; **OpenCV null-Mat crash fixed** (inherited bug); **`save frame`** button dumps detector input. Diagonal-trap investigation: orientation-gate attempt reverted (inert in synthetics, degrades under extreme shear); real saved frame recovers fine (axis-aligned; cleanup → 6×4/24) — the dramatic trap is a steeper pose not yet captured.
  - [x] **DEEP RESEARCH — robust checkerboard grid ASSOCIATION** (`checkerboard-association-research.md`, cited). LANDED `cv::RansacCheckerboardDetector` (`"native-ransac"`): RANSAC affine seed (trap-immune) → de-shear core to true axes → local-step growth → de-shear. Pareto win over greedy growth (fixes diagonal trap φ≈40–55°, keystone parity); φ≲30° + k≳0.5 still fail (growth does too). Tests `cv.checkergrid.ransac_*`; 991/991. Note: `ftdlyc/libcbdetect` is GPL (can't vendor; reimplement from papers).
  - [ ] **coded corners (BCH / topological) — make association ABSOLUTE.** Embed an identity/position code per corner/region (PuzzleBoard family, research doc Family 4; reuse ICLMarkers BCH `FiducialDetector`) so each corner self-identifies → no axis bootstrap, no diagonal trap, partial/occluded boards still calibrate. Fits as a new `CalibrationTarget` (+ generate()) and/or `CheckerboardDetector` backend; ChESS still owns sub-pixel precision, the code owns identity. PuzzleBoard reference is CC0 (vendorable/clean-room).
    - [ ] **ChArUco-style HYBRID target = pragmatic realization of "coded corners"** (idea, park till after Phase B). A checkerboard whose cells carry inset BCH markers: **markers → identity/association, ChESS saddle + `refineCheckerboardCornersSubPix` → precision corners** (best of both — this is exactly OpenCV's ChArUco rationale). Markers do NOT set corner positions (our marker-grid corners ~0.39–0.52px vs saddle ~0.01px → 40× worse; using marker corners would degrade the fit). Big win beyond robustness: it **eliminates grid association entirely** — a decoded marker gives board-id + (col,row) + orientation directly, so no growth/RANSAC/graph, no diagonal trap, no arbitrary-frame ambiguity (the exact wart the Phase-B ground-truth metric had to nearest-match around), and partial/occluded/multi-board Just Works. DESIGN: markers go in cells **inset with a quiet margin** so (a) diagonally-touching quads don't merge and (b) the corner's ~5px saddle ring stays clean; filling black squares edge-to-edge breaks both. A morphological erosion is a fallback for the ID/segmentation pass ONLY (eroded geometry never touches corners), but generation-time insetting is the robust default. All pieces exist (`CheckerboardSaddleDetector`+subpixel, `FiducialDetector` BCH, `CalibrationTarget`+generate) → lands as `CharucoTarget : CalibrationTarget`. CAVEAT (ties to low-res): each cell must budget px for BOTH a decodable code AND the corner margin → at 640×480/6×4 that's tight; fewer/larger cells or higher-res capture.
      - [ ] **FIRST EXPERIMENT (naïve fill → measure, decide inset-vs-fill).** Before designing the inset, test the WORST case: replace the black squares edge-to-edge with markers and measure. Reuses the whole existing harness — generate marker-filled board → `renderView` → `CheckerboardSaddleDetector`+`refineCheckerboardCornersSubPix` → `checkerGroundTruthRMS` gives corner degradation directly vs the plain-checkerboard baseline on record (0.007/0.015/0.016 frontal/keystone/noisy); run `FiducialDetector` on the same frames for marker recovery+decode rate. TWO sharp predictions to confirm: (1) touching quads are an 8- vs 4-connectivity question — 4-connected labeling keeps diagonally-touching black cells separate for free; failure mode is AA/blur bridging at the X-junction (does the quad heuristics/threshold survive, or need erosion/inset?). (2) Saddle contamination is a **border-thickness vs ring-radius threshold**: internal code edges start ~(border thickness) from the corner, so border ≥ ring radius (~5px) → clean 4-quadrant saddle, border < ring radius → biased/failed corner. Both knobs tunable (thicker BCH border, or smaller saddle ring — op stays valid at small radius). If naïve fill barely degrades corners → SKIP the inset, keep the simpler edge-to-edge target (markers literally ARE the black squares).
  - [ ] Phase B planar intrinsic+extrinsic (native-vs-opencv intrinsics) — targets + detector backends now both in place; comparison harness next.
    - [x] **native intrinsic calibration works + tilt-requirement locked** (`test-cv-intrinsic-calibration`). Verified the unexercised native `cv::IntrinsicCalibrator` (Bouguet/Matlab reimpl, no OpenCV): synthetic planar grid projected through a known GT pinhole in diverse tilted poses → recovers fx/fy/cx/cy **exactly** on clean data, within ~4px under 0.3px noise. Degeneracy test proves fronto-parallel-only views can't constrain focal length (fx error diverges ~4e18 vs 0.0 tilted) — tilt out-of-plane is a REQUIREMENT. Input contract nailed: `DynMatrix(cols,rows)` + `operator()(row,col)`; impoints `(bSize, 2*views)` with `(2v[+1], pt)`, worldpoints `(bSize,3)` with `(coord, pt)`. Distortion recovery verified too: GT radial+tangential (k1=-0.18,k2=0.05,p1,p2) recovered **exactly** on clean data through tilted views of a large frame-spanning board (the forward model — cdist + Brown tangential + normalized skew — reverse-engineered from `project_points2` to match the estimator).
    - [x] **native vs OpenCV parity confirmed** (`cv.intrinsic.native_vs_opencv`). Identical correspondences (shared `makeViews`, +distortion +0.2px noise) → both `IntrinsicCalibrator` and `cv::calibrateCamera` (via `OpenCVCamCalib`); agree to ~0.02px on fx/fy, track each other on every param (even the weakly-observable k2 both miss identically), both recover well-conditioned GT. Redesign core thesis validated. Added correspondence-based `OpenCVCamCalib::addPoints(objMM,imgPx)`+`setImageSize`, dropped `CALIB_FIX_ASPECT_RATIO`, fixed latent `getDistortion()` shape bug (a casualty of the matrix cols-first-ctor vs (row,col)-accessor asymmetry).
    - [x] **end-to-end render→detect→calibrate** (`markers.intrinsic.endtoend_checkerboard`). Renders GT-camera views of a tilted board (H=K[r1 r2 t], auto-centred, inverse-warp+3×SS+noise), detects with real `CheckerboardTarget` (ChESS saddle+subpixel), orders corners canonically by objectPos, calibrates with `IntrinsicCalibrator`. 10/10 boards detected; recovers fx=600.3/fy=600.2 (GT 600), cx=320.0, cy=239.7 — sub-pixel through genuine detection noise (small spurious k1≈0.05 absorbs some). No lens distortion in the render yet (needs inverse-distortion warp) — later increment.
      - [x] **distortion-render gap closed** (`markers.intrinsic.endtoend_distortion`). Render now applies an inverse-distortion warp (`undistortNorm` fixed-point) so the detector sees genuinely curved boards; identity when kc=0 (clean test unchanged). GT barrel k1=-0.15,k2=0.03 → detection survives (10/10), recovers fx=599.9/cx=320.5/cy=239.7 and **k1=-0.155 (within 0.005)** with a big enough board (13×9, corners reach larger radius). k2 (r⁴) stays weakly observable — a complete-board detector can't reach the extreme corners where r⁴ dominates — so not asserted (no longer blows up, but inaccurate); same limit as the perfect-points parity test. NEXT: Phase C multi-cam extrinsics.
  - [ ] Phase C multi-cam one-click extrinsic (3D, fixed intrinsics). Then geom can be deleted.
- [ ] **depth/point-cloud batch** (point-cloud-creator, point-cloud-define-world-frame,
      kinect ×5, rgbd-mapping) — gated on decoupling `PointCloudCreator`/
      `DepthCameraPointCloudGrabber` from the deleted `PointCloudObjectBase` (fill
      `geom2::PointCloud`), geom2 `RayCastOctree` fill-from-`PointCloud`, + cross-cam
      color mapping.
  - [x] **color→depth mapping capability + test bed (LANDED).** `PointCloud::mapColorFromCamera`
    (projects each world point into a *different* color camera + samples — the geom2 replacement
    for `PointCloudCreator::mapImage`). New `icl-stereo-rgbd-simulator` app: one scene through two
    cameras with a horizontal baseline (depth cam0 + offset color cam1), headless `BVHSceneCapture`,
    reconstructs the coloured cloud via `unprojectDepth` + `mapColorFromCamera`. Headless tests
    (test-geom2-pointcloud-mapcolor): zero-baseline == aligned; baseline leaves points unmapped.
  - [x] **point-cloud-creator** ported onto `unprojectDepth` + `mapColorFromCamera` (geom2/apps;
    optional RGBD-out with camera metadata for the viewer/pipe); kinect 11-bit raw→mm folded in.
  - [x] **kinect-pointcloud + rgbd-mapping RETIRED** — both were `(kinect input)+RGB-D mapping`,
    now covered by point-cloud-creator (`-i kinectd`). Lossless (raw decode preserved).
  - kinect-normals, kinect-recorder, fix-kinect-calibration: **STAY** — CV core only (no Scene
    layer), don't block geom deletion.
  - [x] **3 segmenters FUSED** → one geom2 `kinect-segmentation` demo (mode: Segmentation3D
    surfaces/blobs, EuclideanBlobSegmenter). CV cores stay; they bind to `geom2::PointCloud` via
    `selectXYZH`/`selectRGBA32f` (already returned a colour image — no decoupling needed). All 3
    legacy demos retired; `ConfigurableDepthImageSegmenter` the class stays in geom CV.
  - [x] **point-cloud-define-world-frame** ported (geom2) — `RayCastOctree::fill(PointCloud)` added
    (+ test), PointCloudSource + CoordinateFrameNode; CV/PCA unchanged.
  - [x] dead unbuilt legacy point-cloud sources pruned (pipe/viewer/simple/tests).
- [ ] **DECISION**: animated-grid (custom GLSL shader hook vs simplify vs delete)
- [x] **plot-widget-3D** — geom2 `PlotWidget3D` + `Plot3D`/`PlotHandle3D` reimplemented on
      Scene2 (scaled root GroupNode + coordinate box + tics/labels; scatter/surf/linestrip/
      label API). Demo ported. Build-checked; real-display pass owed (box framing, label
      distortion, scatter colour-range). Legacy geom PlotWidget3D stays until geom deletion.
- [x] **point-cloud-primitive-filter** ported — `nodeFromPrimitive3D` converter (Primitive3D→node,
      +tests) shows the primitive; filtering via existing `filterBox`/`filterSphere`. Dropped RSB +
      monolithic `Primitive3DFilter` (cylinder/oriented filtering stays out per earlier decision).
- [ ] Prune dead legacy point-cloud sources (geom/apps: pipe/viewer/simple/tests unbuilt)
- [ ] **Delete geom** (final)
- [ ] **Rename geom2 → geom** (final)

## geom2 capabilities to add
- [ ] **Node→Scene2 back-pointer** (planned, next session) — self-locking high-level
      mutators + auto-invalidation; plan: memory `project_node_scene_backpointer`
- [ ] checkerboard-lab: shadow-casting disturber objects (realistic test images)
- [x] Offscreen GL-framebuffer render→Img (`Scene2::renderToImage` + `SceneCapture`)
- [x] Depth+color buffer → RGBD source sim (`-i scene`, `@format=rgbd`)
- [x] Scene depth → point-cloud source sim (`-i scene` → `PointCloudSource`/`unprojectDepth`)

## physics2 follow-ups
- [ ] Root-cause threaded cloth NaN
- [ ] Restore driving cloth station
- [ ] Driving M5: HUD, reset-on-flip
- [ ] water-rocket: fix or delete
- [ ] maze: real-display framing/feel

## verification debt
- [ ] Real-display visual pass (no GL here)

## Filament renderer follow-ups (S101 — see filament-plan.md / next.md)
- [ ] **A1 readback perf** — benchmark GPU→CPU→GPU cost; if it bites (discrete GPU), do A2 zero-copy IOSurface Metal↔GL.
- [ ] **Per-node backend resources** — invert map<Node*,Cache> → per-node keyed-by-domain slot (deferred release + Scene domain registry); do at GL/Filament convergence.
- [ ] **Soft-shadow softness** — map `LightNode::softShadowRadius` → Filament PCSS.
- [x] **Points / point-clouds — DONE (S103).** `point_billboard.mat` (unlit billboard quads, per-vertex COLOR + CUSTOM0 corner, camera-facing constant-pixel size via the vertex shader). GeometryNode `PrimVertex` (versioned) + `PointCloudNode` (per-frame). Solved the COLOR-attribute bind. Verified `icl-filament-points-test` → `builddir/calib/points-{mesh,cloud}.png`, 1094/1094. Follow-ups: `renderOnTop` depth-disable for point/line overlays; reuse dynamic-cloud VB/IB/MI instead of per-frame recreate.
- [x] **Screen-space refraction — DONE (S103).** `glass_pbr.mat` (lit + `refractionMode:screenspace`/`refractionType:solid`), separate material chosen at buildSolid from `Material::isTransmissive()`; syncGeometry force-rebuilds on glass-ness flip (setMaterial doesn't bump version). Beer-Lambert absorption from attenuationColor/distance. Verified `icl-filament-glass-test` → `builddir/calib/glass.png`, 1094/1094. Deferred: thin-wall (`refractionType:thin` + microThickness).
- [ ] **Linux/Vulkan + Intel** — prebuilt Filament is arm64-mac only.
- [ ] **P5 converge → delete GLRenderBackend** (once parity soaked onscreen).
- [ ] **Prop save-dialog filter says `*.xml`, we write YAML now** — qt config-UI file filter.

## infrastructure / correctness follow-ups
- [ ] **Quick2 buffer-reuse based on image channels** — `QuickContext::getBuffer`
  decides a pooled buffer is free via `Image::isExclusivelyOwned()`, which only
  checks the ImgBase HANDLE (`use_count==1`), NOT the per-channel pixel data. A
  consumer that `shallowCopy()`s a pooled buffer and drops the handle can get it
  recycled while its pixels are still referenced → aliasing (black/white/torn
  frames). Re-base reuse on channel-data ownership (`isIndependent()` / per-channel
  SmartPtr use_counts) or the raw-byte pool in `project_memorypool.md`. TODO marker
  at the reuse check in `QuickContext.cpp`.
