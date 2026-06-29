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
  - [ ] **false-positive suppression** (extreme views add spurious border seeds → inflate dims; build a heuristic on the edge confidence).
  - [ ] **RESEARCH: better checkerboard detectors** — detection bounds calibration accuracy; survey SOTA (findChessboardCornersSB/ROCHADE, libcbdetect/Geiger growth, DL corner detectors) vs our ChESS+growth; wrap the best as a `CornerSeed` provider if it beats native.
  - [ ] Phase B planar intrinsic+extrinsic (native-vs-opencv intrinsics) — `MarkerGridTarget` + comparison harness next.
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
