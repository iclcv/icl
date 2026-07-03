# Model-Fitting Framework Plan (`icl/math/fit`)

Status: **P0–P3 LANDED** (virtual dispatch, Configurable), minus Halíř–Flusser
ellipse (blocked on a general/non-symmetric eigensolver — see §5 P3). P4–P5 remain.
Decisions taken: scope incrementally, virtual interfaces, fitters are Configurable.

P3 landed: `fit/RefiningFitter.h` (RefiningFitter interface + SeededFitter chaining),
`TaubinCircleFitter` (in PrimitiveFitters2D.h — drop-in, less biased than Kåsa),
`fit/GeometricRefiners2D.h` (GeometricCircleRefiner — orthogonal-distance polish via
NelderMeadOptimizer). Also `DynMatrix::eigenVector(largest=false)` — single extreme
eigenvector without materializing the full decomposition; homogeneousNullSpace uses it.
DEFERRED: Halíř–Flusser ellipse + Hartley normalization need a non-symmetric
eigensolver (LAPACK geev) which ICL lacks — add geev as its own infra task first.

Landed so far:
- P0 shared utils: `fit/FitUtils.h` (`homogeneousNullSpace`, `adaptiveRansacIters`),
  `fit/VectorTraits.h`. Deduped the DLT null-space solve (Homography2D +
  LeastSquareModelFitting) and the RANSAC adaptive-iteration formula.
- P1 interfaces: `fit/ModelFitter.h` (Tier C, virtual, Configurable),
  `fit/Optimizer.h` (Tier B, virtual, Configurable). Concrete:
  `fit/PrimitiveFitters2D.h` (Line/Circle/EllipseFitter2D over
  LeastSquareModelFitting2D), `fit/NelderMeadOptimizer.h` (wraps SimplexOptimizer).
- P2 robust decorator: `fit/RobustFitter.h` — RANSAC/MSAC + LO-RANSAC as a
  ModelFitter that wraps a ModelFitter; base surfaced as a child Configurable.
- Supporting: DynMatrix returning `eigen()` / `svd()` overloads (structured-binding
  friendly). Tests in `tests/test-math.cpp`; suite 1052/1052.

## 0. Motivation

`icl/math/fit` is currently seven independent point-solutions with real structural
overlap:

- Three of them (`LeastSquareModelFitting`, `PolynomialRegression`, and LM's step)
  form-and-solve a normal-equation product (`DᵀD`, `AᵀA`, `JᵀJ`) — squaring the
  condition number — instead of working on the matrix directly (QR/SVD/null-space).
- `SimplexOptimizer` hand-rolls a vector-abstraction layer via free-function
  specializations (`vdim()`, `create_zero_vector()`) — an ad-hoc `VectorTraits`.
- `RansacFitter` is conceptually a *decorator over a fitter*, but takes two loose
  `std::function`s instead of a fitter interface, so it can't compose.
- The DLT smallest-eigenvector null-space solve is duplicated between `Homography2D`
  and `LeastSquareModelFitting`.
- Better, well-known methods (Taubin/Halíř–Flusser, MSAC/LO-RANSAC, CMA-ES,
  QR/orthogonal-basis regression) have nowhere to plug in.

Goal: a small, ICL-idiomatic framework where methods that do the same job are
**interchangeable** and **chainable**, and recurring building blocks live in one
place. NOT an over-engineered concept-soup — ICL's rule is "templates only when
necessary".

## 1. Taxonomy — the four roles hiding in the seven files

| Role | Current files | Nature |
|---|---|---|
| **Optimizer** — minimize `f: params → scalar` | Simplex, Stochastic | iterative, derivative-free, param vector |
| **NLS refiner** — minimize `‖r(params)‖²` | LevenbergMarquardt | iterative, residual + Jacobian |
| **Closed-form model fitter** — data → model, one shot | LeastSquareModelFitting, PolynomialRegression | linear-algebra heavy (DynMatrix) |
| **Robust meta-fitter** — wrap a fitter, reject outliers | RansacFitter | decorator over a fitter |
| *(numeric primitive — out of scope)* | PolynomialSolver | root finding, not "fitting" |

Key realization: **RANSAC is a decorator over a fitter; an algebraic-seed →
geometric-refine pipeline is a chain of fitters.** Make "fitter" and "optimizer"
real interfaces and exchangeability + chaining fall out for free.

## 2. The templating tiers (the DynMatrix vs Vector<Scalar> vs generic question)

Choose the abstraction **by what the algorithm touches**, not by taste:

- **Linear algebra on data/Jacobian** (solve / eigen / SVD on an N×D matrix)
  → **`DynMatrix<Scalar>`, template on `Scalar` (float/double) only**, explicit
  instantiation in a `.cpp`. (LeastSquare, LM, Regression, Taubin, Halíř–Flusser.)
  A `FixedMatrix` fast-path is justified *only* for a hot, tiny, compile-time-fixed
  model.
- **Only evaluates a user objective over a param vector** (no LA on that vector)
  → **template on vector type `V` behind `VectorTraits<V>`**, instantiated for
  `FixedColVector<S,D>`, `DynColVector<S>`, `std::vector<S>`. (All derivative-free
  optimizers. Simplex already does exactly this — formalize its free functions.)
- **The "model" is an opaque object** (`Homography2D`, `Pose`, a conic) where you
  only need *sample→fit* + *point→residual* → **template on `<Data,Model>` with a
  virtual interface**. (RANSAC + all chaining; already how RansacFitter works.)

The tiers **meet** at one bridge: a Tier-A closed-form fitter (algebraic ellipse,
DynMatrix internally) *implements* the Tier-C `ModelFitter<Point2f,Conic>`
interface, so Tier-C robust/chaining machinery wraps it without knowing it's
matrix-based inside.

`Scalar` is float/double throughout, explicit instantiation (ICL pattern). Tier-C
generic `<Data,Model>` stays header-only (user-defined types), as RansacFitter is
today.

## 3. Core interfaces (sketch, ICL idiom — `icl::math`)

```cpp
// ---- Tier C: opaque model fitting ----
template<class Data, class Model>
class ModelFitter {
public:
  virtual ~ModelFitter() = default;
  virtual Model  fit(const std::vector<Data> &data) const = 0;
  virtual double residual(const Model &m, const Data &d) const = 0;
  virtual int    minSamples() const = 0;
};

// ---- Robust decorator: is-a ModelFitter, wraps a ModelFitter ----
template<class Data, class Model>
class RobustFitter : public ModelFitter<Data,Model> {
public:
  enum Score { Ransac, Msac };            // 0/1 count vs truncated-quadratic
  RobustFitter(const ModelFitter<Data,Model> *base, double inlierThresh,
               double confidence=0.99, int maxIters=1000,
               Score s=Msac, bool localOpt=true /*LO-RANSAC*/);
  Model  fit(const std::vector<Data>&) const override;   // RANSAC/MSAC(+LO) loop
  double residual(const Model&m,const Data&d) const override { return base->residual(m,d); }
  int    minSamples() const override { return base->minSamples(); }
  const std::vector<Data>& inliers() const;              // valid after fit()
};

// ---- Chaining: seed with one fitter, refine with another ----
template<class Data, class Model>
class SeededFitter : public ModelFitter<Data,Model> {    // algebraic → geometric-LM
  const ModelFitter<Data,Model> *seed, *refiner;
};

// ---- Tier B: derivative-free optimizer ----
template<class V>
class Optimizer {
public:
  using Scalar    = typename VectorTraits<V>::Scalar;
  using Objective = std::function<Scalar(const V&)>;
  struct Result { V params; Scalar error; int iterations; bool converged; };
  virtual Result minimize(const Objective&, const V &init) const = 0;
};
// NelderMead, CmaEs, OnePlusOneES : public Optimizer<V>  → interchangeable
```

Because `RobustFitter` *is a* `ModelFitter`, you can write
`SeededFitter(new RobustFitter(algebraic…), new GeometricRefine(…))` — RANSAC for
clean inliers + coarse model, then an LM geometric polish — and the whole chain is
itself a `ModelFitter`. That is the "chainable in a generic fashion" goal.

## 4. Shared building blocks → `icl/math/fit/detail/`

- `homogeneousNullSpace(const DynMatrix<S>& scatter) -> DynColVector<S>` — the
  smallest-eigenvector DLT core (dedupe Homography2D ↔ LeastSquareModelFitting).
- `numericJacobian(...)` — central difference, extracted from LM's anon struct.
- `HartleyNormalization` — isotropic 2D/ND normalize + model denormalize (homography
  has it; line/ellipse/circle want it).
- `qrSolve` / `leastSquaresSolve` — QR-based, so Regression and the LM step stop
  forming normal equations.
- `adaptiveRansacIters(w, s, confidence)` — the formula now inlined in RansacFitter.
- `VectorTraits<V>` — formalized from Simplex's free functions.
- Optional CRTP convenience base `PointModelFitter<Derived,Data,Model>` supplying
  `residual()` from `model.distanceTo(point)` so concrete fitters write only `fit()`
  + `minSamples()`.

## 5. Phased plan (each phase ships independently, suite green throughout)

- **P0 — Extract shared utilities.** Pure refactor: null-space, numeric Jacobian,
  Hartley, QR solve, `VectorTraits`. No behavior change; existing tests pin it.
- **P1 — Interfaces + retrofit.** Introduce `ModelFitter<Data,Model>` and
  `Optimizer<V>`; adapt existing classes to implement them. Keep old ctors
  (deprecate the std::function RANSAC ctor).
- **P2 — `RobustFitter` (RANSAC/MSAC + LO) as a decorator.** First exchangeability
  proof; MSAC + LO are the high-value robustness upgrades.
- **P3 — `SeededFitter` + better closed-form fitters.** DONE: Taubin circle,
  geometric refine (Nelder-Mead), SeededFitter chaining, `eigenVector()`. DEFERRED:
  Halíř–Flusser ellipse + Hartley normalization — need a non-symmetric eigensolver
  (LAPACK `geev`); ICL only has symmetric `syev`. Add `geev` (Eigen/Accelerate/MKL/
  Cpp backends, like the existing lapack ops) as a prerequisite task, then the
  ellipse fitter + conic denormalization drop in.
- **P4 — Regression QR/SVD + orthogonal (Chebyshev) basis; CMA-ES `Optimizer`,
  deprecate StochasticOptimizer.**
- **P5 — `Configurable` params + `icl-model-fitting` "method playground"** (swap
  fitters/optimizers live), analogous to `icl-filter-playground`.

## 6. Better methods this framework unlocks (from the earlier survey)

- Circles → **Taubin** (near-geometric at algebraic cost); Ellipses →
  **Halíř–Flusser** (stable Fitzgibbon, guarantees an ellipse). + Hartley norm.
- RANSAC → **MSAC** scoring (~free) + **LO-RANSAC**; **MAGSAC++** if SOTA wanted.
- LM → QR step (no normal equations); **geodesic acceleration** for stiff fits.
- Nelder–Mead → **BOBYQA** (smooth) / **CMA-ES** (rugged). StochasticOptimizer is
  superseded by CMA-ES (or at least the Rechenberg 1/5 rule).
- PolynomialRegression → **QR/SVD** instead of `pinv`; **orthogonal basis**;
  centering/scaling; optional ridge.

## 7. Open decisions (need sign-off before P0)

1. **Scope.** [REC] P0–P2 (interfaces + one decorator proof) before committing to
   the rest — validates the design end-to-end. Alternatives: full P0–P5;
   utilities-only (P0); or design-doc-only (this file).
2. **Dispatch style.** [REC] **Virtual interfaces** for Tier-C `ModelFitter` and
   `Optimizer` (matches the "real polymorphic interfaces over registries/
   std::function" preference; dispatch cost negligible vs the fit work; enables
   runtime method-swap + the playground). CRTP only if a hot inner loop demands it
   (→ Hybrid).
3. **Configurable integration.** Make fitters `utils::Configurable` so params
   (iterations, thresholds, damping) are runtime-tunable + auto-rendered by
   `qt::Prop`, consistent with the filter-Op migration. Suggested **yes**, landing
   in P5.
```
