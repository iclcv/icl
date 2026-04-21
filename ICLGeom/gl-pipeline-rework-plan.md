# OpenGL Pipeline Rework Plan

## Problem Statement

The current GL rendering pipeline is a legacy fixed-function OpenGL system (glBegin/glEnd,
glMaterialfv, matrix stack) with a bolted-on "improved shading" GLSL overlay for per-pixel
lighting and shadows. This hybrid approach has accumulated issues:

- **Specular lighting doesn't work** (acknowledged in code comments)
- **Shadow mapping is broken** (light follows camera, shadows don't appear)
- **Material textures are ignored** (baseColorMap, normalMap etc. have no GL path)
- **All geometry uses immediate mode** (glBegin/glEnd — 20+ year old API)
- **Uses deprecated GL extensions** (EXT framebuffers, ARB multitexture)
- **Improved shading shaders use deprecated built-ins** (gl_Vertex, gl_LightSource[], etc.)
- **Cannot work on macOS GL Core Profile** (which is the only supported profile going forward)

## Critical Constraint: Camera Model Alignment

ICL's core feature is that a calibrated camera's rendering perfectly overlays real camera
images. `Camera::getProjectionMatrixGL()` computes a GL projection matrix from camera
intrinsics (fx, fy, cx, cy). **This must be preserved pixel-perfectly.**

The good news: this works identically in modern GL. The projection matrix is just a uniform
in the vertex shader instead of going through glMatrixMode/glLoadMatrixf. Same math, same
result. The camera model is purely a matrix — it doesn't depend on any deprecated GL features.

ICL's unusual convention: `Camera::m_up` points toward positive image-Y (visual down).
The `getCSTransformationMatrixGL()` method handles the GL coordinate system conversion.
Both methods must continue to produce identical matrices.

## What to Keep

- **Camera system** (`Camera.h/.cpp`) — calibration, intrinsics, projection matrices
- **Scene graph** (`Scene.h`, `SceneObject.h`) — object hierarchy, transforms, visibility
- **Light system** (`SceneLight.h/.cpp`) — 8 lights, positions, colors, shadow cameras
- **Material system** (`Material.h/.cpp`) — PBR parameters, texture maps as `Image`
- **Primitive types** (`Primitive.h`) — triangle/quad/polygon/texture/text hierarchy
- **UV indices on primitives** — `i(6..8)` for triangles, `i(8..11)` for quads
- **GLImg** (`GLImg.h/.cpp`) — texture upload/caching, works well

## What to Remove / Replace

| Current | Replace With |
|---------|-------------|
| `glBegin/glEnd` immediate mode | VBO + VAO per SceneObject |
| `glMatrixMode/glPushMatrix` | Uniform matrices in shader |
| `glMaterialfv/glLightfv` | Uniforms in shader |
| `glShadeModel` | Always smooth (shader handles it) |
| `glEnableClientState` | VAO vertex attributes |
| Display lists | VBO (GPU-resident, update on dirty) |
| `GL_LIGHTING` enable/disable | Shader handles everything |
| `glColor4fv` per-vertex legacy | Vertex attribute or material uniform |
| EXT framebuffer extensions | Core GL 3.0+ `glGenFramebuffers` |
| ShaderUtil dynamic GLSL generation | Pre-written shader files with `#ifdef` |
| `gl_Vertex`, `gl_LightSource[]` etc. | `in vec3 position`, uniform arrays |

## Proposed Architecture

### Rendering Pipeline

```
Scene::renderScene(camIndex)
│
├─ 1. Shadow Pass (for each shadow-enabled light)
│  ├─ Bind shadow FBO (core GL, not EXT)
│  ├─ Set viewport to light's shadow map region
│  ├─ Use depth-only shader (trivial vertex shader, no fragment)
│  ├─ For each shadow-casting object:
│  │  └─ Bind VAO, set model matrix uniform, glDrawElements()
│  └─ Unbind FBO
│
├─ 2. Main Pass
│  ├─ Set camera matrices as uniforms (projection, view)
│  ├─ Upload light data as uniform block/array
│  ├─ Bind shadow map to texture unit (e.g., unit 7)
│  ├─ For each object:
│  │  ├─ Set model matrix uniform
│  │  ├─ Set material uniforms (baseColor, metallic, roughness, emissive)
│  │  ├─ Bind material textures (baseColorMap→unit 0, normalMap→unit 1, etc.)
│  │  ├─ Bind VAO
│  │  └─ glDrawElements(GL_TRIANGLES, ...)
│  └─ Unbind
│
└─ 3. Overlay Pass (lines, points, text — disable depth/lighting)
```

### Shader System

**Single shader program** with compile-time `#define` variants:

```glsl
// scene_shader.vert
#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;  // from Camera::getProjectionMatrixGL()
uniform mat3 uNormalMatrix;
uniform mat4 uShadowMatrix[MAX_SHADOW_LIGHTS];

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vShadowCoord[MAX_SHADOW_LIGHTS];

void main() {
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = normalize(uNormalMatrix * aNormal);
    vTexCoord = aTexCoord;
    for (int i = 0; i < NUM_SHADOW_LIGHTS; i++)
        vShadowCoord[i] = uShadowMatrix[i] * worldPos;
    gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
}
```

```glsl
// scene_shader.frag
#version 410 core

// Material
uniform vec4  uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3  uEmissive;

#ifdef HAS_BASE_COLOR_MAP
uniform sampler2D uBaseColorMap;
#endif
#ifdef HAS_NORMAL_MAP
uniform sampler2D uNormalMap;
#endif

// Lights (up to 8)
uniform int   uNumLights;
uniform vec4  uLightPos[8];
uniform vec3  uLightColor[8];

// Shadows
#ifdef HAS_SHADOWS
uniform sampler2DShadow uShadowMap;  // hardware PCF
uniform int uNumShadowLights;
#endif

// Ambient
uniform vec3 uAmbientLight;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vShadowCoord[MAX_SHADOW_LIGHTS];

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 albedo = uBaseColor.rgb;

#ifdef HAS_BASE_COLOR_MAP
    albedo *= texture(uBaseColorMap, vTexCoord).rgb;
#endif

    // Simple Blinn-Phong with material properties
    float shininess = 2.0 / (uRoughness * uRoughness + 1e-4) - 2.0;
    vec3 V = normalize(-vWorldPos);  // view direction (camera at origin in view space)
    vec3 result = uAmbientLight * albedo;

    for (int i = 0; i < uNumLights; i++) {
        vec3 L = normalize(uLightPos[i].xyz - vWorldPos);
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        float shadow = 1.0;
#ifdef HAS_SHADOWS
        // ... shadow test with sampler2DShadow (hardware PCF) ...
#endif
        vec3 diffuse = albedo * NdotL;
        vec3 specular = vec3(pow(NdotH, shininess)) * (1.0 - uRoughness);
        result += shadow * uLightColor[i] * (diffuse + specular);
    }

    result += uEmissive;
    FragColor = vec4(result, uBaseColor.a);
}
```

### VBO/VAO Per SceneObject

Each SceneObject gets a cached VBO/VAO that's rebuilt when geometry is dirty:

```cpp
struct GLGeometryCache {
    GLuint vao = 0;
    GLuint vbo = 0;       // interleaved: pos(3), normal(3), uv(2) = 8 floats/vertex
    GLuint ebo = 0;       // element buffer (triangle indices)
    int numIndices = 0;
    bool dirty = true;
};
```

Rebuild on `prepareForRendering()` when vertices/normals/texcoords/primitives change.
This replaces both immediate mode AND display lists.

### Shadow Mapping Improvements

- Use core `glGenFramebuffers` (not EXT)
- `sampler2DShadow` with `GL_TEXTURE_COMPARE_MODE = GL_COMPARE_REF_TO_TEXTURE` for hardware PCF
- Separate shadow textures per light (not packed into one) — simpler, allows different resolutions
- Front-face culling during shadow pass to reduce Peter Panning
- Consider Cascaded Shadow Maps later for large scenes

## Migration Strategy

**Approach**: Build the new pipeline alongside the old one, switchable via a Scene property.
Once validated, remove the old code. This preserves the ability to compare and verify
camera alignment.

### Phase 1: Core Shader + VBO Infrastructure

1. Write `scene_shader.vert/.frag` as actual GLSL files (or embedded strings)
2. Create `GLGeometryCache` class that builds VBO/VAO from SceneObject
3. Add `Scene::renderSceneModern()` as alternative to `renderSceneObjectRecursive()`
4. Wire material uniforms (baseColor, metallic, roughness) from Material
5. Wire camera matrices as uniforms (projection from `getProjectionMatrixGL()`)
6. **Verify**: render untextured objects, compare camera alignment with legacy path

### Phase 2: Textures

1. Bind Material::baseColorMap → texture unit 0, set `HAS_BASE_COLOR_MAP` define
2. Pass UV coordinates as vertex attribute (from SceneObject::m_texCoords via primitive UV indices)
3. Optional: normalMap on unit 1, metallicRoughnessMap on unit 2
4. **Verify**: DamagedHelmet.glb renders textured in GL

### Phase 3: Shadows

1. Implement shadow FBO with core GL API
2. Depth-only shader for shadow pass
3. `sampler2DShadow` with hardware PCF in main shader
4. Shadow matrix computation (reuse existing `Camera::getProjectionMatrixGL()` from shadow camera)
5. **Verify**: shadows appear on ground plane

### Phase 4: Polish + Remove Legacy

1. Support for lines, points, text (separate simple shader, no lighting)
2. Support TexturePrimitive (backward compat for existing code)
3. Remove old `renderSceneObjectRecursive()`, `ShaderUtil`, display list code
4. Remove all deprecated GL calls
5. **Verify**: all existing demos/apps still work

## Compatibility Notes

- **macOS**: GL 4.1 Core Profile — everything in this plan works
- **Linux**: GL 4.x+ Core Profile — works
- **Existing ICL apps**: Phase 4 removes legacy, but Phase 1-3 keep both paths active
- **TexturePrimitive/TextPrimitive**: Need special handling (they carry their own GLImg).
  Either convert to Material-based texturing, or render via legacy path during transition.
- **PointCloudObjectBase**: Already uses VBOs — can serve as reference implementation

## Non-Goals (for this rework)

- Full PBR (Cook-Torrance, IBL, environment probes) — Cycles handles that
- Metal backend — defer, GL 4.1 is sufficient
- Geometry/tessellation shaders — not needed
- Deferred rendering — overkill for this use case
- HDR/tone mapping in GL — Cycles handles that
