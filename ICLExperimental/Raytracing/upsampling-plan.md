# Upsampling Methods — Implementation Plan

## Overview

Render at reduced internal resolution, upscale to display resolution. Four methods,
ordered by complexity, each building on the previous. Switchable via UI combo box,
controlled by a `renderScale` factor (0.25–1.0).

**Methods:**
| # | Method | Backends | Requires |
|---|--------|----------|----------|
| 1 | Bilinear | all | — |
| 2 | Edge-Aware | all | — |
| 3 | MetalFX Spatial | Metal | MetalFX framework |
| 4 | MetalFX Temporal | Metal | MetalFX + depth + motion vectors |

---

## Phase 1: Enum, API Surface, Orchestrator

### RaytracerTypes.h

Add enum at end of `icl::rt` namespace:

```cpp
enum class UpsamplingMethod {
  None = 0,       // render at full resolution, no upsampling
  Bilinear,       // bilinear interpolation (all backends)
  EdgeAware,      // bilateral edge-preserving upscale (all backends)
  MetalFXSpatial, // Apple MetalFX spatial upscaler (Metal only)
  MetalFXTemporal // Apple MetalFX temporal upscaler (Metal only)
};
```

### RaytracerBackend.h

Add to base class (after existing `setAdaptiveAA`):

```cpp
virtual bool supportsUpsampling(UpsamplingMethod method) const {
  return method == UpsamplingMethod::None ||
         method == UpsamplingMethod::Bilinear ||
         method == UpsamplingMethod::EdgeAware;
}

virtual bool setUpsampling(UpsamplingMethod method) {
  if (!supportsUpsampling(method)) return false;
  m_upsamplingMethod = method;
  return true;
}

virtual void setRenderScale(float scale) {
  m_renderScale = std::max(0.25f, std::min(1.0f, scale));
}

virtual void setDisplaySize(int w, int h) {
  m_displayWidth = w;
  m_displayHeight = h;
}

UpsamplingMethod getUpsamplingMethod() const { return m_upsamplingMethod; }
float getRenderScale() const { return m_renderScale; }
```

Add protected members:

```cpp
protected:
  UpsamplingMethod m_upsamplingMethod = UpsamplingMethod::None;
  float m_renderScale = 1.0f;
  int m_displayWidth = 0;
  int m_displayHeight = 0;
```

### SceneRaytracer.h / .cpp

Add public methods (delegating to backend):

```cpp
bool setUpsampling(UpsamplingMethod method);
void setRenderScale(float scale);
bool supportsUpsampling(UpsamplingMethod method) const;
```

Modify `SceneRaytracer::render()` — apply render scale to the camera before
passing to backend:

```cpp
RTRayGenParams renderCamera = extracted.camera;
float scale = m_backend->getRenderScale();
if (scale < 1.0f && m_backend->getUpsamplingMethod() != UpsamplingMethod::None) {
  m_backend->setDisplaySize(renderCamera.imageWidth, renderCamera.imageHeight);
  renderCamera.imageWidth  = std::max(1, (int)(renderCamera.imageWidth * scale));
  renderCamera.imageHeight = std::max(1, (int)(renderCamera.imageHeight * scale));
}
m_backend->render(renderCamera);
```

---

## Phase 2: Bilinear Upsampling (all backends)

### New file: Upsampling.h / Upsampling.cpp

Shared CPU upsampling utilities used by all backends:

```cpp
namespace icl::rt {

void upsampleBilinear(const core::Img8u &src, core::Img8u &dst,
                       int dstW, int dstH);

void upsampleEdgeAware(const core::Img8u &src, core::Img8u &dst,
                        int dstW, int dstH,
                        float sigma_s = 1.5f, float sigma_r = 30.0f);

void upsampleNearestInt(const std::vector<int32_t> &src, int srcW, int srcH,
                         std::vector<int32_t> &dst, int dstW, int dstH);
}
```

**Bilinear algorithm:**
For each output pixel, map to source coordinates, sample 2×2 neighborhood
with fractional weights. OpenMP parallel over rows.

```cpp
void upsampleBilinear(const Img8u &src, Img8u &dst, int dstW, int dstH) {
  int srcW = src.getWidth(), srcH = src.getHeight();
  dst = Img8u(Size(dstW, dstH), formatRGB);
  float scaleX = (float)srcW / dstW, scaleY = (float)srcH / dstH;

  #pragma omp parallel for schedule(static)
  for (int dy = 0; dy < dstH; dy++) {
    for (int dx = 0; dx < dstW; dx++) {
      float sx = (dx + 0.5f) * scaleX - 0.5f;
      float sy = (dy + 0.5f) * scaleY - 0.5f;
      int x0 = clamp((int)sx, 0, srcW-1), x1 = clamp(x0+1, 0, srcW-1);
      int y0 = clamp((int)sy, 0, srcH-1), y1 = clamp(y0+1, 0, srcH-1);
      float fx = sx - x0, fy = sy - y0;

      for (int c = 0; c < 3; c++) {
        const icl8u *s = src.getData(c);
        float v = s[x0+y0*srcW]*(1-fx)*(1-fy) + s[x1+y0*srcW]*fx*(1-fy)
                + s[x0+y1*srcW]*(1-fx)*fy     + s[x1+y1*srcW]*fx*fy;
        dst.getData(c)[dx + dy*dstW] = (icl8u)clamp(v, 0.f, 255.f);
      }
    }
  }
}
```

**Object ID upsampling:** nearest-neighbor (no interpolation for integer IDs).

### Backend integration (all three)

Each backend's `render()` gains a final step:

```cpp
// After rendering + post-processing (FXAA etc.) at internal resolution:
if (m_upsamplingMethod == UpsamplingMethod::Bilinear && m_displayWidth > 0) {
  upsampleBilinear(m_output, m_output, m_displayWidth, m_displayHeight);
  upsampleNearestInt(m_objectIdBuffer, internalW, internalH,
                      m_objectIdBuffer, m_displayWidth, m_displayHeight);
}
```

Order: FXAA runs on low-res (cheaper), then upscale to display res.

### CMakeLists.txt

Add `Upsampling.cpp` / `Upsampling.h` to `RT_SOURCES` / `RT_HEADERS`.

---

## Phase 3: Edge-Aware Upsampling (all backends)

### Upsampling.cpp

Joint bilateral upsampling. For each output pixel:
1. Map to source coordinates
2. Sample a neighborhood (radius ≈ 2×sigma_s)
3. Weight by spatial distance AND luminance similarity:
   `w = exp(-dist²/2σ_s²) · exp(-ΔL²/2σ_r²)`
4. This preserves edges: pixels across silhouettes get low weight

```cpp
void upsampleEdgeAware(const Img8u &src, Img8u &dst,
                        int dstW, int dstH,
                        float sigma_s, float sigma_r) {
  int srcW = src.getWidth(), srcH = src.getHeight();
  dst = Img8u(Size(dstW, dstH), formatRGB);
  float scaleX = (float)srcW / dstW, scaleY = (float)srcH / dstH;
  float invSS = -0.5f / (sigma_s * sigma_s);
  float invSR = -0.5f / (sigma_r * sigma_r);
  int radius = (int)ceil(2.0f * sigma_s);

  auto luma = [](const icl8u *R, const icl8u *G, const icl8u *B, int i) {
    return 0.299f * R[i] + 0.587f * G[i] + 0.114f * B[i];
  };

  #pragma omp parallel for schedule(dynamic, 4)
  for (int dy = 0; dy < dstH; dy++) {
    for (int dx = 0; dx < dstW; dx++) {
      float sx = (dx+0.5f)*scaleX - 0.5f, sy = (dy+0.5f)*scaleY - 0.5f;
      int cx = clamp((int)(sx+0.5f), 0, srcW-1);
      int cy = clamp((int)(sy+0.5f), 0, srcH-1);
      float cL = luma(src.getData(0), src.getData(1), src.getData(2), cx+cy*srcW);

      float sumR=0, sumG=0, sumB=0, sumW=0;
      for (int ky = -radius; ky <= radius; ky++) {
        for (int kx = -radius; kx <= radius; kx++) {
          int nx = clamp(cx+kx, 0, srcW-1), ny = clamp(cy+ky, 0, srcH-1);
          int ni = nx + ny * srcW;
          float sd = (nx-sx)*(nx-sx) + (ny-sy)*(ny-sy);
          float ld = luma(src.getData(0), src.getData(1), src.getData(2), ni) - cL;
          float w = exp(sd*invSS + ld*ld*invSR);
          sumR += w * src.getData(0)[ni];
          sumG += w * src.getData(1)[ni];
          sumB += w * src.getData(2)[ni];
          sumW += w;
        }
      }
      int di = dx + dy * dstW;
      dst.getData(0)[di] = (icl8u)clamp(sumR/sumW, 0.f, 255.f);
      dst.getData(1)[di] = (icl8u)clamp(sumG/sumW, 0.f, 255.f);
      dst.getData(2)[di] = (icl8u)clamp(sumB/sumW, 0.f, 255.f);
    }
  }
}
```

Backend integration: same pattern as bilinear, just call `upsampleEdgeAware()`.

---

## Phase 4: MetalFX Spatial Upscaler (Metal only)

### Prerequisites: Texture wrapper + buffer↔texture kernels

#### MetalRT.h — add Texture class

```cpp
class Texture {
public:
  Texture();
  ~Texture();
  Texture(const Texture &); Texture &operator=(const Texture &);
  Texture(Texture &&) noexcept; Texture &operator=(Texture &&) noexcept;
  int width() const;
  int height() const;
  explicit operator bool() const;
  void *nativeHandle() const; // id<MTLTexture>
private:
  friend class Device;
  struct Impl;
  std::shared_ptr<Impl> m_impl;
};

// Pixel format constants (match MTLPixelFormat values)
constexpr int PixelFormatRGBA8Unorm = 70;
constexpr int PixelFormatR32Float   = 55;
constexpr int PixelFormatRG16Float  = 102;

// Texture usage constants (match MTLTextureUsage flags)
constexpr int TextureUsageShaderRead  = 0x01;
constexpr int TextureUsageShaderWrite = 0x02;
constexpr int TextureUsageRenderTarget = 0x04;
```

Add to `Device`:

```cpp
Texture newTexture(int pixelFormat, int width, int height, int usage);
```

#### MetalRT.mm — implement Texture

```objc
struct Texture::Impl { id<MTLTexture> texture; };

Texture Device::newTexture(int pixelFormat, int w, int h, int usage) {
  Texture tex;
  auto *desc = [MTLTextureDescriptor
    texture2DDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat
                                 width:w height:h mipmapped:NO];
  desc.usage = (MTLTextureUsage)usage;
  desc.storageMode = MTLStorageModePrivate;
  tex.m_impl = std::make_shared<Texture::Impl>();
  tex.m_impl->texture = [m_impl->device newTextureWithDescriptor:desc];
  return tex;
}
```

#### RaytracerKernel.metal — add conversion kernels

```metal
[[kernel]] void planarToRGBA(
    device const uchar *inR [[buffer(0)]],
    device const uchar *inG [[buffer(1)]],
    device const uchar *inB [[buffer(2)]],
    texture2d<float, access::write> outTex [[texture(0)]],
    constant int &width [[buffer(3)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (tid.x >= uint(width) || tid.y >= outTex.get_height()) return;
  int i = tid.x + tid.y * width;
  outTex.write(float4(float(inR[i])/255, float(inG[i])/255,
                       float(inB[i])/255, 1), tid);
}

[[kernel]] void rgbaToPlanar(
    texture2d<float, access::read> inTex [[texture(0)]],
    device uchar *outR [[buffer(0)]],
    device uchar *outG [[buffer(1)]],
    device uchar *outB [[buffer(2)]],
    constant int &width [[buffer(3)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (tid.x >= uint(width) || tid.y >= inTex.get_height()) return;
  float4 c = inTex.read(tid);
  int i = tid.x + tid.y * width;
  outR[i] = uchar(clamp(c.x, 0.0, 1.0) * 255);
  outG[i] = uchar(clamp(c.y, 0.0, 1.0) * 255);
  outB[i] = uchar(clamp(c.z, 0.0, 1.0) * 255);
}
```

#### RaytracerBackend_Metal.mm — MetalFX Spatial integration

Guard import:

```objc
#if __has_include(<MetalFX/MetalFX.h>)
#import <MetalFX/MetalFX.h>
#define ICL_HAVE_METALFX 1
#endif
```

Add to `Impl`:

```cpp
mtl::ComputePipeline planarToRGBAPipeline, rgbaToPlanarPipeline;
mtl::Texture renderColorTex, displayColorTex;
mtl::Buffer displayOutR, displayOutG, displayOutB;
#if ICL_HAVE_METALFX
id<MTLFXSpatialScaler> spatialScaler = nil;
#endif
```

Setup:

```cpp
void setupSpatialScaler(int renderW, int renderH, int dispW, int dispH) {
#if ICL_HAVE_METALFX
  MTLFXSpatialScalerDescriptor *desc = [[MTLFXSpatialScalerDescriptor alloc] init];
  desc.inputWidth = renderW;   desc.inputHeight = renderH;
  desc.outputWidth = dispW;    desc.outputHeight = dispH;
  desc.colorTextureFormat = MTLPixelFormatRGBA8Unorm;
  desc.outputTextureFormat = MTLPixelFormatRGBA8Unorm;
  desc.colorProcessingMode = MTLFXSpatialScalerColorProcessingModePerceptual;
  spatialScaler = [desc newSpatialScalerWithDevice:
    (__bridge id<MTLDevice>)device.nativeDevice()];

  renderColorTex = device.newTexture(PixelFormatRGBA8Unorm, renderW, renderH,
    TextureUsageShaderRead | TextureUsageShaderWrite);
  displayColorTex = device.newTexture(PixelFormatRGBA8Unorm, dispW, dispH,
    TextureUsageShaderRead | TextureUsageShaderWrite);
#endif
}
```

Per-frame flow:

```
1. Raytrace at render resolution → outR/G/B (existing)
2. planarToRGBA kernel: outR/G/B → renderColorTex
3. MTLFXSpatialScaler: renderColorTex → displayColorTex
4. rgbaToPlanar kernel: displayColorTex → displayOutR/G/B
5. Readback displayOutR/G/B → m_output at display resolution
```

All three steps in a single command buffer.

Override `supportsUpsampling()`:

```cpp
bool MetalRTBackend::supportsUpsampling(UpsamplingMethod m) const {
  if (m == UpsamplingMethod::MetalFXSpatial || m == UpsamplingMethod::MetalFXTemporal)
    return ICL_HAVE_METALFX && m_impl->valid;
  return RaytracerBackend::supportsUpsampling(m);
}
```

#### CMakeLists.txt

In the `if(APPLE AND METAL_FRAMEWORK)` block:

```cmake
find_library(METALFX_FRAMEWORK MetalFX)
if(METALFX_FRAMEWORK)
  target_link_libraries(ICLRaytracing ${METALFX_FRAMEWORK})
  message(STATUS "MetalFX found — spatial/temporal upscaling available")
endif()
```

---

## Phase 5: MetalFX Temporal Upscaler (Metal only)

The most complex method. Requires depth buffer, motion vectors, and jitter.

### Step 5a: Depth buffer output

Add `device float *depthOut` parameter to both Metal kernels (`raytrace` and
`pathTrace`). Write `result.distance` on primary hit, `camera.farClip` on miss.

```metal
// In raytrace kernel, after primary intersection:
if (bounce == 0) {
  depthOut[idx] = (result.type != intersection_type::none)
                    ? result.distance : camera.farClip;
}
```

Add `depthBuf` (render-resolution float buffer) to `Impl`. Bind it in `dispatch()`.
Update buffer index assignments in both kernels.

### Step 5b: Motion vectors

Add `RTMat4 viewProj` to `RTRayGenParams` (forward view-projection matrix).
Compute it in `GeometryExtractor::extractCamera()`.

Store `prevCamera` in `Impl`. Add a kernel:

```metal
[[kernel]] void computeMotionVectors(
    device const float *depth       [[buffer(0)]],
    texture2d<float, access::write> motionTex [[texture(0)]],
    constant RTRayGenParams &curCam  [[buffer(1)]],
    constant RTMat4 &prevViewProj    [[buffer(2)]],
    uint2 tid [[thread_position_in_grid]])
{
  // 1. Reconstruct world pos from depth + current camera ray
  // 2. Project world pos into previous frame's screen space via prevViewProj
  // 3. motion = (currentPixel - previousPixel) / resolution
  // 4. Write to RG16Float motion texture
}
```

### Step 5c: Jitter

Apply sub-pixel jitter to ray origins for temporal accumulation.
Use Halton sequence (base 2, 3):

```cpp
float halton(int i, int base) {
  float r = 0, f = 1.0f / base;
  while (i > 0) { r += f * (i % base); i /= base; f /= base; }
  return r;
}

// Per frame: jitterX = halton(frame, 2) - 0.5, jitterY = halton(frame, 3) - 0.5
// Offset pixel coordinates in the ray generation: px + jitterX, py + jitterY
```

Pass jitter offsets to the shader via `SceneParams` or as separate `setBytes`.

### Step 5d: Temporal scaler setup

```cpp
void setupTemporalScaler(int renderW, int renderH, int dispW, int dispH) {
#if ICL_HAVE_METALFX
  MTLFXTemporalScalerDescriptor *desc = [[MTLFXTemporalScalerDescriptor alloc] init];
  desc.inputWidth = renderW;   desc.inputHeight = renderH;
  desc.outputWidth = dispW;    desc.outputHeight = dispH;
  desc.colorTextureFormat = MTLPixelFormatRGBA8Unorm;
  desc.depthTextureFormat = MTLPixelFormatR32Float;
  desc.motionTextureFormat = MTLPixelFormatRG16Float;
  desc.outputTextureFormat = MTLPixelFormatRGBA8Unorm;
  temporalScaler = [desc newTemporalScalerWithDevice:device];
#endif
}
```

### Step 5e: Per-frame flow

```
1. Compute jitter, offset camera ray directions
2. Raytrace at render resolution → outR/G/B + depthBuf
3. planarToRGBA → renderColorTex
4. depth buffer → depthTex (copy or compute kernel)
5. computeMotionVectors → motionTex
6. Configure temporal scaler:
     colorTexture = renderColorTex
     depthTexture = depthTex
     motionTexture = motionTex
     outputTexture = displayColorTex
     jitterOffsetX/Y = jitterX, jitterY
     reset = (camera jumped || first frame)
7. Encode temporal scaler
8. rgbaToPlanar → displayOutR/G/B
9. Store current camera as prevCamera
```

### Reset conditions

Reset temporal history when:
- Camera changes (SceneRaytracer already detects this)
- Render scale changes
- Scene geometry changes (BLAS rebuild)

Add `resetTemporalHistory()` to backend API.

### Interaction with path tracing

When MetalFX Temporal is active, disable MSAA jitter (temporal scaler provides
its own AA via jittered subpixels). Path tracing accumulation should also be
disabled — the temporal scaler handles frame-to-frame integration.

---

## Phase 6: Demo UI Integration

### raytracing-physics-demo.cpp

Add controls in `init()` GUI layout:

```cpp
<< Combo("!None,Bilinear,Edge-Aware,MetalFX Spatial,MetalFX Temporal")
     .handle("upsampling").label("Upsampling")
<< Slider(25, 100, 50).handle("renderScale").label("Render Scale %")
```

Add to `run()`:

```cpp
static const UpsamplingMethod methods[] = {
  UpsamplingMethod::None, UpsamplingMethod::Bilinear,
  UpsamplingMethod::EdgeAware,
  UpsamplingMethod::MetalFXSpatial, UpsamplingMethod::MetalFXTemporal
};
int upsIdx = gui["upsampling"].as<ComboHandle>().getSelectedIndex();
if (raytracer->supportsUpsampling(methods[upsIdx]))
  raytracer->setUpsampling(methods[upsIdx]);
raytracer->setRenderScale(gui["renderScale"].as<int>() / 100.0f);
```

Add `-scale` program argument:

```
"-backend(string=auto) -fps(float=30) -size(Size=800x600) -scale(float=1.0)"
```

Update status bar to show internal vs display resolution.

---

## Implementation Order

| Session | Phases | What | Testable after? |
|---------|--------|------|-----------------|
| **A** | 1–3 | Enum + API + Bilinear + Edge-Aware (all backends) + UI | Yes: slider + combo in demo |
| **B** | 4 | MetalFX Spatial (Texture wrapper, conversion kernels, scaler) | Yes: combo selects MFX Spatial |
| **C** | 5 | MetalFX Temporal (depth, motion vectors, jitter, scaler) | Yes: full temporal pipeline |

---

## Notes

- **Path tracing + upsampling**: accumulation runs at internal (low) resolution.
  Changing `renderScale` must reset accumulation.
- **Object IDs**: upsampled via nearest-neighbor (no interpolation for integers).
  Slight picking inaccuracy at edges is acceptable.
- **Buffer index management**: adding depth output shifts kernel buffer indices.
  Consider using named constants for indices to reduce fragility.
- **MetalFX textures**: must be `StorageModePrivate` (GPU-only). Buffer↔texture
  conversion kernels handle the data movement (two lightweight dispatches).
- **Per-object motion vectors**: the Phase 5 implementation uses camera-only
  reprojection (assumes static geometry positions within each frame). For fully
  accurate motion, per-instance previous-frame transforms could be added later.
