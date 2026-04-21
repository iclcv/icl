# Raytracing — Continuation Guide

## Current State (Session 8 — Interactive Cycles + glTF)

### What Was Built (Session 8)

Replaced the custom raytracing backends with Blender Cycles. Built a full
interactive rendering pipeline: non-blocking progressive rendering state machine,
physics demo with real-time object spawning, scene viewer with OBJ/glTF loading,
material presets, checkerboard ground, 4-point lighting, and mesh decimation.

### Previous Sessions (1-7) Built

Custom path tracing pipeline (now replaced by Cycles): CPU/OpenCL/Metal RT
backends, BVH, denoising, tone mapping, PBR materials, GeometryExtractor.
All deleted in Session 7 when Cycles was integrated (~9,870 lines removed).

**Architecture:**
```
Scene / PhysicsScene
  │
  ▼
GeometryExtractor ── tessellate → triangles, per-vertex normals, dirty tracking,
  │                   emissive triangle list (area light sampling)
  ▼
SceneRaytracer ── orchestration, backend auto-selection, camera change detection
  │
  ├──▶ CpuRTBackend     ── SAH BVH, OpenMP, path tracing, FXAA, adaptive AA
  ├──▶ OpenCLRTBackend   ── same BVH on GPU, multi-pass PT within fps budget
  └──▶ MetalRTBackend    ── hardware BVH via Metal RT (intersect<>), Apple Silicon
       │
       ▼
  Post-processing pipeline (virtual stages):
    applyDenoisingStage()    ── GPU: À-Trous / SVGF  │ CPU fallback: all methods
    applyToneMappingStage()  ── GPU: all curves       │ CPU fallback: all curves
    applyUpsamplingStage()   ── GPU: MetalFX          │ CPU fallback: bilinear/edge-aware
```

**Backend capability negotiation:**
```
supportsNativeDenoising(method)   → Metal: ATrous, SVGF
supportsNativeToneMapping(method) → Metal: Reinhard, ACES, Hable
supportsNativeUpscaling(method)   → Metal: MetalFX Spatial, MetalFX Temporal
supportsDenoising(method)         → native OR CPU fallback available?
```

### Source Layout

```
src/Raytracing/
  RaytracerTypes.h           ── POD types, enums (UpsamplingMethod, DenoisingMethod, ToneMapMethod)
  RaytracerBackend.h         ── base class, virtual post-processing stages, G-buffer accessors
  SceneRaytracer.h/.cpp      ── orchestrator (extract → build → render → readback)
  GeometryExtractor.h/.cpp   ── Scene → triangles, emissive triangle list, dirty tracking
  SceneLoader.h/.cpp         ── XML scene file loader (pugixml)
  BVH.h/.cpp                 ── SAH BVH (used by CPU + OpenCL)
  Denoising.h/.cpp           ── CPU denoisers: bilateral, À-Trous, SVGF + SVGFState
  Upsampling.h/.cpp          ── CPU upsamplers: bilinear, edge-aware, nearest-int
  backends/
    Cpu/CpuRTBackend.h/.cpp        ── CPU backend
    Metal/MetalRT.h/.mm             ── Metal RAII wrappers (Device, Buffer, Texture, etc.)
    Metal/MetalRTBackend.h/.mm      ── Metal backend + native post-processing
    Metal/RaytracerKernel.metal     ── MSL kernels (raytrace, pathTrace, denoisers, tone map)
    OpenCL/OpenCLRTBackend.h/.cpp   ── OpenCL backend
    OpenCL/RaytracerKernel.cl       ── OpenCL kernels
scenes/                             ── Composable XML scene files
demos/
  render_offline.cpp                ── Headless renderer for visual verification
  raytracing-physics-demo.cpp       ── Interactive physics demo with GUI
```

### Features by Backend

| Feature | CPU | OpenCL | Metal RT |
|---------|-----|--------|----------|
| BVH traversal | SAH, iterative stack | Same, on GPU | Hardware (intersect<>) |
| Direct lighting (Blinn-Phong) | ✓ | ✓ | ✓ |
| Shadows | ✓ | ✓ | ✓ (hardware any-hit) |
| Reflections (iterative) | 4 bounces | 4 bounces | 4 bounces |
| Emission | ✓ | ✓ | ✓ |
| Emissive area light sampling | ✓ | — | ✓ |
| Path tracing (GI) | ✓ (temporal accum) | ✓ (GPU accum) | ✓ (GPU accum) |
| FXAA / Adaptive AA / MSAA | ✓ | — | — |
| Object ID picking | ✓ | ✓ | ✓ |
| Multi-pass per frame | — | ✓ (fps-budgeted) | ✓ (fps-budgeted) |
| G-buffers (depth, normals, reflectivity) | ✓ | — | ✓ |
| Denoising: Bilateral | CPU | CPU | CPU |
| Denoising: À-Trous | CPU | CPU | **GPU native** |
| Denoising: SVGF | CPU | CPU | **GPU native** |
| Tone mapping (Reinhard/ACES/Hable) | CPU | CPU | **GPU native** |
| MetalFX Spatial upscale | — | — | ✓ |
| MetalFX Temporal upscale | — | — | ✓ |
| Depth buffer output | — | — | ✓ |
| Normals buffer output | ✓ | — | ✓ |

### Key Design Decisions (Session 5)

**Virtual post-processing stages:**
- `applyDenoisingStage()`, `applyToneMappingStage()`, `applyUpsamplingStage()` are
  virtual methods in the base class. Default: CPU fallback. Metal backend overrides
  for GPU-native paths.
- `supportsNativeDenoising()` / `supportsNativeToneMapping()` / `supportsNativeUpscaling()`
  let the UI/orchestrator query capabilities at runtime.
- Pipeline order: raytrace → denoise → tone map → upscale → readback.

**SVGF temporal design:**
- The path tracer does its own temporal accumulation (running average over N frames).
- SVGF does NOT blend colors temporally — it passes the accumulated color through
  unchanged. It only tracks luminance moments (mean, mean²) across frames for
  per-pixel variance estimation.
- The spatial filter uses this variance to adapt edge-stopping weights (σ_lum scales
  with √variance), giving better quality than plain À-Trous.
- Mesh ID edge-stopping: hard `continue` for neighbors with different object IDs
  prevents silhouette halos from path tracing jitter at object boundaries.

**Reflectivity-aware denoising:**
- Per-pixel `mat.reflectivity` output from raytrace kernels.
- Spatial filter weight scaled by `(1-refl)²` — mirrors get minimal spatial blur.
- Applied in both GPU and CPU SVGF implementations.

**Emissive area light sampling:**
- `RTEmissiveTriangle` list built during scene extraction with world-space vertices,
  smooth averaged vertex normals (not cross-product face normals), and triangle area.
- One random triangle per bounce, area-weighted selection, uniform barycentric point.
- Shadow ray: `max_distance = dist - 1.0` to avoid self-intersection.
- Geometric term: `NdotL * lightNdotL * totalEmissiveArea / dist²`.

**Tone mapping:**
- Reinhard, ACES filmic (Narkowicz 2015), Hable/Uncharted 2 curves.
- Exposure multiplier applied before the curve.
- No gamma correction (raytracer output is already in display space).
- Metal GPU kernel operates in-place on float buffers.

**XML scene loader:**
- Composable: `render-offline -scene base.xml objects.xml` merges files.
  First camera wins, lights/objects accumulate.
- Supports: camera, lights (with scale), cube/sphere objects with color,
  reflectivity, shininess, emission, rotation, scale.
- Uses ICL's pugixml wrapper (`ICLUtils/XML.h`).

### Build & Run

```bash
cmake .. -DBUILD_EXPERIMENTAL=ON -DBUILD_DEMOS=ON
cmake --build . -j16

# Interactive demo (starts with curated scene, PT + SVGF + ACES)
./bin/raytracing-physics-demo -backend metal -fps 30 -size 1280x960

# Offline renderer (composable XML scenes)
./bin/render-offline -scene scenes/base.xml scenes/objects-mixed.xml \
    -pt -frames 64 -denoise svgf -tonemap aces -exposure 0.7 -o output.png

# Circle shadow test
./bin/render-offline -scene scenes/circle-test.xml -pt -frames 512 -o shadows.png
```

### Bugs Fixed (Session 5)

1. **MetalFX RG16Float** — pixel format constant was 102, correct value is 105
2. **MetalFX Temporal depth format** — requires `Depth32Float` (252), not `R32Float`
3. **MetalFX Temporal jitter** — Halton jitter must be applied to camera rays before
   dispatch, not just reported to the scaler
4. **1D kernel dispatch** — u8ToFloat/floatToU8/toneMap kernels changed from 1D (n,1)
   to 2D (w,h) dispatch with uint2 tid (1D caused all-black on some configs)
5. **SVGF + tone mapping crash** — SVGF temporal buffers need separate size tracking
   (svgfW/H) from shared denoise ping-pong buffers (denoiseW/H)
6. **Tone mapping gamma** — removed incorrect pow(1/2.2) gamma; output is already
   display-space
7. **SVGF double accumulation** — path tracer accumulates, SVGF passes color through;
   only tracks variance temporally
8. **Silhouette halo** — mesh ID edge-stopping in spatial filter prevents color bleeding
   across object boundaries from path tracing jitter
9. **Emissive shadow artifacts** — use averaged smooth vertex normals instead of
   cross-product face normals for emissive triangles

### Material System (Session 6)

**Phase 1 — Material class + SceneObject integration:**
- `ICLGeom/src/ICLGeom/Material.h/.cpp` — PBR `Material` class with `baseColor`,
  `metallic`, `roughness`, `reflectivity`, `emissive`, texture map slots
  (`baseColorMap`, `normalMap`, `metallicRoughnessMap`, `emissiveMap` as
  `shared_ptr<ImgBase>`), alpha mode, display hints. Static factories
  `fromColor()`, `fromPhong()`, plus `toPhongParams()` for GL fallback.
- `Primitive.h` — `shared_ptr<Material> material` on base Primitive struct
- `SceneObject.h/.cpp` — `m_defaultMaterial`, `setMaterial()`/`getMaterial()` API,
  `getOrCreateMaterial()`. Old `setColor`/`setShininess`/`setSpecularReflectance`/
  `setReflectivity`/`setEmission` deprecated with `[[deprecated]]` — still work,
  auto-sync to material.

**Phase 2 — Raytracer PBR shading:**
- `RTMaterial` updated: `baseColor`, `emissive`, `metallic`, `roughness`,
  `reflectivity` (replaces diffuseColor/specularColor/shininess)
- `GeometryExtractor` reads from Material class when available, legacy fallback
- Cook-Torrance microfacet BRDF (GGX + Smith + Schlick Fresnel) in all three
  backends: CPU, Metal, OpenCL. Energy-conserving Lambert diffuse for dielectrics,
  zero diffuse for metals.

**Phase 3 — OpenGL renderer adaptation:**
- `Scene::renderSceneObjectRecursive()` uses `Material::toPhongParams()` for GL state
- Per-primitive material override in render loop (set/restore GL specular/shininess)

**Phase 4 — Texture support:**
- `Material` stores textures as `shared_ptr<core::ImgBase>` (portable, no Qt dep)
- `RTVertex` extended with `float u, v` (UV coordinates)
- `SceneObject` gets `m_texCoords` vector, `addTexCoord()`/`getTexCoords()`
- `GeometryExtractor` flows UVs through: from per-vertex texCoords, or generates
  (0,0)→(1,1) for TexturePrimitive quads
- `RaytracerBackend::setMaterialTextures()` virtual passes Material pointers
- CPU backend samples all four texture maps at hit UVs:
  - `baseColorMap` → modulates albedo
  - `metallicRoughnessMap` → overrides metallic (R) and roughness (G)
  - `normalMap` → perturbs shading normal via tangent-space transform
  - `emissiveMap` → modulates emission
- GPU texture sampling deferred (Metal/OpenCL need texture upload infrastructure)

**Key design decisions (Session 6):**
- Texture images stored as `ImgBase` (not `GLImg`) for portability across backends
- PBR↔Phong conversion in both directions preserves visual appearance
- Deprecated API auto-creates Material on first use via `getOrCreateMaterial()`
- Per-primitive material override: primitive's material > object's material > legacy
- Normal map uses constructed tangent frame from shading normal (no explicit tangents)

### Session 6 additional improvements

- **Roughness-aware SVGF denoising** — roughness passed through G-buffer alongside
  reflectivity. Spatial filter scales neighbor weights by `roughness²`: smooth
  surfaces get almost no blur (preserves specular highlights), rough surfaces get
  full denoising. Combined with reflectivity via `min(reflScale, roughScale)`.
- **Specular indirect lighting** — path tracer now does MIS between specular and
  diffuse bounces (was diffuse-only). Metals get ~90% specular bounces with
  roughened reflection direction. Fixes metals appearing flat/matte.
- **Brighter sky gradient** — warm horizon (0.6) → blue zenith (0.55), dark below
  horizon. Essential for PBR materials to have something to reflect.
- **Material extraction bug fix** — RTMaterial fields were uninitialized when
  `geomDirty=false`, corrupting all materials on geometry change. Fixed by always
  extracting material via `tessellateExtractMaterial()` + adding default initializers
  to RTMaterial fields.
- **Demo improvements** — PBR material archetypes (roughness gradient, gold, copper,
  plastic, rubber, mirror, emissive), material tooltip on hover, improved physics
  parameters (lower angular damping, initial spin for realistic tumbling).

### Known issues with custom raytracer

The custom Cook-Torrance BRDF implementation has visual quality limitations:
- **Roughness differences barely visible for dielectrics** — F0=0.04 means only 4%
  specular, dwarfed by diffuse. Needs HDR environment maps (not just a sky gradient)
  to show glossy reflections convincingly.
- **Metals look better after specular indirect fix** but still not production quality.
- **Denoiser vs roughness tension** — SVGF spatial filter still erases subtle specular
  differences despite roughness-aware edge-stopping.
- **No importance-sampled GGX** — specular indirect uses roughened reflection
  approximation, not proper GGX importance sampling.

These limitations motivated the decision to integrate Blender Cycles instead.

## Cycles Integration (Session 6 — In Progress)

### Decision

Replace the custom raytracing backends with Blender Cycles as the rendering engine.
Cycles provides production-quality PBR (Principled BSDF), Metal GPU acceleration,
OIDN denoising, environment maps, and 15+ years of battle-tested code.

### Build Status

Cycles standalone is checked out at `3rdparty/cycles/` and builds successfully.
See `3rdparty/cycles/INSTALL_ICL.md` for build instructions.

**Prerequisites (macOS):**
```bash
brew install git-lfs python@3.13
```

**Build:**
```bash
cd 3rdparty/cycles
make update                    # ~1.2 GB precompiled deps via Git LFS
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DPYTHON_ROOT_DIR=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13 \
  -DPYTHON_LIBRARY=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/lib/libpython3.13.dylib \
  -DPYTHON_INCLUDE_DIR=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/include/python3.13
cmake --build . -j16 --target install
```

**Verified:** Metal and CPU devices detected, `install/cycles` executable built.

### Cycles XML Shader Node Issue — FIXED (Session 7)

The standalone `cycles` executable had a **lazy initialization problem**: most
shader node types failed with "Unknown shader node" errors because `NODE_DEFINE`
only registers a node type when `get_node_type()` is explicitly called, but the
XML parser calls `NodeType::find()` which does a plain map lookup. In full Blender,
the UI code constructs every node type (triggering registration); the standalone
never did. **Not a linker issue** — the code was linked, just never called.

**Fix:** Added `xml_register_shader_nodes()` in `src/app/cycles_xml.cpp` — a
bootstrap function that calls `get_node_type()` on all 90 concrete shader node
classes before XML parsing starts. Called at the top of `xml_read_file()`.

**Verified:** All example scenes render correctly — `sky_texture`, `background`,
`checker_texture`, `glass_bsdf`, `noise_texture`, `glossy_bsdf`, `diffuse_bsdf`,
`wave_texture`, `bump`, `emission`, etc. all work.

### Cycles Integration Progress

1. ~~**Fix shader node registration**~~ ✓ Done (Session 7)
2. ~~**Verify full rendering**~~ ✓ Done (Session 7)
3. ~~**CMake + Cycles linking**~~ ✓ Done (Session 7) — `cycles_target_setup()` helper
4. ~~**CyclesRenderer + SceneSynchronizer**~~ ✓ Done (Session 7) — full bridge:
   - ICL Scene → Cycles scene (meshes, materials, camera, lights)
   - PrincipledBsdf shader graphs from ICL Material
   - ICL Camera intrinsics → Cycles FOV
   - Per-object dirty tracking for incremental updates
   - OutputDriver → ICL Img8u with sRGB gamma
   - Quality presets (Preview/Interactive/Final)
5. ~~**Delete old infrastructure**~~ ✓ Done (Session 7) — removed ~9,870 lines
6. ~~**Analytic spheres**~~ ✓ Done (Session 7) — SceneObject::ObjectType::Sphere
   renders via Cycles PointCloud (perfect ray-sphere intersection, zero faceting)
7. ~~**Camera crash fix**~~ ✓ Done (Session 7) — removed `cclCam->update()` call
   that caused SIGBUS; Session::reset() handles it. FOV/clip plane clamping added.
8. ~~**Hosek-Wilkie sky**~~ ✓ Done (Session 7) — realistic environment lighting
   via SkyTextureNode, proper light intensity scaling for scene units

### Source Layout (Post-Session 8)

```
src/Raytracing/
  CyclesRenderer.h/.cpp      ── Public API (PIMPL), 3-state machine, OutputDriver
  SceneSynchronizer.h/.cpp    ── ICL Scene → Cycles Scene bridge, dirty tracking
  GltfLoader.h/.cpp           ── glTF/GLB parser using cgltf.h
demos/
  cycles-physics-demo.cpp     ── Interactive physics + Cycles (Z-up, Bullet)
  cycles-scene-viewer.cpp     ── OBJ/glTF viewer with material presets
  cycles-renderer-test.cpp    ── Offscreen render test (Y-up)
scenes/
  bunny.obj                   ── Stanford bunny (35K verts)
  DamagedHelmet.glb           ── Khronos test model (14K verts, 5 textures)
```

### Build & Run

```bash
# Build Cycles first (one-time)
cd 3rdparty/cycles && make update && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DPYTHON_ROOT_DIR=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13 \
  -DPYTHON_LIBRARY=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/lib/libpython3.13.dylib \
  -DPYTHON_INCLUDE_DIR=/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/include/python3.13
cmake --build . -j16 --target install

# Build ICL with Cycles
cd build
cmake .. -DBUILD_EXPERIMENTAL=ON
cmake --build . -j16

# Render test scene
./bin/cycles-renderer-test -o output.png
```

### Camera Orientation Issue (IN PROGRESS — Session 7)

The camera orientation conversion from ICL to Cycles is partially working but
has a subtle Y-axis flip problem related to ICL's unconventional `up` vector:

**ICL Camera convention** (from Camera.h docs):
- `norm` = view direction (the direction the camera looks)
- `up` = direction pointing toward **positive image Y**, which is the **bottom**
  of the image (image Y increases downward). This is opposite to most 3D APIs
  where "up" means scene-up / image-top.
- `horiz` = computed as `cross(up, norm)`, forms right-handed system with norm and up

**Current state of syncCamera (SceneSynchronizer.cpp):**
- `camUp = -icl_up` (negated, since ICL up points down)
- `right = cross(forward, camUp)`
- `upVec = cross(right, forward)`
- Cycles transform: `tfm.y = -upVec` (negated Y row)
- OutputDriver: no Y-flip (direct copy)

**What works:** Y-up scenes with `Camera::lookAt()` render correctly
(`cycles-renderer-test` shows spheres on ground, right-side up, correct FOV).

**What doesn't work:** Z-up scenes (physics demo). The camera at
`(600, -500, 400)` looking at `(0, 0, -30)` with `up=(0,0,1)` shows only sky.
The camera orientation math produces a valid-looking matrix but the objects
aren't in the field of view.

**`Camera::lookAt()` factory** (added in Session 7, in Camera.h/.cpp):
- Takes intuitive params: position, target, up, resolution, hfov_degrees
- Computes `norm = normalize(target - position)`
- Computes `imageUp` = component of world-up perpendicular to norm
- Derives `f` and `mx` from desired horizontal FOV
- Sets `principalPointOffset` to image center
- Works for Y-up; untested/broken for Z-up

**Key insight still needed:** The relationship between ICL's `up` vector,
the Cycles Transform rows, and the OutputDriver's Y-flip needs to be resolved
as a single coherent convention. The current approach of individual negations
and flips is fragile. Consider studying how the old GeometryExtractor's
`extractCamera` built the ray generation matrix — it worked for both Y-up
and Z-up scenes. That code is in git history at commit 47ee811d0.

**Debug aids in place:**
- `fprintf(stderr, "[Camera] ...")` prints in syncCamera showing pos, fwd, up,
  fov, near/far, and the full 3x4 transform matrix. Remove when fixed.

### Completed (Session 8)

- ✅ Camera orientation fixed for Y-up and Z-up (horiz, -up, forward as columns)
- ✅ Camera::lookAt documented with ICL's unusual up=visual-down convention
- ✅ Light intensity calibrated (300x multiplier for ICL scene distances)
- ✅ Non-blocking progressive rendering (3-state machine: IDLE→WAIT_FOR_START→RENDERING)
- ✅ tag_update(scene) for transform changes (was tag_tfm_modified which was a no-op)
- ✅ session->start() after every session->reset() (session thread goes idle after completion)
- ✅ Physics demo: time-based spawning, delayed object activation, side-by-side GL view
- ✅ Scene viewer: OBJ loading, auto-scale, checkerboard ground, material presets
- ✅ Vertex clustering mesh decimation (-decimate N)
- ✅ renderBlocking() for offscreen rendering

### Completed (Session 9 — Full Texture Pipeline)

- ✅ **Primitive UV indices** — TrianglePrimitive expanded (6→9 ints), QuadPrimitive (8→12),
  adding per-corner texcoord indices i(6..8) / i(8..11) alongside existing normal indices.
  Mirrors the existing separate-pool pattern for normals. Default -1 = no UV.
- ✅ **Material texture slots → Image** — changed from `shared_ptr<ImgBase>` to `core::Image`
  (value semantics, null by default, check via `operator bool()`)
- ✅ **stb_image texture decoding** — `decodeImage()` in GltfLoader decodes PNG/JPEG from
  embedded GLB buffers or external file references into 4-channel Img8u via `stb_image.h`
- ✅ **UV attributes on Cycles meshes** — `tessellateToMesh()` reads UV indices from
  primitives, writes `ATTR_STD_UV` per-corner float2 data on Cycles mesh
- ✅ **ICLImageLoader** — custom `ccl::ImageLoader` subclass in SceneSynchronizer that
  bridges ICL `Image` to Cycles `ImageManager`. Uses `planarToInterleaved()` for fast
  channel conversion (4-ch path), registered via `add_image(unique_ptr<ImageLoader>)`
- ✅ **Texture nodes in shader graph** — `createPrincipledShader()` creates full node graph:
  - baseColorMap → ImageTextureNode → PrincipledBsdf::Base Color (+ Alpha for cutout)
  - metallicRoughnessMap → ImageTextureNode → SeparateColorNode → Blue→Metallic, Green→Roughness
  - normalMap → ImageTextureNode → NormalMapNode (tangent space) → PrincipledBsdf::Normal
  - emissiveMap → ImageTextureNode → PrincipledBsdf::Emission Color
  - Shared TextureCoordinateNode provides UVs to all texture lookups
- ✅ **Emissive fix** — emissiveFactor always stored (was skipped when texture present),
  emissiveMap decoded and forwarded. Per glTF spec: final = factor * texture.
- ✅ **Emission via PrincipledBsdf** — switched from AddClosure(BSDF+Emission) pattern
  to PrincipledBsdf's built-in emission_color/emission_strength (simpler, correct)
- ✅ **GltfLoader passes UV indices** — when UVs present, vertex index = UV index
  (glTF pre-splits at seams), passed through addTriangle(..., ta, tb, tc)
- ✅ **Verified: DamagedHelmet.glb renders with full PBR textures**

### Session 10 — GL 4.1 Core Profile Renderer Rebuild

**Status: Feature-complete PBR renderer with multi-light shadows, env reflections,
and analytical brightness calibration. Visually close to Cycles (3% overall match
on DamagedHelmet with white background).**

Replaced the GLSL 1.20 compatibility-profile renderer with a full GL 4.1 Core
Profile renderer. Switched the widget GL context via `QSurfaceFormat::setDefaultFormat()`
before `ICLApp` construction. Legacy fixed-function GL calls in `ICLWidget` and
`DrawWidget3D` are guarded with `GL_CONTEXT_PROFILE_MASK` checks.

**Completed:**
- ✅ GL 4.1 Core Profile: `#version 410 core`, VAOs, `layout(location=N)`, `in`/`out`
- ✅ Multi-light (8 lights, individually queried uniform locations — NOT `loc+i`)
- ✅ PBR materials: metallic, roughness, emissive, Blinn-Phong specular
- ✅ All texture maps: baseColorMap, normalMap (TBN from dFdx/dFdy),
  metallicRoughnessMap (glTF blue/green), emissiveMap
- ✅ Multi-light shadow mapping: up to 4 shadow maps, per-light shadow cameras,
  `sampler2DShadow` with hardware PCF, `glPolygonOffset` bias
- ✅ Sky gradient background: procedural from inverse VP matrix, shared Sky struct
- ✅ Environment reflections: `reflect(-V, N)` samples sky gradient, Schlick Fresnel,
  roughness-blurred via diffuse/specular env blend
- ✅ Per-pixel multi-bounce approximation: `boost = 1/(1 - albedoLum)` analytically
  models infinite light bounces, replacing hardcoded env multiplier
- ✅ GLImageRenderer: fullscreen textured quad for 2D image display in Core Profile
- ✅ Debug modes: Normals, Albedo, UVs, Lighting Only, NdotL
- ✅ Sky struct (`Sky.h`): Solid/Gradient/Physical/Texture modes, shared between
  GL and Cycles via `Scene::setSky()`/`getSky()`
- ✅ Compare mode (`-compare prefix`): offscreen FBO rendering, 4×4 regional
  brightness grid, per-pixel stats. Uses standalone `QOpenGLContext` +
  `QOffscreenSurface` for worker-thread GL rendering
- ✅ `-background white|black|gradient|physical` CLI flag
- ✅ `-bg` / `-exp` CLI overrides for compare mode
- ✅ `scripts/compare-grid.sh`: validates brightness match across BG%×Exposure% grid
- ✅ Cycles camera FOV fix: pass vertical FOV (not horizontal) to `set_fov()`
- ✅ GL Env % / GL Direct % tuning sliders for real-time calibration

**Brightness calibration results (DamagedHelmet, -background white):**
```
Overall: Cycles=0.909  GL=0.886  ratio=1.03  (3% match)
```
Regional 4×4 grid shows most areas within 0.90–1.15 ratio. Visor area remains
1.33× darker in GL (no glass transmission in rasterizer).

**Key design decisions:**
- **World-space lighting** — avoids view-space light transform bugs from Session 9b
- **Per-pixel bounce correction** — analytically derived from geometric series
  `E/(1-a)`, scene-independent, works across all BG%/Exposure% combinations
- **Separate sky display vs env brightness** — sky background renders at 1× intensity,
  env reflections use 2× to match Cycles' `bgStrength = 2.0 * intensity`
- **No transmission** — dielectric surfaces (visor) stay opaque. Tried a transmission
  approximation `(1-metallic)*(1-roughness)²` but it over-brightened all smooth
  dielectrics. Proper fix needs per-material IOR/transmission parameter in Material.

**Source layout:**
```
ICLGeom/src/ICLGeom/
  SceneRendererGL.h/.cpp    ── GL 4.1 Core renderer (shaders, VAOs, shadows, env)
  Sky.h                     ── Sky/environment struct (shared by GL + Cycles)
  gl-pipeline-rework-plan.md ── Detailed plan document (steps, future work)
ICLExperimental/Raytracing/
  opengl-4x-plan.md         ── Incremental build plan with calibration notes
  demos/cycles-scene-viewer.cpp ── Side-by-side GL + Cycles viewer with compare mode
scripts/
  compare-grid.sh            ── Brightness validation across BG%×Exposure% grid
```

### Next Steps

#### GL Renderer — Immediate

1. **Validate multi-bounce across settings** — run `scripts/compare-grid.sh`, check
   that ratios stay near 1.0 across the full BG%×Exposure% grid
2. **Glass/transmission** — add `Material::transmission` / `Material::ior` fields,
   use in GL shader for transparent dielectrics (visor, glass). This is the biggest
   remaining visual gap vs Cycles.
3. **Cycles gradient background** — the node graph approach (TextureCoordinate →
   SeparateXYZ → MixColor) crashed on wrong socket names. Needs investigation of
   Cycles' exact node API. Currently falls back to weighted-average flat color.

#### Widget Overlay — Core Profile Port

4. **OSD button bar** — blue buttons at top-left, currently skipped in Core Profile
5. **2D draw commands** — `draw->text()`, `draw->line()`, etc. in screen coords
6. **Zoom rectangle** — left-mouse-drag zoom, needs rect overlay + zoom logic
7. **Image info indicator** — bottom-right pixel info display
8. **GLImg::draw2D** — 2D image display for Canvas widgets

Approach options: QPainter overlay, or port GLPaintEngine to GL 4.1 shaders.

#### Legacy Cleanup

9. **Delete legacy renderer** — `Scene::renderSceneObjectRecursive()`, `ShaderUtil`,
   display lists, all `glBegin/glEnd` code
10. **Wire `Scene::getGLCallback()`** to SceneRendererGL for all ICL apps

#### Cycles — Deferred

- HDR environment maps (Sky::Texture mode)
- Area/spot/sun light types
- QEM mesh decimation
- Cycles XML scene loader

### Known Limitations

- No glass/transmission — smooth dielectrics (visor) appear too dark vs Cycles
- Cycles background is flat color (gradient node graph TODO)
- Lights created once per session in Cycles (no dynamic updates)
- Only point lights (no spot/area/sun)
- OBJ loader: no MTL material parsing
- PolygonPrimitive doesn't carry UV indices yet
- Widget 2D overlay (OSD, text, zoom) disabled in Core Profile
