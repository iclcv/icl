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
- [ ] **SEPARATE MULTI-SESSION REWORK**: camera-calibration + camera-calibration-planar
      (markers, ~2500 LOC incl. shared `CameraCalibrationUtils`). NOT a mechanical
      `fromSceneObject` swap — blocked on three things: (a) `GridIndicatorObject`
      uses `addTextTexture` (marker-ID labels) which `fromSceneObject` skips;
      (b) `CameraCalibrationUtils::calibrate/change_plane` take `geom::Scene&` and
      mutate SceneObject transforms/plane at runtime (a static converted snapshot
      would freeze); (c) planar's `AdjustGridMouseHandler` edits grid geometry live.
      Needs a real geom2 rework (node-handle tracking + a geom2 text-on-grid path).
      Keeps geom alive until done (alongside the depth batch). See worklist.
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
