# Plan — `icl-cam-calib-intrinsic` (easy intrinsic calibration app)

Status: **planned, to implement next session.** Companion future app: `icl-cam-calib-extrinsic`
(Phase C extrinsics — separate plan). This app is Phase B of `camera-calibration-redesign.md`
(read that too — it records the depth-drift RCA and the pluggable-`CalibrationTarget` decision).

## Vision

A dead-simple intrinsic-calibration tool:

1. **Pick a calibration target** (type + metric size) from a small menu.
2. **Wave it in front of the camera.** The app **auto-captures** frames only where coverage is
   still poor, and shows a live **image-space heatmap** of where corners have (and have not) been
   observed — so the user can see which regions/angles still need filling (especially the image
   edges/corners where radial `k2` lives).
3. **Calibrate**, then see the result (undistortion preview + reprojection error).
4. **`--sim-input W H`**: instead of a real camera, embed a rotatable 3D scene (à la
   `calib-target-detection-lab`) rendering the *selected* target; the known virtual camera gives
   **ground-truth**, so the app can also report the *true* intrinsic error (recovered vs. known).

## Locked design decisions (this session)

- **Auto-capture:** pose+image-region **bins** now (image position, scale/distance, tilt
  direction; plus which image cells the corners fill), gated by a **pose-stability** check
  (reuse the pose-variance idea from `PlanarCalibrationTools.cpp:47-59`). Leave a hook for an
  uncertainty-gain criterion later.
- **Coverage viz:** image-space **corner heatmap** (accumulated occupancy over all kept frames),
  pseudo-color LUT adapted from `lens-undistortion-calibration.cpp:157-224`.
- **Sim target:** the embedded scene renders **whichever target the user selected** (one shared
  detect→calibrate path for sim and real).
- **V1 scope:** **sim-first vertical slice** — full flow driven by `--sim-input`, real
  `ImageSource` wired but secondary. Verifiable **headlessly** in-sandbox via the offscreen path.

## Solver choice

Use **`cv::IntrinsicCalibrator`** (Zhang/Bouguet planar; `icl/cv/IntrinsicCalibrator.{h,cpp}`),
NOT `MarkerGridBasedUndistortionOptimizer` (marker-grid-specific, deprecated-adjacent). Rationale:
it takes generic image↔world correspondences from ANY `CalibrationTarget`, has a **partial-board
validity mask** overload (needed for coded/ChArUco boards that overrun the frame), and yields the
full pinhole `K` + **k1,k2,p1,p2,k3** as a `filter::ImageUndistortion` (`Result`) that already
serializes to XML.

Gotchas to handle (from the API map):
- **Units/scale:** the static `optimize()` fabricates world coords in *board-index* units → focal
  only correct up to scale. We MUST call the explicit `calibrate(impoints, worldpoints[, mask])`
  overload and pass **metric world points (mm)** from `target->modelPoints()`.
- **Packing:** `impoints` = `2·nViews × bSize` (row `2k`=xs, `2k+1`=ys), `worldpoints` = `3 × bSize`
  (shared, Z=0). `bSize` = full-board corner count.
- **Partial boards:** use the `validMask` (`nViews × bSize`, `!=0` = corner seen in that view).
  Each detected `CalibrationCorrespondence.objectPos` must map to its **full-board corner index**
  (column). Get the canonical order from `modelPoints()`; match `objectPos`→index by nearest mm
  coord (exact in sim). Plan a small `objectPointIndex(objectPos)` helper on the target side or a
  KD/hash match in the app.
- **No built-in error / no geom::Camera:** compute reprojection RMS ourselves; build a
  `geom::Camera` from `getFocalLengthX/Y`, `getPrincipalX/Y`, `getSkew` if we want the 3D overlay.
- **Min views:** needs > 3.

## Reusable building blocks (verified)

| Piece | Where | Use |
|---|---|---|
| `geom2::OffscreenView` | `icl/geom2/OffscreenView.h` | the whole sim camera: GL/Cycles render, GUI/worker threading, **forward lens distortion `distortion.k1/k2`** baked into the frame, `Prop(&view)` for controls, `view.next()→Frame{Img8u,isNew}` |
| `geom2::Scene2` + `Camera::lookAt` + `LightNode` | `icl/geom2/` | sim scene/camera/light scaffold (lab `init:268-271`) |
| `geom2::CheckerboardNode` | `icl/geom2/CheckerboardNode.h` | self-visualizing board **with `innerCorners()` = GT 3D (z=0)**; `setCells()` live |
| textured `MeshNode` boards | lab `buildMarkerBoard/buildCodedBoard:118-174` | flat quad textured via `target->generate(size)` for marker-grid / coded targets |
| `markers::CalibrationTarget` (+ `CheckerboardTarget`, `CodedCheckerboardTarget`, `MarkerGridTarget`) | `icl/markers/` | `detect(Img8u)→vector<CalibrationCorrespondence{objectPos mm z=0, imagePos px}>`, `modelPoints()` GT, `generate()` texture, `name()` |
| `cv::IntrinsicCalibrator` | `icl/cv/IntrinsicCalibrator.h` | the solve (see above) |
| `filter::ImageUndistortion` | `icl/filter/` | undistortion-map preview + result XML (the `Result` IS one) |
| pose-stability idea | `PlanarCalibrationTools.cpp:47-59` | auto-capture "is the board holding still" gate |
| displacement-map viz template | `lens-undistortion-calibration.cpp:157-224` | LUT/overlay pattern to adapt for the coverage heatmap |
| generic `ImageSource` | `-i` prog-arg | real-camera input (free; `create`/`file`/`dc`/`v4l`/`ws`) |

## New components to build (keep the CORE headless & typed)

Avoid every stringly-typed pitfall the survey flagged: no `pa()` reaching inside helpers, no
`gui["..."]` as the data path, enums not combo-strings. Model state lives in typed structs; bind to
GUI only at the edge. Core must run with **no GUI** (for the sim self-test + future gtest).

1. **`TargetSpec`** (typed): enum `Type{Checkerboard, Coded, MarkerGrid}` + metric params
   (cols/rows/squareMM, or grid sizes). Factory `makeTarget(TargetSpec) → unique_ptr<CalibrationTarget>`
   and `makeSceneNode(TargetSpec) → NodePtr` (checkerboard→`CheckerboardNode`, others→textured quad).
   (This is the name→target registry `CalibrationTarget.h:31` foreshadows — build a minimal one.)

2. **`CoverageMap`** (headless): accumulates, over kept frames, an image-space occupancy grid
   (e.g. 16×12 cells) of detected corner positions, plus **pose bins** (image-centroid quadrant ×
   scale/distance band × tilt-direction octant). Query: `isUnderRepresented(currentCorners, pose)`,
   `占 heatmap image` for display, and per-bin fill counts for gauges (deferred). Under-represented =
   corners fall in cells/bins below a fill threshold, or extend coverage to an empty image region.

3. **`AutoCaptureController`** (headless): given the latest detection + a stability gate (corner
   set stable across K frames / low reprojection jitter), decides *capture / skip*, and on capture
   pushes the view into the session. Debounce so one dwell = one capture. Hook point left for an
   "information-gain" criterion.

4. **`IntrinsicSession`** (headless): holds `TargetSpec`, the accumulated views
   (`vector<vector<CalibrationCorrespondence>>` + per-view valid indices), `CoverageMap`. Methods:
   `addView(correspondences)`, `viewCount()`, `calibrate() → Result` (packs matrices + mask, calls
   `IntrinsicCalibrator`, computes per-view + overall reprojection RMS), `save(file)`,
   `undistortionPreview(frame)`.

5. **Sim harness + GT** (headless): given the OffscreenView's known capture `Camera` and injected
   `k1/k2`, project `modelPoints()`/`innerCorners()` → **ground-truth 2D**; expose
   `groundTruthIntrinsics()` (fx,fy from fov+res; cx,cy=center; k1,k2 from the view). Error report:
   detected-vs-GT corner error, calibrated-reprojection error, and **recovered-vs-true intrinsics**.

6. **GUI app** (`icl-cam-calib-intrinsic.cpp`): thin shell wiring the above to widgets.

## GUI layout (interactive)

`HSplit`:
- LEFT (sim only): `Canvas3D "scene"` — the rotatable target scene (`view.callback()` + mouse
  handler), à la the lab. In real mode this pane is hidden.
- CENTER: `Canvas "view"` — the (sim or real) camera frame + detection overlay + **coverage
  heatmap toggle** overlaid/side-by-side.
- RIGHT: control `VBox` (size-capped `Split`, per the layout convention):
  target type combo + size sliders; auto-capture on/off + sensitivity; "capture now" (manual);
  "calibrate"; "save"; `Prop(&view)` (sim) for backend/distortion; live labels: `views N`,
  `coverage %`, `reproj RMS`, and (sim) `true-intrinsic error`; "undistort preview" toggle.

## CLI

```
icl-cam-calib-intrinsic -i <input>                 # real camera (ImageSource)
icl-cam-calib-intrinsic --sim-input W H            # embedded rotatable sim scene at WxH
   [-t checkerboard|coded|marker-grid] [--cells CxR] [--square-mm S]
   [-o out.xml] [--sim-selftest]                   # headless: script poses, calibrate, print err, exit
```
`--sim-selftest` is the in-sandbox verification path (no GUI; drives OffscreenView/GLSceneCapture
like `plot-frame-capture`), and the seed of a future gtest.

## Headless verification (critical — GUI can't run in-sandbox)

Because `QOpenGLWidget` crashes here, all verification goes through `--sim-selftest`:
render the selected board through a KNOWN camera at ~15–20 scripted poses (spanning scale + tilt +
image position, i.e. deliberately exercising the coverage bins), detect, accumulate, calibrate,
and assert recovered fx,fy,cx,cy,k1,k2 are within tolerance of the injected truth. This reuses the
proven `reference_headless_gl_capture` mechanism. Promote to `tests/test-markers-*` once the core
lands in a lib.

## Module placement / build

App in **`icl/markers/apps/`** (like `calib-target-detection-lab` and the other calib apps),
linking `geom2` (sim) + `cv` (IntrinsicCalibrator) + `markers` (targets) + `qt`. Core files as
app-local `icl-cam-calib-intrinsic-*.{h,cpp}` for V1 (compiled into the app + the selftest); a
follow-up promotes `CoverageMap`/`IntrinsicSession` into `markers` as installed classes with a
gtest. Wire a `plot-orient-tuner`-style `executable(...)` in `icl/markers/targets/meson.build`.

## Phased steps (next session)

1. `TargetSpec` + factories (`makeTarget`, `makeSceneNode`); confirm each target's `modelPoints()`
   order and add `objectPointIndex()` matching.
2. `IntrinsicSession.addView/calibrate` over `IntrinsicCalibrator` (+ mask + metric worldpoints +
   RMS). Prove it on a trivially-synthesized full-checkerboard set.
3. Sim harness + GT projection + `--sim-selftest`; get recovered≈truth headlessly. **Milestone.**
4. `CoverageMap` (heatmap + pose bins) and `AutoCaptureController`; script the selftest poses to
   show coverage filling and auto-capture firing.
5. GUI shell: sim scene pane, camera+overlay+heatmap, controls, live labels, save.
6. Wire real `ImageSource` input; polish.

## Open questions / risks

- **Coded-board corner→index matching** under partial views + distortion: nearest-mm match should
  be robust, but verify no aliasing when the board is far/tilted.
- **IntrinsicCalibrator masked path** skips `recompute_extrinsic` refine (`cpp:1448`); check
  accuracy on heavily-partial coded views vs. full checkerboards.
- **Focal scale** correctness hinges on metric `worldpoints` — assert fx recovers the true mm-based
  focal in the selftest.
- Coverage-bin thresholds / auto-capture sensitivity need tuning — expose as sliders.
- Two legacy output formats exist (camera-XML vs udist-XML); pick **one** canonical intrinsic file
  (the `Result`/`ImageUndistortion` XML) for this app.
