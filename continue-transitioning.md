# ICL — Continuation Guide

## Current State (Session 36 — SSR rewrite + reflectivity + demo scene)

### Session 36 Summary (4 commits)

Complete SSR rewrite from broken world-space/texture-space approaches to
working view-space ray march. Added reflectivity uniform, improved demo
scene, fixed Cycles smooth shading bug.

#### A. SSR rewrite — view-space ray march

**Three failed approaches** before finding the working one:
1. World-space stepping + projected depth comparison — banding, wrong depths
2. Screen-space DDA + linear depth interpolation — depth is non-linear for
   rays toward camera (floor reflecting objects above), fundamentally broken
3. Texture-space linear march (ported from 3rdparty/SSR) — self-intersection
   from previous-frame depth mismatch, tight threshold issues

**Working approach**: view-space ray march with per-step projection:
- Transform fragment to previous frame's view space via `uPrevView`
- Step along reflection ray in view space (linear Z — no non-linear artifacts)
- At each step: project to screen via `uPrevProjection`, read depth buffer,
  reconstruct view-space Z via `uPrevInvProjection`, compare in linear space
- 8-iteration binary search refinement on sign change
- **4x supersampled**: traces 4 rays with stratified jitter offsets (0, 0.25,
  0.5, 0.75 of one step), confidence-weighted average eliminates Moiré ring
  artifacts on curved surfaces
- 256 steps, ray distance 4x camera depth, screen-edge + roughness fade
- SSR skipped for non-reflective surfaces (`reflectivity < 0.01 && !metallic`)

**New uniforms**: `uPrevView`, `uPrevProjection`, `uPrevInvProjection`
(uploaded alongside existing `uPrevVP`). Inverse projection computed per
frame via `Mat::inv()`.

**Depth buffer**: upgraded from `GL_DEPTH_COMPONENT24` to
`GL_DEPTH_COMPONENT32F` for better precision.

#### B. Reflectivity uniform

`uReflectivity` wired from `Material::reflectivity` to PBR shader. Controls
reflection strength: `reflFactor = max(envFresnel, vec3(uReflectivity))`.
At reflectivity=0, only Fresnel contributes; at 1.0, full mirror. Previously
the reflectivity field was ignored by the GL renderer.

#### C. Demo scene overhaul

**SSR test scene** (default when no `-scene` files given):
- RGB wireframe cube: 104 CuboidNode voxels forming 12 thick edges, corner
  colors from RGB cube (x,y,z → R,G,B), `smoothShading=false` for crisp edges
- Red sphere (90% reflective, roughness 0.15) — mirror-like SSR test
- Gold metallic sphere (metallic 0.9, roughness 0.35, reflectivity 0.3)
- Brown/black checkerboard ground (2x bigger, 50% reflective)
- Checkerboard back wall
- `-no-checkerboard` flag: flat black ground, no wall
- Camera near/far set tight to scene (20/6400, ratio 320:1) for depth precision

#### D. Cycles smooth shading fix

`SceneSynchronizer.cpp`: smooth shading now based on `Material::smoothShading`
only, not on presence of normals. Previously any geometry with normals
(including flat-faced cuboids) got smooth shading → rounded/glassy appearance.

### What's next

**SSR polish**:
- Step aliasing still visible at extreme close-up (could increase to 512 steps)
- Temporal accumulation: ping-pong feedback should naturally denoise over frames
- Consider Hi-Z acceleration for performance (hierarchical depth mip chain)

**Quick framework rework** (priority):
- `ImgQ` (`Img<icl32f>`) forces all Quick operations to float depth, causing
  unnecessary conversions. Consider reworking to use `Image` (preserves depth)
- Float image display was broken: `GLImageRenderer` converts all depths to
  uint8 but BCI shader applied depth-specific scale (double-scaling → black).
  Fixed in `GLImageRenderer::updateBCI()` — BCI scale is now always relative
  to uint8 since texture is always `GL_UNSIGNED_BYTE`. Also removed deprecated
  `glPixelTransfer` calls from `GLImg.cpp` (no-op in GL Core profile).
- `blur()` in Quick.h — verify it works correctly with large images
- Consider adding `localThresh()` convenience function

**Signature extraction demo** (`icl/cv/demos/signature-extraction.cpp`):
- Uses RotateOp, LocalThresholdOp, RegionDetector, blur, Quick.h functions
- Currently crashes — needs debugging (likely RegionDetector on filtered
  binary image, or ImgQ channel mismatch in the `scaled | blurred` concat)
- GUI: rotation, local threshold (mask size + global offset), min region
  size filter, alpha blur, scale-down factor, save PNG with transparency
- Uses `executeInGUIThread()` for save dialog (blocking, from worker thread)

**Shadow maps** (DONE — Session 37):
- `LightNode::setShadowEnabled()` was already in place
- Shadow depth pass ported from geom GLRenderer (4 maps, 2048×2048, sampler2DShadow)
- Shadow sampling in PBR shader with hardware PCF via `GL_COMPARE_REF_TO_TEXTURE`
- Auto-computed light VP matrix from LightNode world transform (lookAt + 90° perspective)
- `Renderer::setShadowsEnabled()` API added
- Demos updated: geom2-hello + cycles-renderer-test both enable shadows
- TODO: shadow camera direction/FOV controls on LightNode, PCF soft shadow taps,
  directional light ortho projection, debug visualization mode for shadow maps

**geom2 API cleanup**:
- Scene2 getters: return references or shared_ptrs instead of raw pointers
- `getGLCallback()`: return raw ptr instead of shared_ptr (avoid `.get()`)
- CoordinateFrameSceneObject: add PIMPL

**Other work**:
- CI update — meson in GitHub Actions
- ImageMagick 7 / FFmpeg 7+ rewrites
- ConvolutionOp IPP mixed-depth

---

## Previous State (Session 34 — Matrix migration + Cycles geom2 integration)

### Session 34 Summary (8 commits)

Two major areas: completed the matrix (row,col) convention migration across the
entire codebase, and wired Cycles renderer into geom2 with demos.

#### A. Matrix (row,col) migration — COMPLETE

**Full migration**: `operator()(col,row)` → `operator()(row,col)` across 67
files, ~1200 call sites. Three-phase approach:
1. Remove `operator()`, add `index_yx(row,col)` to both `FixedMatrix` and
   `DynMatrixBase`. Automated via `scripts/fix-matrix-indexing.py` (column-based)
   and new `scripts/fix-matrix-indexing2.py` (regex inside-out, handles nested
   calls, deref patterns, chained calls). Iterated build→fix→rebuild until clean.
2. Migrate `at(col,row)` → `at(row,col)` (5 files, ~60 sites).
3. Rename `index_yx` back to `operator()` — clean `M(row, col)` syntax restored.

**False positives caught**: `pow.index_yx`, `atan2.index_yx`, `std::swap.index_yx`,
`.mult.index_yx`, `setSamplingResolution.index_yx`, `Point32f.index_yx`,
`ICL_TEST_EQ.index_yx` — all fixed. Two semantic bugs caught in geom module
review (Camera.cpp principal point, Posit.cpp atan2 arg order).

**384/384 tests pass** after migration.

#### B. Cycles renderer wired into geom2

**Meson build wiring** (`icl/geom2/meson.build`): `CyclesRenderer.cpp` and
`SceneSynchronizer.cpp` now compiled when `cycles_found`. Reuses include paths,
compile flags, and link libraries from geom module's meson config.

**SceneSynchronizer fixes** for current Cycles 4.x API:
- `unique_ptr<ShaderGraph>` for `set_graph()`
- Removed `graph->add()` (nodes auto-added via `create_node`)
- `PointLight` instead of generic `Light`
- Camera FOV uses `getSamplingResolutionY()` + `compute_auto_viewplane()`
- Light color normalized from 0-255 to 0-1 before applying physical intensity
- Gradient sky background added (zenith/horizon/ground blend)
- Analytic sphere path disabled (known offset bug), always tessellates

---

## Previous State (Session 32 — Material refactoring + geom2 scene graph)

### Session 32 Summary (23 commits)

Two major areas of work:

## Previous State (Session 33 — PointCloud, BVH, textures, text, mouse interaction)

Session 33 added PointCloud, PointCloudNode, BVH raytracer, texture/text
rendering, mouse interaction to geom2. Started matrix indexing migration
(added `index_yx`, migrated math module). 5 commits.

### Session 32 Summary (23 commits)

#### A. Material & deprecated warning cleanup (geom module, 7 commits)

**Material restructured** with lazy sub-structs:
- `TextureMaps` behind `shared_ptr` (null for untextured objects, saves ~80 bytes)
- `TransmissionParams` behind `shared_ptr` (null for opaque objects)
- New fields: `lineColor`, `pointColor`, `pointSize`, `lineWidth`
- Non-copyable with explicit `deepCopy()` returning `shared_ptr<Material>`
- New factory: `fromColors(faceColor, wireColor)` for mixed face+wire colors

**SceneObject copy semantics fixed:**
- `operator=` deleted, `copy()` renamed to `deepCopy()`
- Protected copy ctor preserved for subclass deepCopy()
- Old `operator=` was buggy (missing material, reflectivity, emission)
- All PointCloud subclasses updated: `copy()` → `deepCopy()` with override

**GLRenderer line/point rendering:**
- New unlit shader (GLSL 410) for GL_LINES + GL_POINTS
- Separate VAOs in GLGeometryCache for lines and points
- Lines and points now visible in GLRenderer (were silently dropped)

**All 77 deprecated setColor/setShininess calls migrated** across 26 files.

**Bugs fixed:**
- Analytic sphere overlay offset: DemoScene auto-scale modified vertices
  but not `m_sphereCenter`/`m_sphereRadius`. Fixed by updating sphere params
  after transforms + adding `setSphereParams()` setter.

#### B. geom2 — Clean scene graph module from scratch (16 commits)

**Why:** SceneObject was a 940-line monolith (geometry + scene graph + materials +
transforms + rendering + physics base). Every incremental refactor tangled in
backward compatibility. geom2 is a greenfield design in `icl/geom2/` namespace
`icl::geom2`, independent of ICLGeom.

**Node hierarchy (clean separation of concerns):**
```
Node (abstract)              ← transform + visibility + name + locking. PIMPL'd.
├── GroupNode                ← has children. No geometry. Pure container.
├── GeometryNode (abstract)  ← read-only geometry + material (renderer interface)
│   ├── MeshNode             ← mutable geometry leaf (addVertex, getVertices()&)
│   ├── SphereNode           ← parametric sphere (no mutable vertices)
│   ├── CuboidNode           ← parametric box
│   ├── CylinderNode         ← parametric cylinder
│   └── ConeNode             ← parametric cone
├── LightNode                ← point/directional/spot, position from transform
└── CoordinateFrameNode      ← GroupNode with 3 CuboidNode axes (R=X, G=Y, B=Z)
```

**Key design rules:**
- GroupNode has children. Leaf nodes (MeshNode, SphereNode, LightNode) do not.
- GeometryNode provides read-only const access for renderers.
- MeshNode adds mutable access (for physics/dynamic geometry).
- Parametric shapes inherit GeometryNode (NOT MeshNode) — no mutable vertex access.
- All nodes PIMPL'd with `unique_ptr<Data>`, Rule of 5 (copy+move).
- No raw pointer ownership. `shared_ptr` everywhere.
- No display lists. No legacy GL. Core profile only.
- `MeshNode::ingest(MeshData{...})` for zero-copy bulk geometry loading.

**Components implemented:**
- `Node.h/.cpp` — abstract base (~120 lines)
- `GroupNode.h/.cpp` — children container (~80 lines)
- `GeometryNode.h/.cpp` — read-only geometry + material (~230 lines)
- `MeshNode.h/.cpp` — mutable geometry + `ingest()` (~120 lines)
- `Primitive.h` — Line/Triangle/Quad as plain structs (~50 lines)
- `SphereNode`, `CuboidNode`, `CylinderNode`, `ConeNode` — parametric shapes
- `CoordinateFrameNode` — ported from geom's CoordinateFrameSceneObject
- `LightNode` — point/directional/spot with color+intensity
- `Renderer.h/.cpp` — GL 4.1 Core, PBR shader + unlit shader, multi-light (~530 lines)
- `Scene2.h/.cpp` — scene manager with cameras, shared_ptr ownership, GL callback
- `Loader.h/.cpp` — .obj + .glb/.gltf file loading via `ingest()`
- `Raytracer.h`, `CyclesRenderer.h`, `SceneSynchronizer.h/.cpp` — Cycles scaffolding
  (headers ready, .cpp adapted but not compiled — needs Cycles build wiring)
- `PORTING.md` — geom → geom2 migration guide
- `demos/geom2-hello.cpp` — working demo with all shape types + lighting

**Total: 24 files, ~4800 lines, zero warnings.**

**Material is shared** between geom and geom2 (`icl::geom::Material` used by both).

### What's next for geom2

Remaining items from PORTING.md:
- **PointCloudNode** — replaces PointCloudObjectBase (important for CV visualization)
- **Mouse interaction / picking** — hit testing against the scene graph
- **Text primitives / billboard text** — needed for labels and debug overlays
- **Texture primitives** — textured quads
- **Cycles build wiring** — connect CyclesRenderer.cpp + SceneSynchronizer.cpp to Cycles
  libraries in geom2/meson.build (headers and adapted .cpp are ready)
- **ComplexCoordinateFrameNode** — cones + cylinders + text labels
- **More demos** — file loading demo, interactive mouse demo

### Build

```bash
meson setup build --buildtype=debug --wipe
CCACHE_DISABLE=1 meson compile -C build -j16
bin/geom2-hello-demo   # first geom2 visual demo
```

---

## Previous State (Session 31 — Meson build system, directory restructure)

### Session 31 Summary

**Build system migrated from CMake to Meson+Ninja:**

The entire project structure was overhauled in one session. 9 commits.

**Directory restructure:**
- `ICLGeom/src/ICLGeom/Scene.h` → `icl/geom/Scene.h` (matches `icl::geom` namespace)
- All 10 modules renamed: `ICLUtils→icl/utils`, `ICLMath→icl/math`, etc.
- `src/` wrapper dropped — headers and sources coexist directly
- Include paths: `#include <icl/geom/Scene.h>` (was `<ICLGeom/Scene.h>`)
- Library names: `libicl-geom.dylib` (was `libICLGeom.dylib`)

**Demos/apps flattened:**
- 120+ single-file subdirectories eliminated (each had its own CMakeLists.txt)
- Now: `icl/filter/demos/canny-op.cpp` (was `ICLFilter/demos/canny-op/canny-op.cpp`)
- Multi-file targets use naming convention: `camera-calibration-CalibrationGrid.cpp`

**Meson build system:**
- `meson.build` + `meson.options` replace ~170 CMakeLists.txt + 29 cmake modules
- 10 module `meson.build` + 10 target `meson.build` files
- All dependencies detected: Qt6, OpenCV, Eigen3, OpenCL, ImageMagick, FFmpeg,
  Bullet, Accelerate, Cycles (full integration with 21 compile defs, 20+ include
  dirs, 14 static + 18 shared libs)
- Config headers generated without `.in` templates (Meson `configure_file()`)
- Qt MOC via `qt6.compile_moc()` — trimmed to 11 headers with actual Q_OBJECT
- PCH headers in `icl/_pch/` (required: macOS case-insensitive FS shadows
  system `<time.h>` with our `Time.h` — fixed via `implicit_include_directories: false`)
- Post-build hook symlinks all executables into `build/bin/`

**Build result:** 122 targets (10 libs + 68 demos + 35 apps + 8 examples + 1 test)
all compile and link. Tests pass.

**Dead code removed:**
- `VideoGrabber.cpp/.h` — xine dependency no longer exists
- Old CMake system: 518 files deleted (45K lines)
- Old `ICL*` module directories removed
- `ICLExperimental/` removed (Cycles demos moved to `icl/geom/demos/`)

**Scenes and assets:** moved to `icl/geom/scenes/` (9 glb/obj files),
doc images moved into `icl/*/doc/`.

**Key Meson commands:**
```bash
meson setup build                              # configure
meson setup build -Ddemos=true -Dapps=true     # with demos+apps
meson compile -C build -j16                    # build
meson test -C build                            # run tests
meson configure build -Dtests=true             # reconfigure option
```

**Known disabled targets (pre-existing code issues, not build system):**
- 3 OpenCV legacy C API files (`OpenCVCamCalib`, `LensUndistortionCalibrator`,
  `OpenSurfLib`) — need rewrite for OpenCV 4+
- `TemplateTracker` — depends on IPP-only `ProximityOp`
- `corner-detection-css` demo — uses removed `DebugInformation` API
- `heart-rate-detector` demo — needs `ICL_OPENCV_INSTALL_PATH` define
- `octree` demo — needs PCL
- `point-cloud-primitive-filter` app — needs dead RSB dependency

### Next Steps

**Immediate:**
- **CI update** — `.github/workflows/ci.yaml` needs `pip install meson ninja`
  and `meson setup/compile/test` instead of cmake
- **CLAUDE.md update** — build instructions reference cmake, need meson equivalents
- **Docker scripts** — `scripts/docker/` needs meson adaptation

**Build system polish:**
- OpenCL kernel header generation (`scripts/cl2header.py` + `custom_target` in
  `icl/filter/meson.build`) — currently no `.cl` kernels are being compiled
- Fix disabled targets: rewrite OpenCV C API files for OpenCV 4+
- Add `subprojects/*.wrap` for Windows builds (zlib, libpng, libjpeg, gtest)

**Other work (unchanged from Session 30):**
- **ConvolutionOp IPP: mixed-depth support**
- **Benchmark IPP backends**
- **ImageMagick 7** — rewrite for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API

## Previous State (Session 30 — Bilinear fix, LAPACK helpers, IPP backend implementations)

### Session 30 Summary

**C++ bilinear scaling bug fixed (`Img_Cpp.cpp`):**
- Root cause: formula used `(src-1)/dst` instead of `(src-1)/(dst-1)` for the
  bilinear mapping. For identity scale (8→8), this gave `fSX = 7/8 = 0.875`
  instead of `1.0`, producing wrong pixel values (e.g., 8.75 instead of 10).
- Also fixed edge clamping: the old code clamped the base index `xll`, which
  gave the wrong pixel at the last row/column. Now clamps the +1 neighbor
  index (`x1 = min(x0+1, maxX)`) while keeping fractional weights correct.
- Test `Img.scaledCopy_identity` now passes on all platforms.

**LAPACK transpose helpers migrated to Accelerate and MKL backends:**
- Replaced 28 manual transpose loops in `LapackOps_Accelerate.cpp` and
  `LapackOps_Mkl.cpp` with `lapack_row_to_col` / `lapack_col_to_row` calls.
- Added vector-returning overload `auto AT = lapack_row_to_col(A, M, N, lda)`
  that allocates and returns the transposed buffer. Eliminates manual sizing.
- GELSD backward transpose kept manual (BT stride `mx` ≠ row count `N`).
- Net -78 lines across both backend files.

**5 IPP backends implemented (Docker-verified, 384/384 tests pass with IPP+MKL):**

| File | IPP Functions | Depths | Notes |
|------|--------------|--------|-------|
| UnaryArithmeticalOp_Ipp.cpp | ippiAddC/SubC/MulC/DivC + ippiSqr/Sqrt/Ln/Exp/Abs | 8u/16s/32f | withVal + noVal ops |
| LUTOp_Ipp.cpp | ippiReduceBits (modern signature with buffer param) | 8u | noise=0, ippDitherNone |
| MedianOp_Ipp.cpp | ippiFilterMedianBorder | 8u/16s | Fixed + generic sizes |
| ConvolutionOp_Ipp.cpp | ippiFilterBorder (spec-based) | 8u/32f | Odd kernels only; mixed-depth/even delegates to C++ |
| AffineOp_Ipp.cpp | ippiWarpAffineNearest/Linear (spec-based) | 8u/32f | NN + bilinear interpolation |

**MorphologicalOp_Ipp.cpp remains a stub:** IPP 2022.3 removed the general
morphology border API (ippiMorphologyBorderGetSize/Init, ippiDilateBorder,
ippiErodeBorder). Only `ippiDilate3x3_64f_C1R` / `ippiErode3x3_64f_C1R`
remain. The C++ backend handles all morphology; Accelerate provides
vImageDilate/Erode for 8u/32f on macOS.

**ConvolutionOp IPP limitations:** IPP's `ippiFilterBorder` uses center-anchored
convolution, which doesn't match ICL's configurable anchor for even-sized kernels.
Also, mixed-depth cases (8u→16s for Sobel) require `ippiFilterBorder_8u16s_C1R`
which isn't implemented yet. The IPP backend checks for odd kernel + same depth
and delegates all other cases to the C++ backend via explicit `get(Backend::Cpp)`.

**Docker Dockerfile updated:** pinned `intel-oneapi-mkl-devel-2022.2.1` to match
IPP 2022.3 era (avoids potential symbol conflicts between MKL 2025 and IPP 2022).
Confirmed: IPP+MKL work correctly together — earlier "MKL crash" was stale build
artifacts from bisection testing, not a real incompatibility.

**Build: 384/384 tests pass on macOS and Docker Linux (IPP+MKL).**

### Next Steps

**Immediate:**
- **ConvolutionOp IPP: mixed-depth support** — implement `ippiFilterBorder_8u16s_C1R`
  for 8u→16s (Sobel, Laplace) and even-kernel anchor alignment
- **Benchmark IPP backends** — measure speedup vs C++ for the 5 new IPP ops

**Experimental — Raytracing:**
- Real-time raytracer in `ICLExperimental/Raytracing/` — see
  [continue-raytracing.md](ICLExperimental/Raytracing/continue-raytracing.md)
  for full details. CPU backend working (BVH + OpenMP + reflections). Next: Metal RT
  backend for hardware-accelerated raytracing on Apple Silicon.

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers

## Previous State (Session 29 — IPP/MKL Docker verification, LAPACK bug fixes)

### Session 29 Summary

**Docker IPP+MKL build infrastructure:**
- Added `intel-oneapi-ipp-devel` to Dockerfile alongside MKL
- Enabled `-DBUILD_WITH_IPP=ON -DBUILD_WITH_MKL=ON` in docker-test.sh
- Fixed CMake IPP detection: removed dead `iomp5` (Intel OpenMP) and `ippm`
  (deprecated matrix lib) from required libs. IPP doesn't need Intel OpenMP;
  system OpenMP works fine.
- MKL: switched from `mkl_intel_thread`+`iomp5` to `mkl_sequential` (avoids
  Intel OpenMP dependency; ICL's own OpenMP handles parallelism)
- Added oneAPI search paths for MKL
- Fixed `--cpus` in docker-test.sh to use Docker's available CPU count
- Fixed PugiXML.cpp: `#include "pugixml.hpp"` → `"PugiXML.h"` (pugixml.hpp
  was never in git; PugiXML.h IS the header, renamed to ICL convention)

**All 6 real _Ipp.cpp files compile against oneAPI IPP — zero API changes needed.**
APIs verified available in modern oneAPI: ippiMirror, ippiSet, ippiLUTPalette,
ippiMax/Min/Mean, ippiMulC/AddC, ippiCopyReplicateBorder, ippiCopy P↔C,
ippiThreshold, ippiCompareC, ippiAndC/OrC/XorC/Not, ippiRemap, ippiFilterWiener.

**Bug found: cpp_getri (C++ LU inverse) produced completely wrong results.**
- The in-place U-inversion algorithm used `A[i][i]` which had already been
  overwritten to `1/U[i][i]` in a prior iteration. Errors were 20-500x, not
  precision issues.
- Hidden because on macOS the Accelerate backend (higher priority) was always
  selected — the C++ fallback was never exercised.
- Discovered by running tests in Docker where only the C++ backend is available.
- Fix: rewrote to solve `A*x=e_j` column-by-column using the LU factorization
  directly (forward-substitute with L, then back-substitute with U).

**Bug found: MKL geqrf/orgqr/getrf/getri row-major handling was wrong.**
- MKL backend used a dimension-swap trick (passing swapped M,N to LAPACK) to
  handle row-major → column-major. This works for symmetric operations (SVD,
  eigenvalue) but NOT for QR: QR(A^T) ≠ QR(A).
- Caused Docker test `math.dyn.qr_reconstruct` to fail with errors of 17+
  and subsequent heap corruption from wrong Q/R propagating.
- Fix: all four functions now use explicit transposition (matching the
  Accelerate backend pattern). `gelsd` already had this correct.

**Transpose helpers added to LapackOps:**
- `lapack_row_to_col(A, M, N, lda, AT)` and `lapack_col_to_row(AT, M, N, A, lda)`
  centralize the row-major ↔ column-major pattern used by all LAPACK backends.
- Defined in LapackOps.cpp, declared in LapackOps.h, instantiated for float/double.
- Backend callers to be migrated to use helpers in a future pass.

**C++ QR fallback (cpp_geqrf, cpp_orgqr) verified correct.** Standalone testing
with the exact Wikipedia 3×3 matrix from the test suite, plus 4×3, 5×5, and
double precision — all produce correct Q*R=A reconstruction within float epsilon.
Also verified cpp_gelsd (SVD least-squares solve) is correct.

**Pre-existing bug: C++ bilinear scaling produces wrong results for identity scale.**
- Test `Img.scaledCopy_identity` fails in Docker (383/384 pass without MKL).
- Scales 8×6 → 8×6 with `interpolateLIN`, gets `8.75` for pixel value `10`.
- The value `8.75 = 0.875 * 10` suggests `fSX = 7/8 = 0.875` (wrong) instead
  of `fSX = 7/7 = 1.0` (correct for identity scale with the `(src-1)/(dst-1)` formula).
- Investigation in `Img_Cpp.cpp` shows the bilinear code at line 394 computes
  `fSX = (srcSize.width - 1) / (dstSize.width - 1) = 7/7 = 1.0` — which is correct.
- The `scaledCopy` call path passes `getSize() → srcSize` and `poDst->getSize() →
  dstSize`, both `(8,6)` — confirmed correct.
- Cannot reproduce on macOS because Accelerate backend (vImageScale, Lanczos)
  takes priority. Need Docker debug session with print statements in
  `cpp_scaledCopyChannel` to see actual argument values at runtime.
- Hypotheses: (1) some other code path is being called that computes fSX
  differently, (2) the ImgOps dispatch wraps sizes differently, (3) ROI
  handling mutates the sizes before the backend sees them.
- TODO: Add `fprintf(stderr, ...)` debug prints to `cpp_scaledCopyChannel` in
  a Docker shell session to trace actual argument values.

**Docker test results (IPP only, no MKL): 383/384 pass.**
Remaining failure is the bilinear scaling bug above. With MKL: QR and
inverse tests now pass after the fixes, but MKL+Rosetta causes heap
corruption in some LAPACK paths (genuinely Rosetta — the MKL code is
now correct per our analysis). Need real x86 Linux for full MKL verification.

**Build: 100% clean (zero warnings), tests: 384/384 pass on macOS.**

### Next Steps

**Immediate:**
- **Fix C++ bilinear scaling bug** — Docker debug session needed
- **Migrate LAPACK backends to transpose helpers** — replace manual loops
  in Accelerate, MKL, Eigen backends with `lapack_row_to_col`/`lapack_col_to_row`

**IPP stubs (6 files, incremental performance):**
- AffineOp_Ipp.cpp — rewrite with modern `ippiWarpAffineNearest`/`Linear` + spec init
- ConvolutionOp_Ipp.cpp — rewrite with `ippiFilterBorder_*` (spec-based API)
- MorphologicalOp_Ipp.cpp — rewrite with `ippiMorphInit_*` + spec-based API
- MedianOp_Ipp.cpp — rewrite with `ippiFilterMedianBorder_*`
- LUTOp_Ipp.cpp — update `ippiReduceBits` signature (added noise param)
- UnaryArithmeticalOp_Ipp.cpp — write registrations for `ippiAddC/MulC/SubC/DivC`

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers or MKL verification

## Previous State (Session 28 — C++17 Phases 2-6: complete modernization)

### Session 28 Summary

**C++17 Phases 2-6 — mechanical and structural modernization across 183 files:**

Completed the entire C++17 modernization plan (phases 2-6) in a single session.
All changes are zero-semantic-impact except where noted (string_view API, transparent
comparators). Build clean (zero warnings), tests: 384/384 pass throughout.

**Phase 2 — scoped_lock, structured bindings, if-init, [[maybe_unused]] (118 files):**

- `std::scoped_lock` (75 files, 281 replacements): all `std::lock_guard` replaced.
  GradientImage.cpp and DC.cpp manual lock()/unlock() converted to RAII scoped_lock.
- Structured bindings (10 files, ~20 conversions): map iteration loops converted to
  `auto [key, value]` syntax. Pair unpacking in GenericGrabber (`split_at_first`).
  Files: ConfigFile, UnaryOp, GUI, Widget, PointCloudObjectBase,
  PhysicsPaper3ContextMenu, ManipulatablePaper, CCFunctions, GenericGrabber,
  ProcessMonitor.
- If-with-initializer (25 files, ~40 conversions): `find()`+`if` patterns converted to
  `if(auto it = find(); it != end())`. Files: Configurable, ConfigFile, Size,
  UnaryOp, Grabber, FileGrabber, GenericGrabber, Scene, MarkerGridDetector,
  PointCloudObjectBase, Benchmark, SignalHandler, and more.
- `[[maybe_unused]]` (33 files, ~52 conversions): `(void)var` casts replaced with
  `[[maybe_unused]]` on declarations/parameters. Includes 14 ICLFilter Op files
  (static init pattern), function parameters, catch variables, conditional vars.

**Phase 3 — std::filesystem (2 files):**

- `File.cpp`: `break_apart()` rewritten with `fs::path` methods (parent_path, stem,
  extension). Fixes pre-existing bug where `.gz` double-extension handling truncated
  the last character of basename (`substr(0,p-1)` → correct via `fs::path::stem`).
  `file_exists()` → `fs::exists()`, `file_is_dir()` → `fs::is_directory()`,
  `erase()` → `fs::remove()`. Removed `<sys/stat.h>` and `DIR_SEPERATOR`.
- `FileList.cpp`: eliminated all three platform-specific glob implementations
  (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replaced with
  `fs::directory_iterator` + `std::regex_match`. Added tilde expansion via `$HOME`,
  deterministic sort, no-wildcard fast path. Removed `<wordexp.h>`, `<glob.h>`,
  `<windows.h>`, `<cstring>`.

**Phase 4 — std::from_chars / std::to_chars (7 files):**

- Integer parsing: `atoi()` → `std::from_chars` in IntHandle, GUIDefinition,
  FileGrabberPluginCSV, TestImages, JPEGDecoder.
- Float parsing: `atof()` / `istringstream` → `std::strtof` / `std::strtod` in
  FloatHandle, GUIDefinition, FileGrabberPluginCSV, StringUtils (parse_icl32f/64f).
  Apple Clang libc++ lacks `from_chars` for floats; `strtof`/`strtod` are
  locale-independent and avoid istringstream allocation overhead.
- Integer formatting: `snprintf` → `std::to_chars` in StringUtils (i2str, time2str).
  Buffer sizes use `std::numeric_limits<T>::digits10 + 3` for portability.

**Phase 5 — if constexpr cleanup (3 files):**

- `SimdCompat.h`: added non-Apple scalar fallbacks for `add`/`sub`/`smul` in
  `simd_compat` namespace (compiler auto-vectorizes at -O3).
- `FixedMatrix.h`: removed all 9 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in element-wise
  operators (*, /, +, -, negate). Each replaced with a direct `simd_compat::` call
  that works on all platforms — Apple SIMD for 4x4/2x2, scalar fallback otherwise.
- `BackendDispatching.h`: 6 `enable_if` SFINAE constraints replaced with
  `static_assert` (cleaner error messages, `_v` trait aliases).

**Phase 6 — std::string_view + transparent map comparators (52 files):**

- `StringUtils.h/cpp`: `tok`, `toLower`, `toUpper`, `parse<T>`, `startsWith`,
  `endsWith`, `match`, `skipWhitespaces`, `analyseHashes`, `to8u/16s/32s/32f/64f`
  all take `std::string_view` instead of `const std::string&`. Deleted dangerous
  `parse<const char*>` specialization. Functions needing null-terminated strings
  (`strtof`, `regcomp`) construct `std::string` internally.
- `StrTok.h/cpp`: constructor takes `string_view`, internal tokenization updated.
- Transparent map comparators (`std::less<>`) added to all `std::map<std::string,...>`
  declarations across 48 files, enabling heterogeneous lookup with `string_view` keys
  without allocating temporary `std::string`. Modules: ICLUtils (Configurable,
  ConfigFile, ParamList, PluginRegister, MultiTypeMap, Benchmark, IppInterface),
  ICLCore (Color), ICLFilter (UnaryOp), ICLIO (Grabber, FileWriter, GenericGrabber,
  DCDeviceFeatures, SharedMemorySegment, OpenNIUtils, V4L2Grabber), ICLGeom
  (PointCloudObjectBase, PointCloudSerializer, Primitive3DFilter, PCDFileGrabber,
  Scene, DepthCameraPointCloudGrabber), ICLQt (Widget, GUI, Quick, MultiDrawHandle,
  DefineRectanglesMouseHandler, IconFactory), ICLPhysics (ManipulatablePaper,
  PhysicsPaper3ContextMenu).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Modernization — Final Status

| Phase | What | Files | Status |
|---|---|---|---|
| 1 | Nested namespaces, std::clamp, [[nodiscard]] | 821 | Done (session 27) |
| 2 | scoped_lock, structured bindings, if-init, [[maybe_unused]] | 118 | Done (session 28) |
| 3 | std::filesystem (File.cpp, FileList.cpp) | 2 | Done (session 28) |
| 4 | std::from_chars / std::to_chars | 7 | Done (session 28) |
| 5 | if constexpr (SimdCompat, BackendDispatching) | 3 | Done (session 28) |
| 6 | std::string_view + transparent map comparators | 52 | Done (session 28) |
| 7 | std::optional | ~10 | Deferred — do opportunistically |

Phase 7 (std::optional) is low-priority. The guide recommends doing it when touching
the relevant APIs for other reasons (MarkerGridDetector::getPos, ImgBase::getMax/Min,
FilenameGenerator error returns).

### Next Steps

**Remaining non-C++17 work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers. Run on CI or cloud x86 VM.

## Previous State (Session 27 — C++17 Phase 1: namespaces, std::clamp, [[nodiscard]])

### Session 27 Summary

**C++17 Phase 1 — mechanical modernization across 821+ files:**

Three categories of changes, all purely mechanical with zero semantic impact:

**1. Nested namespace fusion (821 files):**
- Fused `namespace icl { namespace X {` → `namespace icl::X {` across the entire
  codebase using `scripts/fuse-namespaces.py`. Script handles the standard case
  (outer `namespace icl {` on one line, inner `namespace X {` on the next).
- 11 files with non-standard patterns fixed manually:
  - Sibling namespaces (`namespace icl { namespace qt {...} namespace geom {...} }`)
    in Scene.h, ShaderUtil.h, SceneLight.h, PhysicsWorld.h, PhysicsUtils.h, Quick.h,
    ConvexHull.cpp, DCDeviceFeatures.cpp — split into separate `namespace icl::X` blocks.
  - Multi-block files (SignalHandler.cpp — two `#ifdef` blocks with separate namespaces).
  - Forward-declaration blocks (Grabber.h, JPEGHandle.h).
- Closing braces updated: `} // namespace X` → `} // namespace icl::X`, outer `}` removed.
- Content dedented by 2 spaces (`scripts/dedent-namespaces.py`) to restore standard
  2-space indent for namespace content (664 files, purely whitespace).

**2. std::clamp (3 files):**
- `ICLUtils/ClippedCast.h` — `clip()` body replaced with `std::clamp(tX, tMin, tMax)`.
  All ~60 call sites across the codebase get std::clamp behavior automatically.
- `ICLFilter/LocalThresholdOp.cpp` — `myclip()` replaced with `std::clamp` + cast.
- `ICLPhysics/demos/physics-maze/physics-maze.cpp` — `std::max(std::min(...))` →
  `std::clamp(val*5, -1., 1.)`.

**3. [[nodiscard]] (3 files):**
- `ICLCore/Image.h` — deepCopy, convert, scaledCopy, selectChannel, selectChannels,
  mirrored (all return new Image values that should never be silently discarded).
- `ICLFilter/UnaryOp.h` — single-arg apply() and operator()().
- `ICLUtils/BackendDispatching.h` — registeredBackends, bestBackendFor,
  applicableBackendsFor (pure query methods).
- `tests/test-filter.cpp` — 19 call sites suppressed with `(void)` where apply() is
  called for side effects (feeding frames into MotionSensitiveTemporalSmoothing).

**New utility scripts:**
- `scripts/fuse-namespaces.py` — fuses two-level `namespace icl { namespace X {` into
  `namespace icl::X {`. Uses last-zero-indent-brace strategy for closing brace removal.
  Skips one-line forward declarations. Does NOT handle sibling-namespace files (those
  need manual fixup).
- `scripts/dedent-namespaces.py` — removes 2 leading spaces from all lines inside
  fused namespace blocks. Skips blank lines and preprocessor directives.
- `scripts/fix-orphan-braces.py` — earlier iteration, superseded by the fuse script
  improvements (can be deleted).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phases 2-6 — COMPLETED (session 28)

See session 28 summary above for details.

#### Phase 7 — std::optional (deferred)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 26 — FixedMatrix compile-time SIMD acceleration)

### Session 26 Summary

**Compile-time SIMD acceleration for FixedMatrix:**

FixedMatrix (`ICLMath/FixedMatrix.h`) handles fixed-size matrices (2x2, 3x3, 4x4)
used in hot paths (camera projection, scene graph transforms, rotation composition).
Previously only 4x4 float multiply had SIMD (SSE2/sse2neon). All other sizes and
double precision fell back to generic `std::inner_product` with strided column
iterators — no vectorization. The runtime BlasOps dispatch mechanism was too costly
for these tiny matrices.

**Solution: compile-time `#ifdef` selection with zero dispatch overhead.**

**Apple SIMD (`<simd/simd.h>`) on macOS:**
- All functions are inline `SIMD_CFUNC` — zero call overhead, native NEON
- Row-major/column-major compatibility solved with zero overhead:
  - 4x4/2x2: `memcpy` reinterpret (same byte size), swap multiply args
  - Matrix-vector: `simd_mul(v, A_cm)` = `A_rm * v`
  - det/inv: transpose-invariant, reinterpret works directly
- Covers: mult, inv for 4x4/2x2 float/double; det for 4x4/2x2; matvec for 4x4
- Element-wise ops (add, sub, scalar mul, negate, div) for 4x4/2x2
- inv checks determinant and throws SingularMatrixException (matching C++ semantics)
- Replaces existing SSE2/sse2neon specializations on macOS

**SSE2/sse2neon (non-macOS fallback):**
- Existing specializations for 4x4 float multiply + matvec
- On Linux x86, clang -O3 auto-vectorizes remaining C++ loops to SSE/AVX

**Benchmark results (Apple M-series, -O3, batch=128 independent ops):**

| Operation | float SIMD | float C++ | Speedup | double Speedup |
|---|---|---|---|---|
| 4x4 multiply | 1.8 ns | 8.6 ns | **4.8x** | **3.2x** |
| 4x4 * vec4 | 0.8 ns | 1.4 ns | **1.6x** | 1.0x |
| 4x4 inverse | 5.4 ns | 20.1 ns | **3.7x** | **2.1x** |
| 4x4 det | 2.0 ns | 2.2 ns | ~1x | ~1x |
| Full pipeline | 2.6 ns | 6.5 ns | **2.5x** | **1.7x** |

**Key findings from benchmarking:**
- 3x3 Apple SIMD is **10x slower** than C++ due to `simd_float3` padding overhead
  (48 vs 36 bytes for 3x3). Excluded — uses C++ closed-form instead.
- cblas/MKL is **25x slower** than C++ for 4x4 (~100ns call overhead for a 4ns op).
  Removed from FixedMatrix — only useful for DynMatrix via BlasOps.
- Element-wise ops (add, smul, det) show ~1x because clang -O3 auto-vectorizes
  the C++ loops to equivalent NEON/SSE code.
- `memcpy` for load/store compiles to identical assembly as `reinterpret_cast`
  at -O2. Kept for correctness (alignment not guaranteed by FixedArray).

**New files:**
- `ICLMath/src/ICLMath/SimdCompat.h` — Apple SIMD load/store helpers + element-wise
  ops (add, sub, smul) for 4x4/2x2 float/double
- `benchmarks/bench-fixedmatrix.cpp` — standalone benchmark (no ICL deps), supports
  Apple SIMD, SSE2 intrinsics, MKL cblas, and plain C++ backends
- `scripts/docker/` — Docker infrastructure for Linux testing:
  `docker-build.sh`, `docker-shell.sh`, `docker-test.sh`, `docker-bench-fixedmatrix.sh`
  Uses rsync + named volumes for fast incremental builds.

**Modified files:**
- `ICLMath/src/ICLMath/FixedMatrix.h`:
  - `#include <ICLMath/SimdCompat.h>`, `<type_traits>`, `<initializer_list>`
  - Apple SIMD specializations: mult (4), matvec (2), inv (4), det (4),
    element-wise ops in operator+/-/*/negate bodies
  - SSE2 block restructured: `#elif defined(ICL_HAVE_SSE2)` (non-macOS fallback)
  - New `std::initializer_list<T>` constructor
  - 3x3 inv/det always use C++ closed-form (even on macOS)
- `tests/test-math.cpp`: 12 new cross-validation tests

**ICP.cpp inner loop heap allocations eliminated:**
- Transform loop did `new DynMatrix`+`delete` per point per iteration — replaced
  with a single stack-allocated temp buffer reused across all points.
- `error()` function simplified to avoid DynMatrix temporaries (direct element access).

**Codebase scan for BlasOps/SIMD wiring — completed:**
Systematic scan of all modules found that remaining hand-written loops (VectorTracker
`eucl_dist`, MathFunctions `euclidian`, KMeans `dist`) are NOT worth wiring through
BlasOps: they operate on small runtime-sized vectors (2-10 elements) where BlasOps
dispatch overhead (~2ns) exceeds the computation itself. The compiler auto-vectorizes
these short loops at -O3. BlasOps is already wired where it matters (DynMatrix),
and SimdCompat/Apple SIMD is already wired where it matters (FixedMatrix 4x4/2x2).

**Image scaling dispatched via ImgOps with Accelerate backend:**
- `scaledCopyChannelROI` (the single worker for all image scaling) added to ImgOps
  as `Op::scaledCopy` with C++ and Accelerate backends.
- C++ implementation moved from Img.cpp to Img_Cpp.cpp (NN, bilinear, region-average).
  Also replaced 8x `new[]`/`delete[]` in RA mode with `std::vector`.
- New `Img_Accelerate.cpp`: vImageScale for icl8u (Planar8), icl16s (Planar16S),
  icl32f (PlanarF). Uses Lanczos resampling (superior to bilinear/RA).
  NN mode uses inline integer indexing. icl32s/icl64f fall back to C++.
- CMakeLists.txt: `_Accelerate.cpp` exclusion added for ICLCore.
- Benchmark results (640x480 single channel, -O3):

  | Operation | Accelerate | C++ | Speedup |
  |---|---|---|---|
  | NN downscale 8u | 46 us | 134 us | **2.9x** |
  | LIN downscale 8u | 47 us | 146 us | **3.1x** |
  | RA downscale 8u | 49 us | 180 us | **3.7x** |
  | LIN downscale 32f | 140 us | 167 us | **1.2x** |
  | LIN upscale 8u | 52 us | 583 us | **11.2x** |

**Tests: 384/384 pass (5 new scaling tests).** Build clean on macOS.

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phase 2 — Structured bindings, if-init, scoped_lock (~40 files, low risk)

**Structured bindings** (~40+ locations with `.first`/`.second`):
- `ICLIO/GenericGrabber.cpp:247-280` — `split_at_first()` result unpacking
- `ICLMarkers/FiducialDetectorPluginICL1.cpp:89-90` — pair unpacking
- `ICLPhysics/PhysicsPaper3.cpp:105-116,1656,1720` — map lookups, coords
- All map iterations using `it->first`/`it->second`

**If-with-initializer** (~25+ map-find-then-check patterns):
- `ICLMarkers/MarkerGridDetector.cpp:42-44` — `auto it = find(); if(it != end())`
- `ICLCore/Color.cpp:43-45` — color map lookup
- `ICLIO/FileGrabber.cpp:154-156` — plugin map lookup
- `ICLIO/DCDeviceFeatures.cpp:302-304` — feature map lookup
- `ICLGeom/PointCloudObjectBase.cpp:340-343` — metadata lookup
- `ICLPhysics/PhysicsPaper3ContextMenu.cpp:35,45` — action map
- `ICLQt/MultiDrawHandle.cpp:113-115` — name→index map
- `ICLUtils/SignalHandler.cpp:151,165,180` — handler maps

**std::scoped_lock** (replaces lock_guard, fixes manual lock/unlock):
- `ICLGeom/PointCloudCreator.cpp:125,294` — `lock_guard<recursive_mutex>` → `scoped_lock`
- `ICLQt/AbstractPlotWidget.cpp:110` — same
- `ICLFilter/GradientImage.cpp:29,42` — **manual lock()/unlock()** → RAII (bug risk)

**[[maybe_unused]]** (replaces `(void)variable` casts):
- `ICLGeom/Scene.cpp`, `ICLQt/GLImg.cpp`, `ICLGeom/PointCloudObjectBase.cpp`,
  `ICLGeom/CoplanarPointPoseEstimator.cpp`, `ICLIO/SharedMemoryGrabber.cpp`, etc.

#### Phase 3 — std::filesystem (~15 files, medium risk)

Biggest single win. Eliminates platform `#ifdef` blocks in FileList.cpp entirely.

**Core targets:**
- `ICLUtils/File.cpp:34-62` — `break_apart()` does manual `rfind('/')`,
  `rfind('.')`, `substr()` for dir/basename/suffix/filename. Replace with
  `path::parent_path()`, `stem()`, `extension()`. Special `.gz` double-extension
  handling needs care.
- `ICLUtils/File.cpp:110-119` — `file_exists()` and `file_is_dir()` use
  `struct stat`. Direct replacement with `std::filesystem::exists()`/`is_directory()`.
- `ICLUtils/File.cpp:590` — `File::erase()` uses C `remove()` →
  `std::filesystem::remove()`.
- `ICLIO/FileList.cpp:40-126` — Three platform-specific directory listing
  impls (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replace with
  `std::filesystem::directory_iterator`. Note: glob pattern matching still needed
  on top of directory_iterator (std::filesystem doesn't do glob natively).
- `ICLIO/V4L2Grabber.cpp:91-95,147-150` — string concatenation with `"/"`
  for device paths → `operator/`; `stat()` for device existence → `fs::exists()`.
- `ICLQt/Widget.cpp:703-705` — `QDir("/").mkpath()` →
  `std::filesystem::create_directories()`.
- `ICLIO/V4L2Grabber.cpp:434-435` — hardcoded `/tmp/` →
  `std::filesystem::temp_directory_path()`.

#### Phase 4 — std::from_chars / std::to_chars (~10 files, low risk)

Faster, locale-independent number parsing/formatting:
- `ICLQt/IntHandle.cpp:17` — `atoi()`
- `ICLQt/FloatHandle.cpp:17` — `atof()`
- `ICLQt/GUIDefinition.cpp` — multiple `atoi`/`atof`
- `ICLIO/FileGrabberPluginCSV.cpp:26,110,118` — CSV float parsing with `atof`
- `ICLIO/TestImages.cpp:130,424-427` — XPM parsing with `atoi`
- `ICLIO/JPEGDecoder.cpp:140,142` — timestamp/ROI parsing
- `ICLUtils/StringUtils.cpp:22-42` — `snprintf` for number→string

#### Phase 5 — if constexpr cleanup (~5 files, medium risk)

- `ICLUtils/BackendDispatching.h:268-327` — 6 `enable_if` SFINAE overloads for
  enum/integral key types → single template with `if constexpr(is_enum_v<K>)`
- `ICLMath/FixedMatrix.h:300-336` — 4 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in
  operator bodies → `if constexpr` with a constexpr flag
- Depth switch statements (~240 instances) — many could leverage the existing
  `dispatchEnum` fold-expression helper from EnumDispatch.h more broadly

#### Phase 6 — std::string_view (20+ files, higher risk — API change)

Changes function signatures. Start with leaf utilities (no virtual overrides):

**Leaf utilities (safe to change first):**
- `StringUtils.h` — `tok()`, `toLower()`, `toUpper()`, `parse<T>()`,
  `startsWith()`, `endsWith()`, `match()`
- `File.h` — `read_file()`, `read_lines()`, `write_file()`, constructors

**Map-lookup APIs (key parameter only reads):**
- `Configurable.h` — `prop()`, `addProperty()`, `getPropertyType/Info/Tooltip()`,
  `supportsProperty()`, `deactivateProperty()`
- `MultiTypeMap.h` — `allocValue()`, `getValue()`, `contains()`, `getType()`
- `ConfigFile.h` — `load()`, `save()`, `set()`, `check_type()`

**Caveat:** Functions that call `.c_str()` internally (fopen, etc.) need the string
materialized — string_view doesn't guarantee null termination.

#### Phase 7 — std::optional (10+ files, highest risk — API change)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 25 — NeighborhoodOp fix, dead IPP cleanup, Accelerate mapping)

### Session 25 Summary

**NeighborhoodOp even-mask IPP workaround removed:**
- `computeROI()` unconditionally shrunk output ROI by 1px for even-sized masks —
  this was an IPP-specific workaround (`#ifdef ICL_HAVE_IPP`) that had been
  applied unconditionally in the rewritten "NEW Code" path. The C++ backend
  has no anchor bug, so the shrink was incorrect (lost a valid row/column).
- Removed the workaround and cleaned up dead commented-out old code block.
- Added 3 even-kernel tests: 4x4 identity (verifies 7x7 output from 10x10),
  2x2 sum (verifies 7x7 from 8x8), 4x4 cross-validate across backends.

**Dead CONFIGURE_GTEST macro removed:**
- Removed the unused `CONFIGURE_GTEST` function from ICLHelperMacros.cmake
  (~67 lines). No module called it — tests migrated to centralized `tests/`
  directory in session 19. The per-module test stubs no longer exist.

**Dead `#if 0` IPP blocks removed (~1,000 lines):**
- 5 _Ipp.cpp files cleaned to TODO stubs with modern API + Accelerate notes:
  AffineOp_Ipp.cpp (57→16 lines), ConvolutionOp_Ipp.cpp (261→17),
  LUTOp_Ipp.cpp (35→14), MedianOp_Ipp.cpp (110→16), MorphologicalOp_Ipp.cpp (229→19)
- CannyOp.cpp: removed 22-line `#if 0` IPP block, kept C++ fallback
- IntegralImgOp.cpp: removed 40-line `ICL_HAVE_IPP_DEACTIVATED` block
  (C++ loop-unrolled version was already faster)
- CoreFunctions.cpp: removed 448-line SSE block (moved to PixelOps.cpp)
  and 45-line dead IPP histogram block

**NeighborhoodOp.h cleanup:**
- Removed stale `TODO:: check!!` and `TODO: later private` comments
- Fixed typos in class documentation (shrinked→shrunk, adaptROI→computeROI)

**Accelerate-IPP mapping reference created:**
- `claude.insights/accelerate-ipp-mapping.md` documents which IPP operations
  have Apple Accelerate (vImage) equivalents, with function names and limitations
- Good coverage: affine warp, convolution, histogram, basic morphology
- Gaps: no median filter, no Canny, no integral image in vImage

**Accelerate filter backends — ConvolutionOp, MorphologicalOp, AffineOp:**
- `ConvolutionOp_Accelerate.cpp` using `vImageConvolve_PlanarF` for icl32f
  - Handles even-sized kernels by padding to odd dimensions (vImage requires odd)
- `MorphologicalOp_Accelerate.cpp` using `vImageDilate/Erode_Planar8/PlanarF`
  for icl8u and icl32f (all 11 optypes: basic, 3x3, borderReplicate, composites)
  - Planar8: uses ICL's binary mask directly (unsigned char, nonzero=include)
  - PlanarF: converts binary mask to float kernel (0.0=include, -INF=exclude)
    because vImage PlanarF morphology uses additive structuring elements
  - Composite ops (open/close/tophat/blackhat/gradient) create sub-ops that
    dispatch through Accelerate for inner dilate/erode
- `AffineOp_Accelerate.cpp` using `vImageAffineWarp_Planar8/PlanarF` for
  icl8u and icl32f with bilinear interpolation; NN falls back to C++ inverse map
  - Maps ICL's 2x3 forward matrix to `vImage_AffineTransform` struct
  - Background fill with 0 for out-of-bounds pixels
- All registered as `Backend::Accelerate` (priority 6, above C++ fallback)
- Accelerate header included before ICL to avoid macOS Point/Size name conflicts
- ICLFilter CMakeLists.txt: `_Accelerate.cpp` excluded when `NOT ACCELERATE_FOUND`;
  Accelerate framework linked via 3RDPARTY_LIBS
- All cross-validated against C++ backend across all test cases

**Benchmark results (640x480 images, 50 iterations):**

| Benchmark | Accelerate | C++ | Speedup |
|---|---|---|---|
| convolution gauss3x3 32f | 235 us | 314 us | 1.3x |
| convolution gauss5x5 32f | 339 us | 9904 us | **29x** |
| morphology dilate3x3 8u | 243 us | 6477 us | **27x** |

**DynMatrixBase<bool> for non-float/double users:**
- Replaced `DynMatrix<bool>` → `DynMatrixBase<bool>` across 18 files (85 occurrences)
- DynMatrix<T> method bodies are out-of-line, only instantiated for float/double;
  DynMatrix<bool> only "worked" because callers used inherited DynMatrixBase methods
- DynMatrixBase<bool> is the correct type: fully header-only, all operations available
- Files: GraphCutter.h/.cpp, 12 ICLGeom segmentation files, PhysicsWorld.cpp

**SmartArray<T> replaced with std::shared_ptr<T[]>:**
- C++17 `shared_ptr<T[]>` provides native array semantics (operator[], delete[])
- Replaced all 38 SmartArray usage sites across 10 files
- Owning: `shared_ptr<T[]>(ptr)`. Non-owning: `shared_ptr<T[]>(ptr, [](T*){})`
- PlotWidget::Buffer changed from inheritance to composition (can't inherit shared_ptr)
- `SmartArray.h` deleted — zero remaining consumers
- Key files: Img.h/cpp (channel storage), Array2D.h, SOM.h, Scene.h, PlotWidget/LowLevelPlotWidget

**Debug-mode warnings fixed (zero remaining):**
- AffineOp.cpp: member initializer order
- BinaryArithmeticalOp_Simd.cpp: unused lambda capture
- DynVector.cpp: struct/class mismatch in explicit instantiation

**Accelerate vDSP arithmetic backends (UnaryArithmeticalOp + BinaryArithmeticalOp):**
- `UnaryArithmeticalOp_Accelerate.cpp`: vDSP_vsadd/vsmul (add/sub/mul/div),
  vDSP_vsq (sqr), vvsqrtf/vvlogf/vvexpf (sqrt/ln/exp), vDSP_vabs — for icl32f
- `BinaryArithmeticalOp_Accelerate.cpp`: vDSP_vadd/vsub/vmul/vdiv + vabs — for icl32f
- Scalar multiply benchmark: **5.7x speedup** over C++

**Full BLAS Layer (Level 1/2/3) in BlasOps:**
- Extended BlasOps with complete BLAS coverage via backend dispatch:
  - Level 3: gemm (existing)
  - Level 2: gemv (matrix-vector multiply) — new
  - Level 1: vadd/vsub/vmul/vdiv/vsadd/vsmul (existing), dot/nrm2/asum/axpy/scal (new)
- C++ fallbacks in BlasOps_Cpp.cpp, Accelerate backends (cblas + vDSP) in
  BlasOps_Accelerate.cpp — both float and double
- Cached inline dispatch: `BlasOps<T>::dot(a, b, n)` — function-local static
  resolves backend on first call, ~1-2ns overhead thereafter
- DynMatrix element-wise ops (operator+/-/*, elementwise_mult/div) wired through
  BlasOps dispatch instead of std::transform

**Accelerate-IPP mapping updated:**
- Deep-dive of all Accelerate APIs (vImage, vDSP, vForce, BNNS, simd)
- Tier 1-3 opportunities ranked by impact
- See `claude.insights/accelerate-ipp-mapping.md`

**Tests: 367/367 pass.** Build clean, zero warnings on macOS.

### Next Steps

- **Wire BlasOps through codebase** — grep for hand-written dot products, norm
  calculations, axpy-style accumulations, and matrix-vector multiplies across
  ICLMath, ICLGeom, ICLCV, ICLFilter; replace with `BlasOps<T>::dot/nrm2/axpy/gemv`
- **Image scaling Accelerate backend** — `vImageScale_Planar8/PlanarF` for
  `scaledCopyChannelROI()` in Img.cpp (needs backend dispatch added first)
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **C++17 source pass** — std::filesystem, std::string_view, structured bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs

## Previous State (Session 24 — LapackOps expansion, API cleanup, DynMatrixBase split)

### Session 24 Summary

**New LapackOps (3 new ops, 2 gap-fills):**
- `geqrf` + `orgqr` (QR factorization via Householder reflectors) — all 4 backends
- `gelsd` (SVD least-squares solve) — all 4 backends
- `getrf`/`getri` added to MKL and Eigen backends (were missing)

**Accelerate backend row-major fix:**
- getrf, getri, geqrf, orgqr, gelsd now transpose explicitly to/from column-major
  before calling LAPACK. Ensures packed output (L/U, Householder reflectors)
  matches C++ backend convention. gesdd/syev keep dimension-swap trick.

**Consumer wiring:**
- `decompose_QR()` → geqrf + orgqr (was hand-written Gram-Schmidt)
- `decompose_LU()` → getrf + unpack (was hand-written partial pivoting)
- `solve()` → gelsd directly (was string-based method dispatch with "lu"/"svd"/"qr"/"inv")
- `pinv()` → always reduced SVD + BLAS gemm (was bool useSVD with QR fallback)
- `mult()` → BLAS gemm for float/double (was hand-written inner_product loop)
- `matrix_mult_t()` → gemm transpose flags for float/double (no temp copies)

**API simplification:**
- Removed `big_matrix_pinv()` — merged into `pinv()`
- Removed `big_matrix_mult_t()` — merged into `matrix_mult_t()`
- Removed `pinv(bool useSVD)` parameter — always SVD
- Removed `solve(b, string method)` parameter — always gelsd
- Removed `solve_upper_triangular()` / `solve_lower_triangular()` — dead code
- Removed `PolynomialRegression::apply()` useSVD parameter
- Removed dead `#if 0` block in DynMatrixUtils.h (old svd_cpp_64f)
- Removed dead EigenICLConverter (zero callers)

**DynMatrixBase<T> refactor — compilation time reduction:**
- New `DynMatrixBase<T>` (header-only, ~290 lines): storage, element access,
  flat iterators, properties, operator<</>>
- `DynMatrix<T>` inherits DynMatrixBase, adds col/row iterators, DynMatrixColumn,
  all arithmetic and linalg — declarations only in header (~200 lines)
- All method bodies in `DynMatrix.cpp` (~700 lines), whole-class instantiation
  for float/double: `template class ICLMath_API DynMatrix<float/double>;`
- `DynVector.h` declarations only, `DynVector.cpp` with whole-class instantiation
- DynMatrix.h includes only `<iterator>` beyond ICL headers (was `<numeric>`,
  `<functional>`, `<vector>`, `<cmath>`, `<algorithm>`)
- Concatenation operators (operator,/%) moved out of line

**License header cleanup:**
- Replaced 29-line decorative headers with 3-line SPDX across 1038 files (~27,600 lines removed)
- Format: `SPDX-License-Identifier: LGPL-3.0-or-later` / `ICL - Image Component Library (URL)` / `Copyright (original authors)`
- New top-level LICENSE file with project info and EXC 277 acknowledgment

**Tests: 364/364 pass (15 new).** Build clean, zero warnings on macOS.

### LapackOps Summary (8 operations, 4 backends)

| Op | Signature | C++ | Accelerate | MKL | Eigen |
|---|---|---|---|---|---|
| gesdd | SVD divide-and-conquer | ✓ | ✓ | ✓ | ✓ |
| syev | Symmetric eigenvalue | ✓ | ✓ | ✓ | ✓ |
| getrf | LU factorization | ✓ | ✓ | ✓ | ✓ |
| getri | LU inverse | ✓ | ✓ | ✓ | ✓ |
| geqrf | QR factorization | ✓ | ✓ | ✓ | ✓ |
| orgqr | Form Q from reflectors | ✓ | ✓ | ✓ | ✓ |
| gelsd | SVD least-squares solve | ✓ | ✓ | ✓ | ✓ |
| gemm | Matrix multiply (BlasOps) | ✓ | ✓ | ✓ | — |

### DynMatrix File Layout

```
DynMatrixBase.h   — storage, element access, streaming (header-only, any type)
DynMatrix.h       — col/row iterators, arithmetic/linalg declarations (~200 lines)
DynMatrix.cpp     — all method bodies + whole-class instantiation float/double
DynVector.h       — DynColVector/DynRowVector declarations
DynVector.cpp     — all method bodies + whole-class instantiation float/double
```

## Previous State (Session 23 — Full Backend Dispatch Architecture)

### Session 23 Summary

**FFTOps_Cpp.cpp segfault fixed:**
- Root cause: bug in `fft2D_cpp` — `buf(src.rows(),src.cols())` was element access
  (operator()) on a null pointer, not resize. Fix: one-liner → `buf.setBounds(...)`.
- FFTOps_Cpp.cpp keeps non-owning wrappers for src/dst (zero-copy, no overhead).

**FFTUtils.cpp fully wired to FFTOps — 21 MKL blocks eliminated:**
- Generic `fft2D`/`ifft2D` templates dispatch through FFTOps with type conversion.
- FFTDispatching.h/.cpp deleted. All explicit MKL specializations removed.

**SVD deduplicated — svd_dyn dispatches through LapackOps:**
- Single generic `svd_dyn<T>` template, no float/double specializations.
- Handles LAPACK Vt → ICL V convention via post-dispatch transpose.

**GLImg.cpp — last active IPP block removed.**

**Dead code cleanup — ~1,600 lines removed total:**
- `#if 0` blocks in FFTUtils.cpp, DynMatrixUtils.cpp, DynMatrix.cpp deleted.
- PThreadFix.h stripped to empty header.
- `jacobi_iterate_vtk` + `find_eigenvectors` removed from DynMatrix.cpp (moved to
  LapackOps_Cpp.cpp as `cpp_syev`).
- `get_minor_matrix` removed (inv() no longer uses cofactor expansion).

**Apple Accelerate backend:**
- CMake auto-detects on macOS via `find_library(Accelerate)`.
- `BlasOps_Accelerate.cpp`: cblas_sgemm/dgemm.
- `FFTOps_Accelerate.cpp`: vDSP_DFT row-column decomposition (arbitrary sizes).
- `LapackOps_Accelerate.cpp`: sgesdd/dgesdd, ssyev/dsyev, sgetrf/dgetrf, sgetri/dgetri.

**LapackOps dispatcher — clean BLAS/LAPACK separation:**
- `LapackOps.h`: `enum LapackOp { gesdd, syev, getrf, getri }`.
- `LapackOps.cpp`: constructor, instance(), toString().
- gesdd moved from BlasOps to LapackOps (all backends updated, all consumers rewired).
- BlasOps now contains only GEMM.

**LapackOps backends:**
- `LapackOps_Cpp.cpp`: gesdd (Golub-Kahan), syev (Jacobi), getrf (LU), getri (LU inverse).
- `LapackOps_Accelerate.cpp`: gesdd, syev, getrf, getri via Apple LAPACK.
- `LapackOps_Mkl.cpp`: gesdd, syev via MKL LAPACK.
- `LapackOps_Eigen.cpp`: gesdd (JacobiSVD), syev (SelfAdjointEigenSolver) via Eigen3.

**Backend enum expanded:**
- Added `Backend::Eigen = 3` between OpenBlas and Ipp.
- Priority order: Cpp(0) < Simd(1) < OpenBlas(2) < Eigen(3) < Ipp(4) < FFTW(5)
  < Accelerate(6) < Mkl(7) < OpenCL(8).

**Consumer wiring:**
- `svd_dyn()` → `LapackOps<T>::gesdd`
- `big_matrix_pinv()` → `LapackOps<T>::gesdd` + `BlasOps<T>::gemm`
- `DynMatrix::eigen()` → `LapackOps<T>::syev` (resolveOrThrow, C++ fallback always available)
- `DynMatrix::inv()` → `LapackOps<T>::getrf` + `getri` (O(n³) LU, replaces O(n!) cofactor)
- `DynMatrix::det()` for n>4 → `LapackOps<T>::getrf` (product of U diagonal, replaces O(n!) cofactor)

**C++ fallback convention enforced:**
- All C++ backend registrations now in `_Cpp.cpp` files.
- MathOps C++ backends moved from DynMatrixUtils.cpp → `MathOps_Cpp.cpp`.
- ImgOps channelMean moved from CoreFunctions.cpp → `Img_Cpp.cpp`.
- Only exception: ImgBorder.cpp (depends on file-local `_copy_border`, documented).

**Design decisions:**
- BlasOps for GEMM only; element-wise ops stay inlined (bandwidth-bound).
- FixedMatrix (4x4): keep inlined, IPP ippm dropped, compiler auto-vectorization sufficient.
- det() keeps special cases for 1x1–4x4 (zero overhead); n>4 uses LU.

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Add more LapackOps** — geqrf (QR factorization) to accelerate decompose_QR(), solve()
- **Investigate legacy test stubs** — Per-module test executables compile to 0 tests
- **NeighborhoodOp.cpp** — Two `#ifdef ICL_HAVE_IPP` workaround blocks (anchor bug)

## Previous State (Session 22 — BlasOps/FFTOps + BackendDispatching Refactor + BLAS Consumer Wiring)

### Session 22 Summary (continued)

**New BLAS/FFT abstraction layer + BackendDispatching refactor.**

**BackendDispatching refactored: array → sorted vector:**
- `std::array<ImplPtr, NUM_BACKENDS>` replaced with sorted `std::vector<Entry>`
- Backends sorted by priority descending (highest first); only registered backends occupy space
- `NUM_BACKENDS` constant removed entirely — enum can grow freely
- New `resolve()` / `resolveOrThrow()` no-arg overloads for dummy-context singletons
- New `Entry` struct: `{ Backend backend; shared_ptr<ImplBase> impl; }`
- `setImpl()` private helper maintains sorted order on insertion

**Backend enum expanded (values encode priority):**
```
Cpp=0, Simd=1, OpenBlas=2, Ipp=3, FFTW=4, Accelerate=5, Mkl=6, OpenCL=7
```

**BlasOps\<T\> — BLAS/LAPACK abstraction (new):**
- `BlasOps.h` — singleton template, `enum class BlasOp { gemm, gesdd }`
- `BlasOps.cpp` — constructor (addSelector), instance(), toString()
- `BlasOps_Cpp.cpp` — C++ fallback: naive GEMM + SVD via existing `svd_dyn`
- `BlasOps_Mkl.cpp` — MKL backend: `cblas_sgemm/dgemm` + `sgesdd/dgesdd`
- Raw pointer interface (no DynMatrix dependency in signatures)
- Consumer code will use `resolveOrThrow()` — C++ fallback always available

**FFTOps\<T\> — FFT abstraction (new, replaces FFTDispatching):**
- `FFTOps.h` — singleton template, `enum class FFTOp { r2c, c2c, inv_c2c }`
- `FFTOps.cpp` — constructor, instance(), toString()
- `FFTOps_Cpp.cpp` — C++ fallback via existing `fft2D_cpp` / `ifft2D_cpp`
- `FFTOps_Mkl.cpp` — MKL DFTI backend (forward/inverse, real/complex)
- Raw pointer interface; DynMatrix wrapping stays in consumers
- `FFTOps<float>` + `FFTOps<double>` — separate singletons per precision

**FFTDispatching transitional rename:**
- `FFTOp` enum → `LegacyFFTOp` to avoid conflict with FFTOps
- FFTDispatching struct kept temporarily until FFTUtils.cpp is rewired
- Will be deleted once FFTUtils.cpp uses FFTOps directly

**Convention: C++ fallbacks in `_Cpp.cpp` files:**
- All backend implementations (including C++ fallback) go in `_<Backend>.cpp` files
- The `.cpp` file (e.g., `BlasOps.cpp`) only contains constructor, instance(), toString()
- Pattern: `BlasOps_Cpp.cpp`, `BlasOps_Mkl.cpp`, `FFTOps_Cpp.cpp`, `FFTOps_Mkl.cpp`

**CMake: new exclusion patterns in ICLMath/CMakeLists.txt:**
- `_FFTW.cpp` excluded when `!FFTW_FOUND`
- `_Accelerate.cpp` excluded when `!ACCELERATE_FOUND`
- `_OpenBlas.cpp` excluded when `!OPENBLAS_FOUND`

**BLAS consumers wired (DynMatrix + DynMatrixUtils):**
- `DynMatrix.cpp`: `big_matrix_pinv` now uses `BlasOps<T>::gesdd` + `gemm` via
  `resolveOrThrow()`. No fallback logic — C++ backend always available.
- `DynMatrixUtils.cpp`: `big_matrix_mult_t` now uses `BlasOps<T>::gemm` via
  `resolveOrThrow()`. MKL explicit specializations removed.
- All `#ifdef ICL_HAVE_MKL` removed from DynMatrix.h, DynMatrix.cpp, DynMatrixUtils.cpp (6 blocks).
- `BlasOps_Cpp.cpp` GEMM optimized: compile-time dispatch of transA/transB/alpha/beta
  via template specialization + `if constexpr` (eliminates inner-loop conditionals).
  Epsilon-based `isZero()`/`isOne()` for float-safe alpha/beta comparison.
- `BlasOps_Cpp.cpp` SVD: full Golub-Kahan bidiagonalization (`svd_bidiag`) moved here
  from DynMatrixUtils.cpp. C++ GESDD backend always works (no more stub returning -1).

**FFT consumer wiring attempted but reverted (segfault):**
- Rewrote `fft2D<T1,T2>` and `ifft2D<T1,T2>` generic templates to use FFTOps dispatch.
  All 19 explicit specializations with `#ifdef ICL_HAVE_MKL` eliminated (replaced by
  template instantiations). The generic template handles type conversion + dispatch.
- **Segfault in multi-threaded tests.** Root cause: `FFTOps_Cpp.cpp` wraps raw pointers
  in non-owning `DynMatrix(cols, rows, ptr, false)` and passes to `fft2D_cpp`. The FFT
  row-column decomposition writes to `dst` with transposed dimensions — the non-owning
  wrapper's buffer may be too small or have wrong stride. Fix: use owned DynMatrix
  intermediates in the C++ backend, copy result back to raw pointer at the end.
- **Changes reverted** to keep branch green. FFTUtils.cpp still has 21 MKL blocks.

**Remaining `ICL_HAVE_MKL` references:**
- FFTUtils.cpp: 21 blocks (2 wrapper blocks + 19 specialization blocks, all still active)
- Camera.cpp: 1 (dead comment)
- PThreadFix.h: 1 (dead, inside `#if 0`)
- ICLConfig.h.in / doxyfile.in: build system definitions (must stay)

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Fix FFTOps_Cpp.cpp segfault** — Use owned DynMatrix in C++ FFT backend instead of
  non-owning wrappers. The issue is `fft2D_cpp`'s row-column decomposition writes `dst`
  with transposed buffer layout. Either: (a) allocate an owned DynMatrix, call fft2D_cpp,
  copy result to raw pointer; or (b) rewrite the C++ FFT backend to work directly on
  raw pointers without going through fft2D_cpp.
- **Wire FFTUtils.cpp to FFTOps** — The generic template rewrite is ready (tested
  single-threaded), just needs the C++ backend fix above. Once fixed, 21 MKL blocks
  disappear and FFTDispatching.h/.cpp can be deleted.
- **Deduplicate SVD** — `svd_internal` now lives in both `BlasOps_Cpp.cpp` and
  `DynMatrixUtils.cpp`. Wire `svd_dyn()` to dispatch through `BlasOps<T>::gesdd`,
  then remove the old copy from DynMatrixUtils.cpp.
- **Camera.cpp** — Remove dead MKL comment (trivial)
- **GLImg.cpp** — Last active `#ifdef ICL_HAVE_IPP` block (min/max, could use ImgOps)
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Consider LapackOps** — Separate dispatch for LAPACK operations (eigenvalue
  decomposition, Cholesky, LU, etc.) beyond just GESDD
- **FixedMatrix BLAS** — For small matrices (commonly 4x4), BlasOps dispatch overhead
  is too high. Keep inlined implementations. IPP's `ippm` module (small matrix ops) was
  dropped from modern IPP. Best choice for small matrices: compiler auto-vectorization
  of the inlined code, or hand-written SIMD for the hot 4x4 case.

## Previous State (Session 21 — A2 + B + C/IPP Complete)

### Session 21 Summary

**A2 complete, B complete, C/IPP complete for all modules.** All active
`#ifdef ICL_HAVE_IPP` operational code across ICLUtils/ICLCore/ICLMath/
ICLFilter/ICLIO has been migrated to dispatch or removed with TODOs.

**A2 — all three phases:**

**Phase 1 — Global string registry removed:**
- Deleted `detail::RegistryEntry`, `detail::globalRegistry()`, `detail::addToRegistry()`
- Deleted `registerBackend()`, `registerStatefulBackend()` static methods
- Deleted `loadFromRegistry()` private method
- Deleted `BackendDispatching.cpp` (only contained the registry impl)
- Removed `m_selectorByName` map and `m_prefix` string member
- Removed `initDispatching()` — all 15 filters, ImgOps, FFTDispatching updated
- Removed string-keyed `addSelector(string)`, `getSelector(string)`, `selectorByName(string)`
- Enum-keyed `addSelector(K)` is now standalone (no delegation to string version)
- Added `selector(K)` returning `BackendSelectorBase*` (replaces `selectorByName` for tests)
- Fixed `ThresholdOp_Simd.cpp` — switched from global registry to prototype direct registration

**Phase 2 — Stateful backend cloning (factory pattern):**
- `ImplBase` now has `std::function<shared_ptr<ImplBase>()> cloneFn` (null for stateless)
- `BackendSelector::clone()`: calls `cloneFn()` for stateful backends, shares `shared_ptr` for stateless
- `BackendSelector::addStateful(b, factory, applicability, desc)` — factory called per clone
- `BackendDispatching::addStatefulBackend<Sig>(key, b, factory, app, desc)` — convenience
- Migrated 4 stateful backends:
  - `WienerOp_Ipp.cpp` — IPP scratch buffer now per-instance
  - `WarpOp_OpenCL.cpp` — CLWarpState (GPU buffers/kernels) now per-instance
  - `BilateralFilterOp_OpenCL.cpp` — CLBilateralState now per-instance
  - `MorphologicalOp_Ipp.cpp` — MorphIppState (IPP state objects) now per-instance

**Phase 3 — AffineOp test bug fixed:**
- `AffineOp_Cpp.cpp`: bilinear interpolation bounds check now correctly validates
  the full 2x2 neighborhood (`x2 < width-1, y2 < height-1`)
- Edge pixels outside the safe bilinear zone fall back to nearest-neighbor
- **349/349 tests pass both single-threaded AND multi-threaded** (SIGTRAP eliminated)

**FFTDispatching migrated to enum pattern:**
- Added `enum class FFTOp : int { fwd32f, inv32f, fwd32fc }` with `toString()`
- Selector setup moved to `FFTDispatching()` constructor
- `FFTUtils.cpp` now uses `getSelector<Sig>(FFTOp::xxx)` (enum-keyed O(1))

**Test changes:**
- All `selectorByName("name")` calls replaced with `selector(FilterOp::Op::name)`

**B: ICLCore IPP blocks — all active blocks migrated:**
- `CCFunctions.cpp`: `planarToInterleaved`/`interleavedToPlanar` IPP specializations
  extracted to `Img_Ipp.cpp` as ImgOps selectors. Generic template now dispatches
  through ImgOps for same-type cases (S==D), falling back to `_Generic` if no backend.
  The `#ifdef` block + conditional instantiations removed. Two new `ImgOps::Op` values added.
- `BayerConverter.h/.cpp`: Removed dead IPP code. `nnInterpolationIpp()` was defined but
  never called (the `apply()` method always calls `nnInterpolation()` instead).
  `m_IppBayerPattern` member and constructor switch statements removed.
- Only `Types.h` retains compile-time `ICL_HAVE_IPP` guards (enum definitions, stays).

**BackendProxy for terser registration:**
- Added `BackendProxy` struct + `backends(Backend b)` method on `BackendDispatching`
- All ~65 `addBackend`/`addStatefulBackend` call sites across _Cpp/_Ipp/_Simd/_OpenCL files
  migrated to use the proxy: `auto cpp = proto.backends(Backend::Cpp); cpp.add<Sig>(...);`
- Removed `addBackend`/`addStatefulBackend` as public API — proxy calls getSelector().add() directly

**C/ICLMath — MathOps dispatch framework + all IPP removed:**
- Created `MathOps<T>` template singletons (`MathOps<float>`, `MathOps<double>`)
  using `BackendDispatching<int>` (dummy context, no applicability checks)
- `enum class MathOp` with 11 selectors: mean, var, meanvar, min, max, minmax,
  unaryInplace, unaryCopy, unaryConstInplace, unaryConstCopy, binaryCopy
- Sub-operation enums: `UnaryMathFunc` (12 ops), `UnaryConstFunc` (5 ops), `BinaryMathFunc` (6 ops)
- `DynMatrixUtils.cpp`: entire `#ifdef ICL_HAVE_IPP` block (lines 80-548) replaced with
  MathOps dispatch + C++ backend registration
- `DynMatrix.h`: removed IPP specializations (sqrDistanceTo, distanceTo, elementwise_div,
  mult-by-scalar, norm), added TODO
- `MathFunctions.h`: removed IPP mean specializations, added TODO
- `FixedMatrix.h`: removed unused `#include <ipp.h>`
- `CMakeLists.txt`: added `_Ipp.cpp`/`_Mkl.cpp` exclusion patterns

**C/ICLIO — all IPP removed:**
- `DC.cpp`: removed `ippiRGBToGray_8u_C3C1R`, always use weighted-sum loop
- `ColorFormatDecoder.cpp`: removed `ippiYUVToRGB_8u_C3R` + `ippiCbYCr422ToRGB_8u_C2C3R`
- `PylonColorConverter.h/.cpp`: removed `Yuv422ToRgb8Icl` and `Yuv422YUYVToRgb8Icl`
  IPP-only classes, always use PylonColorToRgb fallback
- TODOs added at every removal site

### Remaining `ICL_HAVE_IPP` References

Only these remain in the codebase:

| Category | Files | Status |
|---|---|---|
| Type definitions | BasicTypes.h, Size.h, Point.h, Rect.h, Types.h | Compile-time aliases, must stay |
| `#if 0` dead code | 5 *_Ipp.cpp files, FFTUtils (2), CannyOp, ImageRectification, CoreFunctions.cpp (histogramEven), DynMatrixUtils.cpp (ippm matrix ops) | Disabled, contains code for future re-enablement |
| IPP bug workaround | NeighborhoodOp.cpp (2 blocks) | Minor, stays |
| Qt GPU upload | GLImg.cpp (1 block) | ICLQt, deferred |
| Comments | CCFunctions.cpp (2 `/* */` blocks), DynMatrix.cpp (`#if 1 // was:`), DynMatrixUtils.cpp (SVD comment), IntegralImgOp.cpp (`ICL_HAVE_IPP_DEACTIVATED_...`) | Dead |
| CMake / config | CMakeLists.txt, ICLConfig.h.in, doxyfile.in | Build system definitions |
| Dead (nested `#if 0`) | PThreadFix.h | Inside outer `#if 0`, unreachable |

### Future IPP Optimization Opportunities (TODOs in code)

| Location | IPP Function | What It Accelerates |
|---|---|---|
| BayerConverter.cpp | `ippiCFAToRGB_8u_C1C3R` | Bayer → RGB nearest-neighbor |
| DynMatrix.h | `ippsNormDiff_L2`, `ippsDiv`, `ippsMulC`, `ippsNorm_L1/L2` | Matrix distance, element-wise div, scalar mult, norms |
| MathFunctions.h | `ippsMean_32f/64f` | Scalar mean over float/double arrays |
| DynMatrixUtils.cpp | All math ops already dispatched | Re-enable via `DynMatrixUtils_Ipp.cpp` |
| DC.cpp | `ippiRGBToGray_8u_C3C1R` | RGB → grayscale conversion |
| ColorFormatDecoder.cpp | `ippiYUVToRGB_8u_C3R`, `ippiCbYCr422ToRGB_8u_C2C3R` | YUV → RGB color conversion |
| PylonColorConverter.cpp | `ippiCbYCr422ToRGB_8u_C2C3R`, `ippiYCbCr422ToRGB_8u_C2C3R` | Pylon YUV422 → RGB |
| ConvolutionOp_Ipp.cpp | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | 34 Sobel/Gauss/Laplace specializations |
| MorphologicalOp_Ipp.cpp | `ippiDilate/Erode_*_C1R_L` + spec | Modern morphology with border |
| AffineOp_Ipp.cpp | `ippiWarpAffineNearest/Linear_*` + spec | Affine warp |
| MedianOp_Ipp.cpp | `ippiFilterMedianBorder_*_C1R` | Median filter with border |
| LUTOp_Ipp.cpp | `ippiReduceBits` (new signature) | Bit reduction |
| CannyOp.cpp | Modern `ippiCanny` with border spec | Canny edge detection |

### Next Steps

- **Wire consumers to BlasOps/FFTOps** — Replace 27 `#ifdef ICL_HAVE_MKL` blocks
  in DynMatrix.cpp, DynMatrixUtils.cpp, FFTUtils.cpp with BlasOps/FFTOps dispatch.
  Remove MKL includes, delete FFTDispatching.h/.cpp and LegacyFFTOp.
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Re-enable disabled IPP backends** — update to modern oneAPI APIs (see table above)
- **GLImg.cpp** — 1 remaining `ICL_HAVE_IPP` block in ICLQt
- **NeighborhoodOp.cpp** — 2 IPP bug workaround blocks (minor)
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD vs MKL comparison

### Key Files (Updated)

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework (sorted vector storage)
ICLMath/src/ICLMath/BlasOps.h                  — BLAS/LAPACK dispatch singleton
ICLMath/src/ICLMath/BlasOps.cpp                — constructor, instance, toString
ICLMath/src/ICLMath/BlasOps_Cpp.cpp            — C++ fallback (naive GEMM + svd_dyn SVD)
ICLMath/src/ICLMath/BlasOps_Mkl.cpp            — MKL backend (cblas_xgemm + xgesdd)
ICLMath/src/ICLMath/FFTOps.h                   — FFT dispatch singleton
ICLMath/src/ICLMath/FFTOps.cpp                 — constructor, instance, toString
ICLMath/src/ICLMath/FFTOps_Cpp.cpp             — C++ fallback (row-column FFT)
ICLMath/src/ICLMath/FFTOps_Mkl.cpp             — MKL DFTI backend
ICLMath/src/ICLMath/FFTDispatching.h           — TRANSITIONAL (LegacyFFTOp, delete after wiring)
ICLMath/src/ICLMath/FFTDispatching.cpp         — TRANSITIONAL (delete after wiring)
```

## Previous State (Session 20 — All Filters Migrated to Prototype+Clone)

### Session 20 Summary

**All 15 filters now use the prototype+clone pattern.** The global string
registry is no longer needed for filter dispatch.

Migrated the remaining 14 filters (ThresholdOp was already done in session 19):

| Filter | Selectors | Backend Files | Notes |
|---|---|---|---|
| WienerOp | apply | _Cpp, _Ipp | Now always built (removed IPP-only exclusion from CMakeLists) |
| LUTOp | reduceBits | _Cpp, _Ipp | Two constructors, both clone from prototype |
| AffineOp | apply | _Cpp, _Ipp | C++ uses FixedMatrix for inverse transform |
| ConvolutionOp | apply | _Cpp, _Ipp | Large dispatch chain moved to _Cpp.cpp |
| MorphologicalOp | apply | _Cpp, _Ipp | Buffer accessors added for _Cpp.cpp access |
| BilateralFilterOp | apply | _Cpp, _OpenCL | C++ bilateral filter (all depths) |
| BinaryLogicalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryArithmeticalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryCompareOp | compare, compareEqTol | _Cpp, _Simd | 2 selectors |
| WarpOp | warp | _Cpp, _Ipp, _OpenCL | 3 backend files |
| MedianOp | fixed, generic | _Cpp, _Ipp, _Simd | Sorting networks + Huang median in _Cpp |
| UnaryArithmeticalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryLogicalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryCompareOp | compare, compareEqTol | _Cpp, _Ipp, _Simd | 2 selectors |

**Pattern for each migrated filter:**
1. **Header** — `enum class Op : int { ... }`, `static prototype()`,
   `toString(Op)` declaration with `ICLFilter_API`
2. **_Cpp.cpp** — C++ backend implementations, registers via
   `prototype().addBackend<Sig>(Op::x, Backend::Cpp, fn, desc)`
3. **_Ipp/_Simd/_OpenCL.cpp** — same registration pattern into prototype
4. **.cpp** — constructor is `ImageBackendDispatching(prototype())`,
   `apply()` uses `getSelector<Sig>(Op::x)` (enum-indexed, O(1))
5. `toString(Op)` free function for ADL (used by `addSelector(K)`)

**CMake change:** WienerOp is no longer excluded when `!IPP_FOUND`. Its C++
backend throws an exception (IPP required), but the class is always available.

**MorphologicalOp special handling:** The C++ backend accesses private
composite-operation buffers. Added public buffer accessors
(`openingBuffer()`, `gradientBuffer1()`, `gradientBuffer2()`) so the free
function in `_Cpp.cpp` can use them via the `MorphologicalOp&` parameter.

**14 new _Cpp.cpp files created:**
```
ICLFilter/src/ICLFilter/WienerOp_Cpp.cpp
ICLFilter/src/ICLFilter/LUTOp_Cpp.cpp
ICLFilter/src/ICLFilter/AffineOp_Cpp.cpp
ICLFilter/src/ICLFilter/ConvolutionOp_Cpp.cpp
ICLFilter/src/ICLFilter/MorphologicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BilateralFilterOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryCompareOp_Cpp.cpp
ICLFilter/src/ICLFilter/WarpOp_Cpp.cpp
ICLFilter/src/ICLFilter/MedianOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryCompareOp_Cpp.cpp
```

**Test results:** 349/349 pass (single-threaded). Multi-threaded test crash
is a **pre-existing bug** in the AffineOp test: heap-buffer-overflow in
`Img<icl8u>::subPixelLIN()` when bilinear-interpolating a 2×2 image
(ASAN confirmed at `AffineOp_Cpp.cpp:45`, `test-filter.cpp:745`).

### Known Issue: Stateful Backend Sharing

With the prototype+clone pattern, `ImplBase` objects are shared across all
instances via `shared_ptr`. This is correct for stateless backends (free
functions), but **stateful backends share mutable state** across instances:

| Backend | Shared State | Impact |
|---|---|---|
| WienerOp_Ipp.cpp | IPP scratch buffer (`vector<icl8u>`) | Concurrent calls corrupt buffer |
| WarpOp_OpenCL.cpp | `CLWarpState` (GPU buffers/kernels) | Concurrent calls corrupt GPU state |
| BilateralFilterOp_OpenCL.cpp | `CLBilateralState` | Same |
| MorphologicalOp_Ipp.cpp | `MorphIppState` (IPP state objects) | Same |

**Fix needed:** Add a factory/creator pattern to `ImplBase` so that stateful
backends can create fresh state per clone. The `BackendSelector::clone()`
should call `impl->cloneImpl()` instead of copying the `shared_ptr`:

```
Option A: virtual cloneImpl() on ImplBase
  - Stateless impls: return shared_from_this() (share as before)
  - Stateful impls: call factory, return new Impl with fresh state

Option B: Store optional factory lambda on ImplBase
  - add() takes optional Factory parameter
  - clone() calls factory if present, else shares shared_ptr
  - Backends register: proto.addStatefulBackend<Sig>(key, backend,
      factory, applicability, desc)
  - The factory lambda creates fresh state each time
```

**Important:** If the state contains `Image` or `Img<T>` members, the factory
must `deepCopy()` them — ICL images use shallow copy by default.

## Previous State (Session 19 — Full ImgOps Dispatch Migration)

### Session 19 Summary

**Img.cpp + Img.h now have zero `#ifdef ICL_HAVE_IPP` blocks.**

Migrated all 7 remaining IPP-guarded operations in Img.cpp/Img.h to the
ImgOps BackendDispatch framework (Img_Cpp.cpp / Img_Ipp.cpp):

| Operation | IPP Functions | Depths |
|---|---|---|
| clearChannelROI | `ippiSet_*_C1R` | 8u, 16s, 32s, 32f |
| lut | `ippiLUTPalette_8u_C1R` | 8u |
| getMax | `ippiMax/MaxIndx_*_C1R` | 8u, 16s, 32f |
| getMin | `ippiMin/MinIndx_*_C1R` | 8u, 16s, 32f |
| getMinMax | `ippiMinMax/MinMaxIndx_*_C1R` | 8u, 32f |
| normalize | `ippiMulC/AddC_32f_C1IR` | 32f |
| flippedCopyChannelROI | `ippiMirror_*_C1R` | 8u, 32f |

**Key design details:**
- **Type-erased dispatch signatures** — operations that return typed values
  (getMax, getMin) use `icl64f` return through dispatch, cast back at call site
- **const methods** — `getMax`/`getMin`/`getMinMax`/`lut` use `const_cast` to
  pass `const ImgBase*` through `ImgBase*` dispatch context (safe: resolve
  only checks depth, backends don't modify source)
- **clearChannelROI** — dispatch at `Img<T>::clear()` level, not in the header
  template itself (avoids circular include: Img.h→ImgOps.h→Image.h→Img.h).
  Direct callers of `clearChannelROI<T>()` get the C++ path.
- **Mirror helpers moved to Img_Cpp.cpp** — `getPointerOffset`,
  `getMirrorPointerOffsets`, `getMirrorPointers`, plus the per-channel
  `Img<T>::mirror(axis, int, Point, Size)` definition (with explicit
  instantiations). These were only used by mirror and flippedCopy.
- **scaledCopyChannelROI** `#if 0` dead code cleaned up — just uses
  `ICL_INSTANTIATE_ALL_DEPTHS` now (IPP APIs deprecated, TODO for future)
- **`getStartIndex`/`getEndIndex`** are protected on `Img<T>` — backends
  inline the logic: `startC = ch < 0 ? 0 : ch`

**New dispatch signatures in ImgOps:**
```
ClearChannelROISig  = void(ImgBase&, int ch, icl64f val, const Point& offs, const Size& size)
LutSig             = void(ImgBase& src, const void* lut, ImgBase& dst, int bits)
GetMaxSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinMaxSig       = void(ImgBase&, int ch, icl64f* minVal, icl64f* maxVal, Point* minCoords, Point* maxCoords)
NormalizeSig       = void(ImgBase&, int ch, icl64f srcMin, icl64f srcMax, icl64f dstMin, icl64f dstMax)
FlippedCopySig     = void(axis, ImgBase& src, int srcC, const Point& srcOffs, const Size& srcSize,
                          ImgBase& dst, int dstC, const Point& dstOffs, const Size& dstSize)
```

**BackendDispatching rewrite (also this session):**
- `map<Backend,ImplPtr>` → `array<shared_ptr<ImplBase>, 4>` (fixed array, shared for cloning)
- Removed `backendPriority[]` — reverse iteration over enum values
- `BackendSelector` un-nested from `BackendDispatching` (standalone template)
- `resolve`/`resolveOrThrow`/`get` now const
- Added `virtual clone()` + clone constructor for per-instance dispatch
- Enum-keyed `addSelector(K)` with index assertion + ADL `toString(K)`
- Enum-keyed `getSelector<Sig>(K)` — O(1) vector index
- ImgOps backends register directly into singleton (no global registry)
- ThresholdOp migrated as proof of concept for prototype+clone pattern:
  all backends in `_Cpp.cpp` / `_Ipp.cpp`, constructor just clones prototype
- Removed dead code (`callWith`, `qualifiedName`, `backendPriority[]`)
- CoreFunctions `channel_mean` + ImgBorder `replicateBorder` migrated to ImgOps

**CMake note:** `FILE(GLOB)` is evaluated at configure time. After adding new
`_Cpp.cpp` / `_Ipp.cpp` files, re-run `cmake ..` to pick them up.

### Previous Session Summary (Session 18)

**Docker IPP build — now green:**
- Fixed `ContourDetector.cpp` missing `#include <cstring>`
- Fixed `CornerDetectorCSS.cpp` removed `ippsConv_32f` (deprecated, use C++ fallback)
- Fixed `CV.cpp` removed `ippiCrossCorrValid_Norm` / `ippiSqrDistanceValid_Norm` (deprecated)
- Fixed `TemplateTracker.h` missing `#include <ICLUtils/Point32f.h>`
- Fixed `DataSegment.h` missing `#include <cstring>`
- Fixed `ICLMarkers/CMakeLists.txt` spurious Qt PCH headers
- Fixed `FiducialDetectorPluginART.cpp` dead `Quick.h` include
- Fixed `FiducialDetectorPluginICL1.cpp` guarded `Quick.h`, added proper includes
- Fixed `ProximityOp.cpp` — provided stub implementations (was entirely inside `#if 0`)
- All modules compile and tests pass on Linux/IPP (Docker) and macOS

**Incremental Docker builds:**
- `build-and-test.sh` now uses `rsync` (preserves timestamps) instead of `cp -a`
- Use named Docker volume for persistent build cache
- Dockerfile adds `rsync` package

**ICLFilter IPP migration — complete:**
- Extracted `WarpOp` inline IPP (`ippiRemap_8u/32f_C1R`) to `WarpOp_Ipp.cpp` as `Backend::Ipp`
- Removed `#ifdef ICL_HAVE_IPP` from NeighborhoodOp.cpp (anchor workaround now always-on)
- Removed `#ifdef ICL_HAVE_IPP` from LocalThresholdOp.cpp (C++ path is both faster and higher quality)
- Removed `#ifdef ICL_HAVE_IPP` from UnaryOp.cpp (Canny creator — works without IPP now)
- Removed redundant `#ifdef ICL_HAVE_IPP` guards from all `_Ipp.cpp` files (CMake already excludes them)
- ICLFilter now has zero active `#ifdef ICL_HAVE_IPP` in non-backend files

**ImgOps singleton + ImgBaseBackendDispatching (new this session):**

Established the pattern for migrating `Img<T>` utility functions to dispatched backends.
First operation migrated: **mirror**.

Key design decisions:
- **ImgBaseBackendDispatching** (`BackendDispatching<ImgBase*>`) — new dispatch context for
  `Img<T>` methods, so they can dispatch via `this` without constructing an `Image` wrapper
- **ImgOps singleton** — inherits from `ImgBaseBackendDispatching`, owns `BackendSelector`s
  for Img utility operations (mirror, and later: min/max, lut, normalize, etc.)
- **ALL implementations in separate files** — `Img_Cpp.cpp` has the C++ fallback,
  `Img_Ipp.cpp` has the IPP backend. The `Img<T>` method itself is dispatch-only.
  This ensures dispatch is always used regardless of call path (`Image::mirror()` or
  `Img<T>::mirror()` directly).
- **`applicableToBase<Ts...>()`** — applicability helper for `ImgBase*` context (checks depth)
- **`resolveOrThrow()`** — safe dispatch that throws `std::logic_error` with selector name
  instead of returning nullptr

**Mirror migration details:**

```
Call chain:
  Image::mirror(axis)
    → ImgBase::mirror(axis, bool)  [virtual]
      → Img<T>::mirror(axis, bool)  [dispatch-only, calls resolveOrThrow]
        → ImgOps::instance().getSelector<MirrorSig>("mirror").resolveOrThrow(this)
          → Backend::Ipp: Img_Ipp.cpp — ippiMirror_8u/16u/32s_C1IR (4 depths)
          → Backend::Cpp: Img_Cpp.cpp — calls Img<T>::mirror(axis, ch, offset, size)

The per-channel Img<T>::mirror(axis, int, Point, Size) is the raw C++ swap
implementation. It never dispatches — backends call it directly.
```

Files created:
- `ICLCore/src/ICLCore/ImgOps.h` — singleton header, dispatch signatures
- `ICLCore/src/ICLCore/ImgOps.cpp` — singleton impl, creates selectors
- `ICLCore/src/ICLCore/Img_Cpp.cpp` — C++ backend (mirror)
- `ICLCore/src/ICLCore/Img_Ipp.cpp` — IPP backend (mirror)
- `ICLCore/src/ICLCore/ImageBackendDispatching.h` — added `ImgBaseBackendDispatching` + `applicableToBase<>`

Changes:
- `ICLCore/CMakeLists.txt` — added `_Ipp.cpp` exclusion pattern (same as ICLFilter)
- `ICLCore/src/ICLCore/Img.cpp` — `Img<T>::mirror(axis, bool)` is now dispatch-only
- `ICLCore/src/ICLCore/Img.h` — made per-channel mirror/normalize public (backends need access)
- `ICLUtils/src/ICLUtils/BackendDispatching.h` — added `resolveOrThrow()`, `#include <stdexcept>`

### Important Rules (Learned This Session)

1. **Never delete IPP/MKL code** — extract to `_Ipp.cpp`/`_Mkl.cpp` backend files.
   IPP specializations have real performance value (BLAS, image ops, etc.).

2. **No `#ifdef ICL_HAVE_IPP` in `_Ipp.cpp` files** — CMake excludes them via
   `list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")` when `!IPP_FOUND`.

3. **All implementations in backend files** — both `_Cpp.cpp` AND `_Ipp.cpp`.
   The main code is dispatch-only. This ensures dispatch works regardless of call path.

4. **MKL follows the same pattern** — `_Mkl.cpp` files, `Backend::Mkl` enum (to be added).

### Previous Session Summary (Session 17)

**BackendDispatching refactoring:**
- Nested `BackendSelectorBase`, `BackendSelector<Sig>`, `ApplicabilityFn` inside `BackendDispatching<Context>`
- `ImageBackendDispatching` is now just `using = BackendDispatching<Image>` (removed `Dispatching` alias)
- All 15 filter headers updated from `core::Dispatching` to `core::ImageBackendDispatching`
- `dispatchEnum` applied to BinaryOp SIMD backends to eliminate inner-loop branching

**Cross-validation tests (20 new, 349 total):**
- Added `crossValidateBackends()` template helper (forces C++ ref, iterates all backend combos)
- All 15 BackendDispatch filters now have cross-validation tests
- Tests cover per-depth validation for all applicable depths

**Benchmarks (25 filter benchmarks):**
- All benchmarks use 1000x1000 (1M pixels) baseline
- Backend parameter: `-p backend=cpp/simd/ipp/auto` for direct comparison

### IPP APIs — What's Active vs Disabled

**ACTIVE (compiles with modern oneAPI IPP 2022+):**

| Backend File | IPP Functions | Filter |
|---|---|---|
| ThresholdOp_Ipp.cpp | `ippiThreshold_LTVal/GTVal_*` | ThresholdOp |
| UnaryCompareOp_Ipp.cpp | `ippiCompareC_*` | UnaryCompareOp |
| UnaryLogicalOp_Ipp.cpp | `ippiAndC/OrC/XorC_*` | UnaryLogicalOp |
| WienerOp_Ipp.cpp | `ippiFilterWiener_*` | WienerOp |
| WarpOp_Ipp.cpp | `ippiRemap_*` | WarpOp |
| Img_Ipp.cpp | `ippiMirror_*` | Img::mirror (new) |
| Img.cpp (inline) | `ippiLUTPalette_*`, `ippiMin/Max*`, `ippiMulC/AddC_*` | Img utilities (TODO) |
| CoreFunctions.cpp (inline) | `ippiMean_*` | channel mean (TODO) |
| DynMatrixUtils.cpp (inline) | `ippsMean_*`, `ippsStdDev_*`, `ippsMeanStdDev_*` | matrix stats (TODO) |
| DynMatrix.h (inline) | `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`, `ippsNormDiff_*` | matrix ops (TODO) |
| MathFunctions.h (inline) | `ippsMean_*` | math mean (TODO) |

**DISABLED (deprecated/removed APIs — TODO re-add via BackendDispatch):**

| Location | Deprecated API | Modern Replacement | Priority |
|---|---|---|---|
| `ConvolutionOp_Ipp.cpp` | `ippiFilterSobelHoriz/Vert/Laplace/Gauss_*` | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | HIGH — 34 specializations |
| `MorphologicalOp_Ipp.cpp` | `ippiMorphologyInitAlloc_*`, `ippiDilate/Erode_*_C1R` | `ippiDilate/Erode_*_C1R_L` + spec buffers | HIGH |
| `AffineOp_Ipp.cpp` | `ippiWarpAffine_*_C1R` | `ippiWarpAffineNearest/Linear_*` + spec | MEDIUM |
| `MedianOp_Ipp.cpp` | `ippiFilterMedian_*_C1R` | `ippiFilterMedianBorder_*_C1R` | MEDIUM |
| `LUTOp_Ipp.cpp` | `ippiReduceBits_8u_C1R` | Modern `ippiReduceBits` (added noise param) | LOW |
| `CannyOp.cpp` (inline) | `ippiCanny_32f8u/16s8u_C1R` | Modern `ippiCanny` with border spec | MEDIUM |
| `ProximityOp.cpp` | `ippiSqrDistance/CrossCorr Full/Same/Valid_Norm_*` | `ippiSqrDistanceNorm_*` | LOW |
| `Img.cpp` (inline) | `ippiResizeSqrPixel_*` | `ippiResizeLinear/Nearest_*` | MEDIUM |
| `CoreFunctions.cpp` (inline) | `ippiHistogramEven_*` | `ippiHistogram_*` (new API) | LOW |
| `FFTUtils.cpp` (inline) | `ippiFFTInitAlloc_*` | `ippiFFTInit_*` + manual buffers | MEDIUM (or use MKL) |
| `DynMatrix.h/.cpp` (inline) | `ippmMul_mm/Invert/Det/Eigen_*` | MKL BLAS/LAPACK | MEDIUM (ippm module dropped entirely) |
| `DynMatrixUtils.cpp` (inline) | `ippmAdd/Sub/Mul_mm/tm/tt_*` | MKL BLAS | MEDIUM |

### Backend Dispatch Framework

```
BackendDispatching<Context>           — ICLUtils (header-only, no .cpp)
  BackendSelectorBase<Context>        — abstract per-selector base
  BackendSelector<Context, Sig>       — typed dispatch table
    .add(b, f, applicability, desc)   — register stateless backend
    .addStateful(b, factory, app, d)  — register stateful backend (factory per clone)
    .resolve(ctx) → ImplBase*         — returns nullptr if no match
    .resolveOrThrow(ctx) → ImplBase*  — throws logic_error if no match
    .clone()                          — stateful: calls cloneFn(); stateless: shares shared_ptr
  ApplicabilityFn<Context>            — std::function<bool(const Context&)>
  ImplBase::cloneFn                   — optional factory for stateful backends

API on BackendDispatching<Context>:
  addSelector<Sig>(K key)             — enum-keyed only (no string overloads)
  getSelector<Sig>(K key)             — O(1) vector index
  selector(K key)                     — returns BackendSelectorBase* (introspection/tests)
  addBackend<Sig>(K, b, f, app, desc) — convenience for getSelector().add()
  addStatefulBackend<Sig>(K, b, factory, app, desc) — convenience for getSelector().addStateful()

Two context types:
  ImageBackendDispatching             — BackendDispatching<Image>
  ImgBaseBackendDispatching           — BackendDispatching<ImgBase*>

ImgOps singleton                      — ICLCore (enum class Op, 10 selectors)
FFTDispatching singleton              — ICLMath (enum class FFTOp, 3 selectors)

Filter prototype+clone pattern        — all 15 ICLFilter ops
  Static prototype() holds selectors + ImplBase objects
  Constructor clones: ImageBackendDispatching(prototype())
  Stateful backends get fresh state per instance via factory cloneFn
  _Cpp.cpp / _Ipp.cpp / _Simd.cpp / _OpenCL.cpp register into prototype()

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
       _Cpp.cpp always built
```

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks to Migrate

**ICLCore — Img.cpp and Img.h are DONE (zero `#ifdef ICL_HAVE_IPP`).**

Remaining ICLCore files:
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions (compile-time, may stay)

**ICLMath — needs own dispatch singleton (similar pattern):**
- `DynMatrix.h` — `ippsNormDiff_L2_*`, `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`
- `MathFunctions.h` — `ippsMean_*`
- `DynMatrixUtils.cpp` — mean/stddev/meanstddev (3 blocks), unary math functions (large block)

**ICLIO — needs own dispatch or extend ImgOps:**
- `DC.cpp` — `ippiRGBToGray_8u_C3C1R`
- `ColorFormatDecoder.cpp` — `ippiYUVToRGB_8u_C3R`
- `PylonColorConverter.cpp/.h` — YUV conversion classes

### Docker Build Commands

```bash
# First run (full build with persistent volume):
docker build --platform linux/amd64 -t icl-ipp packaging/docker/noble-ipp
docker run --platform linux/amd64 --rm -e JOBS=16 -e BUILD_DIR=/build-cache \
  -v $(pwd):/src:ro -v icl-ipp-build:/build-cache \
  icl-ipp bash /src/packaging/docker/noble-ipp/build-and-test.sh

# Subsequent runs (incremental — only recompiles changed files):
# Same command — volume "icl-ipp-build" persists CMake state + object files
```

### Key Files

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework template (header-only, no .cpp)
ICLUtils/src/ICLUtils/EnumDispatch.h           — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h  — Image + ImgBase* typedefs
ICLCore/src/ICLCore/ImgOps.h                   — singleton header, dispatch signatures
ICLCore/src/ICLCore/ImgOps.cpp                 — singleton impl, creates selectors
ICLCore/src/ICLCore/Img_Cpp.cpp                — C++ backends (8 ops + mirror helpers)
ICLCore/src/ICLCore/Img_Ipp.cpp                — IPP backends (8 ops)
ICLMath/src/ICLMath/FFTDispatching.h           — FFTOp enum, FFTDispatching singleton
ICLMath/src/ICLMath/FFTDispatching.cpp         — FFT C++ backends
ICLFilter/src/ICLFilter/*_Cpp.cpp              — 15 C++ backend files (one per filter)
ICLFilter/src/ICLFilter/*_Ipp.cpp              — IPP backends (excluded when !IPP_FOUND)
ICLFilter/src/ICLFilter/*_Simd.cpp             — SIMD backends (always built)
ICLFilter/src/ICLFilter/*_OpenCL.cpp           — OpenCL backends (excluded when !OPENCL_FOUND)
tests/test-filter.cpp                          — 349 tests
benchmarks/bench-filter.cpp                    — 25 filter benchmarks
packaging/docker/noble-ipp/                    — Docker IPP build
.github/workflows/ci.yaml                     — CI with IPP job
```

### Next Steps

#### A. ~~Migrate all 14 remaining filters to prototype+clone pattern~~ **DONE** (Session 20)

All 15 filters now use prototype+clone. See Session 20 summary above.

#### A2. ~~Remove global string registry + add stateful backend cloning~~ **DONE** (Session 21)

See Session 21 summary above. All three phases complete.

#### B. ~~Remaining ICLCore IPP blocks~~ **DONE** (Session 21)

- ~~CoreFunctions.cpp — channel_mean~~ **DONE** (Session 19)
- ~~ImgBorder.cpp — border replication~~ **DONE** (Session 19)
- ~~CCFunctions.cpp — planarToInterleaved/interleavedToPlanar~~ **DONE** (Session 21, added to ImgOps)
- ~~BayerConverter.h/.cpp~~ **DONE** (Session 21, removed dead IPP code — `nnInterpolationIpp` was never called)
- Types.h — enum value definitions (compile-time, stays as-is)

#### C. Other modules

- ~~**ICLMath IPP**~~ **DONE** (Session 21) — MathOps<T> singletons, all `ICL_HAVE_IPP` removed
  from headers + DynMatrixUtils.cpp. CMake `_Ipp.cpp`/`_Mkl.cpp` exclusion added.
- **ICLMath MKL** — 27+ `#ifdef ICL_HAVE_MKL` blocks remain (DynMatrix, DynMatrixUtils,
  FFTUtils). Need `_Mkl.cpp` files + `Backend::Mkl` enum value. Deferred.
- ~~**ICLIO**~~ **DONE** (Session 21) — DC.cpp, ColorFormatDecoder.cpp, PylonColorConverter
  IPP guards removed, C++ fallbacks always used, TODOs added
- **Update disabled IPP backends** to modern oneAPI APIs
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
