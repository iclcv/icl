# Backlog

Terse running TODO index (1–5 words each) so nothing gets forgotten. Detail lives in the
linked plans; this is just the index. Check off / prune as done.

## geom → geom2 retirement (active arc)
Plan + per-app detail: `geom-retirement-worklist.md`. Keep demos/apps split (CLAUDE.md).
- [x] Port scene-object (demo), simplex-3D (demo)
- [ ] Port remaining keepers geom→geom2
- [ ] Prune already-moved demos
- [x] Delete `ray-cast-octree` (geom2 has it)
- [x] point-cloud viewer + pipe (geom2; share `PointCloudSource`; legacy viewer/simple/pipe retired)
- [x] show-scene → scene-viewer app (geom2/apps; fuse w/ cycles later)
- [ ] Fuse kinect segmenters
- [ ] marker-detection: review separately
- [ ] 1-view ↔ n-view/source generalization
- [ ] `SuperquadricNode` (new node type)
- [ ] animated-grid: shader hook or simplify
- [ ] plot-widget-3D: geom2 reimpl or delete
- [ ] camera-calibration 2D overlay
- [x] geom2 `apps/` dir + wiring
- [ ] markers dep: add geom2
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
