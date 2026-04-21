// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <cstdint>
#include <cstring>

namespace icl::rt {

// GPU-compatible POD types — alignas(16) for Metal/SIMD compatibility.
// These are pure data; no ICL dependencies so they can be included from shaders or GPU code.

struct alignas(16) RTFloat3 {
  float x, y, z;
  float _pad = 0;

  RTFloat3() : x(0), y(0), z(0), _pad(0) {}
  RTFloat3(float x, float y, float z) : x(x), y(y), z(z), _pad(0) {}

  RTFloat3 operator+(const RTFloat3 &v) const { return {x+v.x, y+v.y, z+v.z}; }
  RTFloat3 operator-(const RTFloat3 &v) const { return {x-v.x, y-v.y, z-v.z}; }
  RTFloat3 operator*(float s) const { return {x*s, y*s, z*s}; }
  float dot(const RTFloat3 &v) const { return x*v.x + y*v.y + z*v.z; }
  RTFloat3 cross(const RTFloat3 &v) const {
    return {y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x};
  }
};

struct alignas(16) RTFloat4 {
  float x, y, z, w;

  RTFloat4() : x(0), y(0), z(0), w(0) {}
  RTFloat4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  RTFloat4(const RTFloat3 &v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
};

/// 4x4 matrix in column-major order (Metal convention).
/// ICL uses row-major (Mat4D32f), so conversion is needed.
struct alignas(16) RTMat4 {
  float cols[4][4]; // cols[col][row]

  RTMat4() { std::memset(cols, 0, sizeof(cols)); }

  /// Create identity matrix.
  static RTMat4 identity() {
    RTMat4 m;
    m.cols[0][0] = m.cols[1][1] = m.cols[2][2] = m.cols[3][3] = 1.0f;
    return m;
  }
};

// ---- Scene geometry types ----

/// Per-vertex data packed for GPU buffers.
struct alignas(16) RTVertex {
  RTFloat3 position;
  RTFloat3 normal;
  RTFloat4 color; // RGBA [0,1]
  float u = 0, v = 0;  // texture coordinates
  float _vtxPad[2] = {};
};

/// Triangle: 3 indices into the per-object vertex array.
struct RTTriangle {
  uint32_t v0, v1, v2;
  uint32_t materialIndex;
};

/// Per-object material properties (PBR metallic-roughness).
struct alignas(16) RTMaterial {
  RTFloat4 baseColor;       // albedo (RGBA, [0,1])
  RTFloat4 emissive;        // emission color (RGB * intensity, [0,1]+)
  float metallic = 0;       // 0 = dielectric, 1 = metal
  float roughness = 0.5f;   // 0 = mirror, 1 = fully diffuse
  float reflectivity = 0;   // explicit mirror reflection (0=matte, 1=mirror)
  float _pad = 0;
};

/// Light source — mirrors ICL's SceneLight properties.
struct alignas(16) RTLight {
  RTFloat3 position;
  RTFloat4 ambient;
  RTFloat4 diffuse;
  RTFloat4 specular;
  RTFloat3 spotDirection;
  RTFloat3 attenuation;     // (constant, linear, quadratic)
  float spotCutoff;         // degrees (180 = omnidirectional)
  float spotExponent;
  int32_t on;               // boolean as int for GPU compat
  float _pad = 0;
};

static constexpr int RT_MAX_LIGHTS = 8;

/// Per-object instance in the two-level acceleration structure.
/// BLAS is in object-local coords; this transform places it in world space.
struct alignas(16) RTInstance {
  RTMat4 transform;         // object-to-world (column-major)
  RTMat4 transformInverse;  // world-to-object (for transforming rays)
  int32_t blasIndex;        // which BLAS this instance references
  int32_t materialIndex;    // base material index for this object
  int32_t vertexOffset;     // offset into global vertex buffer
  int32_t triangleOffset;   // offset into global triangle buffer
};

/// Camera parameters for ray generation.
struct alignas(16) RTRayGenParams {
  RTFloat3 cameraPos;       // camera position in world space
  RTMat4 invViewProj;       // inverse of combined view-projection (for pixel→ray)
  RTMat4 viewProj;          // forward view-projection (for motion vector reprojection)
  int32_t imageWidth;
  int32_t imageHeight;
  float nearClip;
  float farClip;
};

/// Upsampling method used when renderScale < 1.0.
enum class UpsamplingMethod {
  None = 0,       ///< No upsampling — render at full resolution
  Bilinear,       ///< Bilinear interpolation (all backends)
  EdgeAware,      ///< Bilateral edge-preserving upscale (all backends)
  MetalFXSpatial, ///< Apple MetalFX spatial upscaler (Metal only)
  MetalFXTemporal ///< Apple MetalFX temporal upscaler (Metal only)
};

/// Denoising method for path-traced output.
enum class DenoisingMethod {
  None = 0,       ///< No denoising
  Bilateral,      ///< Edge-preserving bilateral filter (all backends)
  ATrousWavelet,  ///< À-Trous wavelet filter, 5 passes (all backends)
  SVGF,           ///< Spatiotemporal variance-guided filtering (depth + normals + motion)
  // Future:
  // OIDN,        ///< Intel Open Image Denoise (library dependency)
};

/// Emissive triangle for area light sampling in path tracing.
/// Precomputed during scene build — one entry per emissive triangle.
struct alignas(16) RTEmissiveTriangle {
  RTFloat3 v0, v1, v2;   // world-space vertex positions
  RTFloat3 normal;        // face normal (for oriented emission)
  RTFloat3 emission;      // emission color (RGB * intensity)
  float area;             // triangle area (for PDF)
  float _pad[3] = {};
};

/// Tone mapping curve for HDR → LDR conversion.
enum class ToneMapMethod {
  None = 0,       ///< Clamp to [0,1] (current default)
  Reinhard,       ///< Reinhard: color / (1 + color)
  ACES,           ///< ACES filmic (industry standard, rich contrast)
  Hable,          ///< Uncharted 2 / Hable filmic (nice shoulder)
};

} // namespace icl::rt
