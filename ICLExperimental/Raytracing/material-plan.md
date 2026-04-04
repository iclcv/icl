# Material System for ICL — Implementation Plan

## Context

ICL currently has no Material concept. Visual properties are scattered:
- **Per-primitive**: `Primitive::color` (diffuse only, RGBA)
- **Per-object**: `SceneObject::m_shininess`, `m_specularReflectance`, `m_reflectivity`, `m_useSmoothShading`
- **Per-primitive (textures only)**: `TexturePrimitive::texture`, `alphaFunc`, `alphaValue`

This makes it impossible to assign different materials to different faces of an object, or to share a material across objects. The raytracer's `RTMaterial` struct is already a step toward a unified material, but it's raytracer-internal.

**Goal**: Introduce a proper `Material` class in ICLGeom with PBR metallic-roughness parameters, shared via `shared_ptr`, supporting both the existing OpenGL renderer and the raytracer. Old per-primitive color / per-object shininess API is deprecated in favor of materials.

## Decisions

| Decision | Choice |
|----------|--------|
| Location | ICLGeom (core, not experimental) |
| Shading model | PBR metallic-roughness (glTF/USD-compatible) |
| Textures | Material holds optional textures; legacy TexturePrimitive continues working |
| Ownership | `shared_ptr<Material>`, global — can live anywhere, ref-counted |
| Migration | Deprecate `setColor`/`setShininess`/`setSpecularReflectance`; convenience methods still work but create default materials internally |

## Material Class Design

```cpp
// ICLGeom/src/ICLGeom/Material.h
namespace icl::geom {

class Material {
public:
  /// PBR metallic-roughness parameters
  GeomColor baseColor{200, 200, 200, 255};  // albedo (stored [0,1] internally)
  float metallic = 0.0f;         // 0 = dielectric, 1 = metal
  float roughness = 0.5f;        // 0 = mirror, 1 = diffuse
  float reflectivity = 0.0f;     // explicit mirror reflections (for raytracing)
  GeomColor emissive{0,0,0,255}; // self-illumination

  /// Optional textures (nullptr = not used)
  std::shared_ptr<qt::GLImg> baseColorMap;   // albedo texture
  std::shared_ptr<qt::GLImg> normalMap;      // tangent-space normal map
  std::shared_ptr<qt::GLImg> metallicRoughnessMap; // R=metallic, G=roughness
  std::shared_ptr<qt::GLImg> emissiveMap;

  /// Alpha handling
  float alphaCutoff = 0.5f;      // discard fragments below this
  enum AlphaMode { Opaque, Mask, Blend } alphaMode = Opaque;

  /// Display hints (from SceneObject, now per-material)
  bool smoothShading = true;
  bool doubleSided = false;

  /// Name for debugging/serialization
  std::string name;

  /// Convenience: create from legacy parameters
  static std::shared_ptr<Material> fromColor(const GeomColor &color,
                                              float shininess = 128,
                                              float reflectivity = 0);

  /// Convenience: create from Phong parameters (auto-converts to PBR)
  /// roughness ≈ 1 - shininess/255, metallic from specular intensity
  static std::shared_ptr<Material> fromPhong(const GeomColor &diffuse,
                                              const GeomColor &specular,
                                              float shininess);
};

} // namespace icl::geom
```

### Phong-to-PBR Conversion

For backwards compatibility, the `fromPhong()` factory converts:
- `baseColor` = diffuse color
- `roughness` ≈ `sqrt(2.0 / (shininess + 2))` (standard approximation)
- `metallic` ≈ `luminance(specularColor) > 0.5 ? 1 : 0` (heuristic)
- `reflectivity` = carried through directly

### PBR-to-Phong Fallback (for OpenGL renderer)

Until the OpenGL renderer supports PBR natively:
- `glColor4fv(material.baseColor)` — diffuse
- `shininess` ≈ `2.0 / (roughness² + ε) - 2` (inverse of above)
- `specularColor` = `baseColor * metallic + white * (1-metallic) * (1-roughness)`
- `glMaterialfv(GL_SPECULAR, ...)` from computed specular

## Primitive Integration

### Option: Material pointer on Primitive base

```cpp
struct Primitive {
  Type type;
  GeomColor color;  // DEPRECATED — kept for legacy
  std::shared_ptr<Material> material;  // NEW — if set, overrides color + object material

  // Returns effective material: primitive's own > object default > auto-generated from color
  const Material* getEffectiveMaterial(const Material* objectDefault) const;
};
```

### SceneObject changes

```cpp
class SceneObject {
  // NEW: default material for all primitives that don't have their own
  std::shared_ptr<Material> m_defaultMaterial;

  // NEW API
  void setMaterial(std::shared_ptr<Material> mat);  // set default
  void setMaterial(int primIdx, std::shared_ptr<Material> mat);  // per-primitive
  std::shared_ptr<Material> getMaterial() const;
  std::shared_ptr<Material> getMaterial(int primIdx) const;

  // DEPRECATED (still functional, creates/modifies default material internally)
  [[deprecated]] void setColor(Primitive::Type t, const GeomColor &color, bool recursive = true);
  [[deprecated]] void setShininess(icl8u value);
  [[deprecated]] void setSpecularReflectance(const GeomColor &values);
  [[deprecated]] void setReflectivity(float value);
};
```

When `setColor()` is called on an object that has no material yet, it auto-creates one via `Material::fromColor()`. Subsequent `setShininess()` etc. modify the same material.

## Raytracer Integration

The `GeometryExtractor` currently builds `RTMaterial` from scattered properties. With the Material class:

```cpp
// GeometryExtractor.cpp — simplified
RTMaterial extractMaterial(const Material &mat) {
  RTMaterial rt;
  rt.baseColor = {mat.baseColor[0], mat.baseColor[1], mat.baseColor[2], mat.baseColor[3]};
  rt.metallic = mat.metallic;
  rt.roughness = mat.roughness;
  rt.reflectivity = mat.reflectivity;
  rt.emissive = {mat.emissive[0], mat.emissive[1], mat.emissive[2], mat.emissive[3]};
  return rt;
}
```

The `RTMaterial` struct gets updated to match PBR:
```cpp
struct alignas(16) RTMaterial {
  RTFloat4 baseColor;
  RTFloat4 emissive;
  float metallic;
  float roughness;
  float reflectivity;
  float alphaCutoff;
};
```

The CPU backend's shading function transitions from Blinn-Phong to a PBR BRDF (Cook-Torrance microfacet model), which is actually simpler code-wise and more physically correct.

## OpenGL Renderer Adaptation

Minimal changes in `Scene::renderScene()`:

```cpp
// Before rendering each object's primitives:
const Material *mat = obj->getMaterial().get();
if (mat) {
  // Convert PBR → legacy GL material state
  auto phong = mat->toPhongParams();
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, phong.specular.data());
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &phong.shininess);
  glColor4fv(mat->baseColor.data());
}
```

Per-primitive material override happens in `Primitive::render()`:
```cpp
if (material) {
  auto phong = material->toPhongParams();
  glMaterialfv(...);
  glColor4fv(material->baseColor.data());
}
```

## Files to Create/Modify

### New files
- `ICLGeom/src/ICLGeom/Material.h` — Material class definition
- `ICLGeom/src/ICLGeom/Material.cpp` — fromColor(), fromPhong(), toPhongParams()

### Modified files
- `ICLGeom/src/ICLGeom/Primitive.h` — add `shared_ptr<Material> material` to base Primitive
- `ICLGeom/src/ICLGeom/SceneObject.h` — add `m_defaultMaterial`, new API, deprecate old API
- `ICLGeom/src/ICLGeom/SceneObject.cpp` — implement material-aware setColor etc.
- `ICLGeom/src/ICLGeom/Scene.cpp` — material state setup in render path
- `ICLGeom/src/ICLGeom/Primitive.cpp` — use material in render() methods
- `ICLExperimental/Raytracing/src/Raytracing/RaytracerTypes.h` — update RTMaterial for PBR
- `ICLExperimental/Raytracing/src/Raytracing/GeometryExtractor.cpp` — extract from Material
- `ICLExperimental/Raytracing/src/Raytracing/RaytracerBackend_Cpu.cpp` — PBR shading (Cook-Torrance)

## Phased Implementation

### Phase 1: Material class + SceneObject integration
- Create `Material.h/.cpp` in ICLGeom
- Add `m_defaultMaterial` and `setMaterial()`/`getMaterial()` to SceneObject
- Add `shared_ptr<Material>` to Primitive base
- Mark old APIs `[[deprecated]]`
- Existing behavior unchanged — deprecated methods auto-create materials

### Phase 2: Raytracer PBR shading
- Update `RTMaterial` to PBR fields
- Update `GeometryExtractor` to read from Material
- Implement Cook-Torrance BRDF in CPU backend (replaces Blinn-Phong)
- Test: existing demo should look similar (Phong→PBR conversion preserves appearance)

### Phase 3: OpenGL renderer adaptation
- Update `Scene::renderScene()` to read material state
- Update `Primitive::render()` for per-primitive material
- PBR→Phong fallback for legacy GL

### Phase 4: Texture support in materials
- Wire `baseColorMap` through to raytracer (UV interpolation + texture sampling)
- Wire through to OpenGL (bind texture in render)
- Convert existing TexturePrimitive usage to use material textures where possible

## Verification

1. **Backwards compat**: existing demos compile and run unchanged (deprecated warnings only)
2. **Material demo**: new demo with explicit materials — metal sphere, rough wood, glossy plastic
3. **Raytracer**: PBR shading produces visually correct results (energy-conserving, Fresnel)
4. **A/B test**: same scene rendered with old Phong and new PBR-via-Phong-conversion should look ~similar
