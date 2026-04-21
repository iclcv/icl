# Raytracing — Continuation Guide

## Current State (Session 1 — CPU backend, BVH, reflections)

### What Was Built

A complete CPU raytracing pipeline that renders ICL `Scene` objects in real-time.

**Architecture:**
```
Scene (SceneObjects, Cameras, SceneLights)
  │
  ▼
GeometryExtractor ── tessellate quads/polygons → triangles, extract materials/lights
  │
  ▼
SceneRaytracer ── dirty tracking, orchestration
  │
  └──▶ CpuRTBackend ── SAH BVH (two-level), Möller-Trumbore, Blinn-Phong, OpenMP
```

**Files created:**

| File | Purpose |
|------|---------|
| `src/Raytracing/RaytracerTypes.h` | GPU-compatible POD structs (alignas(16)): RTVertex, RTTriangle, RTMaterial, RTLight, RTInstance, RTRayGenParams, RTMat4 |
| `src/Raytracing/RaytracerBackend.h` | Abstract backend interface: buildBLAS/TLAS, setSceneData, render, readback |
| `src/Raytracing/GeometryExtractor.h/.cpp` | Walks Scene graph, tessellates quads/polygons to triangles, extracts lights/camera, per-object dirty tracking |
| `src/Raytracing/BVH.h/.cpp` | SAH BVH with binning (12 bins), flat array layout (left child at idx+1), iterative stack-based traversal, ray-AABB slab test, Möller-Trumbore ray-triangle |
| `src/Raytracing/RaytracerBackend_Cpu.h/.cpp` | CPU backend: two-level BVH (per-object BLAS + scene TLAS via instance transforms), Blinn-Phong shading, shadow rays, recursive reflections (up to 4 bounces), OpenMP parallel scanlines |
| `src/Raytracing/SceneRaytracer.h/.cpp` | Orchestrator: extracts geometry, builds/updates accel structures, renders via backend, auto-selects best backend |
| `demos/raytracing-scene-demo.cpp` | Interactive demo: desk scene with lamp, shapes, mouse camera control (SceneMouseHandler), FPS overlay |
| `demos/test_rt.cpp` | Headless verification test |
| `CMakeLists.txt` | Builds ICLRaytracing shared lib + demos, links OpenMP via Homebrew libomp on macOS |

**ICLGeom additions (public API):**
- `SceneObject::getShininess()` / `getSpecularReflectance()` — new getters for existing protected members
- `SceneObject::setReflectivity(float)` / `getReflectivity()` — new property (0=matte, 1=mirror), stored as `m_reflectivity`
- `SceneLight` — 12 new getters: `getPosition()`, `getAmbient()`, `getDiffuse()`, `getSpecular()`, `getSpotDirection()`, `getSpotExponent()`, `getSpotCutoff()`, `getAttenuation()`, `isAmbientEnabled()`, `isDiffuseEnabled()`, `isSpecularEnabled()`

### Bugs Fixed During Development

1. **BVH build order** — Both children were allocated before recursing, breaking the flat BVH invariant (left child at idx+1). Fixed by depth-first allocation: allocate left → recurse left → allocate right → recurse right.

2. **Primitive vs vertex colors** — `setColor(Primitive::quad, ...)` sets the primitive's color, not vertex colors. GeometryExtractor now applies primitive colors to vertices during tessellation.

3. **Shadow acne** — Shadow ray origin bias was 0.1mm, too small for mm-scale geometry. Increased to 1.0mm offset along normal + 0.5mm tMin in object space.

4. **Image Y-flip** — ICL camera pixel y=0 is bottom of view, but Img8u row 0 is top. Fixed by writing to `row (h-1-y)` in the render loop.

5. **Specular view direction** — `shade()` now receives the actual view ray direction instead of approximating from the origin.

### Build & Run

```bash
# Configure (from build/)
cmake .. -DBUILD_EXPERIMENTAL=ON -DBUILD_DEMOS=ON

# Build
cmake --build . --target raytracing-scene-demo -j16

# Run (interactive, mouse: left-drag orbit, right-drag pan, wheel zoom)
./bin/raytracing-scene-demo

# Headless test
cmake --build . --target test-raytracer -j16
./bin/test-raytracer
```

### Performance (Session 1, CPU only)

- ~71 fps at 640x480 with simple scene (3 objects), single-threaded
- Real-time at 800x600 with desk scene (8 objects, 2 lights, reflections), with OpenMP on M3 Max

### Design Decisions

- **Two-level BVH**: per-object BLAS in object-local coords, world-space TLAS via instance transforms. Matches Metal RT's architecture so migration is natural.
- **Column-major RTMat4**: Metal convention. `matToRTMat4()` transposes from ICL's row-major `Mat4D32f`.
- **Primitive colors over vertex colors**: During tessellation, each primitive's color is stamped onto its vertices. This matches how most ICL demos use `setColor(Primitive::quad, ...)`.
- **invalidateAll() per frame**: Camera changes from SceneMouseHandler are not tracked, so full re-extraction runs each frame. Camera params are cheap; geometry is skipped if vertex/primitive counts + transforms haven't changed.

## Next Steps

### Phase 4: Metal RT Backend (primary goal)
- Vendor `metal-cpp` in `3rdparty/metal-cpp/`
- `RaytracerBackend_Metal.h/.mm` — MTLDevice, MTLCommandQueue, hardware BLAS/TLAS build
- `shaders/Raytracing.metal` — ray gen compute kernel, hardware intersection, Blinn-Phong + shadows + reflections
- `MetalWidget.h/.mm` — QWidget wrapping CAMetalLayer for zero-copy display in ICLQt
- CMake: detect Metal framework, compile .metal → .metallib via xcrun
- Expected: 5-10x speedup over CPU at current scene complexity

### Phase 5: Improvements
- **Camera dirty detection** — avoid `invalidateAll()` every frame; compare camera params
- **Texture support** — TexturePrimitive/TextureGridPrimitive UV mapping and texture sampling
- **Ground plane / infinite plane** — analytic intersection (no triangulation needed)
- **Analytic shapes** — sphere/cylinder/cone as native primitives via Metal custom intersection functions
- **Soft shadows** — area lights with jittered shadow rays
- **Ambient occlusion** — short-range occlusion rays for contact shadows

### Phase 6: Optimization
- **Double-buffered TLAS** — render frame N while building TLAS for frame N+1
- **NEON SIMD** — 4-wide ray packets for CPU backend
- **Adaptive resolution** — half-res during camera motion, full-res when static
- **Offscreen rendering** — render to Img8u without a window for video encoding / filter pipeline

### Known Limitations
- No texture primitive support (rendered as flat-colored quads)
- No line rendering (lines are skipped)
- No text/billboard primitives
- `invalidateAll()` called every frame (camera dirty detection missing)
- Geometry changes within a SceneObject (e.g., deforming mesh vertices in-place) not detected — must call `invalidateObject()` explicitly
- Shadow bias (1mm) may cause light leaking on very thin geometry
