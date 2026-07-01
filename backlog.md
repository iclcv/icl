# Backlog

Terse running TODO index (1–5 words each) so nothing gets forgotten. Detail lives in the
linked plans; this is just the index. Check off / prune as done.

## geom → geom2 retirement (active arc)
Plan + per-app detail: `geom-retirement-worklist.md`. Keep demos/apps split (CLAUDE.md).
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
    - [x] **native vs OpenCV parity confirmed** (`cv.intrinsic.native_vs_opencv`). Identical correspondences (shared `makeViews`, +distortion +0.2px noise) → both `IntrinsicCalibrator` and `cv::calibrateCamera` (via `OpenCVCamCalib`); agree to ~0.02px on fx/fy, track each other on every param (even the weakly-observable k2 both miss identically), both recover well-conditioned GT. Redesign core thesis validated. Added correspondence-based `OpenCVCamCalib::addPoints(objMM,imgPx)`+`setImageSize`, dropped `CALIB_FIX_ASPECT_RATIO`, fixed latent `getDistortion()` shape bug (a casualty of the matrix cols-first-ctor vs (row,col)-accessor asymmetry). NEXT: end-to-end render+detect→calibrate.
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
