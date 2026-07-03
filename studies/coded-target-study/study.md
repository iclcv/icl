# Calibration-target detectability & homography-quality study

Goal: quantify how well the different calibration boards can be **detected** and how
accurately their **corners localise**, as a function of target type, cell count,
marker size, lens distortion and (later) image size — with an eye on eventual
**intrinsic calibration**. Central questions:

- How small (in image pixels) can the coded markers get and still be robustly detected?
- How good is the recoverable checker-corner lattice / homography per target type?
  (markers inside black cells may weaken the saddle points.)

## Method

**Analytic projective renderer** (`renderView`) — not GL/Cycles — so we have
**exact ground-truth corner pixels**: each board-mm corner is projected through a
pinhole `K` + pose `[R|t]` + radial distortion `(k1,k2)`. The image is rendered by
the inverse map (per output pixel: undistort → back-project ray → intersect the
board plane → bilinear-sample the `generate()` texture), then lightly degraded
(2× gauss blur + additive noise) for realism. Ground-truth alignment verified:
coded-white 9×7, tilted, k1=0 → **corner RMS vs GT = 0.056 px** (48/48).

Board-mm ↔ texture-px uses `generate()`'s own layout (`px=min(W/(C+2),H/(R+2))`,
centred, 1-cell quiet zone). `detect()`'s `objectPos (0,0)` is the first INNER
corner = board-mm `(sq,sq)` in the renderer frame (origin = checker outer corner).

**Apparent size axis:** `cellpx = f·sq/D`; we fix `f` per `cellpx` at distance
`D=1000mm`, so "cellpx" is the on-screen checker-cell size and `markerPx =
fill·cellpx` (white-cell) or `cellpx` (black-cell) is the marker size.

**Per config** we render `NP` poses (fronto + tilts) and report, over the poses:
- `cornerRecoveryPct` — fraction of poses where ≥90 % of inner corners were found
  with RMS < 3 px (i.e. a usable view);
- `medRMSpx` — median corner-localisation RMS vs ground truth.

Black-cell coded targets use `pp.filter = dilatation` (see the black-cell finding:
it's the only op that separates the diagonally-touching black cells); white-cell
and plain use `none`.

## Grid (Tier A)

- type: coded-white, coded-black, plain-checker
- boards: 7×5, 9×7, 13×9
- cellpx: 40, 30, 24, 20, 16, 13, 10
- k1: 0, −0.20
- 6 poses each

Outputs `results.csv`.

## Deferred / future tiers

- **Tier B — intrinsic calibration**: feed multi-view correspondences of the best
  configs into `cv::IntrinsicCalibrator`; report recovered `f/k1/k2` error.
- **Image size sweep** (640×480 → 1280×960 …).
- **Cycles realism tier**: re-render a few winning configs with the CPU raytracer
  (no GL needed) to confirm the analytic-renderer conclusions under real shading.
- **Quad-assisted plain checkerboard**: does dil/erode + quad detection improve the
  reconstruction of a marker-less checkerboard?

## Build / run

`studies/coded-target-study/run.sh` (from repo root) — compiles `study.cpp`
against the built dylibs and runs it.
