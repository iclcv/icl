// ICL Raytracer — Metal Shading Language kernels
// Hardware-accelerated BVH traversal via intersect<triangle_data, instancing>()

#include <metal_stdlib>
#include <metal_raytracing>

using namespace metal;
using namespace raytracing;

// ---- Struct layouts (must match C++ RaytracerTypes.h byte-for-byte) ----

struct RTFloat3 { float x, y, z, _pad; };
struct RTFloat4 { float x, y, z, w; };
struct RTMat4   { float cols[4][4]; };

struct RTVertex {
  RTFloat3 position;
  RTFloat3 normal;
  RTFloat4 color;
};

struct RTTriangle {
  uint v0, v1, v2;
  uint materialIndex;
};

struct RTMaterial {
  RTFloat4 diffuseColor;
  RTFloat4 specularColor;
  RTFloat4 emission;
  float shininess;
  float reflectivity;
  float _pad[2];
};

struct RTLight {
  RTFloat3 position;
  RTFloat4 ambient;
  RTFloat4 diffuse;
  RTFloat4 specular;
  RTFloat3 spotDirection;
  RTFloat3 attenuation;
  float spotCutoff;
  float spotExponent;
  int on;
  float _pad;
};

struct RTRayGenParams {
  RTFloat3 cameraPos;
  RTMat4 invViewProj;
  RTMat4 viewProj;
  int imageWidth;
  int imageHeight;
  float nearClip;
  float farClip;
};

// ---- Per-instance shading data (match MetalInstanceData in backend .mm) ----

struct InstanceData {
  RTMat4 transform;
  RTMat4 transformInverse;
  int vertexOffset;
  int triangleOffset;
  int materialIndex;
  int _pad;
};

// ---- Scene-wide constant parameters ----

struct SceneParams {
  int numLights;
  int numInstances;
  int frameNumber;
  int numEmissives;
  RTFloat4 bgColor;
  float totalEmissiveArea;
  float _scenePad[3];
};

struct RTEmissiveTriangle {
  RTFloat3 v0, v1, v2;
  RTFloat3 normal;
  RTFloat3 emission;
  float area;
  float _pad[3];
};

// ---- Vector helpers ----

inline float3 f3(RTFloat3 v)  { return float3(v.x, v.y, v.z); }
inline float3 f3v(RTFloat4 v) { return float3(v.x, v.y, v.z); }

inline float3 transformPoint(device const RTMat4 &m, float3 p) {
  return float3(
    m.cols[0][0]*p.x + m.cols[1][0]*p.y + m.cols[2][0]*p.z + m.cols[3][0],
    m.cols[0][1]*p.x + m.cols[1][1]*p.y + m.cols[2][1]*p.z + m.cols[3][1],
    m.cols[0][2]*p.x + m.cols[1][2]*p.y + m.cols[2][2]*p.z + m.cols[3][2]);
}

inline float3 transformNormal(device const RTMat4 &invT, float3 n) {
  return float3(
    invT.cols[0][0]*n.x + invT.cols[0][1]*n.y + invT.cols[0][2]*n.z,
    invT.cols[1][0]*n.x + invT.cols[1][1]*n.y + invT.cols[1][2]*n.z,
    invT.cols[2][0]*n.x + invT.cols[2][1]*n.y + invT.cols[2][2]*n.z);
}

inline float3 generateRayDir(constant RTRayGenParams &cam, float px, float py) {
  constant RTMat4 &Qi = cam.invViewProj;
  return normalize(float3(
    Qi.cols[0][0]*px + Qi.cols[1][0]*py + Qi.cols[2][0],
    Qi.cols[0][1]*px + Qi.cols[1][1]*py + Qi.cols[2][1],
    Qi.cols[0][2]*px + Qi.cols[1][2]*py + Qi.cols[2][2]));
}

// ---- Interpolate surface hit ----

struct SurfaceHit {
  float3 position;
  float3 normal;
  float3 color;
  int materialIndex;
};

inline SurfaceHit interpolate(device const InstanceData &inst,
                               device const RTVertex *vertices,
                               device const RTTriangle *triangles,
                               uint primId, float2 bary) {
  device const RTTriangle &tri = triangles[inst.triangleOffset + primId];
  device const RTVertex &v0 = vertices[inst.vertexOffset + tri.v0];
  device const RTVertex &v1 = vertices[inst.vertexOffset + tri.v1];
  device const RTVertex &v2 = vertices[inst.vertexOffset + tri.v2];

  float w0 = 1.0f - bary.x - bary.y;
  float w1 = bary.x;
  float w2 = bary.y;

  SurfaceHit s;
  float3 localNormal = float3(
    v0.normal.x*w0 + v1.normal.x*w1 + v2.normal.x*w2,
    v0.normal.y*w0 + v1.normal.y*w1 + v2.normal.y*w2,
    v0.normal.z*w0 + v1.normal.z*w1 + v2.normal.z*w2);
  s.normal = normalize(transformNormal(inst.transformInverse, localNormal));

  s.color = float3(
    v0.color.x*w0 + v1.color.x*w1 + v2.color.x*w2,
    v0.color.y*w0 + v1.color.y*w1 + v2.color.y*w2,
    v0.color.z*w0 + v1.color.z*w1 + v2.color.z*w2);

  float3 localPos = float3(
    v0.position.x*w0 + v1.position.x*w1 + v2.position.x*w2,
    v0.position.y*w0 + v1.position.y*w1 + v2.position.y*w2,
    v0.position.z*w0 + v1.position.z*w1 + v2.position.z*w2);
  s.position = transformPoint(inst.transform, localPos);
  s.materialIndex = inst.materialIndex;
  return s;
}

// ---- Fast RNG (xorshift) ----

inline uint rngNext(thread uint &state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

inline float rngFloat(thread uint &state) {
  return float(rngNext(state) & 0xFFFFFF) / float(0x1000000);
}

// ---- Cosine-weighted hemisphere sample ----

inline float3 randomHemisphere(float3 N, thread uint &rng) {
  float u1 = rngFloat(rng);
  float u2 = rngFloat(rng);
  float r = sqrt(u1);
  float theta = 2.0f * M_PI_F * u2;
  float x = r * cos(theta);
  float y = r * sin(theta);
  float z = sqrt(max(0.0f, 1.0f - u1));

  float3 up = abs(N.y) < 0.999f ? float3(0,1,0) : float3(1,0,0);
  float3 tangent = normalize(cross(up, N));
  float3 bitangent = cross(N, tangent);
  return normalize(tangent * x + bitangent * y + N * z);
}

// ---- Direct lighting (Blinn-Phong) with hardware shadow rays ----

inline float3 directLight(float3 hitPos, float3 N, float3 viewDir,
                           float3 baseColor,
                           device const RTMaterial &mat,
                           device const RTLight *lights, int numLights,
                           instance_acceleration_structure accelStruct) {
  float3 color = float3(0);

  intersector<instancing> shadowInter;
  shadowInter.accept_any_intersection(true);

  for (int li = 0; li < numLights; li++) {
    if (!lights[li].on) continue;
    float3 lightPos = f3(lights[li].position);
    float3 L = lightPos - hitPos;
    float dist = length(L);
    if (dist < 1e-6f) continue;
    L /= dist;

    float NdotL = max(0.0f, dot(N, L));
    if (NdotL <= 0) continue;

    // Hardware shadow ray
    ray shadowRay;
    shadowRay.origin = hitPos + N * 1.0f;
    shadowRay.direction = L;
    shadowRay.min_distance = 0.5f;
    shadowRay.max_distance = dist;

    auto sr = shadowInter.intersect(shadowRay, accelStruct);
    if (sr.type != intersection_type::none) continue;

    float atten = 1.0f / (lights[li].attenuation.x +
                           lights[li].attenuation.y * dist +
                           lights[li].attenuation.z * dist * dist);

    float3 diffuse = f3v(lights[li].diffuse) * baseColor * NdotL * atten;

    float3 V = normalize(-viewDir);
    float3 H = normalize(L + V);
    float spec = pow(max(0.0f, dot(N, H)), mat.shininess);
    float3 specular =
        f3v(lights[li].specular) * f3v(mat.specularColor) * spec * atten;

    float3 ambient = f3v(lights[li].ambient) * baseColor;

    color += diffuse + specular + ambient;
  }
  return color;
}

// ==========================================================================
// Buffer ↔ Texture conversion kernels (for MetalFX upscaling)
// ==========================================================================

/// Convert planar ICL Img8u (separate R, G, B channel buffers) → RGBA8 texture.
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
  outTex.write(float4(float(inR[i]) / 255.0f, float(inG[i]) / 255.0f,
                       float(inB[i]) / 255.0f, 1.0f), tid);
}

/// Convert RGBA8 texture → planar ICL Img8u (separate R, G, B channel buffers).
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
  outR[i] = uchar(clamp(c.x, 0.0f, 1.0f) * 255.0f);
  outG[i] = uchar(clamp(c.y, 0.0f, 1.0f) * 255.0f);
  outB[i] = uchar(clamp(c.z, 0.0f, 1.0f) * 255.0f);
}

/// Copy float depth buffer → R32Float depth texture.
[[kernel]] void depthToTexture(
    device const float *depthBuf [[buffer(0)]],
    texture2d<float, access::write> depthTex [[texture(0)]],
    constant int &width [[buffer(1)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (tid.x >= uint(width) || tid.y >= depthTex.get_height()) return;
  int i = tid.x + tid.y * width;
  depthTex.write(float4(depthBuf[i], 0, 0, 0), tid);
}

/// Compute per-pixel motion vectors by reprojecting current world positions
/// through the previous frame's view-projection matrix.
/// Output: RG16Float texture with (dx, dy) in pixel units.
[[kernel]] void computeMotionVectors(
    device const float *depthBuf          [[buffer(0)]],
    texture2d<float, access::write> motionTex [[texture(0)]],
    constant RTRayGenParams &curCam       [[buffer(1)]],
    constant RTMat4 &prevViewProj         [[buffer(2)]],
    uint2 tid [[thread_position_in_grid]])
{
  int w = curCam.imageWidth;
  int h = curCam.imageHeight;
  if (int(tid.x) >= w || int(tid.y) >= h) return;

  int idx = tid.x + tid.y * w;
  float depth = depthBuf[idx];

  // Reconstruct world position: camPos + depth * rayDir
  float px = float(tid.x) + 0.5f;
  float py = float(tid.y) + 0.5f;
  float3 rayDir = generateRayDir(curCam, px, py);
  float3 worldPos = f3(curCam.cameraPos) + rayDir * depth;

  // Project through previous frame's viewProj (Q-matrix)
  // Q is stored as 4×4 with rows 0-2 being the 3×4 projection.
  // result = Q * (x, y, z, 1), then screen = result.xy / result.z
  constant RTMat4 &Q = prevViewProj;
  float qx = Q.cols[0][0]*worldPos.x + Q.cols[1][0]*worldPos.y +
             Q.cols[2][0]*worldPos.z + Q.cols[3][0];
  float qy = Q.cols[0][1]*worldPos.x + Q.cols[1][1]*worldPos.y +
             Q.cols[2][1]*worldPos.z + Q.cols[3][1];
  float qz = Q.cols[0][2]*worldPos.x + Q.cols[1][2]*worldPos.y +
             Q.cols[2][2]*worldPos.z + Q.cols[3][2];

  float2 motion = float2(0);
  if (abs(qz) > 1e-6f) {
    float prevPx = qx / qz;
    float prevPy = qy / qz;
    // Motion in pixels, normalized to [0,1] range for MetalFX
    motion.x = (px - prevPx) / float(w);
    motion.y = (py - prevPy) / float(h);
  }

  motionTex.write(float4(motion.x, motion.y, 0, 0), tid);
}

// ==========================================================================
// Denoiser parameter structs (shared by À-Trous and SVGF)
// ==========================================================================

struct ATrousParams {
  int width;
  int height;
  int stepSize;
  float sigmaColor;
  float sigmaDepth;
  float sigmaNormal;
};

// ==========================================================================
// SVGF — Spatiotemporal Variance-Guided Filtering
// ==========================================================================

struct SVGFTemporalParams {
  int width;
  int height;
  int hasPrevFrame;   // boolean
  int _pad;
};

/// Temporal accumulation: reproject current frame through previous camera,
/// blend with previous filtered result, update luminance moments and history.
[[kernel]] void svgfTemporalAccum(
    // Current frame
    device const float *curR      [[buffer(0)]],
    device const float *curG      [[buffer(1)]],
    device const float *curB      [[buffer(2)]],
    device const float *depth     [[buffer(3)]],
    device const float *normalX   [[buffer(4)]],
    device const float *normalY   [[buffer(5)]],
    device const float *normalZ   [[buffer(6)]],
    // Previous frame state (read)
    device const float *prevR     [[buffer(7)]],
    device const float *prevG     [[buffer(8)]],
    device const float *prevB     [[buffer(9)]],
    device const float *prevDepth [[buffer(10)]],
    device const float *prevNX    [[buffer(11)]],
    device const float *prevNY    [[buffer(12)]],
    device const float *prevNZ    [[buffer(13)]],
    device const float *prevMom1  [[buffer(14)]],
    device const float *prevMom2  [[buffer(15)]],
    device const int   *prevHist  [[buffer(16)]],
    // Output (write)
    device float       *outR      [[buffer(17)]],
    device float       *outG      [[buffer(18)]],
    device float       *outB      [[buffer(19)]],
    device float       *outMom1   [[buffer(20)]],
    device float       *outMom2   [[buffer(21)]],
    device int         *outHist   [[buffer(22)]],
    device float       *outVar    [[buffer(23)]],
    // Camera params
    constant RTRayGenParams &camera    [[buffer(24)]],
    constant RTMat4         &prevVP    [[buffer(25)]],
    constant SVGFTemporalParams &params [[buffer(26)]],
    uint2 tid [[thread_position_in_grid]])
{
  int w = params.width, h = params.height;
  int px = int(tid.x), py = int(tid.y);
  if (px >= w || py >= h) return;

  int idx = px + py * w;
  float cr = curR[idx], cg = curG[idx], cb = curB[idx];
  float lum = 0.2126f * cr + 0.7152f * cg + 0.0722f * cb;

  bool valid = false;
  int prevIdx = 0;

  if (params.hasPrevFrame) {
    // Reconstruct world position
    float fpx = float(px) + 0.5f, fpy = float(py) + 0.5f;
    float3 rayDir = generateRayDir(camera, fpx, fpy);
    float3 worldPos = f3(camera.cameraPos) + rayDir * depth[idx];

    // Reproject through previous viewProj
    float qx = prevVP.cols[0][0]*worldPos.x + prevVP.cols[1][0]*worldPos.y +
               prevVP.cols[2][0]*worldPos.z + prevVP.cols[3][0];
    float qy = prevVP.cols[0][1]*worldPos.x + prevVP.cols[1][1]*worldPos.y +
               prevVP.cols[2][1]*worldPos.z + prevVP.cols[3][1];
    float qz = prevVP.cols[0][2]*worldPos.x + prevVP.cols[1][2]*worldPos.y +
               prevVP.cols[2][2]*worldPos.z + prevVP.cols[3][2];

    if (abs(qz) > 1e-6f) {
      int ppx = int(qx / qz);
      int ppy = int(qy / qz);
      if (ppx >= 0 && ppx < w && ppy >= 0 && ppy < h) {
        prevIdx = ppx + ppy * w;
        float depthRatio = (prevDepth[prevIdx] > 1e-6f)
            ? abs(depth[idx] - prevDepth[prevIdx]) / prevDepth[prevIdx] : 1.0f;
        float ndot = normalX[idx]*prevNX[prevIdx] +
                     normalY[idx]*prevNY[prevIdx] +
                     normalZ[idx]*prevNZ[prevIdx];
        valid = (depthRatio < 0.1f && ndot > 0.9f);
      }
    }
  }

  if (valid) {
    int hist = min(255, prevHist[prevIdx] + 1);
    float alpha = max(1.0f / float(hist), 0.05f);
    outR[idx] = prevR[prevIdx] * (1-alpha) + cr * alpha;
    outG[idx] = prevG[prevIdx] * (1-alpha) + cg * alpha;
    outB[idx] = prevB[prevIdx] * (1-alpha) + cb * alpha;
    float m1 = prevMom1[prevIdx] * (1-alpha) + lum * alpha;
    float m2 = prevMom2[prevIdx] * (1-alpha) + lum*lum * alpha;
    outMom1[idx] = m1;
    outMom2[idx] = m2;
    outVar[idx] = max(0.0f, m2 - m1*m1);
    outHist[idx] = hist;
  } else {
    // Disoccluded: use current frame, estimate spatial variance
    outR[idx] = cr;
    outG[idx] = cg;
    outB[idx] = cb;
    outMom1[idx] = lum;
    outMom2[idx] = lum * lum;
    outHist[idx] = 1;

    // 3×3 spatial variance
    float sumL = 0, sumL2 = 0;
    int cnt = 0;
    for (int ky = -1; ky <= 1; ky++) {
      int ny = clamp(py+ky, 0, h-1);
      for (int kx = -1; kx <= 1; kx++) {
        int nx = clamp(px+kx, 0, w-1);
        int ni = nx + ny * w;
        float l = 0.2126f*curR[ni] + 0.7152f*curG[ni] + 0.0722f*curB[ni];
        sumL += l; sumL2 += l*l; cnt++;
      }
    }
    outVar[idx] = max(0.0f, sumL2/float(cnt) - (sumL/float(cnt))*(sumL/float(cnt)));
  }
}

/// SVGF variant of À-Trous: uses per-pixel variance for luminance weight,
/// and reflectivity to reduce filtering on mirror-like surfaces.
[[kernel]] void svgfATrousPass(
    device const float *inR        [[buffer(0)]],
    device const float *inG        [[buffer(1)]],
    device const float *inB        [[buffer(2)]],
    device float       *outR       [[buffer(3)]],
    device float       *outG       [[buffer(4)]],
    device float       *outB       [[buffer(5)]],
    device const float *depth      [[buffer(6)]],
    device const float *normalX    [[buffer(7)]],
    device const float *normalY    [[buffer(8)]],
    device const float *normalZ    [[buffer(9)]],
    device const float *variance   [[buffer(10)]],
    device float       *outVar     [[buffer(11)]],
    device const float *reflectivity [[buffer(12)]],
    constant ATrousParams &params  [[buffer(13)]],
    uint2 tid [[thread_position_in_grid]])
{
  int w = params.width, h = params.height;
  int px = int(tid.x), py = int(tid.y);
  if (px >= w || py >= h) return;

  int ci = px + py * w;
  float cR = inR[ci], cG = inG[ci], cB = inB[ci];
  float cD = depth[ci];
  float cNx = normalX[ci], cNy = normalY[ci], cNz = normalZ[ci];
  float cLum = 0.2126f*cR + 0.7152f*cG + 0.0722f*cB;
  float cVar = variance[ci];
  float lumSigma = params.sigmaColor * sqrt(max(1e-6f, cVar)) + 1e-6f;

  // Reduce spatial filtering for reflective surfaces — their color variation
  // is from the reflected scene, not noise.
  float refl = reflectivity[ci];
  float reflScale = max(0.05f, 1.0f - refl);

  const float bspline[5] = {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f};
  int step = params.stepSize;

  float sumR = 0, sumG = 0, sumB = 0, sumW = 0, sumVar = 0;
  for (int ky = -2; ky <= 2; ky++) {
    int ny = clamp(py + ky*step, 0, h-1);
    for (int kx = -2; kx <= 2; kx++) {
      int nx = clamp(px + kx*step, 0, w-1);
      int ni = nx + ny * w;

      float ws = bspline[ky+2] * bspline[kx+2];
      float dd = abs(depth[ni] - cD);
      float wd = exp(-dd / (params.sigmaDepth * abs(cD) + 1e-6f));
      float ndot = normalX[ni]*cNx + normalY[ni]*cNy + normalZ[ni]*cNz;
      float wn = pow(max(0.0f, ndot), params.sigmaNormal);
      float nLum = 0.2126f*inR[ni] + 0.7152f*inG[ni] + 0.0722f*inB[ni];
      float wl = exp(-abs(nLum - cLum) / lumSigma);

      float wt = ws * wd * wn * wl;
      // Scale down neighbor contribution for reflective pixels
      if (kx != 0 || ky != 0) wt *= reflScale;
      sumR += wt * inR[ni];
      sumG += wt * inG[ni];
      sumB += wt * inB[ni];
      sumVar += wt * variance[ni];
      sumW += wt;
    }
  }

  if (sumW > 1e-10f) {
    outR[ci] = sumR/sumW; outG[ci] = sumG/sumW; outB[ci] = sumB/sumW;
    outVar[ci] = sumVar/sumW;
  } else {
    outR[ci] = cR; outG[ci] = cG; outB[ci] = cB;
    outVar[ci] = cVar;
  }
}

// ==========================================================================
// À-Trous wavelet denoiser (guided by depth + normals)
// ==========================================================================
//
// One pass of the 5×5 B3-spline wavelet filter with edge-stopping weights
// driven by depth discontinuity and normal divergence.
// Run 5 times with step sizes 1, 2, 4, 8, 16 (passed as 'stepSize').

[[kernel]] void atrousFilter(
    device const float *inR      [[buffer(0)]],
    device const float *inG      [[buffer(1)]],
    device const float *inB      [[buffer(2)]],
    device float       *outR     [[buffer(3)]],
    device float       *outG     [[buffer(4)]],
    device float       *outB     [[buffer(5)]],
    device const float *depth    [[buffer(6)]],
    device const float *normalX  [[buffer(7)]],
    device const float *normalY  [[buffer(8)]],
    device const float *normalZ  [[buffer(9)]],
    constant ATrousParams &params [[buffer(10)]],
    uint2 tid [[thread_position_in_grid]])
{
  int w = params.width;
  int h = params.height;
  int px = int(tid.x);
  int py = int(tid.y);
  if (px >= w || py >= h) return;

  int ci = px + py * w;
  float cR = inR[ci], cG = inG[ci], cB = inB[ci];
  float cD = depth[ci];
  float cNx = normalX[ci], cNy = normalY[ci], cNz = normalZ[ci];
  float cLum = 0.2126f * cR + 0.7152f * cG + 0.0722f * cB;

  // B3-spline 1D weights: [1/16, 4/16, 6/16, 4/16, 1/16]
  const float bspline[5] = {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f};

  int step = params.stepSize;
  float sumR = 0, sumG = 0, sumB = 0, sumW = 0;

  for (int ky = -2; ky <= 2; ky++) {
    int ny = clamp(py + ky * step, 0, h - 1);
    for (int kx = -2; kx <= 2; kx++) {
      int nx = clamp(px + kx * step, 0, w - 1);
      int ni = nx + ny * w;

      // Spatial weight (separable B3-spline)
      float ws = bspline[ky + 2] * bspline[kx + 2];

      // Depth weight
      float dd = abs(depth[ni] - cD);
      float wd = exp(-dd / (params.sigmaDepth * abs(cD) + 1e-6f));

      // Normal weight
      float ndot = normalX[ni]*cNx + normalY[ni]*cNy + normalZ[ni]*cNz;
      float wn = pow(max(0.0f, ndot), params.sigmaNormal);

      // Luminance weight
      float nLum = 0.2126f * inR[ni] + 0.7152f * inG[ni] + 0.0722f * inB[ni];
      float dl = abs(nLum - cLum);
      float wl = exp(-dl / (params.sigmaColor + 1e-6f));

      float wt = ws * wd * wn * wl;
      sumR += wt * inR[ni];
      sumG += wt * inG[ni];
      sumB += wt * inB[ni];
      sumW += wt;
    }
  }

  if (sumW > 1e-10f) {
    outR[ci] = sumR / sumW;
    outG[ci] = sumG / sumW;
    outB[ci] = sumB / sumW;
  } else {
    outR[ci] = cR;
    outG[ci] = cG;
    outB[ci] = cB;
  }
}

struct ImageDims { int width; int height; };

/// Convert uint8 planar R/G/B → float [0,1] planar R/G/B.
[[kernel]] void u8ToFloat(
    device const uchar *inR   [[buffer(0)]],
    device const uchar *inG   [[buffer(1)]],
    device const uchar *inB   [[buffer(2)]],
    device float       *outR  [[buffer(3)]],
    device float       *outG  [[buffer(4)]],
    device float       *outB  [[buffer(5)]],
    constant ImageDims &dims  [[buffer(6)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (int(tid.x) >= dims.width || int(tid.y) >= dims.height) return;
  int i = tid.x + tid.y * dims.width;
  outR[i] = float(inR[i]) / 255.0f;
  outG[i] = float(inG[i]) / 255.0f;
  outB[i] = float(inB[i]) / 255.0f;
}

/// Convert float [0,1] planar R/G/B → uint8 planar R/G/B.
[[kernel]] void floatToU8(
    device const float *inR   [[buffer(0)]],
    device const float *inG   [[buffer(1)]],
    device const float *inB   [[buffer(2)]],
    device uchar       *outR  [[buffer(3)]],
    device uchar       *outG  [[buffer(4)]],
    device uchar       *outB  [[buffer(5)]],
    constant ImageDims &dims  [[buffer(6)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (int(tid.x) >= dims.width || int(tid.y) >= dims.height) return;
  int i = tid.x + tid.y * dims.width;
  outR[i] = uchar(clamp(inR[i], 0.0f, 1.0f) * 255.0f);
  outG[i] = uchar(clamp(inG[i], 0.0f, 1.0f) * 255.0f);
  outB[i] = uchar(clamp(inB[i], 0.0f, 1.0f) * 255.0f);
}

// ==========================================================================
// Tone mapping (applied in-place on float RGB buffers)
// ==========================================================================

struct ToneMapParams {
  int width;
  int height;
  int method;     // 0=none, 1=reinhard, 2=aces, 3=hable
  float exposure;
};

inline float hableFunc(float x) {
  float A=0.15f, B=0.50f, C=0.10f, D=0.20f, E=0.02f, F=0.30f;
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

[[kernel]] void toneMap(
    device float *R [[buffer(0)]],
    device float *G [[buffer(1)]],
    device float *B [[buffer(2)]],
    constant ToneMapParams &params [[buffer(3)]],
    uint2 tid [[thread_position_in_grid]])
{
  if (int(tid.x) >= params.width || int(tid.y) >= params.height) return;
  int i = tid.x + tid.y * params.width;

  float r = R[i] * params.exposure;
  float g = G[i] * params.exposure;
  float b = B[i] * params.exposure;

  if (params.method == 1) {
    // Reinhard
    r = r / (1.0f + r);
    g = g / (1.0f + g);
    b = b / (1.0f + b);
  } else if (params.method == 2) {
    // ACES filmic (Narkowicz 2015)
    float a = 2.51f, ba = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    r = clamp((r*(a*r+ba))/(r*(c*r+d)+e), 0.0f, 1.0f);
    g = clamp((g*(a*g+ba))/(g*(c*g+d)+e), 0.0f, 1.0f);
    b = clamp((b*(a*b+ba))/(b*(c*b+d)+e), 0.0f, 1.0f);
  } else if (params.method == 3) {
    // Hable / Uncharted 2
    float ws = 1.0f / hableFunc(11.2f);
    r = hableFunc(r) * ws;
    g = hableFunc(g) * ws;
    b = hableFunc(b) * ws;
  }

  R[i] = clamp(r, 0.0f, 1.0f);
  G[i] = clamp(g, 0.0f, 1.0f);
  B[i] = clamp(b, 0.0f, 1.0f);
}

// ==========================================================================
// Direct lighting kernel (Blinn-Phong + reflections)
// ==========================================================================

[[kernel]]
void raytrace(
    instance_acceleration_structure accelStruct [[buffer(0)]],
    device const InstanceData *instances       [[buffer(1)]],
    device const RTVertex     *vertices        [[buffer(2)]],
    device const RTTriangle   *triangles       [[buffer(3)]],
    device const RTLight      *lights          [[buffer(4)]],
    device const RTMaterial   *materials       [[buffer(5)]],
    device uchar              *outR            [[buffer(6)]],
    device uchar              *outG            [[buffer(7)]],
    device uchar              *outB            [[buffer(8)]],
    device int                *objectIds       [[buffer(9)]],
    device float              *depthOut        [[buffer(10)]],
    device float              *normalXOut     [[buffer(11)]],
    device float              *normalYOut     [[buffer(12)]],
    device float              *normalZOut     [[buffer(13)]],
    device float              *reflectOut     [[buffer(14)]],
    constant RTRayGenParams   &camera          [[buffer(15)]],
    constant SceneParams      &params          [[buffer(16)]],
    uint2 tid [[thread_position_in_grid]])
{
  int px = tid.x;
  int py = tid.y;
  int w = camera.imageWidth;
  int h = camera.imageHeight;
  if (px >= w || py >= h) return;

  int idx = px + py * w;
  float3 camPos = f3(camera.cameraPos);
  float3 dir = generateRayDir(camera, float(px) + 0.5f, float(py) + 0.5f);
  float3 origin = camPos;
  float3 color = float3(0);
  float3 throughput = float3(1);
  int primaryInstance = -1;

  intersector<triangle_data, instancing> inter;
  inter.accept_any_intersection(false);

  for (int bounce = 0; bounce < 4; bounce++) {
    ray r;
    r.origin = origin;
    r.direction = dir;
    r.min_distance = 0.5f;
    r.max_distance = 1e30f;

    auto result = inter.intersect(r, accelStruct);

    if (bounce == 0) {
      primaryInstance = (result.type != intersection_type::none)
                            ? int(result.instance_id)
                            : -1;
      depthOut[idx] = (result.type != intersection_type::none)
                          ? result.distance
                          : camera.farClip;
      normalXOut[idx] = normalYOut[idx] = normalZOut[idx] = 0;
      reflectOut[idx] = 0;
    }

    if (result.type == intersection_type::none) {
      color += throughput * f3v(params.bgColor);
      break;
    }

    SurfaceHit s = interpolate(instances[result.instance_id], vertices,
                                triangles, result.primitive_id,
                                result.triangle_barycentric_coord);

    if (dot(s.normal, dir) > 0) s.normal = -s.normal;

    device const RTMaterial &mat = materials[s.materialIndex];

    if (bounce == 0) {
      normalXOut[idx] = s.normal.x;
      normalYOut[idx] = s.normal.y;
      normalZOut[idx] = s.normal.z;
      reflectOut[idx] = mat.reflectivity;
    }

    // Emission
    color += throughput * f3v(mat.emission);

    // Direct lighting
    float3 direct = directLight(s.position, s.normal, dir, s.color, mat,
                                 lights, params.numLights, accelStruct);

    float refl = mat.reflectivity;
    color += throughput * direct * (1.0f - refl);

    if (refl < 0.01f) break;

    // Reflect
    throughput *= refl;
    dir = dir - s.normal * (2.0f * dot(dir, s.normal));
    dir = normalize(dir);
    origin = s.position + s.normal * 1.0f;
  }

  objectIds[idx] = primaryInstance;
  color = clamp(color, 0.0f, 1.0f);
  outR[idx] = uchar(color.x * 255);
  outG[idx] = uchar(color.y * 255);
  outB[idx] = uchar(color.z * 255);
}

// ==========================================================================
// Path tracing kernel (GI with temporal accumulation)
// ==========================================================================

[[kernel]]
void pathTrace(
    instance_acceleration_structure accelStruct [[buffer(0)]],
    device const InstanceData *instances       [[buffer(1)]],
    device const RTVertex     *vertices        [[buffer(2)]],
    device const RTTriangle   *triangles       [[buffer(3)]],
    device const RTLight      *lights          [[buffer(4)]],
    device const RTMaterial   *materials       [[buffer(5)]],
    device float              *accumR          [[buffer(6)]],
    device float              *accumG          [[buffer(7)]],
    device float              *accumB          [[buffer(8)]],
    device uchar              *outR            [[buffer(9)]],
    device uchar              *outG            [[buffer(10)]],
    device uchar              *outB            [[buffer(11)]],
    device int                *objectIds       [[buffer(12)]],
    device float              *depthOut        [[buffer(13)]],
    device float              *normalXOut     [[buffer(14)]],
    device float              *normalYOut     [[buffer(15)]],
    device float              *normalZOut     [[buffer(16)]],
    device float              *reflectOut     [[buffer(17)]],
    device const RTEmissiveTriangle *emissives [[buffer(18)]],
    constant RTRayGenParams   &camera          [[buffer(19)]],
    constant SceneParams      &params          [[buffer(20)]],
    uint2 tid [[thread_position_in_grid]])
{
  int px = tid.x;
  int py = tid.y;
  int w = camera.imageWidth;
  int h = camera.imageHeight;
  if (px >= w || py >= h) return;

  int idx = px + py * w;
  uint rng = uint(px * 1973 + py * 9277 + params.frameNumber * 26699) | 1u;

  float3 camPos = f3(camera.cameraPos);

  // Jittered pixel position
  float fpx = float(px) + rngFloat(rng);
  float fpy = float(py) + rngFloat(rng);
  float3 dir = generateRayDir(camera, fpx, fpy);
  float3 origin = camPos;
  float3 color = float3(0);
  float3 throughput = float3(1);
  int primaryInstance = -1;

  intersector<triangle_data, instancing> inter;
  inter.accept_any_intersection(false);

  for (int bounce = 0; bounce < 5; bounce++) {
    ray r;
    r.origin = origin;
    r.direction = dir;
    r.min_distance = 0.5f;
    r.max_distance = 1e30f;

    auto result = inter.intersect(r, accelStruct);

    if (bounce == 0) {
      primaryInstance = (result.type != intersection_type::none)
                            ? int(result.instance_id)
                            : -1;
      depthOut[idx] = (result.type != intersection_type::none)
                          ? result.distance
                          : camera.farClip;
      normalXOut[idx] = normalYOut[idx] = normalZOut[idx] = 0;
      reflectOut[idx] = 0;
    }

    if (result.type == intersection_type::none) {
      // Sky gradient
      float t = 0.5f * (dir.y + 1.0f);
      float3 sky = float3(0.05f, 0.05f, 0.08f) * (1-t) +
                   float3(0.1f, 0.12f, 0.2f) * t;
      color += throughput * sky;
      break;
    }

    SurfaceHit s = interpolate(instances[result.instance_id], vertices,
                                triangles, result.primitive_id,
                                result.triangle_barycentric_coord);

    if (dot(s.normal, dir) > 0) s.normal = -s.normal;

    device const RTMaterial &mat = materials[s.materialIndex];

    if (bounce == 0) {
      normalXOut[idx] = s.normal.x;
      normalYOut[idx] = s.normal.y;
      normalZOut[idx] = s.normal.z;
      reflectOut[idx] = mat.reflectivity;
    }

    // Emission
    color += throughput * f3v(mat.emission);

    // Direct lighting (next event estimation) with hardware shadow rays
    intersector<instancing> shadowInter;
    shadowInter.accept_any_intersection(true);

    for (int li = 0; li < params.numLights; li++) {
      if (!lights[li].on) continue;
      float3 lightPos = f3(lights[li].position);
      float3 L = lightPos - s.position;
      float dist = length(L);
      if (dist < 1e-6f) continue;
      L /= dist;
      float NdotL = max(0.0f, dot(s.normal, L));
      if (NdotL <= 0) continue;

      ray sr;
      sr.origin = s.position + s.normal * 1.0f;
      sr.direction = L;
      sr.min_distance = 0.0f;
      sr.max_distance = dist;

      auto shadowResult = shadowInter.intersect(sr, accelStruct);
      if (shadowResult.type != intersection_type::none) continue;

      float atten = 1.0f / (lights[li].attenuation.x +
                             lights[li].attenuation.y * dist +
                             lights[li].attenuation.z * dist * dist);
      color += throughput * f3v(lights[li].diffuse) * s.color * NdotL * atten;
    }

    // Area light sampling: pick one random emissive triangle
    if (params.numEmissives > 0 && params.totalEmissiveArea > 0) {
      // Select triangle weighted by area
      float r = rngFloat(rng) * params.totalEmissiveArea;
      float cumArea = 0;
      int ei = 0;
      for (int j = 0; j < params.numEmissives; j++) {
        cumArea += emissives[j].area;
        if (cumArea >= r) { ei = j; break; }
      }

      // Random point on triangle (uniform barycentric)
      float u1 = rngFloat(rng), u2 = rngFloat(rng);
      if (u1 + u2 > 1.0f) { u1 = 1.0f - u1; u2 = 1.0f - u2; }
      float3 lv0 = f3(emissives[ei].v0);
      float3 lv1 = f3(emissives[ei].v1);
      float3 lv2 = f3(emissives[ei].v2);
      float3 lightPt = lv0 * (1-u1-u2) + lv1 * u1 + lv2 * u2;
      float3 lightN = f3(emissives[ei].normal);

      float3 toLight = lightPt - s.position;
      float dist = length(toLight);
      if (dist > 1e-4f) {
        float3 L = toLight / dist;
        float NdotL = max(0.0f, dot(s.normal, L));
        float lightNdotL = max(0.0f, -dot(lightN, L));
        if (NdotL > 0 && lightNdotL > 0) {
          ray sr;
          sr.origin = s.position + s.normal * 1.0f;
          sr.direction = L;
          sr.min_distance = 0.0f;
          sr.max_distance = dist - 1.0f;

          auto shadowResult = shadowInter.intersect(sr, accelStruct);
          if (shadowResult.type == intersection_type::none) {
            float geomTerm = NdotL * lightNdotL * params.totalEmissiveArea / (dist * dist);
            color += throughput * f3(emissives[ei].emission) * s.color * geomTerm;
          }
        }
      }
    }

    // Mirror reflection or diffuse bounce
    float refl = mat.reflectivity;
    if (refl > 0.01f && rngFloat(rng) < refl) {
      dir = dir - s.normal * (2.0f * dot(dir, s.normal));
      dir = normalize(dir);
    } else {
      dir = randomHemisphere(s.normal, rng);
      throughput *= s.color;
    }
    origin = s.position + s.normal * 1.0f;

    // Russian roulette after bounce 2
    if (bounce > 1) {
      float p = max(throughput.x, max(throughput.y, throughput.z));
      if (rngFloat(rng) > p) break;
      throughput /= p;
    }
  }

  // Running average accumulation
  float weight = 1.0f / float(params.frameNumber);
  accumR[idx] += (color.x - accumR[idx]) * weight;
  accumG[idx] += (color.y - accumG[idx]) * weight;
  accumB[idx] += (color.z - accumB[idx]) * weight;

  // Object ID on first frame only
  if (params.frameNumber == 1) objectIds[idx] = primaryInstance;

  outR[idx] = uchar(clamp(accumR[idx], 0.0f, 1.0f) * 255);
  outG[idx] = uchar(clamp(accumG[idx], 0.0f, 1.0f) * 255);
  outB[idx] = uchar(clamp(accumB[idx], 0.0f, 1.0f) * 255);
}
