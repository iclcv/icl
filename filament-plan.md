# Filament rendering backend — plan

Rework viz3d's **real-time** rendering backend onto **Google Filament** (PBR real-time
renderer, Apache-2.0). Planning + de-risking done in session S100; **implementation is a
fresh session**. See memory [[project_filament_backend]].

---

## Goal & scope

Replace the hand-written GL 4.1 `viz3d/render/Renderer` (the interactive rasterizer) with
Filament, **keeping ICL's own `Camera` and `Node` scene graph**. Filament is *only* the
rasterization backend — everything above it stays pure ICL.

**In scope:** the real-time path. **Out of scope:** `CyclesRenderer` (offline photoreal — a
different tool, stays as-is) and `BVH`/`RayCastOctree` (picking / offscreen raycast, stays).

## Locked-in decisions (from the S100 design discussion with CE)

1. **Replace, not permanent multi-backend.** End state = one real-time backend (Filament).
   Introduce a thin abstract `RenderBackend` seam *as migration scaffolding* so the GL path and
   Filament coexist during the transition, validate parity, then **delete the GL renderer**.
   (A GL fallback can stay behind the seam if it proves cheap to keep — decide at P5.)
2. **Filament is fully wrapped — never exposed.** Zero `filament::` types in any installed (or
   even non-`detail/`) header. It lives in `viz3d/render/detail/` behind a PIMPL. The public
   world sees only `Scene` / `Node` / `Camera` / a `RenderBackend` seam whose signature mentions
   **only ICL types**. This is ICL's self-containment principle + the [[feedback_detail_strict_rule]]
   pattern we just used for the ICP backends.
3. **ICL `Camera` is the sole source of projection truth — 1:1, calibrated.** Filament never
   computes its own fov-based projection; we inject ICL's projection matrix verbatim via
   `filament::Camera::setCustomProjection(...)`. A 3D point must rasterize to exactly the pixel
   `cam.project(p)` returns, so 2D/AR overlays drawn with the calibrated camera line up on the
   render by construction. **This is a hard gate (see Projection-parity gate).**
4. **Two-layer compositing.** Filament renders the **3D layer** (solid meshes + lines + points)
   into an ICL `Image`/texture; the **2D annotation layer** (text, image-space primitives, CV
   debug draws) composites on top — exactly as `ICLDrawWidget` already layers over the 3D. Text
   is never a Filament job: world-anchored labels project via the (1:1) ICL camera and draw in 2D.
5. **Support onscreen *and* offscreen; onscreen (macOS) first.** Both are first-class in Filament
   (`SwapChain(nativeWindow)` vs headless `SwapChain`+`RenderTarget`); the `View` renders
   identically. Design the seam **target-agnostic** so the switch is a flag. Nothing above the
   seam may assume a target.
6. **Prebuilt Filament, mirroring the Cycles precedent** — optional dep, gated, no source build.

## Verified in S100 (de-risking — all green)

- **No self-build needed.** Prebuilt `filament-v1.72.1-mac.tgz` (arm64, ~47 MB) extracted to
  `3rdparty/filament/` (`include/`, `lib/arm64/*.a`, `bin/matc`). Links clean against our
  Apple Clang / C++20 / libc++ — **no ABI mismatch** (missing symbols were just un-listed
  archives, not ABI errors).
- **Headless Metal runs in-sandbox.** `Engine::create(METAL)` → `Apple M3 Max`, feature level 3.
- **Offscreen render + readback works in-sandbox.** Headless `SwapChain(CONFIG_READABLE)` +
  clear + `Renderer::readPixels` returned the clear colour `(0.2,0.4,0.8)` as `(153,186,214,255)`
  after Filament's tonemap+sRGB. **⇒ the offscreen path is self-testable here with golden images
  before any onscreen work.**
- **Onscreen works on CE's Mac** via the bundled sample: `./3rdparty/filament/bin/material_sandbox`
  (self-contained; SDL2 statically baked in — `otool -L` shows only system frameworks).
- **Size is contained.** Linked archives total 8.8 MB (with debug info); a minimal render binary
  is 4.7 MB unstripped → **3.3 MB stripped**. `libicl-viz3d` (~11 MB today) likely grows to
  ~20–25 MB — same order as other ICL dylibs, embedded **once** (shared dylib, apps link it
  dynamically), gated optional, and far smaller than the already-accepted Cycles static lib.

### Exact link recipe (captured from the S100 smoke tests)

```
-I3rdparty/filament/include
-L3rdparty/filament/lib/arm64
  -lfilament -lbackend -lfilabridge -lfilaflat -lutils -lgeometry -lsmol-v -libl
  -lbluegl -lbluevk -lzstd
frameworks: Metal Foundation CoreVideo QuartzCore IOKit Cocoa CoreGraphics Carbon AppKit
```
(Do NOT link gltfio/viewer/draco/basis/civetweb/etc. — unused, keeps the footprint down.)

---

## Architecture

```
            ICL Camera (calibrated)  ── getProjectionMatrix() ──┐
                     │                                          │  (same matrix)
   Node graph ──►  RenderBackend (ICL types only)               ▼
                     │            ┌─ GLRenderBackend  (existing, transitional)
                     └──render()──┤
                                  └─ FilamentRenderBackend  ── detail/ ── filament::*
                                        │  target-agnostic: SwapChain(window) | RenderTarget(headless)
                                        ▼
                              3D image/texture ──► [2D annotation layer composites on top]
                                                     text, lines, CV overlays  (cam.project, 1:1)
```

- **`RenderBackend`** (new abstract): `render(const std::vector<shared_ptr<Node>>&, viewMatrix,
  projectionMatrix, target)` + the existing `Renderer` setters (exposure, ambient, overlay alpha,
  SSR, shadows, lighting, sky, sky-up, debug mode, invalidateCache). Signature = ICL types only.
- **`GLRenderBackend`**: today's `viz3d/render/Renderer` GL 4.1 code, unchanged, adapted to the
  interface. Transitional.
- **`FilamentRenderBackend`** (`viz3d/render/detail/`): translates the Node graph → Filament
  `Renderable`/`Material`/`Light`/`Camera`, keyed on the existing Node **version counter** for
  cache invalidation (same mechanism the GL renderer uses; see
  [[reference_geom2_renderer_cache_versions]] — version counters must be process-global).

---

## The projection-parity gate (the linchpin — nothing lands until green)

ICL's projection encodes real calibration (focal length, principal-point offset, skew). Map it
into Filament's clip space via `Camera::setCustomProjection`, resolving the convention deltas
(y-flip, depth range [0,1] vs [-1,1], handedness) between our GL path and Filament.

**Test (headless, in-sandbox):** for a set of known 3D points and a calibrated camera with real
principal-point offset (NOT centered-only — that hides the row-3/col-3 & offset bugs, cf.
[[project_pose_estimation_bugs]]), render points and assert the rasterized pixel == `cam.project(p)`
to sub-pixel. Mine the existing GL `Renderer`'s handling of `render(nodes, view, projection)` —
it already consumes ICL's projection, so the mapping exists in-tree.

---

## Phased plan (each phase independently landable + suite green)

- **P0 — Spike + build wiring.**
  - meson: `filament` option, gate on `fs.exists('3rdparty/filament/lib/arm64/libfilament.a')`
    + `-DICL_HAVE_FILAMENT`, link recipe above. Mirror the Cycles block in `meson.build`.
  - Port the two S100 smoke programs into a gated `viz3d` build target (link proof) + a headless
    render+readback (the golden-test rig). **Onscreen** spike on CE's Mac: a triangle in a
    Metal-backed native view embedded via `QWidget::createWindowContainer(QWindow::fromWinId(...))`
    — proves the hardest Qt-integration question. (In-sandbox I can only do the offscreen half.)
- **P1 — Backend seam.** Extract abstract `RenderBackend`; make today's GL renderer
  `GLRenderBackend` implement it. `Scene` owns a `RenderBackend*` instead of a concrete
  `Renderer`. **No behaviour change; suite green.** (Pure refactor, reversible.)
- **P2 — Filament backend, geometry + projection gate.** `FilamentRenderBackend`: Engine/Scene/
  View lifecycle, target abstraction (headless first), GeometryNode → Renderable (VB/IB) keyed on
  node version, ICL camera → `setCustomProjection`. **Land the projection-parity gate.** Shaded
  meshes appear; points match `cam.project` sub-pixel (headless golden test).
- **P3 — Parity.** Lights/IBL, cascaded shadows, sky/env, tone-mapping; then **lines & points**
  (Filament `LINES`/`POINTS` + unlit material — watch thick-line/point-size: Metal clamps GL line
  width, so `setLineWidth` may need quad/billboard expansion, cf. [[feedback_lines_points]]);
  then SSR + debug modes 0–8. Validate each against the GL path with headless golden images
  ([[reference_headless_gl_capture]] harness).
- **P4 — Offscreen/onscreen switch + overlay compositing.** Route `SceneCapture`/`OffscreenView`/
  `renderToImage` through Filament's headless target. Wire the 2D annotation layer to composite
  over the Filament 3D image (+ optional depth readback for overlays that must be occluded).
  Expose the onscreen↔offscreen switch as a flag. Filament's own render thread likely **dissolves
  the macOS "second-GL-context-starves-the-GUI" pain** that `OffscreenView` works around today.
- **P5 — Converge to one.** Flip the default to Filament; soak demos/apps on the real display;
  **delete `GLRenderBackend`** (or keep it as a documented fallback if cheap). Update docs.

## Materials

Our `viz3d/render/Material` (albedo/metallic/roughness/colors) → a **small fixed set of Filament
materials** compiled offline at build time by `3rdparty/filament/bin/matc` (version-locked to the
runtime lib — hard rule). Variants: lit PBR, unlit (lines/points), overlay-alpha. `.mat` sources
live in `viz3d/render/detail/materials/`; matc runs as a meson custom_target producing `.filamat`
blobs embedded via `resgen`/`xxd`.

## Open questions (decide in the implementation session)

- **Qt onscreen embedding**: `createWindowContainer(QWindow::fromWinId)` vs a layer-backed NSView —
  settle in the P0 onscreen spike on CE's Mac.
- **Overlay occlusion**: do any 2D overlays need to be occluded by 3D (⇒ depth readback), or is
  paint-on-top always fine?
- **Node coverage**: enumerate every `viz3d/nodes/*` type and its Filament mapping (CuboidNode,
  point clouds, grids, coordinate frames, custom geometry, textures).
- **Linux/Vulkan + Intel/universal**: prebuilt is arm64-mac only. Linux CI + Intel Macs need the
  Linux/universal tarball or a source build — scope when it matters (arm-mac-first for now).

## Testing strategy

- **In-sandbox (headless Metal, proven):** golden-image tests for geometry, projection-parity,
  lines/points, materials — I can self-verify all of this before CE runs anything onscreen (same
  model as the RobustPoseEstimator sim + the GPU ICP backends).
- **On CE's Mac (onscreen):** `material_sandbox` sanity now; later the ICL demos/apps on a real
  display for the onscreen path + interaction.
