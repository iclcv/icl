// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/SceneRendererGL.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/SceneLight.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Material.h>
#include <ICLGeom/Primitive.h>
#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>
#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <cstdio>
#include <unordered_map>

using namespace icl::math;
using namespace icl::core;
using namespace icl::utils;

namespace icl::geom {

// ---- Shader sources ----

static const char *VERT_SHADER = R"(
#version 120

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

varying vec3 vViewPos;
varying vec3 vNormal;
varying vec2 vTexCoord;

void main() {
    // GL_TRUE gives shader the ICL matrix M directly (same as legacy glLoadMatrixf).
    // Standard column-vector multiply: P * V * M * pos
    vec4 viewPos = uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
    vViewPos = viewPos.xyz;
    mat3 normalMat = mat3(uViewMatrix * uModelMatrix);
    vNormal = normalize(normalMat * aNormal);
    vTexCoord = aTexCoord;
    gl_Position = uProjectionMatrix * viewPos;
}
)";

static const char *FRAG_SHADER = R"(
#version 120

uniform vec4  uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform vec4  uEmissive;

uniform int uHasBaseColorMap;
uniform sampler2D uBaseColorMap;

uniform int  uNumLights;
uniform vec4 uLightPos[8];
uniform vec4 uLightColor[8];
uniform vec4 uAmbientLight;
uniform int uDebugMode;

varying vec3 vViewPos;
varying vec3 vNormal;
varying vec2 vTexCoord;

void main() {
    vec3 N = normalize(vNormal);
    // Two-sided lighting: flip normal for back faces
    if (!gl_FrontFacing) N = -N;

    vec4 albedo = uBaseColor;
    if (uHasBaseColorMap != 0) {
        albedo *= texture2D(uBaseColorMap, vTexCoord);
    }

    float shininess = max(2.0 / (uRoughness * uRoughness + 1e-4) - 2.0, 1.0);
    vec3 V = normalize(-vViewPos);

    vec3 result = uAmbientLight.rgb * albedo.rgb;

    for (int i = 0; i < uNumLights && i < 8; i++) {
        vec3 lightDir = uLightPos[i].xyz - vViewPos;
        float dist = length(lightDir);
        vec3 L = lightDir / dist;
        vec3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        float atten = 1.0 / (1.0 + 0.0001 * dist * dist);
        vec3 lightCol = uLightColor[i].rgb * uLightColor[i].w * atten;

        vec3 diffuse = albedo.rgb * NdotL;
        float specPow = pow(NdotH, shininess);
        vec3 specColor = mix(vec3(0.04), albedo.rgb, uMetallic);
        vec3 specular = specColor * specPow * (1.0 - uRoughness * 0.8);

        result += lightCol * (diffuse + specular);
    }

    result += uEmissive.rgb;

    // Debug: visualize normals when uDebugMode=1
    if (uDebugMode == 1) {
      gl_FragColor = vec4(N * 0.5 + 0.5, 1.0);
      return;
    }
    gl_FragColor = vec4(result, albedo.a);
}
)";

// ---- GLGeometryCache: VBO/VAO per SceneObject ----

struct GLGeometryCache {
  GLuint vbo = 0;
  GLuint ebo = 0;
  int numIndices = 0;
  GLuint baseColorTex = 0;  // cached GL texture from Material::baseColorMap

  ~GLGeometryCache() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
    if (baseColorTex) glDeleteTextures(1, &baseColorTex);
  }

  void build(const SceneObject *obj) {
    const auto &verts = obj->getVertices();
    const auto &norms = obj->getNormals();
    const auto &texCoords = obj->getTexCoords();
    const auto &prims = obj->getPrimitives();

    if (verts.empty()) return;

    bool hasNormals = (norms.size() == verts.size());
    bool hasUVs = !texCoords.empty();

    // Collect triangles with per-corner data
    // Each corner: pos(3) + normal(3) + uv(2) = 8 floats
    struct Vertex { float px, py, pz, nx, ny, nz, u, v; };
    std::vector<Vertex> vertexData;
    std::vector<unsigned int> indices;

    // We can't share vertices across different UV corners, so we emit
    // per-corner vertices (3 per triangle). EBO is sequential.
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
            // Auto-compute face normal
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  /// Bind VBO + EBO and set up vertex attrib pointers (call before glDrawElements)
  void bind(GLuint posLoc, GLuint normalLoc, GLuint texCoordLoc) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  }

  void unbind(GLuint posLoc, GLuint normalLoc, GLuint texCoordLoc) {
    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(normalLoc);
    glDisableVertexAttribArray(texCoordLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void uploadBaseColorMap(const core::Image &img) {
    if (img.isNull()) return;
    if (baseColorTex) return; // already uploaded

    const auto &img8u = img.as<icl8u>();
    int w = img8u.getWidth(), h = img8u.getHeight(), ch = img8u.getChannels();
    std::vector<icl8u> rgba(w * h * 4, 255);
    if (ch >= 3) {
      planarToInterleaved(&img8u, rgba.data());
    }

    glGenTextures(1, &baseColorTex);
    glBindTexture(GL_TEXTURE_2D, baseColorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, ch >= 4 ? GL_RGBA : GL_RGB,
                 w, h, 0, ch >= 4 ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
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
    fprintf(stderr, "[ModernGL] Shader compile error:\n%s\n", log);
    glDeleteShader(s);
    return 0;
  }
  return s;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  // Bind attribute locations before linking
  glBindAttribLocation(prog, 0, "aPosition");
  glBindAttribLocation(prog, 1, "aNormal");
  glBindAttribLocation(prog, 2, "aTexCoord");
  glLinkProgram(prog);
  GLint ok = 0;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
    fprintf(stderr, "[ModernGL] Program link error:\n%s\n", log);
    glDeleteProgram(prog);
    return 0;
  }
  return prog;
}

// ---- SceneRendererGL::Data ----

struct SceneRendererGL::Data {
  GLuint program = 0;
  bool shaderReady = false;
  std::unordered_map<const SceneObject*, std::unique_ptr<GLGeometryCache>> geometryCache;

  void setUniformMat4(const char *name, const Mat &m) {
    GLint loc = glGetUniformLocation(program, name);
    // GL_TRUE: GL transposes the row-major data, giving the shader M directly.
    // This matches glLoadMatrixf(M.transp().data()) used by the legacy pipeline.
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_TRUE, m.data());
  }
  void setUniformVec4(const char *name, float x, float y, float z, float w) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc >= 0) glUniform4f(loc, x, y, z, w);
  }
  void setUniformFloat(const char *name, float v) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc >= 0) glUniform1f(loc, v);
  }
  void setUniformInt(const char *name, int v) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc >= 0) glUniform1i(loc, v);
  }
};

// ---- Implementation ----

SceneRendererGL::SceneRendererGL() : m_data(new Data) {}

SceneRendererGL::~SceneRendererGL() {
  if (m_data->program) glDeleteProgram(m_data->program);
  delete m_data;
}

void SceneRendererGL::ensureShaderCompiled() {
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
    fprintf(stderr, "[ModernGL] Shader compiled and linked successfully\n");
  }
}

void SceneRendererGL::render(const Scene &scene, int camIndex) {
  ensureShaderCompiled();
  if (!m_data->program) return;

  const Camera &cam = scene.getCamera(camIndex);
  Mat projGL = cam.getProjectionMatrixGL();
  Mat viewGL = cam.getCSTransformationMatrixGL();

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
  glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(m_data->program);

  // ICL matrices use row-vector convention (pos * M).
  // GLSL's vec*mat operator does row-vector multiply, so we pass matrices
  // with GL_TRUE (transpose) which gives the shader the ICL matrix directly.
  // The shader uses: gl_Position = vec4(pos,1) * modelView * projection
  m_data->setUniformMat4("uProjectionMatrix", projGL);
  m_data->setUniformMat4("uViewMatrix", viewGL);

  // Lights: transform to view space using row-vector convention
  int numLights = 0;
  for (int i = 0; i < 8; i++) {
    const auto &light = scene.getLight(i);
    if (!light.isOn()) continue;

    Vec wp = light.getPosition();
    // Column-vector: viewPos = V * worldPos (same as shader does)
    Vec vp = viewGL * wp;

    auto d = light.getDiffuse();
    char buf[64];
    snprintf(buf, sizeof(buf), "uLightPos[%d]", numLights);
    m_data->setUniformVec4(buf, vp[0], vp[1], vp[2], 1);
    snprintf(buf, sizeof(buf), "uLightColor[%d]", numLights);
    m_data->setUniformVec4(buf, d[0]/255.f, d[1]/255.f, d[2]/255.f, 1.f);
    numLights++;
  }
  m_data->setUniformInt("uNumLights", numLights);
  m_data->setUniformVec4("uAmbientLight", 0.15f, 0.17f, 0.2f, 1);
  m_data->setUniformInt("uBaseColorMap", 0);
  m_data->setUniformInt("uDebugMode", 0);  // 0=normal shading, 1=show normals

  for (int i = 0; i < scene.getObjectCount(); i++) {
    const SceneObject *obj = scene.getObject(i);
    if (!obj->isVisible()) continue;
    renderObject(obj, viewGL);
  }

  glUseProgram(0);
}

void SceneRendererGL::renderObject(const SceneObject *obj,
                                    const FixedMatrix<float,4,4> &viewMatrix) {
  auto &cache = m_data->geometryCache[obj];
  if (!cache) {
    cache = std::make_unique<GLGeometryCache>();
    cache->build(obj);
    auto mat = obj->getMaterial();
    if (mat && !mat->baseColorMap.isNull()) {
      cache->uploadBaseColorMap(mat->baseColorMap);
    }
  }

  if (cache->numIndices == 0) return;

  Mat modelMatrix = obj->getTransformation(true);
  m_data->setUniformMat4("uModelMatrix", modelMatrix);

  auto mat = obj->getMaterial();
  if (mat) {
    m_data->setUniformVec4("uBaseColor", mat->baseColor[0], mat->baseColor[1],
                            mat->baseColor[2], mat->baseColor[3]);
    m_data->setUniformFloat("uMetallic", mat->metallic);
    m_data->setUniformFloat("uRoughness", mat->roughness);
    m_data->setUniformVec4("uEmissive", mat->emissive[0], mat->emissive[1],
                            mat->emissive[2], 0);
  } else {
    m_data->setUniformVec4("uBaseColor", 0.8f, 0.8f, 0.8f, 1.0f);
    m_data->setUniformFloat("uMetallic", 0.0f);
    m_data->setUniformFloat("uRoughness", 0.5f);
    m_data->setUniformVec4("uEmissive", 0, 0, 0, 0);
  }

  bool hasTexture = cache->baseColorTex != 0;
  m_data->setUniformInt("uHasBaseColorMap", hasTexture ? 1 : 0);
  if (hasTexture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cache->baseColorTex);
  }

  cache->bind(0, 1, 2);
  glDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, 0);
  cache->unbind(0, 1, 2);

  if (hasTexture) glBindTexture(GL_TEXTURE_2D, 0);

  for (int c = 0; c < obj->getChildCount(); c++) {
    renderObject(obj->getChild(c), viewMatrix);
  }
}

} // namespace icl::geom
