# Raytracing — Continuation Guide

## Current State (Session 5 — Denoisers, Tone Mapping, Area Lights, Tooling)

### What Was Built

Full real-time path tracing pipeline with hardware-accelerated BVH on Apple
Silicon, MetalFX upscaling, GPU-native denoising (À-Trous + SVGF), tone mapping,
emissive area light sampling, XML scene loading, and an offline renderer for
visual verification — all with a modular backend architecture.

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

## Next Steps

### Material System (Phase 5)

Approved plan in `material-plan.md`. PBR metallic-roughness model in ICLGeom,
shared via `shared_ptr<Material>`. Old `setColor`/`setShininess` deprecated.

### Remaining Work

- **GPU SVGF improvements** — the variance-guided spatial filter works but could
  benefit from tuning sigma parameters; temporal variance tracking could be made
  more robust for dynamic scenes
- **glTF import** — extend SceneLoader with tinygltf for standard 3D model loading
- **UpsamplingOp in ICLFilter** — promote bilinear/edge-aware to a proper UnaryOp
- **Environment maps** — HDR sky/environment instead of flat background
- **Texture/UV support** — UV mapping and texture sampling in materials
- **MetalWidget** — QWidget wrapping CAMetalLayer for zero-copy display in ICLQt

### Known Limitations

- No texture primitive support (rendered as flat-colored quads)
- No line / text / billboard rendering
- Shadow bias (1mm) may cause light leaking on very thin geometry
- OpenCL path on macOS goes through cl2Metal translation (deprecated)
- Metal RT requires macOS 13+ and Apple Silicon
- MetalFX Temporal requires RGBA16Float color format + Depth32Float
- SVGF reflectivity preservation reduces denoising on mirror surfaces (by design)
