# GL Renderer Rebuild Plan

## Context

The SceneRendererGL was built as a modern VBO/shader alternative to the legacy
fixed-function pipeline for side-by-side comparison with Cycles. The geometry
extraction, camera alignment, and texture upload work correctly. However, the
lighting pipeline hit an "endless trial-and-error loop" with uniform arrays.

**Goal**: A working GL 4.1 renderer built up incrementally, verifying each step
before moving on. Eventually looks as similar to Cycles as rasterization allows.

## Verdict: Go directly to GL 4.1 Core Profile

No GLSL 1.20 scaffolding phase. Build for GL 4.1 Core from the start.

**Keep:**
- `GLGeometryCache::build()` — triangle extraction from SceneObject works
- `GLGeometryCache::uploadBaseColorMap()` — texture upload works
- Class interface (`SceneRendererGL.h`)
- GLCallback integration pattern in cycles-scene-viewer.cpp
- Camera matrix pipeline (pixel-perfect alignment verified)

**Rewrite:**
- Shaders: `#version 410 core` with `in`/`out`, `layout(location=N)`
- Add VAOs (required in Core Profile)
- `render()` and `renderObject()` uniform logic
- Widget GL context setup (request 4.1 Core via QSurfaceFormat)

**Fix (minimal):**
- ICLWidget::initializeGL() uses `glMatrixMode`, `glLoadIdentity` etc. — these
  break in Core Profile. Need to strip/guard the legacy GL calls in the widget
  init path. The cycles-scene-viewer's GL pane doesn't use the legacy renderer,
  so we just need the widget to initialize without crashing.

## Incremental Steps

### Step 0: GL 4.1 Core Profile Context

- Set `QSurfaceFormat::setDefaultFormat()` in cycles-scene-viewer's `main()`
  before `ICLApp` construction: version 4.1, Core Profile
- Fix ICLWidget::initializeGL() to survive Core Profile (guard legacy GL calls
  behind a compatibility check, or strip them — they're just setting up default
  state that the shader pipeline doesn't need)
- Fix DrawWidget3D::customPaintEvent() similarly (uses `glMatrixMode`,
  `gluPerspective`, `GL_LIGHTING` etc.)

**Verification:** App launches without GL errors. Both panes appear (may be
blank/black — that's fine, we haven't wired rendering yet).

**Files:**
- `ICLExperimental/Raytracing/demos/cycles-scene-viewer.cpp` (main)
- `ICLQt/src/ICLQt/Widget.cpp` (initializeGL)
- `ICLQt/src/ICLQt/DrawWidget3D.cpp` (customPaintEvent)

### Step 1: Flat Color Rendering (no lighting)

Minimal `#version 410 core` shaders:
```glsl
// vertex
layout(location = 0) in vec3 aPosition;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPosition, 1.0); }

// fragment
uniform vec4 uBaseColor;
out vec4 FragColor;
void main() { FragColor = uBaseColor; }
```

- Add VAO to GLGeometryCache (wrap VBO/EBO binding)
- Compute MVP = projection * view * model on CPU, pass as single uniform
- No lighting, no textures, no normals

**Verification:** GL pane shows correct geometry silhouettes in flat color,
matching Cycles outlines. Camera orbit works.

### Step 2: Single Hardcoded Light

Add normals to vertex shader, hardcode one directional light in fragment:
```glsl
vec3 lightDir = normalize(vec3(0.5, 1.0, -0.3));
float NdotL = max(dot(N, lightDir), 0.0);
FragColor = vec4(uBaseColor.rgb * (0.1 + 0.9 * NdotL), 1.0);
```

**Verification:** Objects show 3D shading (lit vs shadow side).

**Decision:** Compute lighting in **world space** — avoids the view-space
light transform that contributed to previous debugging difficulty.

### Step 3: Single Light via Uniform

Replace hardcoded light with uniforms:
- `uniform vec4 uLightPos;`
- `uniform vec3 uLightColor;`
- Set from Scene's light 0 (key light)

**Verification:** Same shading, driven by Scene light data.

### Step 4: Multi-Light Array

Expand to 8 lights:
- Query each `glGetUniformLocation("uLightPos[0]")` etc. individually
- Store in `GLint locations[8]` array
- NOT the `loc + i` shortcut from before

**Verification:** Multiple lights visible (key + fill + rim + top).

### Step 5: Material Properties + Specular + Light Calibration

- `uBaseColor`, `uMetallic`, `uRoughness`, `uEmissive`
- Blinn-Phong specular, shininess from roughness
- Metallic tints specular color
- Exposure multiplier

**Light/ambient calibration (GL ↔ Cycles):**
This is the right point to establish a common brightness convention between
the two renderers, since all material params and multi-light support are in
place. Specifically:
- Match Cycles' light intensity scaling (300x multiplier) against GL light
  colors. Define a common convention: "diffuse color 255 = intensity X" in
  both renderers.
- Map the ambient/BG slider to something physically motivated: Cycles sky
  brightness vs GL ambient term, so the same slider value produces similar
  overall brightness.
- Tune exposure so the same scene with the same materials produces similar
  brightness in both panes.
- Before Step 5, we're comparing a single hardcoded Lambert light against a
  full path tracer with environment lighting — no meaningful calibration
  target exists yet.

**Verification:** Material presets show visible differences. Same scene with
same settings produces quantitatively similar brightness in GL and Cycles.

### Step 6: baseColorMap Textures

- `sampler2D uBaseColorMap` + `uHasBaseColorMap`
- UVs already in VBO, texture already uploaded

**Verification:** DamagedHelmet.glb shows texture in GL pane.

### Step 7: Debug Modes

Normals, Albedo, UVs, Lighting-only, NdotL visualizations.

### Future Steps (after basics are solid)

- Shadow mapping (depth FBO, sampler2DShadow, hardware PCF)
- Normal maps (TBN matrix)
- Metallic-roughness texture maps
- Delete legacy `Scene::renderSceneObjectRecursive()`, ShaderUtil, display lists
- Wire `Scene::getGLCallback()` to SceneRendererGL for all ICL apps

### Widget Overlay & 2D Rendering (Core Profile port)

The Core Profile switch currently skips all legacy 2D rendering (GLPaintEngine,
GLImg::draw2D, OSD buttons). These must be re-enabled with GL 4.1 Core
implementations to restore full Canvas/Canvas3D functionality:

1. **OSD button bar** — the row of blue buttons at the top-left of each widget
   (record, screenshot, menu, etc.). Currently rendered via GLPaintEngine using
   `glBegin/glEnd`. Needs a Core Profile 2D quad/text renderer.

2. **2D draw commands** — `draw->text()`, `draw->line()`, `draw->rect()`,
   `draw->circle()`, etc. in screen pixel coordinates. Used by all ICL apps
   for annotations on Canvas and Canvas3D widgets. Needs a Core Profile
   implementation of GLPaintEngine (or a replacement using a simple 2D shader +
   VBO, or QPainter overlay).

3. **Zoom rectangle** — left-mouse-drag draws a rectangle overlay, then zooms
   into that region of the 2D image or 3D scene. Needs both the rectangle
   rendering (2D overlay) and the zoom logic (already in Widget.cpp, just needs
   the rect to be visible).

4. **Image info indicator** — bottom-right corner display showing pixel
   coordinates, color values, image dimensions. Rendered via GLPaintEngine.

5. **GLImg::draw2D** — 2D image display in Canvas widgets. Currently uses
   `glBegin/glEnd` textured quads. Needs a Core Profile path (shader-based
   textured quad, similar to GLImageRenderer but integrated into ICLWidget).

**Approach options:**
- (a) Port GLPaintEngine to Core Profile (new GL 4.1 backend for 2D primitives)
- (b) Replace with QPainter overlay (Qt supports QPainter on QOpenGLWidget)
- (c) Hybrid: QPainter for text, GL shader for images and geometric primitives

This ensures that switching to GL 4.1 Core doesn't regress any existing widget
functionality — all ICL apps continue to work as before.

## Key Files

| File | Role |
|------|------|
| `ICLGeom/src/ICLGeom/SceneRendererGL.h` | Public API (minimal changes) |
| `ICLGeom/src/ICLGeom/SceneRendererGL.cpp` | Main rewrite target |
| `ICLExperimental/Raytracing/demos/cycles-scene-viewer.cpp` | Test harness + QSurfaceFormat |
| `ICLQt/src/ICLQt/Widget.cpp` | Fix initializeGL for Core Profile |
| `ICLQt/src/ICLQt/DrawWidget3D.cpp` | Fix customPaintEvent for Core Profile |
| `ICLGeom/src/ICLGeom/SceneLight.h` | Light API reference |
| `ICLGeom/src/ICLGeom/Camera.h` | Camera matrix API reference |
| `ICLGeom/src/ICLGeom/Material.h` | Material properties reference |

## Verification

After each step:
```bash
cmake --build build -j16
./build/bin/cycles-scene-viewer -scene ICLExperimental/Raytracing/scenes/bunny.obj
```
GL pane (right) should render correctly. Compare with Cycles pane (left).
