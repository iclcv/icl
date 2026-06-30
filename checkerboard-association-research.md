# Robust checkerboard grid-association — research + backend plan

Deep-research pass (Session 89) handed off from `next.md` Session 88. Goal: replace the
fragile greedy-growth bootstrap that falls into the **diagonal-lattice trap** under steep
oblique views, with a principled, foreshortening-robust **association** (topology recovery),
implemented as pluggable `cv::CheckerboardDetector` backends. Constraint: **ICL-native,
LGPL-v3-safe** (vendored code must be BSD/MIT/Apache/CC0; GPL excluded). OpenCV stays a
comparison baseline only.

## Problem localisation (verified)

The diagonal trap is **entirely in the downstream association stage**, not the corner
operator. ChESS (Bennett & Lasenby, arXiv:1301.5491) is a *localization + strength
(+orientation)* operator only; the paper explicitly leaves "which responses are true
features … to application-specific constraints." So the corners + 2nd-harmonic orientation
cue we already produce are fine — the fix must target how they're linked into the lattice.

## Method survey (peer-reviewed / authoritative, license-tagged)

### Family 1 — local energy / greedy growth (same family as ICL today → shares the trap)
- **Geiger et al. ICRA 2012 / libcbdetect** — global energy `E = E_corners + E_struct`, but
  optimised by *local greedy 2×2-seed border expansion*. `E_struct` is a purely-local
  collinearity (midpoint/second-difference) term; the authors claim fisheye tolerance **from
  that locality**. Robust to radial distortion, but the nearest-neighbour axis bootstrap is
  the same mechanism that produces our diagonal trap under foreshortening.
  *License:* paper free; the canonical C++ port **ftdlyc/libcbdetect is GPL-3.0 → NOT
  vendorable.* `lambdaloop/checkerboard` is a BSD-2 **Python** reimpl (readable reference).
- **OpenCV `findChessboardCornersSB`** (Duda & Frese, BMVC 2018) — sector/symmetry detector
  (localized Radon via box filters), sub-pixel ~1/100 px, native per-corner orientation. But
  association is **still greedy growth** (3×3 seed, k-d-tree neighbour add, cross-ratio
  validation; shipped code uses 1D-homography extrapolation for prediction). Same family.
  *License:* OpenCV calib3d Apache-2.0. **Refuted** by verification: the BMVC detector is *not*
  a drop-in portable SB reference. We already have an `opencv` backend — keep as baseline.
- **RCDN (2023)** and **Yoon et al. multi-task CNN (MMSP 2021)** — CNNs improve corner
  *localization* under distortion/extreme pose; RCDN's recovery is still "improved region
  growth" derived from Geiger → same association family. Heavyweight DL deps → poor LGPL-v3
  C++ fit. Note only.

### Family 2 — model-anchored growth (structural fix for the trap)
- **Hoffmann et al. VISAPP 2017** (Fraunhofer IIS) — grow by predicting the **four direct
  neighbours in INTEGER model-space coordinates** `(x±1,y),(x,y±1)`, mapped to the image
  through a **continuously re-fit homography H + Brown-Conrady distortion d** (`k1,k2,p1,p2`),
  re-solved by least-squares reprojection over all matched corners after every iteration.
  Because candidate positions are enumerated in integer model space and projected through
  H+d, **image-space nearest-neighbour distance cannot hijack axis assignment** — the exact
  mechanism behind the diagonal trap. Designed for strong radial distortion (90–120° FOV).
  *License:* paper; **clean-room reimplementable**. Maps onto homography/DLT + RANSAC + LM.
  ⚠ Open question (flagged in verification): the **initial 2×2/unit-grid seed** is the one
  remaining local decision — must be chosen robustly or the bootstrap itself can start on a
  diagonal.

### Family 3 — topology by construction (no local axis bootstrap at all)
- **ROCHADE** (Placht et al., ECCV 2014) — build a whole-image edge/centerline graph; inner
  corners are saddle points = graph nodes with ≥3 neighbours; accept only when the **adjacency
  structure equals a grid topology**. No BFS seed-growth → globally consistent by
  construction; explicitly designed for strong lens distortion + extreme pose. (Self-reported
  wide-baseline 91/103 vs legacy OpenCV 8 — caveat: legacy detector, author benchmark.)
  *License:* paper; clean-room reimplementable. Needs Delaunay/edge-graph + connected
  components. **ROCHADE's famous cone-kernel sub-pixel refine is separable** from its
  topology stage — we'd want only the topology stage (ChESS already does sub-pixel).
- **Deltille** (Ha et al., ICCV 2017, Facebook) — **Delaunay-triangulation** topology
  recovery (triangles→quads→indexing by polarity); aimed at triangular (deltille) grids but
  the checker path applies. *License:* **LGPL v2.1** — GPL-family; per-file "or later" upgrade
  clause must be checked before vendoring → treat as reference, prefer reimplementation.
- **"brutesac" / global homography-RANSAC** (Elucidation/ChessboardDetect, BSD; same idea as
  US patent 12094152 — *avoid copying the patent, the idea is generic prior art*) — fit H from
  a minimal candidate axis pair, **warp all corners to an ideal unit grid, score grid
  consistency** (RANSAC without randomness over hypotheses), keep best. Naturally global;
  **sidesteps the seed problem by trying many hypotheses** instead of committing to one
  bootstrap. Maps directly onto homography + RANSAC.

### Family 4 — coded targets (changes the physical pattern)
- **PuzzleBoard** (Stelldinger et al., GCPR 2024) — checkerboard + 18-bit de-Bruijn position
  code per 3×3 piece; corner detection → MST (consistent orientation) → cross-correlation
  decode + majority-vote error correction → **absolute per-corner grid index**, occlusion-
  robust, no axis bootstrap. *License:* reference code **CC0-1.0** (most permissive), but
  **Python**, and **requires changing the calibration target**. Most robust of all *if* we
  control the pattern → fits naturally as a new `CalibrationTarget`, not just a detector
  backend. (A stronger "every 3×3 unique under translation+rotation" framing was **refuted**
  1-2; rely on the verified MST + cross-correlation + majority-vote description.)
- **BabelCalib** (MIT) — global distortion-aware calibration framework, but **MATLAB-dominant
  (88%)** → reimplement-only.

## Caveats carried from verification
Most robustness numbers are **authors' own benchmarks**, not third-party. ROCHADE's
91/103-vs-8 uses *legacy* OpenCV (overstates the gap vs SB). No surveyed paper names or tests
a "diagonal-lattice trap" under the specific *foreshortening* regime ICL hits — the
trap-avoidance of the homography/model-anchored methods is a **mechanistically sound
inference**, not a measured result. ⇒ We must validate against our own lab frames
(`checkerboard-frame.png` + the save-frame tool), not trust the literature's numbers.

## Mapping to ICL building blocks (what we already have)
| Primitive | ICL location | Status |
|---|---|---|
| Linear assignment (Hungarian) | `cv/HungarianAlgorithm.h` | ✅ already used in `refineCheckerboardGrid` |
| Homography DLT + Hartley norm + SVD LS | `math/transform/Homography2D.h` (`GenericHomography2D`) | ✅ |
| Nonlinear LS (Levenberg-Marquardt) | `math/fit/LevenbergMarquardtFitter.h` | ✅ |
| SVD solve (LAPACK gelsd) | `math/la/DynMatrix.h::solve` | ✅ |
| Nearest-neighbour | `math/tree/KDTree.h` | ✅ |
| Convex hull | `math/transform/ConvexHull.h` | ✅ |
| Brown-Conrady distortion + warp maps | `filter/affine/ImageUndistortion.h` (MatlabModel5Params = k1,k2,k3,p1,p2) | ✅ |
| Saddle corners + orientation | `cv/CheckerboardSaddleDetector.h` (`CornerSeed{pos,score,orientation}`) | ✅ |
| Detector backend seam | `cv/CheckerboardDetector.h` (polymorphic, `Hints{boardCells,undistortion}`) | ✅ |
| Calibration-target seam | `markers/CalibrationTarget.h` + `CheckerboardTarget` | ✅ |
| **Delaunay triangulation** | — | ❌ **missing** (needed only for the ROCHADE/graph backend) |
| **RANSAC driver (generic)** | — | ⚠ no generic RANSAC utility found; homography-RANSAC would inline it |

ICL already has **everything** the model-anchored + global-homography backends need. Only the
graph/Delaunay backend requires a genuinely new reusable primitive (a Delaunay triangulation
utility, which belongs in ICLMath and is broadly reusable).

## Ranked recommendation (backends behind `cv::CheckerboardDetector`)

**1. (Primary) Global homography-RANSAC seed → model-anchored re-fit growth.**
A *unified* backend that fixes both the trap and Hoffmann's open seed problem:
  1. From ChESS corners, generate a small set of **axis-pair hypotheses** (use the orientation
     cue *only to propose* candidate axis directions — cheap pruning, not a hard reject, so its
     degradation under extreme shear can't break us).
  2. For each hypothesis fit `H` (`GenericHomography2D`), **warp all corners to the unit
     lattice, score grid consistency** (inlier count at integer nodes). Keep the best — a
     diagonal hypothesis explains far fewer corners as a consistent grid, so it loses. *This is
     what structurally defeats the diagonal trap.*
  3. From that global seed, grow/complete in **integer model space** through `H + Brown-Conrady`
     (`ImageUndistortion`), **re-fitting H+d** (`LevenbergMarquardtFitter`) each round
     (Hoffmann). Finish with the existing **Hungarian** assignment + border-trim.
  *Reuse:* `GenericHomography2D`, `HungarianAlgorithm`, `LevenbergMarquardtFitter`,
  `ImageUndistortion`, `KDTree` — **no new primitive needed.** This is largely *promoting the
  existing `refineCheckerboardGrid` (homography-DLT + Hungarian) from a local cleanup to the
  primary global associator*, seeded by RANSAC instead of by a grown grid. Lowest new-code
  risk, highest leverage.

**2. (Secondary, genuinely non-growth) ROCHADE-style graph-adjacency association.**
Whole-image saddle-adjacency graph → accept when adjacency ≈ grid. Feed it our existing ChESS
corners + orientation (skip ROCHADE's centerline front-end and its sub-pixel cone refine —
ChESS already covers both). *Requires adding a Delaunay triangulation utility to ICLMath*
(reusable framework asset). Best worst-case robustness; more new code. Good as the second
backend so the comparison harness can pit "global model-fit" vs "global graph" head-to-head.

**3. (Optional, target-controlled) PuzzleBoard as a new `CalibrationTarget` + decoder.**
CC0, occlusion-robust, absolute indexing — strongest of all *if* we can change the printed
pattern. Fits the `CalibrationTarget` seam, not just the detector seam. Reimplement the
decoder in C++ from the CC0 Python (MST via union-find + cross-correlation + majority vote).

**Not recommended for vendoring:** ftdlyc/libcbdetect (GPL-3.0), Deltille (LGPL-2.1, triangle-
focused), DL detectors (dependency weight). OpenCV SB stays the **baseline** backend we
already have.

### Architecture note (user's ask)
All three land as independent `cv::CheckerboardDetector` subclasses behind the existing
polymorphic seam — `CheckerboardTarget::setDetector(...)` already swaps them, and the lab's
`backend` combo already lists them. When the backend count grows, migrate selection onto
**ICL's generic backend-dispatching framework** (cf. `feedback_ipp_migration` / `backends(Backend)`
pattern) so `-i`-style string selection + registry self-registration replace the hand-rolled
combo — consistent with the `SourceBackend`/compression-plugin registries.

## Prototype results (Session 89 — recommendation #1 validated)

Built a standalone prototype of the RANSAC-homography associator (scratchpad, links
the built ICL libs; will move into `CheckerboardGrid.cpp`). Pipeline:
1. **Global RANSAC affine seed** — try every (center, axis-pair) hypothesis from ChESS
   seeds, score by the number of distinct integer lattice cells corners snap to. The true
   diagonal trap (a diagonal basis) snaps only half the corners to integers → low score →
   loses. **This is what defeats the trap.**
2. **Homography ICP** (Hoffmann-style) — fit homography image→lattice from inliers, re-snap
   all seeds, refit ×4. Folds perspective into the model.
3. **Compactness de-shear** — inlier count is invariant under unimodular basis changes, so
   the winner may be a *sheared* equivalent (true axis + a diagonal, e.g. a 4×6 board labeled
   as a 4×12 parallelogram). Relabel onto the unimodular basis maximizing fill ratio
   `count/(bboxW·bboxH)` — the true axes pack a complete board into a 100%-filled rectangle.
   **Parameter-free; needs no edge probe** (edge evidence was too mushy at this image scale).
Then the existing `refineCheckerboardGrid` (homography + Hungarian + trim) runs unchanged.

**Diagonal-trap synthetic** (sheared affine, equal-length axes at angle φ; trap when the cell
diagonal `|a−b|` < axis length, i.e. φ<60°), 9×6 grid:
| φ | `|a−b|` vs `|a|` | OLD (growth) | NEW (ransac) |
|---|---|---|---|
| 90°–60° | ≥40 vs 40 | OK | OK |
| 50° | 33.8 < 40 | **BAD 6×14/54 (trap)** | **OK 54/54** |
| 40° | 27.4 < 40 | **BAD 6×14/54 (trap)** | **OK 54/54** |
| 30° | 20.7 < 40 | **BAD 6×1/6** | **OK 54/54** |

NEW recovers the full grid across the entire tilt range; OLD collapses into a diagonal
sub-lattice for φ≤50°. **Real saved frame** (`checkerboard-frame.png`, the mild pose): NEW
recovers the complete 4×6/24 board at 0.65 px homography residual, matching OLD's refined output.

**Perspective gap — CLOSED (final architecture).** The first prototype used homography ICP for
step 2, which was brittle under strong keystone (a global homography extrapolates poorly from a
central inlier core when some labels are perspective-drifted). Replaced with **option (a) — RANSAC
seed → local-step growth**: the production `recoverCheckerboardGridRansac` is (1) RANSAC affine
hypothesis (trap-immune axis bootstrap), (2) affine-snap a central CORE + **de-shear it to the TRUE
axes** (compactness), (3) grow ONCE from a central cell with those true axes using the SAME
per-cell local-step `growFixedPoint` engine as the greedy path — so it inherits growth's
perspective + lens-distortion tracking — (4) final compactness de-shear. Growth must get the *true*
axes (not RANSAC's arbitrary unimodular basis), or a sheared basis makes growth reach cells only via
a 4-connected staircase and stall; de-shearing the core first fixes that. Band upper bound widened
to 3×median-spacing so the true (long) axes stay reachable when the shortest vector is the diagonal.

Result is a **clean Pareto improvement over greedy growth**: NEW recovers the full grid everywhere
growth does, PLUS the entire diagonal-trap region (oblique shear φ≈40–55°) where growth collapses.
Only φ≲30° (extreme grazing — growth mislabels) and keystone k≳0.5 (top edge squeezed >50%) still
fail, and growth fails there too. Keystone parity reached at k≤0.34 (where the ICP prototype failed).

**LANDED (Session 89):** `cv::RansacCheckerboardDetector` (`"native-ransac"`) — a 3rd
`CheckerboardDetector` backend wrapping `recoverCheckerboardGridRansac(seeds)` in
`CheckerboardGrid.cpp` (shares `growFixedPoint`/`buildGridFromCells`/`deshearCells`/`dedupSeeds`
helpers with the growth path), wired into the lab's `backend` combo
(`native-growth,native-ransac,opencv`). Tests `cv.checkergrid.ransac_clean`,
`ransac_diagonal_trap` (φ=55/45/35° — RANSAC recovers full where growth traps), `ransac_keystone`
(strong perspective). Suite **991/991**.

## Open questions to resolve during implementation
1. Robust seed for model-anchored growth — the RANSAC hypothesis stage (rec. 1) is the
   proposed answer; validate it actually beats the trap on the real saved frame.
2. Can ROCHADE topology run on ChESS corners alone (no centerline front-end)?
3. Sub-pixel/runtime cost of homography-model-fit vs graph association on ICL image sizes.
4. No independent benchmark exists for *foreshortening* specifically → our lab harness is the
   arbiter.

## Sources (primary unless noted)
- Geiger et al., ICRA 2012 — cvlibs.net/publications/Geiger2012ICRA.pdf
- Bennett & Lasenby (ChESS), arXiv:1301.5491
- Duda & Frese, BMVC 2018 — bmvc2018.org/contents/papers/0508.pdf; OpenCV PR #12147
- Hoffmann et al., VISAPP 2017 — scitepress.org/papers/2017/61043/61043.pdf
- Placht et al. (ROCHADE), ECCV 2014 — Placht14-RRC.pdf
- Ha et al. (Deltille), ICCV 2017; github.com/facebookincubator/deltille (LGPL-2.1)
- Stelldinger et al. (PuzzleBoard), arXiv:2409.20127; github.com/PStelldinger/PuzzleBoard (CC0)
- Lochman et al. (BabelCalib), arXiv:2109.09704 (MIT, MATLAB)
- ftdlyc/libcbdetect (GPL-3.0); lambdaloop/checkerboard (BSD-2); Elucidation/ChessboardDetect (brutesac)
- RCDN arXiv:2307.03505; Yoon et al. MMSP 2021 (IEEE 9733619)
</content>
</invoke>
