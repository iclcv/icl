// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Renderer.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/TextNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

#ifdef ICL_HAVE_OPENGL
#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#endif

#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdio>

namespace icl::geom2 {

  // ---- Shaders ----

  static const char *PBR_VERT = R"(
#version 410 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;
void main() {
    vec4 wp = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos = wp.xyz;
    vNormal = mat3(uModelMatrix) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProjectionMatrix * uViewMatrix * wp;
}
)";

  static const char *PBR_FRAG = R"(
#version 410 core
#define MAX_LIGHTS 8
uniform vec4 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uReflectivity;
uniform vec4 uEmissive;
uniform float uAmbient;
uniform float uExposure;
uniform float uOverlayAlpha;
uniform vec3 uCameraPos;
uniform int uNumLights;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec3 uLightColor[MAX_LIGHTS];

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

// Screen-space reflections
uniform int uSSREnabled;
uniform sampler2D uPrevColorMap;
uniform sampler2D uPrevDepthMap;
uniform mat4 uPrevVP;
uniform mat4 uPrevView;
uniform mat4 uPrevProjection;
uniform mat4 uPrevInvProjection;
uniform vec2 uScreenSize;

uniform int uDebugMode;
uniform int uUnlit;

// Per-light shadow: slot index into shadow map array (-1 = no shadow)
uniform int uLightShadowSlot[MAX_LIGHTS];
uniform mat4 uShadowMatrix[4];
uniform sampler2DShadow uShadowMap0;
uniform sampler2DShadow uShadowMap1;
uniform sampler2DShadow uShadowMap2;
uniform sampler2DShadow uShadowMap3;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 FragColor;

float sampleShadow(int slot, vec3 coord) {
    if (slot == 0) return texture(uShadowMap0, coord);
    if (slot == 1) return texture(uShadowMap1, coord);
    if (slot == 2) return texture(uShadowMap2, coord);
    return texture(uShadowMap3, coord);
}

// Approximate sky sampling for environment reflections (matches Sky defaults)
vec3 sampleSky(vec3 dir) {
    vec3 zenith  = vec3(0.55, 0.65, 0.85);
    vec3 horizon = vec3(0.95, 0.93, 0.90);
    vec3 ground  = vec3(0.30, 0.27, 0.25);
    float y = dir.y;
    if (y > 0.0) {
        return mix(horizon, zenith, pow(y, 0.4));
    } else {
        return mix(horizon, ground, min(-y * 3.0, 1.0));
    }
}

// Reconstruct view-space position from depth buffer value at a UV
vec3 viewPosFromDepth(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = uPrevInvProjection * clip;
    return viewPos.xyz / viewPos.w;
}

// Single SSR ray march with jitter offset (0..1 fraction of one step)
vec4 traceSSRSingle(vec3 viewPos, vec3 reflectView, float stepSize,
                     int maxSteps, float jitter) {
    vec3 dir = reflectView * stepSize;
    vec3 hitCoord = viewPos + dir * jitter; // jitter start

    for (int i = 0; i < maxSteps; i++) {
        hitCoord += dir;

        vec4 projCoord = uPrevProjection * vec4(hitCoord, 1.0);
        projCoord.xy /= projCoord.w;
        projCoord.xy = projCoord.xy * 0.5 + 0.5;

        if (projCoord.x < 0.0 || projCoord.x > 1.0 ||
            projCoord.y < 0.0 || projCoord.y > 1.0) break;

        float sceneDepth = texture(uPrevDepthMap, projCoord.xy).r;
        if (sceneDepth > 0.999) continue;

        vec3 sceneViewPos = viewPosFromDepth(projCoord.xy, sceneDepth);
        float dDepth = hitCoord.z - sceneViewPos.z;

        if (dDepth <= 0.0 && abs(dDepth) < length(dir) * 1.5) {
            // Binary search refinement (8 iterations)
            vec3 refDir = dir * 0.5;
            for (int j = 0; j < 8; j++) {
                hitCoord += (dDepth > 0.0 ? 1.0 : -1.0) * refDir;
                projCoord = uPrevProjection * vec4(hitCoord, 1.0);
                projCoord.xy /= projCoord.w;
                projCoord.xy = projCoord.xy * 0.5 + 0.5;
                sceneDepth = texture(uPrevDepthMap, projCoord.xy).r;
                sceneViewPos = viewPosFromDepth(projCoord.xy, sceneDepth);
                dDepth = hitCoord.z - sceneViewPos.z;
                refDir *= 0.5;
            }

            vec2 edgeDist = min(projCoord.xy, 1.0 - projCoord.xy);
            float confidence = smoothstep(0.0, 0.05, edgeDist.x)
                             * smoothstep(0.0, 0.05, edgeDist.y);
            confidence *= clamp(-reflectView.z, 0.0, 1.0);
            return vec4(texture(uPrevColorMap, projCoord.xy).rgb, confidence);
        }
    }
    return vec4(0.0);
}

// SSR: 4x supersampled view-space ray march.
// Traces 4 rays with stratified jitter offsets and averages the result,
// eliminating Moiré ring artifacts on curved surfaces.
vec4 traceSSR(vec3 worldPos, vec3 N, vec3 reflectDir, float roughness) {
    if (uSSREnabled == 0 || roughness > 0.7) return vec4(0.0);

    // Transform to previous frame's view space
    vec3 viewPos = (uPrevView * vec4(worldPos, 1.0)).xyz;
    vec3 viewNorm = normalize((uPrevView * vec4(N, 0.0)).xyz);
    vec3 viewDir = normalize(viewPos);
    vec3 reflectView = reflect(viewDir, viewNorm);

    float maxRayDist = -viewPos.z * 4.0;
    int maxSteps = 256;
    float stepSize = maxRayDist / float(maxSteps);

    // 4 stratified samples: offsets at 0, 0.25, 0.5, 0.75 of one step
    vec4 s0 = traceSSRSingle(viewPos, reflectView, stepSize, maxSteps, 0.0);
    vec4 s1 = traceSSRSingle(viewPos, reflectView, stepSize, maxSteps, 0.25);
    vec4 s2 = traceSSRSingle(viewPos, reflectView, stepSize, maxSteps, 0.5);
    vec4 s3 = traceSSRSingle(viewPos, reflectView, stepSize, maxSteps, 0.75);

    // Average: weight by confidence (alpha)
    float totalWeight = s0.a + s1.a + s2.a + s3.a;
    if (totalWeight < 0.001) return vec4(0.0);

    vec3 color = (s0.rgb * s0.a + s1.rgb * s1.a + s2.rgb * s2.a + s3.rgb * s3.a)
               / totalWeight;
    float confidence = totalWeight * 0.25;
    confidence *= 1.0 - smoothstep(0.3, 0.7, roughness);

    return vec4(color, confidence);
}

void main() {
    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing) N = -N;
    vec3 geomN = N;

    // Normal map: perturb N using tangent-space normal from texture
    if (uHasNormalMap != 0) {
        vec3 tsNormal = texture(uNormalMap, vTexCoord).rgb * 2.0 - 1.0;
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
    float selfOcclusion = max(dot(N, geomN), 0.0);

    vec3 V = normalize(uCameraPos - vWorldPos);

    // Base color
    vec4 baseCol = uBaseColor;
    if (uHasBaseColorMap != 0) {
        baseCol *= texture(uBaseColorMap, vTexCoord);
    }

    // Unlit mode: output baseColor directly (used for billboard text)
    if (uUnlit != 0) {
        if (baseCol.a < 0.01) discard;
        FragColor = vec4(baseCol.rgb, baseCol.a * uOverlayAlpha);
        return;
    }

    vec3 albedo = baseCol.rgb;
    float alpha = baseCol.a;

    // Metallic-roughness map (glTF: green=roughness, blue=metallic)
    float metallic = uMetallic;
    float roughness = uRoughness;
    if (uHasMetallicRoughnessMap != 0) {
        vec4 mr = texture(uMetallicRoughnessMap, vTexCoord);
        metallic = mr.b * uMetallic;
        roughness = mr.g * uRoughness;
    }

    float shininess = 2.0 / (roughness * roughness + 0.0001) - 2.0;
    shininess = clamp(shininess, 1.0, 512.0);

    vec3 specColor = mix(vec3(0.04), albedo, metallic);

    // Environment reflection
    vec3 R = reflect(-V, N);
    vec3 envColor = sampleSky(R);
    vec3 diffuseEnv = sampleSky(N);
    vec3 envReflection = mix(envColor, diffuseEnv, roughness);

    // SSR: blend with screen-space reflection where available
    // Skip SSR for non-reflective surfaces (saves 4 ray marches)
    vec4 ssrResult = vec4(0.0);
    if (uReflectivity > 0.01 || metallic > 0.5)
        ssrResult = traceSSR(vWorldPos, N, R, roughness);
    envReflection = mix(envReflection, ssrResult.rgb, ssrResult.a);

    // Debug modes: 0=shaded, 1=normals, 2=albedo, 3=UVs, 4=lighting only,
    //              5=NdotL, 6=SSR confidence, 7=depth buffer, 8=SSR only
    if (uDebugMode == 1) { FragColor = vec4(N * 0.5 + 0.5, 1.0); return; }
    if (uDebugMode == 2) { FragColor = vec4(albedo, 1.0); return; }
    if (uDebugMode == 3) { FragColor = vec4(vTexCoord, 0.0, 1.0); return; }
    if (uDebugMode == 4) {
        vec3 litOnly = vec3(uAmbient);
        for (int i = 0; i < uNumLights && i < MAX_LIGHTS; i++) {
            vec3 L2 = normalize(uLightPos[i] - vWorldPos);
            litOnly += uLightColor[i] * max(dot(N, L2), 0.0);
        }
        FragColor = vec4(clamp(litOnly * uExposure, 0.0, 1.0), 1.0); return;
    }
    if (uDebugMode == 5) {
        float maxNdL = 0.0;
        for (int i = 0; i < uNumLights && i < MAX_LIGHTS; i++) {
            vec3 L2 = normalize(uLightPos[i] - vWorldPos);
            maxNdL = max(maxNdL, dot(N, L2));
        }
        FragColor = vec4(vec3(maxNdL), 1.0); return;
    }
    if (uDebugMode == 6) {
        // SSR confidence: green=hit, red=sky fallback
        FragColor = vec4(1.0 - ssrResult.a, ssrResult.a, 0.0, 1.0); return;
    }
    if (uDebugMode == 7) {
        // Depth buffer: linearized for visualization
        float d = gl_FragCoord.z;
        float near = 0.1, far = 100000.0;
        float lin = (2.0 * near) / (far + near - d * (far - near));
        FragColor = vec4(vec3(lin), 1.0); return;
    }
    if (uDebugMode == 8) {
        // SSR only: shows reflected color where SSR hits, black elsewhere
        FragColor = vec4(ssrResult.rgb * ssrResult.a, 1.0); return;
    }

    // Fresnel: roughness-aware Schlick (Lagarde 2014)
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = pow(1.0 - NdotV, 5.0);
    vec3 envFresnel = specColor + (max(vec3(1.0 - roughness), specColor) - specColor) * fresnel;

    // Reflectivity: scales the specular reflection (Fresnel is the minimum)
    // At reflectivity=0, only Fresnel contributes; at 1.0, full mirror.
    vec3 reflFactor = max(envFresnel, vec3(uReflectivity));

    // Energy-conserving ambient
    vec3 kD = (vec3(1.0) - reflFactor) * (1.0 - metallic);
    vec3 ambientDiffuse = albedo * diffuseEnv * kD;
    vec3 ambientSpecular = envReflection * reflFactor;

    // Occlusion
    float occlusion = selfOcclusion;
    if (uHasOcclusionMap != 0) {
        occlusion *= texture(uOcclusionMap, vTexCoord).r;
    }

    vec3 color = (ambientDiffuse + ambientSpecular) * occlusion;

    for (int i = 0; i < uNumLights && i < MAX_LIGHTS; i++) {
        vec3 L = normalize(uLightPos[i] - vWorldPos);
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

        vec3 diffuse = albedo * (1.0 - metallic) * NdotL;
        float specPow = pow(NdotH, shininess);
        vec3 specular = specColor * specPow;

        color += shadow * uLightColor[i] * (diffuse + specular * NdotL);
    }

    // Fallback: if no lights, use a default directional light
    if (uNumLights == 0) {
        vec3 L = normalize(vec3(0.5, 1.0, 0.3));
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        color += albedo * NdotL * (1.0 - metallic);
        color += specColor * pow(NdotH, shininess) * NdotL;
    }

    // Emissive
    vec3 emissive = uEmissive.rgb;
    if (uHasEmissiveMap != 0) {
        emissive = texture(uEmissiveMap, vTexCoord).rgb * max(uEmissive.rgb, vec3(1.0));
    }
    color += emissive;

    // Tone mapping (linear, matches geom GLRenderer)
    color *= uExposure;
    color = clamp(color, 0.0, 1.0);

    alpha *= uOverlayAlpha;
    if (alpha < 0.01) discard;
    FragColor = vec4(color, alpha);
}
)";

  static const char *UNLIT_VERT = R"(
#version 410 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec4 aColor;
uniform mat4 uMVP;
uniform float uPointSize;
out vec4 vColor;
void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    gl_PointSize = uPointSize;
    vColor = aColor;
}
)";

  static const char *UNLIT_FRAG = R"(
#version 410 core
in vec4 vColor;
out vec4 FragColor;
void main() { FragColor = vColor; }
)";

  // ---- Blit shader (fullscreen quad for SSR composite) ----

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

  // ---- Geometry Cache ----

  struct GeomCache {
    GLuint triVao = 0, triVbo = 0, triEbo = 0;
    int numTriIndices = 0;

    GLuint lineVao = 0, lineVbo = 0;
    int numLineVerts = 0;

    GLuint pointVao = 0, pointVbo = 0;
    int numPointVerts = 0;

    ~GeomCache() {
      if (triVao) glDeleteVertexArrays(1, &triVao);
      if (triVbo) glDeleteBuffers(1, &triVbo);
      if (triEbo) glDeleteBuffers(1, &triEbo);
      if (lineVao) glDeleteVertexArrays(1, &lineVao);
      if (lineVbo) glDeleteBuffers(1, &lineVbo);
      if (pointVao) glDeleteVertexArrays(1, &pointVao);
      if (pointVbo) glDeleteBuffers(1, &pointVbo);
    }

    void build(const GeometryNode *node) {
      const auto &verts = node->getVertices();
      const auto &norms = node->getNormals();
      const auto &uvs = node->getTexCoords();
      if (verts.empty()) return;

      // Get material colors for lines/points
      auto mat = node->getMaterial();
      GeomColor defaultLineColor(1,1,1,1);
      GeomColor defaultPointColor(1,1,1,1);
      if (mat) {
        defaultLineColor = (mat->lineColor[3] > 0) ? mat->lineColor : mat->baseColor;
        defaultPointColor = (mat->pointColor[3] > 0) ? mat->pointColor : mat->baseColor;
      }

      // ---- Triangles ----
      struct Vertex { float px,py,pz, nx,ny,nz, u,v; };
      std::vector<Vertex> triData;
      std::vector<unsigned int> triIdx;

      auto emitTri = [&](int va, int vb, int vc, int na, int nb, int nc,
                         int ta, int tb, int tc) {
        int base = (int)triData.size();
        auto emitV = [&](int vi, int ni, int ti) {
          Vertex v;
          v.px = verts[vi][0]; v.py = verts[vi][1]; v.pz = verts[vi][2];
          if (ni >= 0 && ni < (int)norms.size()) {
            v.nx = norms[ni][0]; v.ny = norms[ni][1]; v.nz = norms[ni][2];
          } else {
            // auto-normal from triangle
            const auto &a = verts[va], &b = verts[vb], &c = verts[vc];
            float e1x = b[0]-a[0], e1y = b[1]-a[1], e1z = b[2]-a[2];
            float e2x = c[0]-a[0], e2y = c[1]-a[1], e2z = c[2]-a[2];
            v.nx = e1y*e2z - e1z*e2y;
            v.ny = e1z*e2x - e1x*e2z;
            v.nz = e1x*e2y - e1y*e2x;
            float len = std::sqrt(v.nx*v.nx + v.ny*v.ny + v.nz*v.nz);
            if (len > 1e-8f) { v.nx/=len; v.ny/=len; v.nz/=len; }
          }
          if (ti >= 0 && ti < (int)uvs.size()) {
            v.u = uvs[ti].x; v.v = uvs[ti].y;
          } else { v.u = 0; v.v = 0; }
          triData.push_back(v);
        };
        emitV(va, na, ta); emitV(vb, nb, tb); emitV(vc, nc, tc);
        triIdx.push_back(base); triIdx.push_back(base+1); triIdx.push_back(base+2);
      };

      if (node->isPrimitiveVisible(PrimTriangle)) {
        for (const auto &t : node->getTriangles()) {
          emitTri(t.v[0], t.v[1], t.v[2], t.n[0], t.n[1], t.n[2],
                  t.t[0], t.t[1], t.t[2]);
        }
      }
      if (node->isPrimitiveVisible(PrimQuad)) {
        for (const auto &q : node->getQuads()) {
          emitTri(q.v[0], q.v[1], q.v[2], q.n[0], q.n[1], q.n[2],
                  q.t[0], q.t[1], q.t[2]);
          emitTri(q.v[0], q.v[2], q.v[3], q.n[0], q.n[2], q.n[3],
                  q.t[0], q.t[2], q.t[3]);
        }
      }

      if (!triIdx.empty()) {
        numTriIndices = (int)triIdx.size();
        if (!triVao) glGenVertexArrays(1, &triVao);
        glBindVertexArray(triVao);
        if (!triVbo) glGenBuffers(1, &triVbo);
        glBindBuffer(GL_ARRAY_BUFFER, triVbo);
        glBufferData(GL_ARRAY_BUFFER, triData.size()*sizeof(Vertex), triData.data(), GL_STATIC_DRAW);
        if (!triEbo) glGenBuffers(1, &triEbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIdx.size()*sizeof(unsigned int), triIdx.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
        glBindVertexArray(0);
      }

      // ---- Lines ----
      struct ColorVert { float px,py,pz, r,g,b,a; };
      std::vector<ColorVert> lineData;

      if (node->isPrimitiveVisible(PrimLine)) {
        for (const auto &l : node->getLines()) {
          GeomColor c = (l.color[3] > 0.001f) ? l.color : defaultLineColor;
          for (int j = 0; j < 2; j++) {
            int vi = (j == 0) ? l.a : l.b;
            lineData.push_back({verts[vi][0], verts[vi][1], verts[vi][2],
                                c[0], c[1], c[2], c[3]});
          }
        }
      }

      if (!lineData.empty()) {
        numLineVerts = (int)lineData.size();
        if (!lineVao) glGenVertexArrays(1, &lineVao);
        glBindVertexArray(lineVao);
        if (!lineVbo) glGenBuffers(1, &lineVbo);
        glBindBuffer(GL_ARRAY_BUFFER, lineVbo);
        glBufferData(GL_ARRAY_BUFFER, lineData.size()*sizeof(ColorVert), lineData.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
        glBindVertexArray(0);
      }

      // ---- Points ----
      std::vector<ColorVert> pointData;
      const auto &vc = node->getVertexColors();

      if (node->isPrimitiveVisible(PrimVertex)) {
        for (int i = 0; i < (int)verts.size(); i++) {
          GeomColor c = (i < (int)vc.size() && vc[i][3] > 0.001f) ? vc[i] : defaultPointColor;
          pointData.push_back({verts[i][0], verts[i][1], verts[i][2],
                               c[0], c[1], c[2], c[3]});
        }
      }

      if (!pointData.empty()) {
        numPointVerts = (int)pointData.size();
        if (!pointVao) glGenVertexArrays(1, &pointVao);
        glBindVertexArray(pointVao);
        if (!pointVbo) glGenBuffers(1, &pointVbo);
        glBindBuffer(GL_ARRAY_BUFFER, pointVbo);
        glBufferData(GL_ARRAY_BUFFER, pointData.size()*sizeof(ColorVert), pointData.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
        glBindVertexArray(0);
      }
    }
  };

  // ---- Point Cloud Cache ----

  struct PCCache {
    GLuint vao = 0, vbo = 0;
    int numPoints = 0;

    ~PCCache() {
      if (vao) glDeleteVertexArrays(1, &vao);
      if (vbo) glDeleteBuffers(1, &vbo);
    }

    void upload(const PointCloud &cloud, const GeomColor &fallbackColor) {
      int n = cloud.getDim();
      if (n == 0) { numPoints = 0; return; }

      struct ColorVert { float px,py,pz, r,g,b,a; };
      std::vector<ColorVert> data(n);

      auto xyz = cloud.selectXYZ();
      bool hasColor = cloud.supports(PointCloud::RGBA32f);
      core::DataSegment<float,4> rgba = hasColor
        ? cloud.selectRGBA32f()
        : core::DataSegment<float,4>();

      for (int i = 0; i < n; i++) {
        auto &v = xyz[i];
        data[i].px = v[0]; data[i].py = v[1]; data[i].pz = v[2];
        if (hasColor) {
          auto &c = rgba[i];
          // Normalize: if colors are in [0,255] range, convert to [0,1]
          if (c[0] > 1.01f || c[1] > 1.01f || c[2] > 1.01f) {
            data[i].r = c[0]/255.f; data[i].g = c[1]/255.f;
            data[i].b = c[2]/255.f; data[i].a = c[3]/255.f;
          } else {
            data[i].r = c[0]; data[i].g = c[1];
            data[i].b = c[2]; data[i].a = c[3];
          }
        } else {
          data[i].r = fallbackColor[0]; data[i].g = fallbackColor[1];
          data[i].b = fallbackColor[2]; data[i].a = fallbackColor[3];
        }
      }

      if (!vao) glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      if (!vbo) glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, n * sizeof(ColorVert), data.data(), GL_DYNAMIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(3*sizeof(float)));
      glBindVertexArray(0);

      numPoints = n;
    }
  };

  // ---- Renderer Data ----

  struct LightInfo {
    float pos[3];
    float color[3];
    bool shadowEnabled = false;
    int sceneLightIdx = -1;   // index in traversal order
  };

  struct Renderer::Data {
    GLuint pbrProgram = 0;
    GLuint unlitProgram = 0;
    bool shaderReady = false;
    float exposure = 1.0f;
    float ambient = 0.15f;
    float overlayAlpha = 1.0f;
    Mat currentProjection;
    Mat currentView;

    // PBR uniform locations
    GLint locModel = -1, locView = -1, locProj = -1;
    GLint locBaseColor = -1, locMetallic = -1, locRoughness = -1, locReflectivity = -1;
    GLint locEmissive = -1, locAmbient = -1, locExposure = -1, locOverlayAlpha = -1;
    GLint locCameraPos = -1;
    GLint locNumLights = -1;
    GLint locLightPos[8] = {};
    GLint locLightColor[8] = {};

    // Texture uniform locations
    GLint locBaseColorMap = -1, locHasBaseColorMap = -1;
    GLint locNormalMap = -1, locHasNormalMap = -1;
    GLint locMetallicRoughnessMap = -1, locHasMetallicRoughnessMap = -1;
    GLint locEmissiveMap = -1, locHasEmissiveMap = -1;
    GLint locOcclusionMap = -1, locHasOcclusionMap = -1;

    // SSR uniform locations
    GLint locSSREnabled = -1, locPrevColorMap = -1, locPrevDepthMap = -1;
    GLint locPrevVP = -1, locScreenSize = -1;
    GLint locPrevView = -1, locPrevProjection = -1, locPrevInvProjection = -1;

    // Shadow uniforms in PBR shader
    GLint locLightShadowSlot[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
    GLint locShadowMatrix[4] = {-1,-1,-1,-1};
    GLint locShadowMap[4] = {-1,-1,-1,-1};

    // Shadow maps (up to 4 shadow-casting lights)
    static const int MAX_SHADOWS = 4;
    GLuint shadowFBO[4] = {};
    GLuint shadowTex[4] = {};
    int shadowMapSize = 2048;
    bool shadowsEnabled = true;

    // Shadow depth program
    GLuint shadowProgram = 0;
    GLint shadowLocModelMatrix = -1;
    GLint shadowLocLightVP = -1;

    // Debug mode / unlit
    int debugMode = 0;
    GLint locDebugMode = -1;
    GLint locUnlit = -1;

    // Unlit uniform locations
    GLint locUnlitMVP = -1, locUnlitPointSize = -1;

    // Blit shader
    GLuint blitProgram = 0;
    GLint blitLocTex = -1, blitLocAlpha = -1;

    // SSR ping-pong buffers
    GLuint ssrFBO[2] = {};
    GLuint ssrColorTex[2] = {};
    GLuint ssrDepthTex[2] = {};
    int ssrCurrentIdx = 0;
    int ssrWidth = 0, ssrHeight = 0;
    bool ssrFirstFrame = true;
    bool ssrEnabled = true;
    Mat prevView, prevProj;

    // Fullscreen quad VAO for blit
    GLuint quadVAO = 0, quadVBO = 0;

    // Per-material texture cache
    struct MatTextures {
      GLuint baseColor = 0, normalMap = 0, metallicRoughness = 0;
      GLuint emissive = 0, occlusion = 0;
    };
    std::unordered_map<const geom::Material*, MatTextures> texCache;

    std::unordered_map<const GeometryNode*, std::unique_ptr<GeomCache>> cache;
    std::unordered_map<const PointCloudNode*, std::unique_ptr<PCCache>> pcCache;

    // Lights collected during traversal
    std::vector<LightInfo> lights;

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        glGenFramebuffers(1, &ssrFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, ssrColorTex[i], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, ssrDepthTex[i], 0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ensureQuadVAO() {
      if (quadVAO) return;
      float quad[] = {-1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1};
      glGenVertexArrays(1, &quadVAO);
      glBindVertexArray(quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
      glBindVertexArray(0);
    }
  };

  // ---- Shader helpers ----

  static GLuint compileShader(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[1024];
      glGetShaderInfoLog(s, sizeof(log), nullptr, log);
      fprintf(stderr, "[geom2::Renderer] Shader error:\n%s\n", log);
      glDeleteShader(s);
      return 0;
    }
    return s;
  }

  static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
      char log[1024];
      glGetProgramInfoLog(p, sizeof(log), nullptr, log);
      fprintf(stderr, "[geom2::Renderer] Link error:\n%s\n", log);
      glDeleteProgram(p);
      return 0;
    }
    return p;
  }

  // ---- Renderer implementation ----

  Renderer::Renderer() : m_data(std::make_unique<Data>()) {}

  Renderer::~Renderer() {
    if (m_data->pbrProgram) glDeleteProgram(m_data->pbrProgram);
    if (m_data->unlitProgram) glDeleteProgram(m_data->unlitProgram);
    if (m_data->blitProgram) glDeleteProgram(m_data->blitProgram);
    if (m_data->shadowProgram) glDeleteProgram(m_data->shadowProgram);
    for (int i = 0; i < Data::MAX_SHADOWS; i++) {
      if (m_data->shadowFBO[i]) glDeleteFramebuffers(1, &m_data->shadowFBO[i]);
      if (m_data->shadowTex[i]) glDeleteTextures(1, &m_data->shadowTex[i]);
    }
    for (int i = 0; i < 2; i++) {
      if (m_data->ssrFBO[i]) glDeleteFramebuffers(1, &m_data->ssrFBO[i]);
      if (m_data->ssrColorTex[i]) glDeleteTextures(1, &m_data->ssrColorTex[i]);
      if (m_data->ssrDepthTex[i]) glDeleteTextures(1, &m_data->ssrDepthTex[i]);
    }
    if (m_data->quadVAO) glDeleteVertexArrays(1, &m_data->quadVAO);
    if (m_data->quadVBO) glDeleteBuffers(1, &m_data->quadVBO);
    // Prevent unique_ptr from double-deleting GL resources
    m_data->pbrProgram = 0;
    m_data->unlitProgram = 0;
    m_data->blitProgram = 0;
    m_data->shadowProgram = 0;
  }

  void Renderer::setExposure(float e) { m_data->exposure = e; }
  void Renderer::setAmbient(float a) { m_data->ambient = a; }
  void Renderer::setOverlayAlpha(float a) { m_data->overlayAlpha = a; }
  void Renderer::setSSREnabled(bool e) { m_data->ssrEnabled = e; }
  void Renderer::setShadowsEnabled(bool e) { m_data->shadowsEnabled = e; }
  void Renderer::setDebugMode(int mode) { m_data->debugMode = mode; }
  void Renderer::invalidateCache() {
    m_data->cache.clear();
    m_data->pcCache.clear();
    for (auto &[_, mt] : m_data->texCache) {
      GLuint texs[] = {mt.baseColor, mt.normalMap, mt.metallicRoughness, mt.emissive, mt.occlusion};
      for (auto t : texs) if (t) glDeleteTextures(1, &t);
    }
    m_data->texCache.clear();
  }

  void Renderer::ensureShaderCompiled() {
    if (m_data->shaderReady) return;
    m_data->shaderReady = true;

    // PBR shader
    GLuint vs = compileShader(GL_VERTEX_SHADER, PBR_VERT);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, PBR_FRAG);
    if (vs && fs) {
      m_data->pbrProgram = linkProgram(vs, fs);
      m_data->locModel = glGetUniformLocation(m_data->pbrProgram, "uModelMatrix");
      m_data->locView = glGetUniformLocation(m_data->pbrProgram, "uViewMatrix");
      m_data->locProj = glGetUniformLocation(m_data->pbrProgram, "uProjectionMatrix");
      m_data->locBaseColor = glGetUniformLocation(m_data->pbrProgram, "uBaseColor");
      m_data->locMetallic = glGetUniformLocation(m_data->pbrProgram, "uMetallic");
      m_data->locRoughness = glGetUniformLocation(m_data->pbrProgram, "uRoughness");
      m_data->locReflectivity = glGetUniformLocation(m_data->pbrProgram, "uReflectivity");
      m_data->locEmissive = glGetUniformLocation(m_data->pbrProgram, "uEmissive");
      m_data->locAmbient = glGetUniformLocation(m_data->pbrProgram, "uAmbient");
      m_data->locExposure = glGetUniformLocation(m_data->pbrProgram, "uExposure");
      m_data->locOverlayAlpha = glGetUniformLocation(m_data->pbrProgram, "uOverlayAlpha");
      m_data->locCameraPos = glGetUniformLocation(m_data->pbrProgram, "uCameraPos");
      m_data->locNumLights = glGetUniformLocation(m_data->pbrProgram, "uNumLights");
      m_data->locBaseColorMap = glGetUniformLocation(m_data->pbrProgram, "uBaseColorMap");
      m_data->locHasBaseColorMap = glGetUniformLocation(m_data->pbrProgram, "uHasBaseColorMap");
      m_data->locNormalMap = glGetUniformLocation(m_data->pbrProgram, "uNormalMap");
      m_data->locHasNormalMap = glGetUniformLocation(m_data->pbrProgram, "uHasNormalMap");
      m_data->locMetallicRoughnessMap = glGetUniformLocation(m_data->pbrProgram, "uMetallicRoughnessMap");
      m_data->locHasMetallicRoughnessMap = glGetUniformLocation(m_data->pbrProgram, "uHasMetallicRoughnessMap");
      m_data->locEmissiveMap = glGetUniformLocation(m_data->pbrProgram, "uEmissiveMap");
      m_data->locHasEmissiveMap = glGetUniformLocation(m_data->pbrProgram, "uHasEmissiveMap");
      m_data->locOcclusionMap = glGetUniformLocation(m_data->pbrProgram, "uOcclusionMap");
      m_data->locHasOcclusionMap = glGetUniformLocation(m_data->pbrProgram, "uHasOcclusionMap");
      m_data->locSSREnabled = glGetUniformLocation(m_data->pbrProgram, "uSSREnabled");
      m_data->locPrevColorMap = glGetUniformLocation(m_data->pbrProgram, "uPrevColorMap");
      m_data->locPrevDepthMap = glGetUniformLocation(m_data->pbrProgram, "uPrevDepthMap");
      m_data->locPrevVP = glGetUniformLocation(m_data->pbrProgram, "uPrevVP");
      m_data->locPrevView = glGetUniformLocation(m_data->pbrProgram, "uPrevView");
      m_data->locPrevProjection = glGetUniformLocation(m_data->pbrProgram, "uPrevProjection");
      m_data->locPrevInvProjection = glGetUniformLocation(m_data->pbrProgram, "uPrevInvProjection");
      m_data->locScreenSize = glGetUniformLocation(m_data->pbrProgram, "uScreenSize");
      m_data->locDebugMode = glGetUniformLocation(m_data->pbrProgram, "uDebugMode");
      m_data->locUnlit = glGetUniformLocation(m_data->pbrProgram, "uUnlit");
      for (int i = 0; i < 8; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "uLightPos[%d]", i);
        m_data->locLightPos[i] = glGetUniformLocation(m_data->pbrProgram, buf);
        snprintf(buf, sizeof(buf), "uLightColor[%d]", i);
        m_data->locLightColor[i] = glGetUniformLocation(m_data->pbrProgram, buf);
      }
    }
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    // Unlit shader
    vs = compileShader(GL_VERTEX_SHADER, UNLIT_VERT);
    fs = compileShader(GL_FRAGMENT_SHADER, UNLIT_FRAG);
    if (vs && fs) {
      m_data->unlitProgram = linkProgram(vs, fs);
      m_data->locUnlitMVP = glGetUniformLocation(m_data->unlitProgram, "uMVP");
      m_data->locUnlitPointSize = glGetUniformLocation(m_data->unlitProgram, "uPointSize");
    }
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    // Blit shader (SSR composite)
    vs = compileShader(GL_VERTEX_SHADER, BLIT_VERT);
    fs = compileShader(GL_FRAGMENT_SHADER, BLIT_FRAG);
    if (vs && fs) {
      m_data->blitProgram = linkProgram(vs, fs);
      m_data->blitLocTex = glGetUniformLocation(m_data->blitProgram, "uBlitTex");
      m_data->blitLocAlpha = glGetUniformLocation(m_data->blitProgram, "uAlpha");
    }
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    // Shadow shader
    {
      GLuint svs = compileShader(GL_VERTEX_SHADER, SHADOW_VERT);
      GLuint sfs = compileShader(GL_FRAGMENT_SHADER, SHADOW_FRAG);
      if (svs && sfs) {
        m_data->shadowProgram = linkProgram(svs, sfs);
        m_data->shadowLocModelMatrix = glGetUniformLocation(m_data->shadowProgram, "uModelMatrix");
        m_data->shadowLocLightVP = glGetUniformLocation(m_data->shadowProgram, "uLightVP");
      }
      if (svs) glDeleteShader(svs);
      if (sfs) glDeleteShader(sfs);
    }

    // Shadow FBOs + depth textures
    if (m_data->shadowProgram) {
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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, m_data->shadowTex[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
          fprintf(stderr, "[geom2::Renderer] Shadow FBO %d incomplete: 0x%x\n", i, fbStatus);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }
      fprintf(stderr, "[geom2::Renderer] %d shadow maps (%dx%d) ready\n", Data::MAX_SHADOWS, sz, sz);
    }

    // Cache shadow-related uniform locations in PBR shader
    if (m_data->pbrProgram) {
      for (int i = 0; i < 8; i++) {
        char name[48];
        snprintf(name, sizeof(name), "uLightShadowSlot[%d]", i);
        m_data->locLightShadowSlot[i] = glGetUniformLocation(m_data->pbrProgram, name);
      }
      for (int i = 0; i < 4; i++) {
        char name[48];
        snprintf(name, sizeof(name), "uShadowMatrix[%d]", i);
        m_data->locShadowMatrix[i] = glGetUniformLocation(m_data->pbrProgram, name);
      }
      m_data->locShadowMap[0] = glGetUniformLocation(m_data->pbrProgram, "uShadowMap0");
      m_data->locShadowMap[1] = glGetUniformLocation(m_data->pbrProgram, "uShadowMap1");
      m_data->locShadowMap[2] = glGetUniformLocation(m_data->pbrProgram, "uShadowMap2");
      m_data->locShadowMap[3] = glGetUniformLocation(m_data->pbrProgram, "uShadowMap3");

      // Initialize shadow uniforms to safe defaults (all shadows disabled)
      glUseProgram(m_data->pbrProgram);
      for (int i = 0; i < 8; i++)
        if (m_data->locLightShadowSlot[i] >= 0)
          glUniform1i(m_data->locLightShadowSlot[i], -1);
      for (int i = 0; i < 4; i++)
        if (m_data->locShadowMap[i] >= 0)
          glUniform1i(m_data->locShadowMap[i], 5 + i);  // texture units 5-8
      glUseProgram(0);
    }

    if (m_data->pbrProgram) {
      fprintf(stderr, "[geom2::Renderer] Shaders compiled OK (shadow locs: %d %d %d %d)\n",
              m_data->locShadowMap[0], m_data->locShadowMap[1],
              m_data->locShadowMap[2], m_data->locShadowMap[3]);
    }
  }

  // Upload an ICL Image to a GL texture (matches geom GLRenderer)
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

  static void setUniformMat4(GLint loc, const Mat &m) {
    float gl[16];
    const float *d = m.data();
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        gl[j*4+i] = d[i*4+j];  // transpose: ICL row-major → GL column-major
    glUniformMatrix4fv(loc, 1, GL_FALSE, gl);
  }

  static void collectLights(Node *node, std::vector<LightInfo> &lights) {
    if (!node || !node->isVisible()) return;
    if (auto *light = dynamic_cast<LightNode*>(node)) {
      Mat t = light->getTransformation(true);
      GeomColor c = light->getColor();
      // Normalize: if any channel > 1, assume legacy 0-255 convention
      if (c[0] > 1.01f || c[1] > 1.01f || c[2] > 1.01f)
        c = c * (1.0f / 255.0f);
      float intensity = light->getIntensity();
      int idx = (int)lights.size();
      lights.push_back({{t(0, 3), t(1, 3), t(2, 3)},
                         {c[0]*intensity, c[1]*intensity, c[2]*intensity},
                         light->getShadowEnabled(), idx});
    }
    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        collectLights(group->getChild(i), lights);
    }
  }

  /// Build a light view-projection matrix from a LightNode's world transform.
  /// The light looks along its local -Z axis (OpenGL convention).
  static Mat buildLightVP(const LightInfo &li) {
    // Light position
    float ex = li.pos[0], ey = li.pos[1], ez = li.pos[2];

    // Default: light looks toward origin. If already at origin, look along -Z.
    float tx = 0, ty = 0, tz = 0;
    float dx = tx - ex, dy = ty - ey, dz = tz - ez;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (dist < 1.0f) {
      tx = ex; ty = ey; tz = ez - 1000.0f;
      dx = 0; dy = 0; dz = -1000.0f;
      dist = 1000.0f;
    }

    // lookAt: z = normalize(eye - target), x = normalize(cross(up, z)), y = cross(z, x)
    float zx = -dx/dist, zy = -dy/dist, zz = -dz/dist;
    // up = (0,1,0) unless looking straight up/down
    float upx = 0, upy = 1, upz = 0;
    if (std::abs(zy) > 0.99f) { upx = 0; upy = 0; upz = -1; }
    // x = cross(up, z)
    float xx = upy*zz - upz*zy, xy = upz*zx - upx*zz, xz = upx*zy - upy*zx;
    float xlen = std::sqrt(xx*xx + xy*xy + xz*xz);
    xx /= xlen; xy /= xlen; xz /= xlen;
    // y = cross(z, x)
    float yx = zy*xz - zz*xy, yy = zz*xx - zx*xz, yz = zx*xy - zy*xx;

    Mat view(xx, xy, xz, -(xx*ex + xy*ey + xz*ez),
             yx, yy, yz, -(yx*ex + yy*ey + yz*ez),
             zx, zy, zz, -(zx*ex + zy*ey + zz*ez),
             0,  0,  0,  1);

    // Perspective projection: 90° FOV, 1:1 aspect, near=10mm, far=50000mm
    float n = 10.0f, f = 50000.0f;
    float t = 1.0f;  // tan(45°) = 1
    Mat proj(1/t, 0,   0,            0,
             0,   1/t, 0,            0,
             0,   0,   -(f+n)/(f-n), -2*f*n/(f-n),
             0,   0,   -1,           0);

    return proj * view;
  }

  void Renderer::renderNodeShadow(Node *node) {
    if (!node || !node->isVisible()) return;
    if (dynamic_cast<LightNode*>(node)) return;
    if (dynamic_cast<TextNode*>(node)) return;  // text labels don't cast shadows

    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        renderNodeShadow(group->getChild(i));
    }
    else if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      auto &cache = m_data->cache[geom];
      if (!cache) {
        cache = std::make_unique<GeomCache>();
        cache->build(geom);
      }
      if (cache->numTriIndices > 0) {
        Mat modelMatrix = node->getTransformation(true);
        setUniformMat4(m_data->shadowLocModelMatrix, modelMatrix);
        glBindVertexArray(cache->triVao);
        glDrawElements(GL_TRIANGLES, cache->numTriIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
      }
    }
  }

  void Renderer::render(const std::vector<std::shared_ptr<Node>> &nodes,
                         const Mat &viewMatrix,
                         const Mat &projectionMatrix) {
    ensureShaderCompiled();
    if (!m_data->pbrProgram) return;

    m_data->currentProjection = projectionMatrix;
    m_data->currentView = viewMatrix;

    // Collect lights from scene graph
    m_data->lights.clear();
    for (auto &node : nodes)
      collectLights(node.get(), m_data->lights);

    // ---- Shadow passes (one per shadow-casting light, up to 4) ----
    struct ShadowInfo { int lightIdx; Mat lightVP; };
    ShadowInfo shadowInfos[4];
    int numShadows = 0;
    int lightToShadowSlot[8];
    for (int i = 0; i < 8; i++) lightToShadowSlot[i] = -1;

    if (m_data->shadowsEnabled && m_data->shadowProgram) {
      int numLights = std::min((int)m_data->lights.size(), 8);
      for (int i = 0; i < numLights && numShadows < Data::MAX_SHADOWS; i++) {
        if (m_data->lights[i].shadowEnabled) {
          shadowInfos[numShadows].lightIdx = i;
          shadowInfos[numShadows].lightVP = buildLightVP(m_data->lights[i]);
          lightToShadowSlot[i] = numShadows;
          numShadows++;
        }
      }

      if (numShadows > 0) {
        GLint prevFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
        GLint prevViewport[4];
        glGetIntegerv(GL_VIEWPORT, prevViewport);

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
          setUniformMat4(m_data->shadowLocLightVP, shadowInfos[s].lightVP);

          for (auto &node : nodes)
            renderNodeShadow(node.get());
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
      }
    }

    // SSR: get viewport, set up ping-pong FBOs
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int vpW = viewport[2], vpH = viewport[3];

    GLint mainFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mainFBO);

    // SSR has two independent concerns:
    // - ssrWrite: render to FBO, blit back, update ping-pong (only in normal mode)
    // - ssrRead: bind prev-frame textures so traceSSR() works (always when data exists)
    // In debug modes, we read SSR data but don't write to the FBO, so the
    // ping-pong stays frozen from the last normal frame instead of degrading.
    bool needSSR = m_data->ssrEnabled || (m_data->debugMode >= 6 && m_data->debugMode <= 8);
    bool ssrAvail = needSSR && vpW > 0 && vpH > 0;
    bool ssrWrite = ssrAvail && m_data->debugMode == 0;
    bool ssrRead = ssrAvail && !m_data->ssrFirstFrame;

    if (ssrAvail) {
      m_data->ensureSSRBuffers(vpW, vpH);
      m_data->ensureQuadVAO();
    }
    if (ssrWrite) {
      int writeIdx = m_data->ssrCurrentIdx;
      glBindFramebuffer(GL_FRAMEBUFFER, m_data->ssrFBO[writeIdx]);
      glViewport(0, 0, vpW, vpH);
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_data->pbrProgram);
    setUniformMat4(m_data->locProj, projectionMatrix);
    setUniformMat4(m_data->locView, viewMatrix);
    glUniform1f(m_data->locAmbient, m_data->ambient);
    glUniform1f(m_data->locExposure, m_data->exposure);
    glUniform1f(m_data->locOverlayAlpha, ssrWrite ? 1.0f : m_data->overlayAlpha);
    glUniform1i(m_data->locDebugMode, m_data->debugMode);

    // Camera position from view matrix
    float camX = -(viewMatrix(0, 0)*viewMatrix(0, 3) + viewMatrix(1, 0)*viewMatrix(1, 3) + viewMatrix(2, 0)*viewMatrix(2, 3));
    float camY = -(viewMatrix(0, 1)*viewMatrix(0, 3) + viewMatrix(1, 1)*viewMatrix(1, 3) + viewMatrix(2, 1)*viewMatrix(2, 3));
    float camZ = -(viewMatrix(0, 2)*viewMatrix(0, 3) + viewMatrix(1, 2)*viewMatrix(1, 3) + viewMatrix(2, 2)*viewMatrix(2, 3));
    glUniform3f(m_data->locCameraPos, camX, camY, camZ);

    // Upload lights + shadow slot mapping
    int numLights = std::min((int)m_data->lights.size(), 8);
    glUniform1i(m_data->locNumLights, numLights);
    for (int i = 0; i < numLights; i++) {
      glUniform3fv(m_data->locLightPos[i], 1, m_data->lights[i].pos);
      glUniform3fv(m_data->locLightColor[i], 1, m_data->lights[i].color);
      glUniform1i(m_data->locLightShadowSlot[i], lightToShadowSlot[i]);
    }
    // Clear unused light shadow slots
    for (int i = numLights; i < 8; i++)
      glUniform1i(m_data->locLightShadowSlot[i], -1);

    // Bind shadow maps on texture units 5..8
    for (int s = 0; s < numShadows; s++) {
      glActiveTexture(GL_TEXTURE5 + s);
      glBindTexture(GL_TEXTURE_2D, m_data->shadowTex[s]);
      glUniform1i(m_data->locShadowMap[s], 5 + s);
      setUniformMat4(m_data->locShadowMatrix[s], shadowInfos[s].lightVP);
    }

    // SSR: bind previous frame's textures for reading
    glUniform1i(m_data->locSSREnabled, ssrRead ? 1 : 0);
    if (ssrRead) {
      int readIdx = 1 - m_data->ssrCurrentIdx;
      glActiveTexture(GL_TEXTURE9);
      glBindTexture(GL_TEXTURE_2D, m_data->ssrColorTex[readIdx]);
      glUniform1i(m_data->locPrevColorMap, 9);
      glActiveTexture(GL_TEXTURE10);
      glBindTexture(GL_TEXTURE_2D, m_data->ssrDepthTex[readIdx]);
      glUniform1i(m_data->locPrevDepthMap, 10);
      glUniform2f(m_data->locScreenSize, (float)vpW, (float)vpH);

      Mat prevVP = m_data->prevProj * m_data->prevView;
      // Upload directly with GL_TRUE transpose (matches geom GLRenderer)
      glUniformMatrix4fv(m_data->locPrevVP, 1, GL_TRUE, prevVP.data());
      glUniformMatrix4fv(m_data->locPrevView, 1, GL_TRUE, m_data->prevView.data());
      glUniformMatrix4fv(m_data->locPrevProjection, 1, GL_TRUE, m_data->prevProj.data());
      Mat invProj = m_data->prevProj.inv();
      glUniformMatrix4fv(m_data->locPrevInvProjection, 1, GL_TRUE, invProj.data());
    }
    glActiveTexture(GL_TEXTURE0);

    // Render scene
    for (auto &node : nodes) {
      renderNode(node.get(), viewMatrix);
    }

    // Unbind shadow maps
    for (int s = 0; s < numShadows; s++) {
      glActiveTexture(GL_TEXTURE5 + s);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Unbind prev-frame SSR textures
    if (ssrRead) {
      glActiveTexture(GL_TEXTURE9);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE10);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE0);
    }

    // SSR write path: composite FBO back to main framebuffer, update ping-pong
    if (ssrWrite) {
      glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);
      glViewport(viewport[0], viewport[1], vpW, vpH);

      int writeIdx = m_data->ssrCurrentIdx;

      if (m_data->overlayAlpha >= 0.999f) {
        // Standard mode: fast blit
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_data->ssrFBO[writeIdx]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainFBO);
        glBlitFramebuffer(0, 0, vpW, vpH,
                          viewport[0], viewport[1], viewport[0]+vpW, viewport[1]+vpH,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);
      } else {
        // Overlay mode: alpha-blended blit
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(m_data->blitProgram);
        glUniform1i(m_data->blitLocTex, 0);
        glUniform1f(m_data->blitLocAlpha, m_data->overlayAlpha);
        glBindTexture(GL_TEXTURE_2D, m_data->ssrColorTex[writeIdx]);

        glBindVertexArray(m_data->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
      }

      // Store matrices for next frame's reprojection, swap indices
      m_data->prevView = viewMatrix;
      m_data->prevProj = projectionMatrix;
      m_data->ssrCurrentIdx = 1 - m_data->ssrCurrentIdx;
      m_data->ssrFirstFrame = false;
    }
  }

  void Renderer::renderNode(Node *node, const Mat &viewMatrix) {
    if (!node || !node->isVisible()) return;

    Mat modelMatrix = node->getTransformation(true);

    // Skip lights (already collected in render())
    if (dynamic_cast<LightNode*>(node)) return;

    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++) {
        renderNode(group->getChild(i), viewMatrix);
      }
    }
    else if (auto *pcn = dynamic_cast<PointCloudNode*>(node)) {
      auto cloud = pcn->getPointCloud();
      if (cloud && cloud->getDim() > 0 && m_data->unlitProgram) {
        auto &pc = m_data->pcCache[pcn];
        if (!pc) pc = std::make_unique<PCCache>();

        // Determine fallback color from material or default white
        GeomColor fallback(1,1,1,1);
        auto mat = pcn->getMaterial();
        if (mat) fallback = mat->baseColor;

        // Lock cloud, re-upload every frame (point clouds are dynamic)
        cloud->lock();
        pc->upload(*cloud, fallback);
        cloud->unlock();

        if (pc->numPoints > 0) {
          glUseProgram(m_data->unlitProgram);
          Mat mvp = m_data->currentProjection * viewMatrix * modelMatrix;
          setUniformMat4(m_data->locUnlitMVP, mvp);
          float ps = pcn->getPointSize();
          glUniform1f(m_data->locUnlitPointSize, ps);
          glEnable(GL_PROGRAM_POINT_SIZE);
          glBindVertexArray(pc->vao);
          glDrawArrays(GL_POINTS, 0, pc->numPoints);
          glBindVertexArray(0);
          glDisable(GL_PROGRAM_POINT_SIZE);
          glUseProgram(m_data->pbrProgram);
        }
      }
    }
    else if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      auto &cache = m_data->cache[geom];
      if (!cache) {
        cache = std::make_unique<GeomCache>();
        cache->build(geom);
      }

      // Billboard: cancel view rotation so quad always faces camera
      if (auto *text = dynamic_cast<TextNode*>(node); text && text->isBillboard()) {
        float tx = modelMatrix(0, 3), ty = modelMatrix(1, 3), tz = modelMatrix(2, 3);
        // Inverse view rotation = transpose of upper-left 3x3
        Mat bill = Mat::id();
        for (int i = 0; i < 3; i++)
          for (int j = 0; j < 3; j++)
            bill(j, i) = viewMatrix(i, j);
        bill(0, 3) = tx;
        bill(1, 3) = ty;
        bill(2, 3) = tz;
        // ICL projection flips Y (image convention: Y-down). The text quad
        // has Y-up vertices, so flip the billboard's local Y axis to match.
        bill(0, 1) = -bill(0, 1);
        bill(1, 1) = -bill(1, 1);
        bill(2, 1) = -bill(2, 1);
        modelMatrix = bill;
      }

      // Set model matrix
      setUniformMat4(m_data->locModel, modelMatrix);

      // Material uniforms
      auto mat = geom->getMaterial();
      if (mat) {
        glUniform4f(m_data->locBaseColor, mat->baseColor[0], mat->baseColor[1],
                    mat->baseColor[2], mat->baseColor[3]);
        glUniform1f(m_data->locMetallic, mat->metallic);
        glUniform1f(m_data->locRoughness, mat->roughness);
        glUniform1f(m_data->locReflectivity, mat->reflectivity);
        glUniform4f(m_data->locEmissive, mat->emissive[0], mat->emissive[1],
                    mat->emissive[2], 0);

        // Upload all textures for this material (cached)
        auto &mt = m_data->texCache[mat.get()];
        if (mat->textures) {
          if (!mt.baseColor && !mat->textures->baseColorMap.isNull())
            mt.baseColor = uploadTexture(mat->textures->baseColorMap);
          if (!mt.normalMap && !mat->textures->normalMap.isNull())
            mt.normalMap = uploadTexture(mat->textures->normalMap);
          if (!mt.metallicRoughness && !mat->textures->metallicRoughnessMap.isNull())
            mt.metallicRoughness = uploadTexture(mat->textures->metallicRoughnessMap);
          if (!mt.emissive && !mat->textures->emissiveMap.isNull())
            mt.emissive = uploadTexture(mat->textures->emissiveMap);
          if (!mt.occlusion && !mat->textures->occlusionMap.isNull())
            mt.occlusion = uploadTexture(mat->textures->occlusionMap);
        }

        // Bind textures to texture units
        auto bindTex = [](GLint locHas, GLint locSampler, int unit, GLuint tex) {
          glUniform1i(locHas, tex ? 1 : 0);
          if (tex) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, tex);
            glUniform1i(locSampler, unit);
          }
        };
        bindTex(m_data->locHasBaseColorMap, m_data->locBaseColorMap, 0, mt.baseColor);
        bindTex(m_data->locHasNormalMap, m_data->locNormalMap, 1, mt.normalMap);
        bindTex(m_data->locHasMetallicRoughnessMap, m_data->locMetallicRoughnessMap, 2, mt.metallicRoughness);
        bindTex(m_data->locHasEmissiveMap, m_data->locEmissiveMap, 3, mt.emissive);
        bindTex(m_data->locHasOcclusionMap, m_data->locOcclusionMap, 4, mt.occlusion);

        // Billboard text: render unlit so text color comes through directly
        if (auto *text = dynamic_cast<TextNode*>(geom); text && text->isBillboard())
          glUniform1i(m_data->locUnlit, 1);
      } else {
        glUniform4f(m_data->locBaseColor, 0.8f, 0.8f, 0.8f, 1.0f);
        glUniform1f(m_data->locMetallic, 0.0f);
        glUniform1f(m_data->locRoughness, 0.5f);
        glUniform1f(m_data->locReflectivity, 0.0f);
        glUniform4f(m_data->locEmissive, 0, 0, 0, 0);
        glUniform1i(m_data->locHasBaseColorMap, 0);
        glUniform1i(m_data->locHasNormalMap, 0);
        glUniform1i(m_data->locHasMetallicRoughnessMap, 0);
        glUniform1i(m_data->locHasEmissiveMap, 0);
        glUniform1i(m_data->locHasOcclusionMap, 0);
      }

      // Draw triangles
      if (cache->numTriIndices > 0) {
        glBindVertexArray(cache->triVao);
        glDrawElements(GL_TRIANGLES, cache->numTriIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
      }
      glUniform1i(m_data->locUnlit, 0);

      // Unbind textures
      for (int u = 0; u < 5; u++) {
        glActiveTexture(GL_TEXTURE0 + u);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      glActiveTexture(GL_TEXTURE0);

      // Draw lines + points (unlit)
      if ((cache->numLineVerts > 0 || cache->numPointVerts > 0) && m_data->unlitProgram) {
        glUseProgram(m_data->unlitProgram);

        Mat mvp = m_data->currentProjection * viewMatrix * modelMatrix;
        setUniformMat4(m_data->locUnlitMVP, mvp);

        if (cache->numLineVerts > 0) {
          float lw = mat ? mat->lineWidth : 1.0f;
          glLineWidth(lw);
          glBindVertexArray(cache->lineVao);
          glDrawArrays(GL_LINES, 0, cache->numLineVerts);
          glBindVertexArray(0);
        }

        if (cache->numPointVerts > 0) {
          float ps = mat ? mat->pointSize : 3.0f;
          glUniform1f(m_data->locUnlitPointSize, ps);
          glEnable(GL_PROGRAM_POINT_SIZE);
          glBindVertexArray(cache->pointVao);
          glDrawArrays(GL_POINTS, 0, cache->numPointVerts);
          glBindVertexArray(0);
          glDisable(GL_PROGRAM_POINT_SIZE);
        }

        // Restore PBR program
        glUseProgram(m_data->pbrProgram);
      }
    }
  }

} // namespace icl::geom2
