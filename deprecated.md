# Deprecated API Usage Report

Generated from debug build on 2026-04-12 (Meson, Apple Clang, ARM64).

## Summary

77 deprecated-declaration warnings across 23 files in 6 modules.
All stem from two deprecated `SceneObject` methods that should be replaced with `setMaterial()`.

## Deprecated APIs

### `SceneObject::setColor(Primitive::Type, GeomColor, bool recursive)` (76 sites)

**Replacement:** `setMaterial(Material::fromColor(color))` or construct a `Material` directly.

```cpp
// OLD:
obj->setColor(Primitive::quad, GeomColor(200, 200, 200, 255));

// NEW:
obj->setMaterial(Material::fromColor(GeomColor(200, 200, 200, 255)));
```

Note: `setColor` sets color per-primitive-type; `setMaterial` sets a default material
for the whole object. Callers that set different colors for different primitive types
(e.g. line vs quad) will need per-primitive materials via `setMaterial(primIdx, mat)`.

### `SceneObject::setShininess(icl8u, bool recursive)` (1 site)

**Replacement:** Set `Material::roughness` directly.

```cpp
// OLD:
obj->setShininess(128);

// NEW:
auto mat = obj->getOrCreateMaterial();
mat->roughness = 0.125f;  // sqrt(2 / (128 + 2))
```

### Also deprecated but not yet triggered in build

These are marked `[[deprecated]]` in `SceneObject.h` but have no current callers:

- `setSpecularReflectance()` -- use `Material::metallic`
- `setReflectivity()` -- use `Material::reflectivity`
- `setEmission()` -- use `Material::emissive`

## Affected Files

### Library code (migrate first -- affects API consumers)

| File | Lines | API |
|------|-------|-----|
| `icl/geom/ComplexCoordinateFrameSceneObject.cpp` | 60, 61, 67, 68 | setColor |
| `icl/geom/CoordinateFrameSceneObject.h` | 42 | setColor |
| `icl/geom/PlotWidget3D.cpp` | 226, 227, 239, 240, 241 | setColor |
| `icl/geom/Primitive3DFilter.cpp` | 616 | setColor |
| `icl/geom/Scene.cpp` | 349 | setColor |
| `icl/geom/SceneLightObject.cpp` | 154, 160 | setColor |
| `icl/geom/SceneLightObject.cpp` | 162 | setShininess |
| `icl/physics/ManipulatablePaper.cpp` | 74, 213, 263, 274, 285, 377, 468 | setColor |
| `icl/physics/PhysicsPaper3MouseHandler.cpp` | 58, 64 | setColor |
| `icl/physics/PhysicsUtils.cpp` | 53 | setColor |

### Apps/demos (migrate after library code)

| File | Lines | API |
|------|-------|-----|
| `icl/filter/apps/color-segmentation.cpp` | 78, 79, 89, 127 | setColor |
| `icl/geom/apps/depth-camera-simulator.cpp` | 75, 83, 84, 85 | setColor |
| `icl/geom/apps/surf-based-object-tracking.cpp` | 29, 30 | setColor |
| `icl/geom/demos/ray-cast-octree.cpp` | 90 | setColor |
| `icl/geom/demos/scene-graph.cpp` | 29 | setColor |
| `icl/geom/demos/scene-object.cpp` | 34, 64 | setColor |
| `icl/geom/demos/scene-shadows.cpp` | 17, 79 | setColor |
| `icl/geom/demos/texture-cube.cpp` | 43, 79, 80 | setColor |
| `icl/markers/apps/camera-calibration-CameraCalibrationUtils.cpp` | 277, 278, 448 | setColor |
| `icl/markers/apps/camera-calibration.cpp` | 328, 329, 330, 337, 339 | setColor |
| `icl/math/demos/polynomial-regression.cpp` | 117, 144 | setColor |
| `icl/physics/demos/phyisics-car.cpp` | 90, 92 | setColor |
| `icl/physics/demos/phyisics-constraints.cpp` | 36 | setColor |
| `icl/physics/demos/physics-maze-HoleObject.h` | 20, 28, 31 | setColor |
| `icl/physics/demos/physics-maze-MazeObject.cpp` | 158-160, 204-206, 305, 318, 323, 325 | setColor |
| `icl/physics/demos/physics-paper-DefaultGroundObject.h` | 24 | setColor |
| `icl/physics/demos/physics-scene.cpp` | 34 | setColor |

## Remaining non-deprecated warnings (external, unfixable)

These 13 warnings come from external headers and cannot be fixed in ICL code:

- **Qt/GLEW/OpenGL conflicts** (9): `qopenglfunctions.h` warns about GLEW incompatibility, `gl3.h` warns about gl.h co-inclusion. Caused by ICL including both GLEW and Qt OpenGL headers.
- **Bullet unused-but-set** (2): `btMatrixX.h:245` (`count`), `btLemkeSolver.h:241` (`m_errorCountTimes`).
- **Cycles macro redefined** (2): `SSE2NEON_PRECISE_MINMAX` in `3rdparty/cycles/src/util/simd.h`.
