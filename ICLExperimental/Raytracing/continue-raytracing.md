# Raytracing — Continuation Guide

## Current State (Session 4 — MetalFX Spatial + Temporal upscaling)

### What Was Built

Four rendering backends including Metal RT with hardware-accelerated BVH on Apple
Silicon, path tracing with global illumination, Bullet physics integration,
interactive object picking, upsampling framework (including MetalFX Spatial and
Temporal upscalers), denoising filters, and depth/motion vector output — all
running in real-time.

**Architecture:**
```
Scene / PhysicsScene
  │
  ▼
GeometryExtractor ── tessellate → triangles, per-vertex normals, dirty tracking
  │
  ▼
SceneRaytracer ── orchestration, backend auto-selection, camera change detection
  │
  ├──▶ CpuRTBackend     ── SAH BVH, OpenMP, path tracing, FXAA, adaptive AA
  ├──▶ OpenCLRTBackend   ── same BVH on GPU, multi-pass PT within fps budget
  └──▶ MetalRTBackend    ── hardware BVH via Metal RT (intersect<>), Apple Silicon
```

**Backend interface** (`RaytracerBackend.h`):
All rendering mode methods are virtual in the base class with default no-ops.
SceneRaytracer delegates directly — no dynamic_cast anywhere.

### Files

| File | Purpose |
|------|---------|
| `src/Raytracing/RaytracerTypes.h` | POD structs: RTVertex, RTTriangle, RTMaterial (with emission), RTLight, RTInstance, RTRayGenParams, RTMat4 |
| `src/Raytracing/RaytracerBackend.h` | Abstract interface with default no-ops for all mode methods |
| `src/Raytracing/GeometryExtractor.h/.cpp` | Scene → triangles, vertex duplication for correct per-face normals, dirty tracking |
| `src/Raytracing/BVH.h/.cpp` | SAH BVH, flat array, exposes getNodes()/getTriIndices() for GPU upload |
| `src/Raytracing/RaytracerBackend_Cpu.h/.cpp` | CPU: BVH, OpenMP, Blinn-Phong, path tracing with accumulation, FXAA, adaptive AA, object ID buffer |
| `src/Raytracing/RaytracerBackend_OpenCL.h/.cpp` | OpenCL GPU: flattened scene upload, direct + PT kernels, multi-pass, GPU accumulation |
| `src/Raytracing/RaytracerKernel.cl` | OpenCL kernels: `raytrace` (direct + reflections) and `pathTraceKernel` (GI + accumulation) |
| `src/Raytracing/MetalRT.h/.mm` | Lightweight C++ RAII wrappers for Metal: Device, Buffer, ComputePipeline, AccelStruct |
| `src/Raytracing/RaytracerBackend_Metal.h/.mm` | Metal RT backend: hardware BVH (BLAS/TLAS), compute dispatch, Obj-C++ with ARC |
| `src/Raytracing/RaytracerKernel.metal` | MSL kernels: `raytrace` and `pathTrace` using intersect<triangle_data, instancing> |
| `src/Raytracing/SceneRaytracer.h/.cpp` | Orchestrator, backend selection via string arg, camera change detection |
| `src/Raytracing/Upsampling.h/.cpp` | CPU upsampling: bilinear, edge-aware bilateral, nearest-int for object IDs |
| `src/Raytracing/Denoising.h/.cpp` | CPU denoising: bilateral filter, À-Trous wavelet (5-pass hierarchical) |
| `src/Raytracing/MetalRT.h/.mm` also has | `Texture` PIMPL class, pixel format/usage constants, `Device::newTexture()` |
| `demos/raytracing-physics-demo.cpp` | Physics demo: Bullet objects, click-to-glow, hover highlight, GUI controls |
| `demos/raytracing-scene-demo.cpp` | Static desk scene demo |
| `demos/benchmark_rt.cpp` | Backend benchmark: static, dynamic, upsampled modes |
| `demos/test_rt.cpp` | Headless verification test |
| `material-plan.md` | Approved plan for PBR Material class in ICLGeom |

### Features by Backend

| Feature | CPU | OpenCL | Metal RT |
|---------|-----|--------|----------|
| BVH traversal | SAH, iterative stack | Same, on GPU | Hardware (intersect<>) |
| Direct lighting (Blinn-Phong) | ✓ | ✓ | ✓ |
| Shadows | ✓ | ✓ | ✓ (hardware any-hit) |
| Reflections (iterative) | 4 bounces | 4 bounces | 4 bounces |
| Emission | ✓ | ✓ | ✓ |
| Path tracing (GI) | ✓ (temporal accum) | ✓ (GPU accum, multi-pass) | ✓ (GPU accum, multi-pass) |
| FXAA | ✓ | — | — |
| Adaptive AA | ✓ | — | — |
| MSAA (multi-sample) | ✓ | — | — |
| Object ID picking | ✓ | ✓ | ✓ |
| Multi-pass per frame | — | ✓ (fps-budgeted) | ✓ (fps-budgeted) |
| OpenMP parallelism | ✓ | N/A (GPU parallel) | N/A (GPU parallel) |
| Unified memory readback | N/A | explicit copy | zero-copy (shared mem) |
| MetalFX Spatial upscale | — | — | ✓ (perceptual) |
| MetalFX Temporal upscale | — | — | ✓ (depth + motion vectors + jitter) |
| Depth buffer output | — | — | ✓ (for MetalFX/SVGF) |

### ICLGeom Additions

- `SceneObject`: getShininess(), getSpecularReflectance(), setReflectivity(recursive),
  setEmission(color, intensity, recursive), getEmission()
- `SceneLight`: 12 getters (getPosition, getDiffuse, getAmbient, getSpecular, etc.)
- **Cylinder normals**: proper per-vertex radial normals for side faces, flat for caps

### Build & Run

```bash
cmake .. -DBUILD_EXPERIMENTAL=ON -DBUILD_DEMOS=ON
cmake --build . --target raytracing-physics-demo -j16

# Run with different backends
./bin/raytracing-physics-demo -backend metal -fps 30   # hardware BVH (Apple Silicon)
./bin/raytracing-physics-demo -backend opencl -fps 30
./bin/raytracing-physics-demo -backend cpu
./bin/raytracing-physics-demo                          # auto (Metal > OpenCL > CPU)

# Controls: right-drag rotate, wheel zoom, left-click pick/glow
# GUI: pause physics, spawn rate, path tracing toggle, AA options
```

### Key Design Decisions

- **Z-up convention**: matches ICL/Bullet default. No image flip needed.
- **Vertex duplication**: each primitive emits its own vertices during tessellation so
  shared vertices get correct per-face normals (flat shading for cubes, smooth for spheres).
- **Primitive colors**: applied per-face during tessellation (matches `setColor(Primitive::quad, ...)`).
- **Backend base class no-ops**: new backends only override what they support.
- **Multi-pass PT**: OpenCL measures first pass time, loops until fps budget is spent.
- **Accumulation reset**: triggered by any scene mutation (BLAS/TLAS/sceneData rebuild).

### Bugs Fixed

1. **BVH build order** — depth-first allocation for flat BVH invariant
2. **Primitive vs vertex colors** — apply primitive color during tessellation
3. **Shadow acne** — 1mm bias + 0.5mm tMin
4. **Smooth normals on cubes** — vertex duplication eliminates shared-vertex averaging artifacts
5. **Cylinder normals** — radial normals in SceneObject constructor (ICLGeom fix)
6. **OpenCL address space** — `__global const RTMat4*` for transform helpers
7. **Object ID readback** — GPU buffer read back to CPU after render for picking
8. **Mouse handler** — left-click reserved for picking, not forwarded to SceneMouseHandler
9. **Accum ghosting** — explicit buffer clear on `m_accumFrame == 0`

### Metal RT Backend Design (Session 3)

Instead of vendoring metal-cpp (Apple's C++ wrappers), we wrote our own thin
C++ RAII wrapper layer (`MetalRT.h/.mm`) using native Obj-C Metal API directly
in `.mm` files, with ARC handling all object lifetime. This avoids an external
dependency and eliminates the retain/release footgun that metal-cpp has.

**Wrapper types** (`icl::rt::mtl` namespace, in MetalRT.h):
- `Device` — wraps id<MTLDevice> + id<MTLCommandQueue>, factory for all other types
- `Buffer` — wraps id<MTLBuffer>, shared (unified) memory, zero-copy on Apple Silicon
- `ComputePipeline` — wraps id<MTLComputePipelineState>, compiled from MSL source
- `AccelStruct` — wraps id<MTLAccelerationStructure>, used for both BLAS and TLAS
- `InstanceDescriptor` — POD struct binary-compatible with MTLAccelerationStructureInstanceDescriptor

All types use PIMPL (shared_ptr<Impl>). The Impl structs hold Obj-C ivars and
are defined only in the .mm file. The header is pure C++ — no Obj-C visible.

**Key design decisions:**
- Native Obj-C API instead of metal-cpp: ARC eliminates lifetime bugs, no vendor dep
- Wrapper handles RAII + factories; command encoding stays in backend .mm as direct Obj-C
- Hardware BVH: `intersect<triangle_data, instancing>()` replaces manual traversal
- Shadow rays: `accept_any_intersection(true)` for early-out hardware shadow test
- Unified memory: `MTLResourceStorageModeShared` — buffer contents() gives CPU pointer
- Scene flattening: same pattern as OpenCL (flat vertex/triangle buffers + per-instance offsets)
- Shader loaded from file at runtime (same search path pattern as OpenCL kernel)
- Packed indices: RTTriangle has materialIndex field, so we extract uint32_t[3] per tri for Metal

**Bugs avoided from continuation guide:**
- Matrix convention already handled by matToRTMat4() (column-major RTMat4)
- ARC in .mm handles lifetime (no manual retain/release)
- Shadow bias: 1mm offset same as CPU/OpenCL backends

### MetalFX Upscaling Design (Session 4)

**Texture wrapper** (`MetalRT.h/.mm`):
- New `Texture` PIMPL class with `width()`, `height()`, `nativeHandle()`.
- Pixel format constants: `PixelFormatRGBA8Unorm` (70), `R32Float` (55), `RG16Float` (102).
- Texture usage flags: `ShaderRead` (0x01), `ShaderWrite` (0x02), `RenderTarget` (0x04).
- All textures use `MTLStorageModePrivate` (GPU-only, required by MetalFX).
- `Device::newTexture()` factory method.

**Buffer ↔ Texture kernels** (`RaytracerKernel.metal`):
- `planarToRGBA`: converts ICL's planar R/G/B uint8 buffers → RGBA8 texture.
- `rgbaToPlanar`: converts RGBA8 texture → planar R/G/B uint8 buffers.
- `depthToTexture`: copies float depth buffer ��� R32Float texture.
- `computeMotionVectors`: reprojects current world positions through previous
  camera's Q-matrix, writes screen-space motion to RG16Float texture.

**MetalFX Spatial** (`RaytracerBackend_Metal.mm`):
- `MTLFXSpatialScaler` with `PerceptualColorProcessingMode`.
- Per-frame flow: raytrace → planarToRGBA → spatial scale → rgbaToPlanar → readback.
- Scaler is lazily created/recreated when resolution changes.
- Fallback: CPU bilinear/edge-aware if MetalFX not available.

**MetalFX Temporal** (`RaytracerBackend_Metal.mm`):
- `MTLFXTemporalScaler` with depth, motion, color, and output textures.
- Requires `viewProj` matrix in `RTRayGenParams` (forward Q-matrix from ICL Camera).
- Halton(2,3) sub-pixel jitter offsets passed via `jitterOffsetX/Y`.
- History reset on camera jump, scene change, or first frame.
- Per-frame flow: raytrace → planarToRGBA → depthToTexture → computeMotionVectors
  → temporal scale → rgbaToPlanar → readback.
- Previous camera stored in `prevCamera` for motion vector reprojection.

**Depth buffer output**:
- Added `depthOut` parameter to both `raytrace` and `pathTrace` Metal kernels.
- Writes `result.distance` on primary ray hit, `camera.farClip` on miss.
- Buffer indices shifted: direct kernel camera/params at 11/12, PT at 14/15.
- Depth buffer always allocated (shared by MetalFX temporal and future SVGF).

**Key decisions:**
- `viewProj` added to `RTRayGenParams` struct (computed from ICL Camera Q-matrix).
  Q is 3×4, stored as 4×4 RTMat4 with 4th row = (0,0,0,1). Reprojection:
  `screen = Q * worldPos`, then `px = screen.x / screen.z`.
- Motion vectors in normalized [0,1] screen space (MetalFX convention).
- Camera-only reprojection (assumes static geometry positions within frame).
  Per-instance motion would need per-instance previous transforms.
- MetalFX detected at compile time via `__has_include(<MetalFX/MetalFX.h>)`,
  linked via `find_library(METALFX_FRAMEWORK MetalFX)`.

## Next Steps

### Material System (Phase 5)

Approved plan in `material-plan.md`. PBR metallic-roughness model in ICLGeom,
shared via `shared_ptr<Material>`. Old `setColor`/`setShininess` deprecated.

### Upsampling (Phase 6) — Sessions A-C done

Detailed plan in `upsampling-plan.md`. All three sessions are implemented:
- **Session A** (done): Bilinear + Edge-Aware CPU upsampling (all backends)
- **Session B** (done): MetalFX Spatial — Texture wrapper, buffer↔texture kernels, spatial scaler
- **Session C** (done): MetalFX Temporal — depth output, motion vectors, jitter, temporal scaler

Remaining:
- **UpsamplingOp in ICLFilter** — promote bilinear/edge-aware upsampling to a proper
  `UnaryOp` filter at `ICLFilter/src/ICLFilter/UpsamplingOp.h/.cpp` so it's available
  outside the raytracing framework as a general-purpose image upscaling filter.

### Denoising (Phase 7)

Path tracing at low sample counts produces noisy images. Three tiers of denoising,
each building on infrastructure from the previous:

**Tier 1 — À-Trous wavelet filter** (no dependencies, all backends)
- Hierarchical edge-preserving filter: 5×5 kernel applied 4-5 times with
  increasing step size (1, 2, 4, 8, 16). Each pass is cheap (~0.1ms on GPU).
- Weights guided by color difference (edge-stopping function).
- No auxiliary buffers needed — works on the color image alone.
- ~200 lines as a Metal compute kernel + CPU fallback.
- Good quality for mild noise (8-32 spp), blurs at very low spp.

**Tier 2 — SVGF** (Spatiotemporal Variance-Guided Filtering)
- Industry standard for real-time PT denoising (used in Quake II RTX, etc.).
- Combines: temporal reprojection (reuse previous frame), per-pixel variance
  estimation (how noisy is this pixel?), and À-Trous spatial filter guided by
  depth + normals + variance.
- 4-5 spatial filter passes per frame, very fast on GPU.
- **Requires: depth buffer, normals buffer, motion vectors** — same prerequisites
  as MetalFX Temporal (Phase 6 Session C). Implement after that.
- Quality: very good at 1-4 spp, enables real-time PT at interactive rates.

**Tier 3 — Intel OIDN** (Open Image Denoise)
- ML-trained neural network denoiser, best single-frame quality.
- Accepts RGB + optional albedo + normal auxiliary inputs.
- CPU backend (SSE/AVX) + GPU backends (SYCL, CUDA, HIP — no Metal yet).
- Apache 2.0, ~50MB library dependency.
- Integration: link `libOpenImageDenoise`, pass Img8u data as float buffers.
- Best for offline/preview quality; CPU cost (~20ms for 1080p) may be too high
  for real-time on CPU, but acceptable as a toggle for final-quality renders.

**Also relevant:**
- **Apple MPS denoisers** — Metal Performance Shaders has temporal/spatial
  denoisers, worth investigating as a Metal-native alternative to OIDN.
- **Non-local means (NLM)** — patch-based denoiser, better than bilateral,
  simpler than SVGF. Could be a middle ground if SVGF is too complex.

**Recommended order:** À-Trous (Tier 1) is done. SVGF is next — the depth
buffer and motion vector infrastructure from MetalFX Temporal (Phase 6C)
is now in place. A normals buffer output would also be needed.

### Future Improvements

- **MetalWidget** — QWidget wrapping CAMetalLayer for zero-copy display in ICLQt
- **Texture support** — UV mapping and texture sampling in materials
- **Analytic shapes** — sphere/cylinder as native primitives (Metal custom intersection)
- **Tone mapping** — HDR → LDR with exposure control (currently just clamp to [0,1])
- **Environment maps** — HDR sky/environment instead of flat background color

### Known Limitations

- No texture primitive support (rendered as flat-colored quads)
- No line / text / billboard rendering
- Physics demo now uses `invalidateTransforms()` for transform-only updates (fixed in session 3)
- Shadow bias (1mm) may cause light leaking on very thin geometry
- OpenCL path on macOS goes through cl2Metal translation (deprecated, may break in future macOS) — Metal RT backend is the preferred alternative
- Metal RT requires macOS 13+ and Apple Silicon (runtime check via supportsRaytracing())
- Mouse hover coordinate mapping may be off when widget is scaled (Retina)
