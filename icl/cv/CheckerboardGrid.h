// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::cv {

  /// A recovered checkerboard lattice: detected inner corners arranged on a
  /// rectangular integer (col,row) grid. Individual cells may be missing (not
  /// detected); use has()/at() to test/read them.
  struct CheckerboardGrid {
    int cols = 0;   ///< number of columns (label range [0,cols))
    int rows = 0;   ///< number of rows    (label range [0,rows))
    int count = 0;  ///< number of cells actually filled
    std::vector<utils::Point32f> points;   ///< cols*rows, row-major image positions
    std::vector<char> filled;              ///< cols*rows, 1 if that cell was detected

    /// Per-edge image-evidence confidence in [0,1], filled by
    /// scoreCheckerboardGridEdges() (empty / scored()==false until then). A real
    /// checkerboard edge runs along a black/white square border → high
    /// perpendicular gradient; a spurious/diagonal edge crosses a uniform square
    /// → low. -1 marks "no edge here".
    std::vector<float> edgeRight;          ///< cols*rows, score of (c,r)-(c+1,r)
    std::vector<float> edgeDown;           ///< cols*rows, score of (c,r)-(c,r+1)

    bool empty() const { return count == 0; }
    bool has(int c, int r) const {
      return c >= 0 && r >= 0 && c < cols && r < rows && filled[(size_t)r*cols + c];
    }
    const utils::Point32f &at(int c, int r) const { return points[(size_t)r*cols + c]; }
    /// true if every cell of the cols*rows lattice is filled (a complete board)
    bool complete() const { return count == cols*rows && cols > 0; }

    bool  scored() const { return !edgeRight.empty(); }
    float rightScore(int c, int r) const { return edgeRight[(size_t)r*cols + c]; }
    float downScore (int c, int r) const { return edgeDown [(size_t)r*cols + c]; }
  };

  /// Recover a checkerboard lattice from saddle corner seeds (growth-based, v1).
  /** Topology comes from the seed geometry alone (the seeds' orientations seed
      the initial grid axes): per-seed nearest-neighbour spacing, then a BFS that
      grows an integer lattice by following the four local axis directions,
      propagating the measured local step vectors so it tracks perspective and
      lens distortion. The lattice grown from the highest-scoring seed is
      returned (its largest connected component); clutter and a few missing
      corners are tolerated. Returns an empty grid when there are too few seeds.

      The (col,row) origin and axis assignment are NOT canonicalised to any
      physical board frame — the labelling is internally consistent but otherwise
      arbitrary (good enough for intrinsic calibration; a target backend resolves
      the board frame).

      @param image optional source image enabling GUIDED growth: the two grid
        axes and every grown link are chosen by image edge evidence (gradient
        across the connection — high on a real black/white border, low on a
        diagonal crossing a uniform square) instead of pure geometry. This is
        REQUIRED to disambiguate strongly-foreshortened views, where the board
        diagonal can be shorter than the long axis and pure geometry grows a
        diagonal lattice. When null, falls back to the geometric heuristic. */
  ICLCV_API CheckerboardGrid recoverCheckerboardGrid(const std::vector<CornerSeed> &seeds,
                                                     const core::Img8u *image = nullptr);

  /// Recover a checkerboard lattice from saddle corner seeds (global RANSAC, v2).
  /** A foreshortening-robust alternative to recoverCheckerboardGrid that does NOT
      bootstrap a single local axis pair (the step that falls into the "diagonal
      trap" under steep oblique views, where the cell diagonal is shorter than the
      board axes so nearest-neighbour growth links along diagonals). Instead it is
      purely geometric and global:

      1. RANSAC over every (centre, axis-pair) affine hypothesis from the seeds,
         scored by how many seeds snap to DISTINCT integer lattice cells — a
         diagonal basis only snaps half of them (the rest hit half-integers), so
         the true axes win on inlier count;
      2. homography ICP (model-anchored, Hoffmann-style): fit a homography
         image→lattice and re-snap all seeds, iterated, folding perspective into
         the model so the lattice stays rectangular on tilted boards;
      3. compactness de-shear: the inlier count is invariant under unimodular
         basis changes, so the winner may be a sheared labelling of the true axes;
         relabel onto the unimodular basis that packs the corners into the most
         filled bounding box.

      Needs no image (no edge evidence) — robust by construction. Pair with
      refineCheckerboardGrid() for the same global homography + Hungarian cleanup /
      border trim as the growth path. Returns an empty grid for <4 seeds or when no
      hypothesis explains ≥6 cells. Labels are internally consistent but not
      canonicalised to a board frame (a target backend resolves that). */
  ICLCV_API CheckerboardGrid recoverCheckerboardGridRansac(const std::vector<CornerSeed> &seeds);

  /// Recover a checkerboard lattice by graph topology (ROCHADE-style, v3).
  /** A third associator that recovers the grid as a GRAPH, not by growth or by a
      global model fit. Topology comes from the corner adjacency, so there is no
      length-based axis bootstrap (the step both growth and the geometric RANSAC
      seed can get wrong) and hence no "diagonal trap":

      1. Delaunay-triangulate the corner seeds → candidate adjacency;
      2. prune it to the true GRID edges: drop over-long chords and, crucially,
         every link lacking black/white border evidence in \a image (a cell
         diagonal crosses a uniform square → ~0 perpendicular gradient). This is
         the ROCHADE idea (topology from the image's edge structure) realised on
         ChESS corners + the existing edge probe instead of a centreline front-end;
      3. assign integer (col,row) by BFS over the pruned ≤4-regular graph, each
         node re-estimating its local axes from assigned neighbours (perspective /
         distortion tracking) and classifying each neighbour as a +/- axis step;
      4. compactness de-shear to canonicalise the basis.

      Unlike the growth and RANSAC paths this REQUIRES the image (the edge evidence
      is what removes the diagonals). Best worst-case robustness under steep oblique
      views; pair with refineCheckerboardGrid() for the same border trim. Returns an
      empty grid for <4 seeds or when the start node has <2 grid neighbours. Labels
      are internally consistent but not canonicalised to a board frame. */
  ICLCV_API CheckerboardGrid recoverCheckerboardGridGraph(const std::vector<CornerSeed> &seeds,
                                                          const core::Img8u &image);

  /// Global cleanup pass: suppress spurious border detections and fix growth
  /// mis-assignments via a model-based re-association.
  /** The greedy grid growth can, under strong foreshortening, grow a phantom
      border row/column out of a few spurious seeds (inflating the recovered
      dimensions), or mis-claim a seed into the wrong cell. This pass replaces
      that local reasoning with a global model:

      1. fit a homography (col,row)→image robustly to the recovered lattice
         (iteratively dropping high-residual cells so phantoms don't bias it);
      2. predict every node position and re-assign the seed pool to the nodes
         with the **Hungarian algorithm** (globally-optimal one-to-one, gated by
         a fraction of the local cell spacing) — phantom seeds fall onto no node,
         real corners snap to their cell;
      3. trim outermost rows/columns that end up weakly supported (few cells
         filled) or, when \a image is given, whose border edges lack black/white
         contrast (mean perpendicular-gradient edge score too low).

      Returns the cleaned grid (labels stay internally consistent; edge scores
      are cleared — call scoreCheckerboardGridEdges() again if needed). A grid
      too small to constrain a homography (cols<2 or rows<2) is returned as-is. */
  ICLCV_API CheckerboardGrid refineCheckerboardGrid(const CheckerboardGrid &grid,
                                                    const std::vector<CornerSeed> &seeds,
                                                    const core::Img8u *image = nullptr);

  /// Validation pass: score every lattice edge of \a grid by the mean image
  /// gradient PERPENDICULAR to the edge, sampled along it on a lightly-blurred
  /// gray of \a image, normalised to ~[0,1]. A true checkerboard edge lies on a
  /// black/white square border (strong perpendicular gradient → high score); a
  /// wrong edge (e.g. a diagonal through a square) crosses a uniform region (low
  /// score). Fills grid.edgeRight / grid.edgeDown (sets grid.scored()). A future
  /// step will feed this confidence back into growth (guided growth).
  ICLCV_API void scoreCheckerboardGridEdges(CheckerboardGrid &grid, const core::Img8u &image);

  /// Parameters for refineCheckerboardCornersSubPix().
  struct SubPixelParams {
    /// Half-size [px] of the square gradient window around each corner. Per grid it
    /// is additionally capped to a fraction of the median cell spacing so the window
    /// can never reach a neighbouring corner.
    int   winRadius = 5;
    /// Maximum refinement iterations per corner.
    int   maxIters  = 20;
    /// Convergence: stop when an iteration moves the corner by less than this [px].
    float eps       = 0.02f;
    // user-provided default ctor (not just DMIs) so `SubPixelParams{}` works as a
    // default argument inside this enclosing namespace
    SubPixelParams() {}
  };

  /// Sub-pixel corner polish for a recovered lattice (OpenCV cornerSubPix-style).
  /** The corners in \a grid come from the ChESS saddle detector, whose position is
      only refined by a parabolic fit on the coarse (integer-grid) response — good
      to ~0.5px but the accuracy floor for calibration. This pass polishes every
      filled corner directly on the grayscale image with the classic gradient
      orthogonality criterion:

      a true X-junction sits where the two crossing black/white edges meet. For any
      pixel p near the corner q, the image gradient g(p) is either ~0 (inside a
      uniform square) or perpendicular to the edge p lies on — and the vector q-p
      then runs ALONG that edge, so g(p)·(q-p)=0. Minimising the weighted sum of
      those dot products over a Gaussian window gives the 2x2 normal system
      (Σ w gg^T) q = Σ w gg^T p, solved and iterated to convergence.

      Because it averages many sub-pixel edge samples and intersects whole edges it
      reaches ~0.01-0.05px, and stays valid under perspective (a projected square's
      sides are still straight). Operates in place on grid.points; edge scores are
      left untouched. A no-op on an empty grid. */
  ICLCV_API void refineCheckerboardCornersSubPix(CheckerboardGrid &grid,
                                                 const core::Img8u &image,
                                                 const SubPixelParams &p = {});

} // namespace icl::cv
