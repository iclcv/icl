# Findings — Tier A (detectability + corner accuracy)

640×480, distance-scaled so `cellpx` = on-screen checker-cell size, 6 poses
(fronto + tilts), additive noise ±4, `k1 ∈ {0, −0.20}`. Metric: **meanRec** = mean
fraction of the (C−1)(R−1) inner corners recovered over the poses; **medRMS** =
median corner-localisation error vs analytic ground truth (px). Renderer verified
against GT to 0.056 px.

`markerPx` = the on-screen marker size: `0.62·cellpx` for white-cell (marker is
shrunk inside the white square), `cellpx` for black-cell (marker fills the cell),
n/a for plain.

## Headline numbers (9×7, k1=0)

| target | cellpx→ | 40 | 30 | 24 | 20 | 16 | 13 |
|---|---|---|---|---|---|---|---|
| **coded-white** meanRec | | 99% | 78% | 67% | 60% | 0% | 0% |
| &nbsp;&nbsp;markerPx | | 24.8 | 18.6 | 14.9 | 12.4 | 9.9 | 8.1 |
| &nbsp;&nbsp;medRMS px | | 0.03 | 0.10 | *2.3* | 0.29 | – | – |
| **coded-black** meanRec | | 92% | 91% | 91% | 85% | 70% | 28% |
| &nbsp;&nbsp;markerPx | | 40 | 30 | 24 | 20 | 16 | 13 |
| &nbsp;&nbsp;medRMS px | | 0.27 | 0.26 | 0.23 | *1.6* | *1.4* | 0.32 |
| **plain-checker** meanRec | | 100% | 100% | 100% | 100% | 100% | 100% |
| &nbsp;&nbsp;medRMS px | | 0.03 | 0.09 | 0.01 | 0.22 | 0.26 | 0.33 |

(*italic* medRMS = gross-mislabel outliers, see below.)

## Answers to the central questions

**1. How small can the markers get?**
- **coded-white**: robust (≥~78 %) at `markerPx ≥ 18–19` (cellpx≥30); marginal at
  `markerPx ≈ 15` (cellpx 24); **dead below `markerPx ≈ 12`** (cellpx≤16 → 0 %).
- **coded-black**: robust (≥~82 %) at `markerPx ≥ 24` (cellpx≥24), usable to
  `markerPx ≈ 16–20`; **dead below `markerPx ≈ 13`**. Because the marker fills the
  whole cell, black-cell stays detectable at ~1.5× smaller *cell* sizes than
  white-cell — but its markers must be that much bigger in absolute px.
- **plain-checker**: no markers → recovers 100 % down to `cellpx ≈ 13`, dies at 10.

**2. Homography / corner quality — markers in black cells DO weaken the saddles.**
Median corner error at healthy sizes:
- plain-checker ≈ **0.03–0.10 px**, coded-white ≈ **0.03–0.10 px** (markers sit in
  the white squares, away from the checker X-junctions → saddles are clean),
- coded-black ≈ **0.23–0.27 px** — **~3–8× worse**. The marker code lives *inside*
  the black square, so the black/white X-junctions have code pixels right next to
  them and the ChESS saddle response is degraded. Quantitatively confirms the
  concern raised up front.

**3. Gross mislabels at marginal marker sizes (important for calibration).**
At the size where marker decoding starts to fail, the *coded* pipelines
occasionally emit corners with the **wrong absolute (col,row) label** → median RMS
jumps to 1.4–2.9 px (the italic cells) instead of just recovering fewer corners.
Those are calibration-poisoning outliers, not benign misses. Plain-checker never
does this (its RMS stays sub-pixel right down to its detection floor). ⇒ for the
coded boards, stay comfortably above the marginal size, or add a homography-based
outlier reject before calibrating.

**4. Distortion (k1=−0.20).** Negligible effect on detection rate or corner RMS —
detection is local and the corners still localise; the distortion is what
*calibration* will estimate, tested in Tier B.

**5. Board cell count.** Minor: 13×9 loses a few % recovery at small sizes vs 9×7
(bigger board → edges clip under tilt); 7×5 has a slightly lower mean-recovery
floor (each missed corner is a bigger fraction). Not a primary driver.

## Practical takeaways

- **Accuracy-first, full board visible → plain checkerboard.** Cleanest saddles,
  smallest usable cells, no gross-mislabel failure mode. (No IDs → no partial-board
  / occlusion tolerance.)
- **Partial-board / robust labeling needed → coded-white** at `markerPx ≥ ~19`
  (cellpx≥30). Saddle quality ≈ plain, plus absolute corner IDs.
- **coded-black** has noisier saddles and needs bigger absolute markers.
  **NOTE — Tier B overturns the naive conclusion here:** on end-to-end calibration
  coded-black actually BEATS coded-white, because its full-cell markers avoid the
  gross-mislabel regime (the saddle noise averages out; mislabels don't). See the
  Tier B section — do not read this Tier-A saddle-noise number as "prefer white".

## Next (deferred)

- **Tier B**: feed multi-view correspondences into `cv::IntrinsicCalibrator`;
  report recovered `f/k1/k2` error per target × size (does the ~0.25 px black-cell
  saddle penalty actually move the intrinsics, and by how much?).
- Image-size sweep (1280×960); Cycles realism re-render of the winners; marker-fill
  sweep for coded-white; quad-assisted plain-checkerboard reconstruction.

---

# Findings — Tier B (end-to-end intrinsic calibration)

Fixed GT camera f=650, cx=320, cy=240, k1=−0.15, k2=0.03; 13×9 board; 14 diverse
tilted+offset poses (board sweeps the FOV, corners reach edges); render→detect→
`cv::IntrinsicCalibrator` (partial-board masked). Recovered vs GT:

| target | fx | cx | k1 | k2 | f%err | k1 err |
|---|---|---|---|---|---|---|
| **plain-checker** | 650.1 | 320.2 | −0.1486 | 0.009 | 0.0% | 0.0014 |
| **coded-black**   | 650.7 | 323.2 | −0.1493 | 0.057 | 0.1% | 0.0007 |
| **coded-white**   | 639.5 | 352.1 | −0.011  | −1.07 | −1.6% | 0.14 |

## The key insight — mislabels, not saddle noise, dominate calibration

Tier A said black-cell saddles are ~3–8× noisier than white/plain. Yet **black
calibrates almost perfectly and white is badly poisoned**. Why:

- **Saddle noise averages out.** With hundreds of corners over 14 views, a
  zero-mean ~0.25 px saddle jitter is absorbed by the bundle adjustment — hence
  coded-black recovers k1 to 0.0007 and f to 0.1 %.
- **Gross corner MISLABELS bias the fit** and do NOT average out. At this board
  scale the poses put coded-white's markers at markerPx ≈ 16–22 — right in the
  marginal zone Tier A flagged, where the coded pipeline occasionally assigns a
  corner the WRONG (col,row) label. A handful of such outliers drag cx by 32 px and
  make k1/k2 meaningless. Coded-black's markers fill the whole cell (markerPx =
  cellpx ≈ 26–35 here) → above the mislabel threshold → clean labels → clean
  calibration.

So the failure mode that actually matters for calibration is **gross mislabels**,
and **white-cell markers hit that regime at larger cell sizes than black-cell**
(their marker is only 0.62× the cell). This inverts the naive "black saddles are
worse ⇒ black calibrates worse".

## Practical guidance (updated)

- **Plain checkerboard** — best when the whole board is guaranteed in view: exact
  intrinsics, no labels to mislabel.
- **coded-black** — the better CODED choice for calibration at a given board/camera:
  full-cell markers stay out of the mislabel regime, and the saddle-noise penalty
  is harmless to the bundle. Needs `pp.filter=dilatation`.
- **coded-white** — only safe when its 0.62×cell markers are kept well above the
  marginal size (bigger physical cells / closer / higher-res, markerPx ≳ 22). Below
  that it silently poisons the calibration via mislabels — the most dangerous mode
  because detection still "succeeds" with plausible-looking corners.
- **Regardless of target: run a homography/RANSAC outlier reject on the
  correspondences before the bundle** — it would have caught the coded-white
  mislabels.

## Caveats / to confirm

- Single pose-seed; should be repeated over seeds. The coded-white poisoning is
  driven by a few outlier views — worth logging per-view reprojection to confirm
  it's mislabels (expected: 1–2 views with large residual).
- k2 is only weakly observable at this FOV/radius (even plain lands k2=0.009 vs GT
  0.03); f, cx/cy, k1 are the trustworthy numbers here.
