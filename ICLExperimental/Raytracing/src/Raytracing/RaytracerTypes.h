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
};

/// Triangle: 3 indices into the per-object vertex array.
struct RTTriangle {
  uint32_t v0, v1, v2;
  uint32_t materialIndex;
};

/// Per-object material properties.
struct alignas(16) RTMaterial {
  RTFloat4 diffuseColor;    // base color (RGBA)
  RTFloat4 specularColor;   // specular reflectance color
  RTFloat4 emission;        // emissive color + intensity (RGB * intensity)
  float shininess;          // Phong exponent (0-255 from ICL)
  float reflectivity;       // mirror reflection (0=matte, 1=mirror)
  float _pad[2] = {};
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
  // Future:
  // SVGF,        ///< Spatiotemporal variance-guided filtering (needs depth + normals + motion)
  // OIDN,        ///< Intel Open Image Denoise (library dependency)
};

} // namespace icl::rt
