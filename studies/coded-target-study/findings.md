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
- **coded-black** buys nothing on accuracy (worse saddles) and needs bigger absolute
  markers; its only edge is packing a marker into every black cell. Prefer white-cell
  unless a specific reason forces black.

## Next (deferred)

- **Tier B**: feed multi-view correspondences into `cv::IntrinsicCalibrator`;
  report recovered `f/k1/k2` error per target × size (does the ~0.25 px black-cell
  saddle penalty actually move the intrinsics, and by how much?).
- Image-size sweep (1280×960); Cycles realism re-render of the winners; marker-fill
  sweep for coded-white; quad-assisted plain-checkerboard reconstruction.
