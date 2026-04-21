// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/GLRenderer.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/Sky.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/SceneLight.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Material.h>
#include <ICLGeom/Primitive.h>
#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>
#include <algorithm>
#include <vector>
#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <ICLQt/DrawWidget3D.h>
#include <ICLQt/Widget.h>

#include <cstdio>
#include <unordered_map>

using namespace icl::math;
using namespace icl::core;
using namespace icl::utils;

namespace icl::geom {

// ---- Shader sources (GL 4.1 Core) ----
// ---- Sky gradient shaders ----

static const char *SKY_VERT = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
out vec2 vNDC;
void main() {
    vNDC = aPos;
    gl_Position = vec4(aPos, 0.999, 1.0);  // at far plane
}
)";

static const char *SKY_FRAG = R"(
#version 410 core
uniform mat4 uInvVP;
uniform int uSkyMode;        // 0=solid, 1=gradient, 2=physical
uniform vec3 uSkyZenith;
uniform vec3 uSkyHorizon;
uniform vec3 uSkyGround;
uniform float uSkySharpness;
uniform vec3 uSkySunDir;
in vec2 vNDC;
out vec4 FragColor;
void main() {
    vec4 worldFar = uInvVP * vec4(vNDC, 1.0, 1.0);
    vec3 dir = normalize(worldFar.xyz / worldFar.w);
    float y = dir.y;

    vec3 color;
    if (uSkyMode == 0) {
        color = uSkyHorizon;
    } else if (uSkyMode == 2) {
        // Physical: simplified Preetham with sun disc
        float sunDot = max(dot(dir, normalize(uSkySunDir)), 0.0);
        float sunDisc = pow(sunDot, 256.0) * 4.0;
        float sunGlow = pow(sunDot, 8.0) * 0.3;
        vec3 skyBase;
        if (y > 0.0) {
            float t = pow(y, uSkySharpness);
            skyBase = mix(uSkyHorizon, uSkyZenith, t);
        } else {
            skyBase = mix(uSkyHorizon, uSkyGround, min(-y * 3.0, 1.0));
        }
        color = skyBase + vec3(1.0, 0.95, 0.8) * (sunDisc + sunGlow);
    } else {
        // Gradient (default)
        if (y > 0.0) {
            float t = pow(y, uSkySharpness);
            color = mix(uSkyHorizon, uSkyZenith, t);
        } else {
            color = mix(uSkyHorizon, uSkyGround, min(-y * 3.0, 1.0));
        }
    }
    FragColor = vec4(color, 1.0);
}
)";

// ---- Shadow pass shaders (depth only) ----

static const char *SHADOW_VERT = R"(
#version 410 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModelMatrix;
uniform mat4 uLightVP;
void main() {
    gl_Position = uLightVP * uModelMatrix * vec4(aPosition, 1.0);
}
)";

static const char *SHADOW_FRAG = R"(
#version 410 core
void main() { }
)";

// ---- Main pass shaders ----

static const char *VERT_SHADER = R"(
#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    mat3 normalMat = mat3(uModelMatrix);
    vNormal = normalize(normalMat * aNormal);

    vTexCoord = aTexCoord;
    gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
}
)";

static const char *FRAG_SHADER = R"(
#version 410 core

uniform vec4  uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform vec4  uEmissive;
uniform float uAmbient;
uniform float uExposure;
uniform float uEnvMul;
uniform float uDirectMul;
uniform vec3  uCameraPos;
uniform int   uNumLights;
uniform vec4  uLightPos[8];
uniform vec3  uLightColor[8];

uniform int uHasBaseColorMap;
uniform sampler2D uBaseColorMap;
uniform int uHasNormalMap;
uniform sampler2D uNormalMap;
uniform int uHasMetallicRoughnessMap;
uniform sampler2D uMetallicRoughnessMap;
uniform int uHasEmissiveMap;
uniform sampler2D uEmissiveMap;
uniform int uHasOcclusionMap;
uniform sampler2D uOcclusionMap;
uniform int uDebugMode;

// Transmission / glass
uniform float uTransmission;
uniform float uIOR;
uniform vec3  uAttenuationColor;

// Screen-space reflections (previous-frame ping-pong)
uniform int uSSREnabled;
uniform sampler2D uPrevColorMap;
uniform sampler2D uPrevDepthMap;
uniform mat4 uPrevVP;
uniform vec2 uScreenSize;

// Per-light shadow: slot index into shadow map array (-1 = no shadow)
uniform int uLightShadowSlot[8];
uniform mat4 uShadowMatrix[4];
uniform sampler2DShadow uShadowMap0;
uniform sampler2DShadow uShadowMap1;
uniform sampler2DShadow uShadowMap2;
uniform sampler2DShadow uShadowMap3;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;

// Evaluate sky for a world-space direction (uses same uniforms as sky shader)
uniform vec3 uEnvZenith;
uniform vec3 uEnvHorizon;
uniform vec3 uEnvGround;
uniform float uEnvSharpness;

vec3 sampleSky(vec3 dir) {
    float y = dir.y;
    if (y > 0.0) {
        float t = pow(y, uEnvSharpness);
        return mix(uEnvHorizon, uEnvZenith, t);
    } else {
        return mix(uEnvHorizon, uEnvGround, min(-y * 3.0, 1.0));
    }
}

float sampleShadow(int slot, vec3 coord) {
    if (slot == 0) return texture(uShadowMap0, coord);
    if (slot == 1) return texture(uShadowMap1, coord);
    if (slot == 2) return texture(uShadowMap2, coord);
    return texture(uShadowMap3, coord);
}

out vec4 FragColor;

// Screen-space reflection: march along reflect direction in previous frame's
// screen space. Returns vec4(hitColor, confidence) where confidence fades
// at screen edges, long distances, and rough surfaces.
vec4 traceSSR(vec3 worldPos, vec3 reflectDir, float roughness) {
    if (uSSREnabled == 0 || roughness > 0.7) return vec4(0.0);

    // Project origin into previous frame's screen space
    vec4 startClip = uPrevVP * vec4(worldPos, 1.0);
    if (startClip.w < 0.001) return vec4(0.0);
    vec3 startNDC = startClip.xyz / startClip.w;
    vec2 startUV = startNDC.xy * 0.5 + 0.5;
    float startDepth = startNDC.z * 0.5 + 0.5;

    // Ray endpoint: use a short world-space ray to get the screen-space direction,
    // then normalize to a fixed screen-space length. This prevents far-away
    // fragments from taking huge steps that overshoot geometry.
    float probeDist = length(worldPos - uCameraPos) * 0.1;
    vec3 probeWorld = worldPos + reflectDir * probeDist;
    vec4 probeClip = uPrevVP * vec4(probeWorld, 1.0);
    if (probeClip.w < 0.001) return vec4(0.0);
    vec3 probeNDC = probeClip.xyz / probeClip.w;
    vec2 probeUV = probeNDC.xy * 0.5 + 0.5;

    // Screen-space ray direction and depth slope from the probe
    vec2 dir2D = probeUV - startUV;
    float dirLen = length(dir2D);
    if (dirLen < 1e-6) return vec4(0.0);

    // Normalize to a max screen-space travel of 0.4 (40% of screen)
    float maxScreenDist = 0.4;
    float scale = maxScreenDist / dirLen;
    vec2 rayDir2D = dir2D * scale;
    float depthSlope = ((probeNDC.z * 0.5 + 0.5) - startDepth) * scale;

    // Depth-adaptive thickness: thicker at far distances where Z precision is lower
    float baseThickness = 0.001;
    float thickness = baseThickness + startDepth * startDepth * 0.01;

    int maxSteps = 64;
    float stepSize = 1.0 / float(maxSteps);

    // Linear march
    float hitT = -1.0;
    vec2 hitUV;
    for (int i = 2; i < maxSteps; i++) {
        float t = float(i) * stepSize;
        vec2 sampleUV = startUV + rayDir2D * t;
        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 ||
            sampleUV.y < 0.0 || sampleUV.y > 1.0) break;

        float rayDepth = startDepth + depthSlope * t;
        float sceneDepth = texture(uPrevDepthMap, sampleUV).r;
        if (sceneDepth > 0.999) continue;  // sky

        float depthDiff = rayDepth - sceneDepth;
        if (depthDiff > 0.0 && depthDiff < thickness) {
            hitT = t;
            hitUV = sampleUV;
            break;
        }
    }
    if (hitT < 0.0) return vec4(0.0);

    // Binary refinement
    float lo = hitT - stepSize, hi = hitT;
    for (int i = 0; i < 6; i++) {
        float mid = (lo + hi) * 0.5;
        vec2 midUV = startUV + rayDir2D * mid;
        float rayD = startDepth + depthSlope * mid;
        float sceneD = texture(uPrevDepthMap, midUV).r;
        if (rayD > sceneD) { hi = mid; hitUV = midUV; }
        else               { lo = mid; }
    }

    // Confidence fading
    float confidence = 1.0;
    vec2 edgeDist = min(hitUV, 1.0 - hitUV);
    confidence *= smoothstep(0.0, 0.05, edgeDist.x) * smoothstep(0.0, 0.05, edgeDist.y);
    confidence *= 1.0 - smoothstep(0.4, 0.8, hitT);
    confidence *= 1.0 - smoothstep(0.3, 0.7, roughness);

    // Depth thickness rejection
    float finalRayD = startDepth + depthSlope * hi;
    float finalSceneD = texture(uPrevDepthMap, hitUV).r;
    confidence *= 1.0 - smoothstep(0.0, thickness, abs(finalRayD - finalSceneD));

    return vec4(texture(uPrevColorMap, hitUV).rgb, confidence);
}

void main() {
    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing) N = -N;
    vec3 geomN = N;  // save geometric normal before perturbation

    // Normal map: perturb N using tangent-space normal from texture
    if (uHasNormalMap != 0) {
        vec3 tsNormal = texture(uNormalMap, vTexCoord).rgb * 2.0 - 1.0;
        // Construct TBN from screen-space derivatives (no explicit tangents needed)
        vec3 dPdx = dFdx(vWorldPos);
        vec3 dPdy = dFdy(vWorldPos);
        vec2 dUVdx = dFdx(vTexCoord);
        vec2 dUVdy = dFdy(vTexCoord);
        float det = dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x;
        if (abs(det) > 1e-6) {
            float invDet = 1.0 / det;
            vec3 T = normalize((dPdx * dUVdy.y - dPdy * dUVdx.y) * invDet);
            vec3 B = normalize((dPdy * dUVdx.x - dPdx * dUVdy.x) * invDet);
            N = normalize(mat3(T, B, N) * tsNormal);
        }
    }
    // Self-occlusion: where the normal map bends normals away from the
    // geometric surface (crevices, borders), reduce ambient light.
    // Approximates the self-shadowing that path tracing computes naturally.
    float selfOcclusion = max(dot(N, geomN), 0.0);

    vec4 baseColor = uBaseColor;
    if (uHasBaseColorMap != 0) {
        baseColor *= texture(uBaseColorMap, vTexCoord);
    }
    vec3 albedo = baseColor.rgb;
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Metallic-roughness map (glTF: green=roughness, blue=metallic)
    float metallic = uMetallic;
    float roughness = uRoughness;
    if (uHasMetallicRoughnessMap != 0) {
        vec4 mr = texture(uMetallicRoughnessMap, vTexCoord);
        metallic = mr.b * uMetallic;    // blue channel, modulated by uniform
        roughness = mr.g * uRoughness;  // green channel, modulated by uniform
    }

    // Shininess from roughness (clamped to avoid invisible pinpoint highlights)
    float shininess = clamp(2.0 / (roughness * roughness + 1e-4) - 2.0, 1.0, 512.0);

    // F0: dielectric = 0.04, metal = albedo
    vec3 specColor = mix(vec3(0.04), albedo, metallic);

    // Environment reflection: sample sky in reflection direction
    vec3 R = reflect(-V, N);
    vec3 envColor = sampleSky(R);
    // Roughness blurs the reflection (approximate by blending toward diffuse sky)
    vec3 diffuseEnv = sampleSky(N);
    vec3 envReflection = mix(envColor, diffuseEnv, roughness);

    // SSR: blend with screen-space reflection where available
    vec4 ssrResult = traceSSR(vWorldPos, R, roughness);
    envReflection = mix(envReflection, ssrResult.rgb, ssrResult.a);

    // Fresnel: roughness-aware Schlick (Lagarde 2014).
    // Smoother surfaces have higher effective reflectivity at grazing angles.
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = pow(1.0 - NdotV, 5.0);
    vec3 envFresnel = specColor + (max(vec3(1.0 - roughness), specColor) - specColor) * fresnel;

    // Energy conservation: diffuse attenuated by specular reflection
    vec3 kD = (vec3(1.0) - envFresnel) * (1.0 - metallic);
    vec3 ambientDiffuse = albedo * diffuseEnv * kD * uEnvMul;
    vec3 ambientSpecular = envReflection * envFresnel * uEnvMul;
    // Occlusion: combine texture AO (if present) with normal-map self-occlusion
    float occlusion = selfOcclusion;
    if (uHasOcclusionMap != 0) {
        occlusion *= texture(uOcclusionMap, vTexCoord).r;
    }
    vec3 result = (ambientDiffuse + ambientSpecular) * occlusion;

    for (int i = 0; i < uNumLights; i++) {
        vec3 L = normalize(uLightPos[i].xyz - vWorldPos);
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        // Per-light shadow lookup
        float shadow = 1.0;
        int slot = uLightShadowSlot[i];
        if (slot >= 0 && slot < 4) {
            vec4 sc4 = uShadowMatrix[slot] * vec4(vWorldPos, 1.0);
            vec3 sc = sc4.xyz / sc4.w;
            sc = sc * 0.5 + 0.5;
            if (sc.x >= 0.0 && sc.x <= 1.0 && sc.y >= 0.0 && sc.y <= 1.0) {
                shadow = sampleShadow(slot, vec3(sc.xy, sc.z - 0.0005));
            }
        }

        // Diffuse: reduced for metals (metals have no diffuse)
        vec3 diffuse = albedo * (1.0 - metallic) * NdotL;

        // Specular: Blinn-Phong
        float specPow = pow(NdotH, shininess);
        vec3 specular = specColor * specPow;

        result += shadow * uLightColor[i] * uDirectMul * (diffuse + specular * NdotL);
    }

    // Emissive: factor * map (per glTF spec)
    vec3 emissive = uEmissive.rgb;
    if (uHasEmissiveMap != 0) {
        emissive = texture(uEmissiveMap, vTexCoord).rgb * max(uEmissive.rgb, vec3(1.0));
    }
    result += emissive;
    result *= uExposure;
    result = clamp(result, 0.0, 1.0);

    // Debug modes: 0=shaded, 1=normals, 2=albedo, 3=UVs, 4=lighting only, 5=NdotL
    if (uDebugMode == 1) { FragColor = vec4(N * 0.5 + 0.5, 1.0); return; }
    if (uDebugMode == 2) { FragColor = baseColor; return; }
    if (uDebugMode == 3) { FragColor = vec4(vTexCoord, 0.0, 1.0); return; }
    if (uDebugMode == 4) {
        vec3 whiteResult = vec3(uAmbient);
        for (int i = 0; i < uNumLights; i++) {
            vec3 L2 = normalize(uLightPos[i].xyz - vWorldPos);
            whiteResult += uLightColor[i] * max(dot(N, L2), 0.0);
        }
        FragColor = vec4(clamp(whiteResult * uExposure, 0.0, 1.0), 1.0); return;
    }
    if (uDebugMode == 5) {
        float maxNdL = 0.0;
        for (int i = 0; i < uNumLights; i++) {
            vec3 L2 = normalize(uLightPos[i].xyz - vWorldPos);
            maxNdL = max(maxNdL, dot(N, L2));
        }
        FragColor = vec4(vec3(maxNdL), 1.0); return;
    }
    if (uDebugMode == 6) {
        // SSR confidence: green=hit, red=sky fallback
        FragColor = vec4(1.0 - ssrResult.a, ssrResult.a, 0.0, 1.0); return;
    }

    // Glass / transmission
    if (uTransmission > 0.001) {
        // Fresnel: IOR-based F0, Schlick approximation
        float f0 = pow((uIOR - 1.0) / (uIOR + 1.0), 2.0);
        float fresnel = f0 + (1.0 - f0) * pow(1.0 - NdotV, 5.0);
        // alpha = how opaque: Fresnel reflections stay, rest transmits
        float alpha = fresnel + (1.0 - fresnel) * (1.0 - uTransmission);
        // Tint specular/env by attenuation color for colored glass
        result *= mix(uAttenuationColor, vec3(1.0), fresnel);
        FragColor = vec4(result, alpha);
    } else {
        FragColor = vec4(result, baseColor.a);
    }
}
)";

// ---- GLGeometryCache: VAO/VBO/EBO per SceneObject ----

struct GLGeometryCache {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  int numIndices = 0;
  GLuint baseColorTex = 0;
  GLuint normalMapTex = 0;
  GLuint metallicRoughnessTex = 0;
  GLuint emissiveMapTex = 0;
  GLuint occlusionMapTex = 0;
  Material *lastMaterial = nullptr;  // track material identity for change detection

  ~GLGeometryCache() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
    if (baseColorTex) glDeleteTextures(1, &baseColorTex);
    if (normalMapTex) glDeleteTextures(1, &normalMapTex);
    if (metallicRoughnessTex) glDeleteTextures(1, &metallicRoughnessTex);
    if (emissiveMapTex) glDeleteTextures(1, &emissiveMapTex);
    if (occlusionMapTex) glDeleteTextures(1, &occlusionMapTex);
  }

  void build(const SceneObject *obj) {
    const auto &verts = obj->getVertices();
    const auto &norms = obj->getNormals();
    const auto &texCoords = obj->getTexCoords();
    const auto &prims = obj->getPrimitives();

    if (verts.empty()) return;

    // Collect triangles with per-corner data
    // Each corner: pos(3) + normal(3) + uv(2) = 8 floats
    struct Vertex { float px, py, pz, nx, ny, nz, u, v; };
    std::vector<Vertex> vertexData;
    std::vector<unsigned int> indices;

    for (const auto *prim : prims) {
      auto addTri = [&](int va, int vb, int vc, int na, int nb, int nc,
                        int ta, int tb, int tc) {
        int baseIdx = (int)vertexData.size();
        auto emitVert = [&](int vi, int ni, int ti) {
          Vertex v;
          v.px = verts[vi][0]; v.py = verts[vi][1]; v.pz = verts[vi][2];
          if (ni >= 0 && ni < (int)norms.size()) {
            v.nx = norms[ni][0]; v.ny = norms[ni][1]; v.nz = norms[ni][2];
          } else {
            const auto &a = verts[va], &b = verts[vb], &c = verts[vc];
            Vec e1 = b - a, e2 = c - a;
            Vec n = cross(e1, e2);
            float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
            if (len > 1e-8f) { n[0] /= len; n[1] /= len; n[2] /= len; }
            v.nx = n[0]; v.ny = n[1]; v.nz = n[2];
          }
          if (ti >= 0 && ti < (int)texCoords.size()) {
            v.u = texCoords[ti].x; v.v = texCoords[ti].y;
          } else {
            v.u = 0; v.v = 0;
          }
          vertexData.push_back(v);
        };
        emitVert(va, na, ta);
        emitVert(vb, nb, tb);
        emitVert(vc, nc, tc);
        indices.push_back(baseIdx);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
      };

      switch (prim->type) {
        case Primitive::triangle: {
          const auto *tp = dynamic_cast<const TrianglePrimitive*>(prim);
          if (!tp) break;
          addTri(tp->i(0), tp->i(1), tp->i(2),
                 tp->i(3), tp->i(4), tp->i(5),
                 tp->i(6), tp->i(7), tp->i(8));
          break;
        }
        case Primitive::quad:
        case Primitive::texture: {
          const auto *qp = dynamic_cast<const QuadPrimitive*>(prim);
          if (!qp) break;
          addTri(qp->i(0), qp->i(1), qp->i(2),
                 qp->i(4), qp->i(5), qp->i(6),
                 qp->i(8), qp->i(9), qp->i(10));
          addTri(qp->i(0), qp->i(2), qp->i(3),
                 qp->i(4), qp->i(6), qp->i(7),
                 qp->i(8), qp->i(10), qp->i(11));
          break;
        }
        case Primitive::polygon: {
          const auto *pp = dynamic_cast<const PolygonPrimitive*>(prim);
          if (!pp || pp->getNumPoints() < 3) break;
          int n = pp->getNumPoints();
          for (int j = 1; j < n - 1; j++) {
            addTri(pp->getVertexIndex(0), pp->getVertexIndex(j), pp->getVertexIndex(j+1),
                   pp->hasNormals() ? pp->getNormalIndex(0) : -1,
                   pp->hasNormals() ? pp->getNormalIndex(j) : -1,
                   pp->hasNormals() ? pp->getNormalIndex(j+1) : -1,
                   -1, -1, -1);
          }
          break;
        }
        default: break;
      }
    }

    if (indices.empty()) return;

    numIndices = (int)indices.size();

    // Create VAO
    if (!vao) glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create VBO
    if (!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex),
                 vertexData.data(), GL_STATIC_DRAW);

    // Create EBO
    if (!ebo) glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
  }

  static GLuint uploadTexture(const core::Image &img) {
    if (img.isNull()) return 0;
    const auto &img8u = img.as<icl8u>();
    int w = img8u.getWidth(), h = img8u.getHeight(), ch = img8u.getChannels();

    std::vector<icl8u> rgba(w * h * 4);
    for (int i = 0; i < w * h; i++) {
      rgba[i*4+0] = (ch > 0) ? img8u.getData(0)[i] : 0;
      rgba[i*4+1] = (ch > 1) ? img8u.getData(1)[i] : 0;
      rgba[i*4+2] = (ch > 2) ? img8u.getData(2)[i] : 0;
      rgba[i*4+3] = (ch > 3) ? img8u.getData(3)[i] : 255;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
  }

  void clearTextures() {
    if (baseColorTex) { glDeleteTextures(1, &baseColorTex); baseColorTex = 0; }
    if (normalMapTex) { glDeleteTextures(1, &normalMapTex); normalMapTex = 0; }
    if (metallicRoughnessTex) { glDeleteTextures(1, &metallicRoughnessTex); metallicRoughnessTex = 0; }
    if (emissiveMapTex) { glDeleteTextures(1, &emissiveMapTex); emissiveMapTex = 0; }
    if (occlusionMapTex) { glDeleteTextures(1, &occlusionMapTex); occlusionMapTex = 0; }
  }

  void uploadMaterialTextures(const std::shared_ptr<Material> &mat) {
    if (!mat) { clearTextures(); lastMaterial = nullptr; return; }

    // Detect material change — clear old textures and re-upload
    if (mat.get() != lastMaterial) {
      clearTextures();
      lastMaterial = mat.get();
    }

    if (!baseColorTex && !mat->baseColorMap.isNull())
      baseColorTex = uploadTexture(mat->baseColorMap);
    if (!normalMapTex && !mat->normalMap.isNull())
      normalMapTex = uploadTexture(mat->normalMap);
    if (!metallicRoughnessTex && !mat->metallicRoughnessMap.isNull())
      metallicRoughnessTex = uploadTexture(mat->metallicRoughnessMap);
    if (!emissiveMapTex && !mat->emissiveMap.isNull())
      emissiveMapTex = uploadTexture(mat->emissiveMap);
    if (!occlusionMapTex && !mat->occlusionMap.isNull())
      occlusionMapTex = uploadTexture(mat->occlusionMap);
  }
};

// ---- Shader compilation helpers ----

static GLuint compileShader(GLenum type, const char *src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, sizeof(log), nullptr, log);
    fprintf(stderr, "[GLRenderer] Shader compile error:\n%s\n", log);
    glDeleteShader(s);
    return 0;
  }
  return s;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  GLint ok = 0;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
    fprintf(stderr, "[GLRenderer] Program link error:\n%s\n", log);
    glDeleteProgram(prog);
    return 0;
  }
  return prog;
}

// ---- GLRenderer::Data ----

struct GLRenderer::Data {
  GLuint program = 0;
  GLuint shadowProgram = 0;
  GLuint skyProgram = 0;
  GLuint skyVAO = 0, skyVBO = 0;
  GLint skyLocInvVP = -1;
  GLint skyLocMode = -1;
  GLint skyLocZenith = -1, skyLocHorizon = -1, skyLocGround = -1;
  GLint skyLocSharpness = -1, skyLocSunDir = -1;
  // Environment uniforms in main shader (for sampleSky function)
  GLint locEnvZenith = -1, locEnvHorizon = -1, locEnvGround = -1, locEnvSharpness = -1;
  bool shaderReady = false;
  float exposure = 1.0f;
  float ambient = 0.1f;
  float envMultiplier = 1.5f;
  float directMultiplier = 1.0f;
  int debugMode = 0;
  std::unordered_map<const SceneObject*, std::unique_ptr<GLGeometryCache>> geometryCache;

  // SSR ping-pong buffers (previous-frame reflections)
  GLuint ssrFBO[2] = {};
  GLuint ssrColorTex[2] = {};
  GLuint ssrDepthTex[2] = {};
  int ssrCurrentIdx = 0;
  int ssrWidth = 0, ssrHeight = 0;
  bool ssrFirstFrame = true;
  bool ssrEnabled = true;
  Mat prevViewMatrix, prevProjectionMatrix;

  // Projection crop matrix for zoom (identity = no crop)
  Mat projCropMatrix = Mat::id();
  bool hasProjCrop = false;

  // Overlay mode
  bool hideSky = false;
  float overlayAlpha = 1.0f;
  GLuint blitProgram = 0;
  GLint blitLocTex = -1, blitLocAlpha = -1;

  // Shadow maps (up to 4 shadow-casting lights)
  static const int MAX_SHADOWS = 4;
  GLuint shadowFBO[4] = {};
  GLuint shadowTex[4] = {};
  int numShadowMaps = 0;
  int shadowMapSize = 2048;
  GLint shadowLocModelMatrix = -1;
  GLint shadowLocLightVP = -1;

  // Uniform locations (cached after shader compilation)
  GLint locModelMatrix = -1;
  GLint locViewMatrix = -1;
  GLint locProjectionMatrix = -1;
  GLint locBaseColor = -1;
  GLint locMetallic = -1;
  GLint locRoughness = -1;
  GLint locEmissive = -1;
  GLint locAmbient = -1;
  GLint locExposure = -1;
  GLint locEnvMul = -1;
  GLint locDirectMul = -1;
  GLint locCameraPos = -1;
  GLint locHasBaseColorMap = -1;
  GLint locBaseColorMap = -1;
  GLint locHasNormalMap = -1;
  GLint locNormalMap = -1;
  GLint locHasMetallicRoughnessMap = -1;
  GLint locMetallicRoughnessMap = -1;
  GLint locHasEmissiveMap = -1;
  GLint locEmissiveMap = -1;
  GLint locHasOcclusionMap = -1;
  GLint locOcclusionMap = -1;
  // SSR uniforms
  GLint locSSREnabled = -1;
  GLint locPrevColorMap = -1;
  GLint locPrevDepthMap = -1;
  GLint locPrevVP = -1;
  GLint locScreenSize = -1;
  GLint locDebugMode = -1;
  GLint locTransmission = -1;
  GLint locIOR = -1;
  GLint locAttenuationColor = -1;
  GLint locLightShadowSlot[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
  GLint locShadowMatrix[4] = {-1,-1,-1,-1};
  GLint locShadowMap[4] = {-1,-1,-1,-1};  // uShadowMap0..3
  GLint locNumLights = -1;
  GLint locLightPos[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
  GLint locLightColor[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

  void cacheUniformLocations() {
    locModelMatrix = glGetUniformLocation(program, "uModelMatrix");
    locViewMatrix = glGetUniformLocation(program, "uViewMatrix");
    locProjectionMatrix = glGetUniformLocation(program, "uProjectionMatrix");
    locBaseColor = glGetUniformLocation(program, "uBaseColor");
    locMetallic = glGetUniformLocation(program, "uMetallic");
    locRoughness = glGetUniformLocation(program, "uRoughness");
    locEmissive = glGetUniformLocation(program, "uEmissive");
    locAmbient = glGetUniformLocation(program, "uAmbient");
    locExposure = glGetUniformLocation(program, "uExposure");
    locEnvMul = glGetUniformLocation(program, "uEnvMul");
    locDirectMul = glGetUniformLocation(program, "uDirectMul");
    locCameraPos = glGetUniformLocation(program, "uCameraPos");
    locHasBaseColorMap = glGetUniformLocation(program, "uHasBaseColorMap");
    locBaseColorMap = glGetUniformLocation(program, "uBaseColorMap");
    locHasNormalMap = glGetUniformLocation(program, "uHasNormalMap");
    locNormalMap = glGetUniformLocation(program, "uNormalMap");
    locHasMetallicRoughnessMap = glGetUniformLocation(program, "uHasMetallicRoughnessMap");
    locMetallicRoughnessMap = glGetUniformLocation(program, "uMetallicRoughnessMap");
    locHasEmissiveMap = glGetUniformLocation(program, "uHasEmissiveMap");
    locEmissiveMap = glGetUniformLocation(program, "uEmissiveMap");
    locHasOcclusionMap = glGetUniformLocation(program, "uHasOcclusionMap");
    locOcclusionMap = glGetUniformLocation(program, "uOcclusionMap");
    locSSREnabled = glGetUniformLocation(program, "uSSREnabled");
    locPrevColorMap = glGetUniformLocation(program, "uPrevColorMap");
    locPrevDepthMap = glGetUniformLocation(program, "uPrevDepthMap");
    locPrevVP = glGetUniformLocation(program, "uPrevVP");
    locScreenSize = glGetUniformLocation(program, "uScreenSize");
    locEnvZenith = glGetUniformLocation(program, "uEnvZenith");
    locEnvHorizon = glGetUniformLocation(program, "uEnvHorizon");
    locEnvGround = glGetUniformLocation(program, "uEnvGround");
    locEnvSharpness = glGetUniformLocation(program, "uEnvSharpness");
    locDebugMode = glGetUniformLocation(program, "uDebugMode");
    locTransmission = glGetUniformLocation(program, "uTransmission");
    locIOR = glGetUniformLocation(program, "uIOR");
    locAttenuationColor = glGetUniformLocation(program, "uAttenuationColor");
    for (int i = 0; i < 8; i++) {
      char name[48];
      snprintf(name, sizeof(name), "uLightShadowSlot[%d]", i);
      locLightShadowSlot[i] = glGetUniformLocation(program, name);
    }
    for (int i = 0; i < 4; i++) {
      char name[48];
      snprintf(name, sizeof(name), "uShadowMatrix[%d]", i);
      locShadowMatrix[i] = glGetUniformLocation(program, name);
    }
    locShadowMap[0] = glGetUniformLocation(program, "uShadowMap0");
    locShadowMap[1] = glGetUniformLocation(program, "uShadowMap1");
    locShadowMap[2] = glGetUniformLocation(program, "uShadowMap2");
    locShadowMap[3] = glGetUniformLocation(program, "uShadowMap3");
    locNumLights = glGetUniformLocation(program, "uNumLights");
    for (int i = 0; i < 8; i++) {
      char name[32];
      snprintf(name, sizeof(name), "uLightPos[%d]", i);
      locLightPos[i] = glGetUniformLocation(program, name);
      snprintf(name, sizeof(name), "uLightColor[%d]", i);
      locLightColor[i] = glGetUniformLocation(program, name);
    }
    fprintf(stderr, "[GLRenderer] Uniforms: model=%d view=%d proj=%d metallic=%d roughness=%d cameraPos=%d\n",
            locModelMatrix, locViewMatrix, locProjectionMatrix,
            locMetallic, locRoughness, locCameraPos);
  }

  void setUniformMat4(GLint loc, const Mat &m) {
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_TRUE, m.data());
  }

  void ensureSSRBuffers(int w, int h) {
    if (ssrWidth == w && ssrHeight == h && ssrFBO[0] != 0) return;
    for (int i = 0; i < 2; i++) {
      if (ssrFBO[i]) glDeleteFramebuffers(1, &ssrFBO[i]);
      if (ssrColorTex[i]) glDeleteTextures(1, &ssrColorTex[i]);
      if (ssrDepthTex[i]) glDeleteTextures(1, &ssrDepthTex[i]);
    }
    ssrWidth = w; ssrHeight = h; ssrFirstFrame = true;
    for (int i = 0; i < 2; i++) {
      glGenTextures(1, &ssrColorTex[i]);
      glBindTexture(GL_TEXTURE_2D, ssrColorTex[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glGenTextures(1, &ssrDepthTex[i]);
      glBindTexture(GL_TEXTURE_2D, ssrDepthTex[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

      glGenFramebuffers(1, &ssrFBO[i]);
      glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrColorTex[i], 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ssrDepthTex[i], 0);
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "[GLRenderer] SSR FBO %d incomplete\n", i);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
};

// ---- Implementation ----

GLRenderer::GLRenderer() : m_data(new Data) {}

GLRenderer::~GLRenderer() {
  if (m_data->program) glDeleteProgram(m_data->program);
  if (m_data->shadowProgram) glDeleteProgram(m_data->shadowProgram);
  if (m_data->skyProgram) glDeleteProgram(m_data->skyProgram);
  if (m_data->skyVAO) glDeleteVertexArrays(1, &m_data->skyVAO);
  if (m_data->skyVBO) glDeleteBuffers(1, &m_data->skyVBO);
  for (int i = 0; i < 4; i++) {
    if (m_data->shadowFBO[i]) glDeleteFramebuffers(1, &m_data->shadowFBO[i]);
    if (m_data->shadowTex[i]) glDeleteTextures(1, &m_data->shadowTex[i]);
  }
  for (int i = 0; i < 2; i++) {
    if (m_data->ssrFBO[i]) glDeleteFramebuffers(1, &m_data->ssrFBO[i]);
    if (m_data->ssrColorTex[i]) glDeleteTextures(1, &m_data->ssrColorTex[i]);
    if (m_data->ssrDepthTex[i]) glDeleteTextures(1, &m_data->ssrDepthTex[i]);
  }
  delete m_data;
}

void GLRenderer::setExposure(float e) { m_data->exposure = e; }
void GLRenderer::setAmbient(float a) { m_data->ambient = a; }
void GLRenderer::setDebugMode(int mode) { m_data->debugMode = mode; }
void GLRenderer::setEnvMultiplier(float m) { m_data->envMultiplier = m; }
float GLRenderer::getEnvMultiplier() const { return m_data->envMultiplier; }
void GLRenderer::setDirectMultiplier(float m) { m_data->directMultiplier = m; }
float GLRenderer::getDirectMultiplier() const { return m_data->directMultiplier; }
void GLRenderer::setSSREnabled(bool enabled) { m_data->ssrEnabled = enabled; }
bool GLRenderer::getSSREnabled() const { return m_data->ssrEnabled; }
void GLRenderer::setHideSky(bool enabled) { m_data->hideSky = enabled; }
void GLRenderer::setOverlayAlpha(float alpha) { m_data->overlayAlpha = alpha; }

void GLRenderer::ensureShaderCompiled() {
  if (m_data->shaderReady) return;
  m_data->shaderReady = true;

  GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SHADER);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SHADER);
  if (vs && fs) {
    m_data->program = linkProgram(vs, fs);
  }
  if (vs) glDeleteShader(vs);
  if (fs) glDeleteShader(fs);

  if (m_data->program) {
    m_data->cacheUniformLocations();
    // Initialize shadow uniforms to safe defaults (all shadows disabled)
    glUseProgram(m_data->program);
    for (int i = 0; i < 8; i++)
      if (m_data->locLightShadowSlot[i] >= 0)
        glUniform1i(m_data->locLightShadowSlot[i], -1);
    for (int i = 0; i < 4; i++)
      if (m_data->locShadowMap[i] >= 0)
        glUniform1i(m_data->locShadowMap[i], 1 + i);  // bind to texture units 1-4
    glUseProgram(0);
    fprintf(stderr, "[GLRenderer] GL 4.1 Core shader compiled OK (shadowMap locs: %d %d %d %d)\n",
            m_data->locShadowMap[0], m_data->locShadowMap[1],
            m_data->locShadowMap[2], m_data->locShadowMap[3]);
  } else {
    fprintf(stderr, "[GLRenderer] SHADER COMPILATION FAILED\n");
  }

  // Sky shader + fullscreen quad
  {
    GLuint svs = compileShader(GL_VERTEX_SHADER, SKY_VERT);
    GLuint sfs = compileShader(GL_FRAGMENT_SHADER, SKY_FRAG);
    if (svs && sfs) {
      m_data->skyProgram = linkProgram(svs, sfs);
      m_data->skyLocInvVP = glGetUniformLocation(m_data->skyProgram, "uInvVP");
      m_data->skyLocMode = glGetUniformLocation(m_data->skyProgram, "uSkyMode");
      m_data->skyLocZenith = glGetUniformLocation(m_data->skyProgram, "uSkyZenith");
      m_data->skyLocHorizon = glGetUniformLocation(m_data->skyProgram, "uSkyHorizon");
      m_data->skyLocGround = glGetUniformLocation(m_data->skyProgram, "uSkyGround");
      m_data->skyLocSharpness = glGetUniformLocation(m_data->skyProgram, "uSkySharpness");
      m_data->skyLocSunDir = glGetUniformLocation(m_data->skyProgram, "uSkySunDir");
    }
    if (svs) glDeleteShader(svs);
    if (sfs) glDeleteShader(sfs);

    float quad[] = { -1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1 };
    glGenVertexArrays(1, &m_data->skyVAO);
    glBindVertexArray(m_data->skyVAO);
    glGenBuffers(1, &m_data->skyVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_data->skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);
  }

  // Shadow shader
  GLuint svs = compileShader(GL_VERTEX_SHADER, SHADOW_VERT);
  GLuint sfs = compileShader(GL_FRAGMENT_SHADER, SHADOW_FRAG);
  if (svs && sfs) {
    m_data->shadowProgram = linkProgram(svs, sfs);
    m_data->shadowLocModelMatrix = glGetUniformLocation(m_data->shadowProgram, "uModelMatrix");
    m_data->shadowLocLightVP = glGetUniformLocation(m_data->shadowProgram, "uLightVP");
  }
  if (svs) glDeleteShader(svs);
  if (sfs) glDeleteShader(sfs);

  // Blit shader (for overlay mode alpha-blended compositing)
  {
    static const char *BLIT_VERT = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
out vec2 vUV;
void main() {
    vUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";
    static const char *BLIT_FRAG = R"(
#version 410 core
uniform sampler2D uBlitTex;
uniform float uAlpha;
in vec2 vUV;
out vec4 FragColor;
void main() {
    vec4 c = texture(uBlitTex, vUV);
    FragColor = vec4(c.rgb, c.a * uAlpha);
}
)";
    GLuint bvs = compileShader(GL_VERTEX_SHADER, BLIT_VERT);
    GLuint bfs = compileShader(GL_FRAGMENT_SHADER, BLIT_FRAG);
    if (bvs && bfs) {
      m_data->blitProgram = linkProgram(bvs, bfs);
      m_data->blitLocTex = glGetUniformLocation(m_data->blitProgram, "uBlitTex");
      m_data->blitLocAlpha = glGetUniformLocation(m_data->blitProgram, "uAlpha");
    }
    if (bvs) glDeleteShader(bvs);
    if (bfs) glDeleteShader(bfs);
  }

  // Shadow FBOs + depth textures (create up to MAX_SHADOWS)
  int sz = m_data->shadowMapSize;
  float borderColor[] = {1, 1, 1, 1};
  for (int i = 0; i < Data::MAX_SHADOWS; i++) {
    glGenTextures(1, &m_data->shadowTex[i]);
    glBindTexture(GL_TEXTURE_2D, m_data->shadowTex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, sz, sz, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_data->shadowFBO[i]);
    glBindFramebuffer(GL_FRAMEBUFFER, m_data->shadowFBO[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_data->shadowTex[i], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "[GLRenderer] Shadow FBO %d incomplete: 0x%x\n", i, fbStatus);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  fprintf(stderr, "[GLRenderer] %d shadow maps (%dx%d) ready\n", Data::MAX_SHADOWS, sz, sz);
}

/// Render a single object in the shadow pass (depth only, recursive)
void GLRenderer::renderObjectShadow(const SceneObject *obj) {
  auto &cache = m_data->geometryCache[obj];
  if (!cache) {
    cache = std::make_unique<GLGeometryCache>();
    cache->build(obj);
    cache->uploadMaterialTextures(obj->getMaterial());
  }
  if (cache->numIndices == 0) goto children;

  {
    Mat modelMatrix = obj->getTransformation(true);
    glUniformMatrix4fv(m_data->shadowLocModelMatrix, 1, GL_TRUE, modelMatrix.data());
    glBindVertexArray(cache->vao);
    glDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

children:
  for (int c = 0; c < obj->getChildCount(); c++) {
    renderObjectShadow(obj->getChild(c));
  }
}

void GLRenderer::render(const Scene &scene, int camIndex) {
  ensureShaderCompiled();
  if (!m_data->program) return;

  // Compute letterbox viewport from camera aspect ratio
  GLint widgetVP[4];
  glGetIntegerv(GL_VIEWPORT, widgetVP);
  int ww = widgetVP[2], wh = widgetVP[3];

  const Size &chip = scene.getCamera(camIndex).getRenderParams().chipSize;
  float camAR = (float)chip.width / chip.height;
  float widgetAR = (float)ww / wh;
  int vpX = 0, vpY = 0, vpW = ww, vpH = wh;
  if (widgetAR > camAR) {
    vpW = (int)(wh * camAR);
    vpX = (ww - vpW) / 2;
  } else {
    vpH = (int)(ww / camAR);
    vpY = (wh - vpH) / 2;
  }

  renderWithViewport(scene, camIndex, vpX, vpY, vpW, vpH);
}

void GLRenderer::render(const Scene &scene, int camIndex,
                              qt::ICLDrawWidget3D *widget) {
  ensureShaderCompiled();
  if (!m_data->program) return;

  if (!widget) {
    render(scene, camIndex);
    return;
  }

  float dpr = widget->devicePixelRatioF();

  if (widget->getFitMode() == qt::ICLWidget::fmZoom) {
    // Crop matrix approach: render at native viewport, modify projection
    // to show only the zoomed sub-region. No oversized FBOs needed.
    Rect32f zr = widget->getZoomRect();
    float zw = std::max(zr.width, 1e-6f);
    float zh = std::max(zr.height, 1e-6f);
    float sx = 1.0f / zw;
    float sy = 1.0f / zh;
    float tx = (1.0f - 2.0f * zr.x - zw) / zw;
    float ty = (2.0f * zr.y + zh - 1.0f) / zh;
    m_data->projCropMatrix = Mat(sx, 0,  0, tx,
                                 0,  sy, 0, ty,
                                 0,  0,  1, 0,
                                 0,  0,  0, 1);
    m_data->hasProjCrop = true;

    // Use native viewport: letterboxed for the crop sub-region's AR
    Size imgSize = widget->getImageSize(true);
    float cropW_px = std::max(1.0f, imgSize.width * zr.width);
    float cropH_px = std::max(1.0f, imgSize.height * zr.height);
    float cropAR = cropW_px / cropH_px;
    Size ws = widget->getSize();
    float widgetAR = (float)ws.width / std::max(1, ws.height);
    int vpX, vpY, vpW, vpH;
    if (cropAR >= widgetAR) {
      float sf = (float)ws.width / cropW_px;
      vpW = (int)(ws.width * dpr);
      vpH = (int)(cropH_px * sf * dpr);
      vpX = 0;
      vpY = (int)((ws.height - cropH_px * sf) * 0.5f * dpr);
    } else {
      float sf = (float)ws.height / cropH_px;
      vpW = (int)(cropW_px * sf * dpr);
      vpH = (int)(ws.height * dpr);
      vpX = (int)((ws.width - cropW_px * sf) * 0.5f * dpr);
      vpY = 0;
    }
    renderWithViewport(scene, camIndex, vpX, vpY, vpW, vpH);
    m_data->hasProjCrop = false;
  } else {
    Rect imageRect = widget->getImageRect(true);
    int vpX = (int)(imageRect.x * dpr);
    int vpY = (int)(imageRect.y * dpr);
    int vpW = (int)(imageRect.width * dpr);
    int vpH = (int)(imageRect.height * dpr);
    renderWithViewport(scene, camIndex, vpX, vpY, vpW, vpH);
  }
}

void GLRenderer::renderWithViewport(const Scene &scene, int camIndex,
                                          int vpX, int vpY, int vpW, int vpH) {
  const Camera &cam = scene.getCamera(camIndex);
  Mat projGL = cam.getProjectionMatrixGL();
  if (m_data->hasProjCrop) {
    projGL = m_data->projCropMatrix * projGL;
  }
  Mat viewGL = cam.getCSTransformationMatrixGL();

  // Collect shadow-enabled lights (up to MAX_SHADOWS)
  struct ShadowInfo { int lightIdx; Mat lightVP; };
  ShadowInfo shadows[4];
  int numShadows = 0;
  // Also build a mapping: compacted light slot → shadow slot (-1 = none)
  int lightShadowSlot[8];
  // We need to build this after we know which lights are active (below),
  // but we need shadow maps rendered first. So collect shadow lights first,
  // then map after compacting active lights.
  int sceneLightToShadow[8];  // scene light index → shadow slot
  for (int i = 0; i < 8; i++) sceneLightToShadow[i] = -1;

  for (int i = 0; i < 8 && numShadows < Data::MAX_SHADOWS; i++) {
    const auto &light = scene.getLight(i);
    if (light.isOn() && light.getShadowEnabled() && light.getShadowCam()) {
      const Camera *sc = light.getShadowCam();
      shadows[numShadows].lightIdx = i;
      shadows[numShadows].lightVP = sc->getProjectionMatrixGL() * sc->getCSTransformationMatrixGL();
      sceneLightToShadow[i] = numShadows;
      numShadows++;
    }
  }

  // ---- Shadow passes (one per shadow-casting light) ----
  if (numShadows > 0 && m_data->shadowProgram) {
    GLint prevFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    glUseProgram(m_data->shadowProgram);
    int sz = m_data->shadowMapSize;

    for (int s = 0; s < numShadows; s++) {
      glBindFramebuffer(GL_FRAMEBUFFER, m_data->shadowFBO[s]);
      glViewport(0, 0, sz, sz);
      glClear(GL_DEPTH_BUFFER_BIT);
      glUniformMatrix4fv(m_data->shadowLocLightVP, 1, GL_TRUE, shadows[s].lightVP.data());

      for (int i = 0; i < scene.getObjectCount(); i++) {
        const SceneObject *obj = scene.getObject(i);
        if (!obj->isVisible()) continue;
        auto mat = obj->getMaterial();
        if (mat && mat->isTransmissive()) continue;  // glass doesn't cast opaque shadows
        renderObjectShadow(obj);
      }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
  }

  // ---- Main pass ----
  glViewport(vpX, vpY, vpW, vpH);

  // SSR: redirect rendering to FBO (previous frame's textures used for reflections)
  m_data->ensureSSRBuffers(vpW, vpH);
  int ssrWriteIdx = m_data->ssrCurrentIdx;
  int ssrReadIdx = 1 - ssrWriteIdx;
  GLint mainPassPrevFBO = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mainPassPrevFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_data->ssrFBO[ssrWriteIdx]);
  glViewport(0, 0, vpW, vpH);  // FBO has no letterboxing offset

  glDisable(GL_CULL_FACE);
  if (m_data->hideSky) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  } else {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Sky background (reads from Scene::getSky())
  // Resolve effective colors: Solid mode uses solidColor for all three
  const Sky &sky = scene.getSky();
  float si = sky.intensity * m_data->exposure;
  GeomColor effZenith, effHorizon, effGround;
  if (sky.mode == Sky::Solid) {
    effZenith = effHorizon = effGround = sky.solidColor;
  } else {
    effZenith = sky.zenithColor;
    effHorizon = sky.horizonColor;
    effGround = sky.groundColor;
  }

  if (m_data->skyProgram && !m_data->hideSky) {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_data->skyProgram);
    Mat vp = projGL * viewGL;
    Mat invVP = vp.inv();
    glUniformMatrix4fv(m_data->skyLocInvVP, 1, GL_TRUE, invVP.data());
    glUniform1i(m_data->skyLocMode, (int)sky.mode);
    glUniform3f(m_data->skyLocZenith, effZenith[0]*si, effZenith[1]*si, effZenith[2]*si);
    glUniform3f(m_data->skyLocHorizon, effHorizon[0]*si, effHorizon[1]*si, effHorizon[2]*si);
    glUniform3f(m_data->skyLocGround, effGround[0]*si, effGround[1]*si, effGround[2]*si);
    glUniform1f(m_data->skyLocSharpness, sky.horizonSharpness);
    glUniform3f(m_data->skyLocSunDir, sky.sunDirection[0], sky.sunDirection[1], sky.sunDirection[2]);
    glBindVertexArray(m_data->skyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
  }

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glUseProgram(m_data->program);

  m_data->setUniformMat4(m_data->locProjectionMatrix, projGL);
  m_data->setUniformMat4(m_data->locViewMatrix, viewGL);
  glUniform1f(m_data->locAmbient, m_data->ambient);
  glUniform1f(m_data->locExposure, m_data->exposure);
  glUniform1f(m_data->locEnvMul, m_data->envMultiplier);
  glUniform1f(m_data->locDirectMul, m_data->directMultiplier);
  glUniform1i(m_data->locDebugMode, m_data->debugMode);

  // SSR uniforms: bind previous frame's color+depth for screen-space reflections
  bool ssrActive = m_data->ssrEnabled && !m_data->ssrFirstFrame;
  glUniform1i(m_data->locSSREnabled, ssrActive ? 1 : 0);
  glUniform2f(m_data->locScreenSize, (float)vpW, (float)vpH);
  if (ssrActive) {
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, m_data->ssrColorTex[ssrReadIdx]);
    glUniform1i(m_data->locPrevColorMap, 9);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, m_data->ssrDepthTex[ssrReadIdx]);
    glUniform1i(m_data->locPrevDepthMap, 10);
    Mat prevVP = m_data->prevProjectionMatrix * m_data->prevViewMatrix;
    m_data->setUniformMat4(m_data->locPrevVP, prevVP);
  }

  // Environment uniforms for sampleSky() in fragment shader.
  // These must NOT include exposure — the fragment shader applies uExposure
  // once at the end to all contributions (env + direct + emissive) uniformly.
  float ei = sky.intensity;
  glUniform3f(m_data->locEnvZenith, effZenith[0]*ei, effZenith[1]*ei, effZenith[2]*ei);
  glUniform3f(m_data->locEnvHorizon, effHorizon[0]*ei, effHorizon[1]*ei, effHorizon[2]*ei);
  glUniform3f(m_data->locEnvGround, effGround[0]*ei, effGround[1]*ei, effGround[2]*ei);
  glUniform1f(m_data->locEnvSharpness, sky.horizonSharpness);

  // Camera position in world space (for specular)
  Vec camPos = cam.getPosition();
  glUniform3f(m_data->locCameraPos, camPos[0], camPos[1], camPos[2]);

  // Bind shadow maps on texture units 1..4
  for (int s = 0; s < numShadows; s++) {
    glActiveTexture(GL_TEXTURE1 + s);
    glBindTexture(GL_TEXTURE_2D, m_data->shadowTex[s]);
    glUniform1i(m_data->locShadowMap[s], 1 + s);
    m_data->setUniformMat4(m_data->locShadowMatrix[s], shadows[s].lightVP);
  }

  // Set all active lights from Scene (world-space positions and colors)
  int numLights = 0;
  static bool lightsPrinted = false;
  for (int i = 0; i < 8; i++) {
    const auto &light = scene.getLight(i);
    if (!light.isOn()) continue;
    if (numLights >= 8) break;

    Vec wp = light.getPosition();
    auto d = light.getDiffuse();
    glUniform4f(m_data->locLightPos[numLights], wp[0], wp[1], wp[2], 1.0f);
    // Normalize legacy 0-255 light colors to [0,1] (matches Cycles convention
    // and keeps direct lighting in a physically reasonable range).
    glUniform3f(m_data->locLightColor[numLights], d[0]/255.0f, d[1]/255.0f, d[2]/255.0f);

    // Map this compacted light slot to its shadow slot (-1 if no shadow)
    glUniform1i(m_data->locLightShadowSlot[numLights], sceneLightToShadow[i]);

    if (!lightsPrinted) {
      fprintf(stderr, "[GLRenderer] Light %d→slot %d: shadow=%d pos=(%.0f,%.0f,%.0f) color=(%.2f,%.2f,%.2f)\n",
              i, numLights, sceneLightToShadow[i], wp[0], wp[1], wp[2], d[0], d[1], d[2]);
    }
    numLights++;
  }
  if (!lightsPrinted && numLights > 0) {
    fprintf(stderr, "[GLRenderer] %d lights, %d shadow maps\n", numLights, numShadows);
    lightsPrinted = true;
  }
  glUniform1i(m_data->locNumLights, numLights);

  // Pass 1: opaque objects
  for (int i = 0; i < scene.getObjectCount(); i++) {
    const SceneObject *obj = scene.getObject(i);
    if (!obj->isVisible()) continue;
    auto mat = obj->getMaterial();
    if (mat && mat->isTransmissive()) continue;
    renderObject(obj, viewGL);
  }

  // Pass 2: transparent objects (back-to-front sorted)
  {
    struct TransObj { const SceneObject *obj; float dist; };
    std::vector<TransObj> transparents;
    Vec camPos = cam.getPosition();
    for (int i = 0; i < scene.getObjectCount(); i++) {
      const SceneObject *obj = scene.getObject(i);
      if (!obj->isVisible()) continue;
      auto mat = obj->getMaterial();
      if (!mat || !mat->isTransmissive()) continue;
      Mat T = obj->getTransformation(true);
      float dx = T(3,0) - camPos[0], dy = T(3,1) - camPos[1], dz = T(3,2) - camPos[2];
      transparents.push_back({obj, dx*dx + dy*dy + dz*dz});
    }
    std::sort(transparents.begin(), transparents.end(),
              [](const TransObj &a, const TransObj &b) { return a.dist > b.dist; });

    if (!transparents.empty()) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
      for (auto &t : transparents) {
        renderObject(t.obj, viewGL);
      }
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
    }
  }

  for (int s = 0; s < numShadows; s++) {
    glActiveTexture(GL_TEXTURE1 + s);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (ssrActive) {
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glActiveTexture(GL_TEXTURE0);
  glUseProgram(0);

  // Blit from SSR FBO to the original framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, mainPassPrevFBO);
  glViewport(vpX, vpY, vpW, vpH);

  if (m_data->hideSky && m_data->blitProgram) {
    // Alpha-blended compositing: draw textured quad from SSR color texture
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_data->blitProgram);
    glUniform1i(m_data->blitLocTex, 0);
    glUniform1f(m_data->blitLocAlpha, m_data->overlayAlpha);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_data->ssrColorTex[ssrWriteIdx]);

    glBindVertexArray(m_data->skyVAO);  // reuse fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
  } else {
    // Standard blit (no blending)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_data->ssrFBO[ssrWriteIdx]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainPassPrevFBO);
    glBlitFramebuffer(0, 0, vpW, vpH,
                      vpX, vpY, vpX + vpW, vpY + vpH,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, mainPassPrevFBO);
  }

  // Store current matrices for next frame's SSR reprojection
  m_data->prevViewMatrix = viewGL;
  m_data->prevProjectionMatrix = projGL;
  m_data->ssrCurrentIdx = 1 - m_data->ssrCurrentIdx;
  m_data->ssrFirstFrame = false;
}

void GLRenderer::renderObject(const SceneObject *obj,
                                    const FixedMatrix<float,4,4> &viewMatrix) {
  auto &cache = m_data->geometryCache[obj];
  if (!cache) {
    cache = std::make_unique<GLGeometryCache>();
    cache->build(obj);
  }
  // Re-upload textures when material changes (e.g. material preset switching)
  cache->uploadMaterialTextures(obj->getMaterial());

  if (cache->numIndices == 0) return;

  Mat modelMatrix = obj->getTransformation(true);
  m_data->setUniformMat4(m_data->locModelMatrix, modelMatrix);

  auto mat = obj->getMaterial();
  if (mat) {
    glUniform4f(m_data->locBaseColor, mat->baseColor[0], mat->baseColor[1],
                mat->baseColor[2], mat->baseColor[3]);
    glUniform1f(m_data->locMetallic, mat->metallic);
    glUniform1f(m_data->locRoughness, mat->roughness);
    glUniform4f(m_data->locEmissive, mat->emissive[0], mat->emissive[1],
                mat->emissive[2], 0);
    glUniform1f(m_data->locTransmission, mat->transmission);
    glUniform1f(m_data->locIOR, mat->ior);
    glUniform3f(m_data->locAttenuationColor,
                mat->attenuationColor[0], mat->attenuationColor[1], mat->attenuationColor[2]);
  } else {
    glUniform4f(m_data->locBaseColor, 0.8f, 0.8f, 0.8f, 1.0f);
    glUniform1f(m_data->locMetallic, 0.0f);
    glUniform1f(m_data->locRoughness, 0.5f);
    glUniform4f(m_data->locEmissive, 0, 0, 0, 0);
    glUniform1f(m_data->locTransmission, 0.0f);
    glUniform1f(m_data->locIOR, 1.5f);
    glUniform3f(m_data->locAttenuationColor, 1.0f, 1.0f, 1.0f);
  }

  // Texture binding (units 0, 5, 6 — units 1-4 reserved for shadow maps)
  glUniform1i(m_data->locHasBaseColorMap, cache->baseColorTex ? 1 : 0);
  glUniform1i(m_data->locBaseColorMap, 0);
  if (cache->baseColorTex) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cache->baseColorTex);
  }

  glUniform1i(m_data->locHasNormalMap, cache->normalMapTex ? 1 : 0);
  glUniform1i(m_data->locNormalMap, 5);
  if (cache->normalMapTex) {
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, cache->normalMapTex);
  }

  glUniform1i(m_data->locHasMetallicRoughnessMap, cache->metallicRoughnessTex ? 1 : 0);
  glUniform1i(m_data->locMetallicRoughnessMap, 6);
  if (cache->metallicRoughnessTex) {
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, cache->metallicRoughnessTex);
  }

  glUniform1i(m_data->locHasEmissiveMap, cache->emissiveMapTex ? 1 : 0);
  glUniform1i(m_data->locEmissiveMap, 7);
  if (cache->emissiveMapTex) {
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, cache->emissiveMapTex);
  }

  glUniform1i(m_data->locHasOcclusionMap, cache->occlusionMapTex ? 1 : 0);
  glUniform1i(m_data->locOcclusionMap, 8);
  if (cache->occlusionMapTex) {
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, cache->occlusionMapTex);
  }

  glBindVertexArray(cache->vao);
  glDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  if (cache->baseColorTex) { glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0); }
  if (cache->normalMapTex) { glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0); }
  if (cache->metallicRoughnessTex) { glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0); }
  if (cache->emissiveMapTex) { glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, 0); }
  if (cache->occlusionMapTex) { glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, 0); }

  for (int c = 0; c < obj->getChildCount(); c++) {
    renderObject(obj->getChild(c), viewMatrix);
  }
}

// GLImageRenderer moved to ICLQt/GLImageRenderer.h/.cpp

Image GLRenderer::renderToImage(const Scene &scene, int camIndex, int width, int height) {
  ensureShaderCompiled();
  if (!m_data->program) return Image();

  // Create temporary FBO with color + depth attachments
  GLuint fbo, colorTex, depthRB;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &colorTex);
  glBindTexture(GL_TEXTURE_2D, colorTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

  glGenRenderbuffers(1, &depthRB);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "[GLRenderer] renderToImage: FBO incomplete\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &colorTex);
    glDeleteRenderbuffers(1, &depthRB);
    return Image();
  }

  // Set viewport and render
  glViewport(0, 0, width, height);
  render(scene, camIndex);

  // Read back pixels
  std::vector<icl8u> pixels(width * height * 4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  // Convert to ICL Image (planar, flip Y since GL reads bottom-up)
  Img8u result(Size(width, height), 3);
  for (int y = 0; y < height; y++) {
    int srcY = height - 1 - y;
    for (int x = 0; x < width; x++) {
      int idx = (srcY * width + x) * 4;
      result(x, y, 0) = pixels[idx + 0];
      result(x, y, 1) = pixels[idx + 1];
      result(x, y, 2) = pixels[idx + 2];
    }
  }

  // Cleanup
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &colorTex);
  glDeleteRenderbuffers(1, &depthRB);

  return Image(result);
}

} // namespace icl::geom
