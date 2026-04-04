# Raytracing — Continuation Guide

## Current State (Session 2 — OpenCL backend, path tracing, physics, interaction)

### What Was Built

Three rendering backends, path tracing with global illumination, Bullet physics
integration, and interactive object picking — all running in real-time.

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
  ├──▶ CpuRTBackend    ── SAH BVH, OpenMP, path tracing, FXAA, adaptive AA
  └──▶ OpenCLRTBackend  ── same BVH on GPU, multi-pass PT within fps budget
       (future: MetalRTBackend — hardware BVH on Apple Silicon)
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
| `src/Raytracing/SceneRaytracer.h/.cpp` | Orchestrator, backend selection via string arg, camera change detection |
| `demos/raytracing-physics-demo.cpp` | Physics demo: Bullet objects, click-to-glow, hover highlight, GUI controls |
| `demos/raytracing-scene-demo.cpp` | Static desk scene demo |
| `demos/test_rt.cpp` | Headless verification test |
| `material-plan.md` | Approved plan for PBR Material class in ICLGeom |

### Features by Backend

| Feature | CPU | OpenCL |
|---------|-----|--------|
| BVH traversal | SAH, iterative stack | Same, on GPU |
| Direct lighting (Blinn-Phong) | ✓ | ✓ |
| Shadows | ✓ | ✓ |
| Reflections (iterative) | 4 bounces | 4 bounces |
| Emission | ✓ | ✓ |
| Path tracing (GI) | ✓ (temporal accum) | ✓ (GPU accum, multi-pass) |
| FXAA | ✓ | — |
| Adaptive AA | ✓ | — |
| MSAA (multi-sample) | ✓ | — |
| Object ID picking | ✓ | ✓ |
| Multi-pass per frame | — | ✓ (fps-budgeted) |
| OpenMP parallelism | ✓ | N/A (GPU parallel) |

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
./bin/raytracing-physics-demo -backend opencl -fps 30
./bin/raytracing-physics-demo -backend cpu
./bin/raytracing-physics-demo                         # auto (OpenCL > CPU)

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

## Next Steps

### Metal RT Backend (Phase 4)

Hardware-accelerated raytracing on Apple Silicon. Expected 2-5x faster than OpenCL
for direct lighting, more for path tracing (hardware BVH traversal).

**Prerequisites:**
- Vendor `metal-cpp` (Apple's C++ Metal headers) in `3rdparty/metal-cpp/`
  - Header-only, ~200KB, download from https://developer.apple.com/metal/cpp/
  - Provides `MTL::Device`, `MTL::CommandQueue`, `MTL::AccelerationStructure`, etc.

**Files to create:**
```
src/Raytracing/RaytracerBackend_Metal.h      — header (C++)
src/Raytracing/RaytracerBackend_Metal.mm     — implementation (Obj-C++ for Metal API)
shaders/Raytracing.metal                     — Metal Shading Language kernels
```

**Implementation plan:**

1. **Device setup** (`RaytracerBackend_Metal` constructor):
   - `MTL::Device::createSystemDefaultDevice()`
   - `device->newCommandQueue()`
   - Check `device->supportsRaytracing()` → fall back if false
   - Compile `.metal` shader from `.metallib` or source

2. **buildBLAS** — per-object acceleration structure:
   - Create `MTL::AccelerationStructureTriangleGeometryDescriptor`
   - Set vertex buffer (positions), index buffer (triangle indices)
   - `device->newAccelerationStructure(descriptor)` → builds BVH in hardware
   - Store per-object: vertex/normal/color buffers + accel structure

3. **buildTLAS** — scene-level instance acceleration structure:
   - Create `MTL::InstanceAccelerationStructureDescriptor`
   - Per instance: transform matrix (column-major) + BLAS reference
   - Rebuild when any transform changes (Metal supports refit for small changes)

4. **render** — compute shader dispatch:
   - Encode compute command with ray generation kernel
   - Bind TLAS, per-instance buffers, light/material buffers, camera params
   - Use `intersect<triangle_data, instancing>()` for hardware BVH traversal
   - Closest-hit: interpolate surface, shade (Blinn-Phong or path trace)
   - Shadow: `intersect(..., accept_any_intersection)` for each light
   - Output to `MTL::Texture` (RGBA8Unorm)
   - Read back to `Img8u` via `getBytes()`

5. **Path tracing kernel** (`pathTraceKernel` in Metal):
   - Same algorithm as OpenCL: cosine hemisphere, NEE, Russian roulette
   - GPU-side accumulation in float buffers
   - Multi-pass within fps budget (same timing approach)
   - Per-thread RNG via xorshift (same as OpenCL kernel)

6. **CMake integration**:
   ```cmake
   if(APPLE)
     find_library(METAL_FRAMEWORK Metal)
     if(METAL_FRAMEWORK)
       list(APPEND RT_SOURCES src/Raytracing/RaytracerBackend_Metal.mm)
       target_link_libraries(ICLRaytracing ${METAL_FRAMEWORK})
       target_include_directories(ICLRaytracing PRIVATE ${ICL_SOURCE_DIR}/3rdparty/metal-cpp)
       # Compile .metal → .metallib
       add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Raytracing.metallib
         COMMAND xcrun -sdk macosx metal -c ${CMAKE_CURRENT_SOURCE_DIR}/shaders/Raytracing.metal
                 -o ${CMAKE_CURRENT_BINARY_DIR}/Raytracing.air
         COMMAND xcrun -sdk macosx metallib ${CMAKE_CURRENT_BINARY_DIR}/Raytracing.air
                 -o ${CMAKE_CURRENT_BINARY_DIR}/Raytracing.metallib
         DEPENDS shaders/Raytracing.metal)
     endif()
   endif()
   ```

**Key differences from OpenCL backend:**
- Hardware BVH: `intersect<>()` replaces the manual traversal loop — major speedup
- Native Metal: no cl2Metal translation layer overhead
- Shared memory: Apple Silicon unified memory avoids explicit CPU↔GPU copies
- Instance acceleration structure: native support for two-level BVH with transforms

**Matrix convention:** ICL uses row-major `Mat4D32f`, Metal uses column-major `float4x4`.
The existing `matToRTMat4()` in GeometryExtractor already handles this conversion.

**Risks:**
- Metal RT requires macOS 13+ and Apple Silicon (M1+)
- `.mm` files need Objective-C++ compilation (`-ObjC++` flag)
- metal-cpp wraps Obj-C retain/release — need careful lifetime management
- Shader compilation errors are runtime, not compile-time

### Material System (Phase 5)

Approved plan in `material-plan.md`. PBR metallic-roughness model in ICLGeom,
shared via `shared_ptr<Material>`. Old `setColor`/`setShininess` deprecated.

### Future Improvements

- **MetalWidget** — QWidget wrapping CAMetalLayer for zero-copy display in ICLQt
- **Texture support** — UV mapping and texture sampling in materials
- **Analytic shapes** — sphere/cylinder as native primitives (Metal custom intersection)
- **Denoising** — bilateral filter or OIDN on noisy path traced output
- **Tone mapping** — HDR → LDR with exposure control (currently just clamp to [0,1])
- **Environment maps** — HDR sky/environment instead of flat background color

### Known Limitations

- No texture primitive support (rendered as flat-colored quads)
- No line / text / billboard rendering
- `invalidateAll()` called every frame when physics runs (no per-property dirty tracking)
- Shadow bias (1mm) may cause light leaking on very thin geometry
- OpenCL path on macOS goes through cl2Metal translation (deprecated, may break in future macOS)
- Mouse hover coordinate mapping may be off when widget is scaled (Retina)
