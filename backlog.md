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
  - **Test bed (user, no hardware): stereo virtual-camera RGBD simulator.** Render ONE
    geom2 scene through **two virtual cameras with a small horizontal baseline offset** —
    one is the depth cam, the other the color cam. Because they sit at different poses, a
    real **color→depth registration/mapping** is required to colour the cloud, so this
    exercises the cross-camera color-mapping path end-to-end in simulation. Extends the
    just-ported `icl-depth-camera-simulator` (already has `-cam`/`-ccam` + `relTM` rigid
    offset and dual `renderToImage`); add a baseline knob + emit depth(cam0)+color(cam1) so
    the consumer must map. This is what makes point-cloud-creator/rgbd-mapping testable here.
- [ ] **DECISION**: animated-grid (custom GLSL shader hook vs simplify vs delete)
- [ ] **DECISION**: plot-widget-3D (geom2 PlotWidget3D reimpl ~500-1000 LOC vs delete)
- [ ] point-cloud-primitive-filter — deferred (needs Primitive3D→node, P3)
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
