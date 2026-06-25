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
- [ ] camera-calibration + camera-calibration-planar (markers) — heavy pure port
      (GridIndicatorObject ~90 LOC → GroupNode; custom mouse handlers; 2D overlay)
- [ ] **DECISION**: depth/point-cloud batch (point-cloud-creator,
      point-cloud-define-world-frame, kinect ×5, rgbd-mapping) — all gated on
      decoupling `PointCloudCreator`/`DepthCameraPointCloudGrabber` from the
      deleted `PointCloudObjectBase` render layer (fill `geom2::PointCloud`).
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
