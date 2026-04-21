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

// ---- Shader sources (GL 4.1 Core) ----
// Step 2: Single hardcoded directional light, world-space lighting

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
uniform vec3  uCameraPos;
uniform int   uNumLights;
uniform vec4  uLightPos[8];
uniform vec3  uLightColor[8];

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing) N = -N;

    vec3 albedo = uBaseColor.rgb;
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Shininess from roughness (clamped to avoid invisible pinpoint highlights)
    float shininess = clamp(2.0 / (uRoughness * uRoughness + 1e-4) - 2.0, 1.0, 512.0);

    // F0: dielectric = 0.04, metal = albedo
    vec3 specColor = mix(vec3(0.04), albedo, uMetallic);

    // Metals get a stronger ambient (approximates environment reflection)
    float metalAmbientBoost = mix(1.0, 3.0, uMetallic * (1.0 - uRoughness));
    vec3 result = albedo * uAmbient * metalAmbientBoost;

    for (int i = 0; i < uNumLights; i++) {
        vec3 L = normalize(uLightPos[i].xyz - vWorldPos);
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        // Diffuse: reduced for metals (metals have no diffuse)
        vec3 diffuse = albedo * (1.0 - uMetallic) * NdotL;

        // Specular: Blinn-Phong
        float specPow = pow(NdotH, shininess);
        vec3 specular = specColor * specPow;

        result += uLightColor[i] * (diffuse + specular * NdotL);
    }

    result += uEmissive.rgb;
    result *= uExposure;
    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, uBaseColor.a);
}
)";

// ---- GLGeometryCache: VAO/VBO/EBO per SceneObject ----

struct GLGeometryCache {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  int numIndices = 0;
  GLuint baseColorTex = 0;

  ~GLGeometryCache() {
    if (vao) glDeleteVertexArrays(1, &vao);
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

  void uploadBaseColorMap(const core::Image &img) {
    if (img.isNull()) return;
    if (baseColorTex) return; // already uploaded

    const auto &img8u = img.as<icl8u>();
    int w = img8u.getWidth(), h = img8u.getHeight(), ch = img8u.getChannels();

    std::vector<icl8u> rgba(w * h * 4);
    for (int i = 0; i < w * h; i++) {
      rgba[i*4+0] = (ch > 0) ? img8u.getData(0)[i] : 0;
      rgba[i*4+1] = (ch > 1) ? img8u.getData(1)[i] : 0;
      rgba[i*4+2] = (ch > 2) ? img8u.getData(2)[i] : 0;
      rgba[i*4+3] = (ch > 3) ? img8u.getData(3)[i] : 255;
    }

    glGenTextures(1, &baseColorTex);
    glBindTexture(GL_TEXTURE_2D, baseColorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
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
    fprintf(stderr, "[SceneRendererGL] Shader compile error:\n%s\n", log);
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
    fprintf(stderr, "[SceneRendererGL] Program link error:\n%s\n", log);
    glDeleteProgram(prog);
    return 0;
  }
  return prog;
}

// ---- SceneRendererGL::Data ----

struct SceneRendererGL::Data {
  GLuint program = 0;
  bool shaderReady = false;
  float exposure = 1.0f;
  float ambient = 0.1f;
  int debugMode = 0;
  std::unordered_map<const SceneObject*, std::unique_ptr<GLGeometryCache>> geometryCache;

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
  GLint locCameraPos = -1;
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
    locCameraPos = glGetUniformLocation(program, "uCameraPos");
    locNumLights = glGetUniformLocation(program, "uNumLights");
    for (int i = 0; i < 8; i++) {
      char name[32];
      snprintf(name, sizeof(name), "uLightPos[%d]", i);
      locLightPos[i] = glGetUniformLocation(program, name);
      snprintf(name, sizeof(name), "uLightColor[%d]", i);
      locLightColor[i] = glGetUniformLocation(program, name);
    }
    fprintf(stderr, "[SceneRendererGL] Uniforms: model=%d view=%d proj=%d metallic=%d roughness=%d cameraPos=%d\n",
            locModelMatrix, locViewMatrix, locProjectionMatrix,
            locMetallic, locRoughness, locCameraPos);
  }

  void setUniformMat4(GLint loc, const Mat &m) {
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_TRUE, m.data());
  }
};

// ---- Implementation ----

SceneRendererGL::SceneRendererGL() : m_data(new Data) {}

SceneRendererGL::~SceneRendererGL() {
  if (m_data->program) glDeleteProgram(m_data->program);
  delete m_data;
}

void SceneRendererGL::setExposure(float e) { m_data->exposure = e; }
void SceneRendererGL::setAmbient(float a) { m_data->ambient = a; }
void SceneRendererGL::setDebugMode(int mode) { m_data->debugMode = mode; }

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
    m_data->cacheUniformLocations();
    fprintf(stderr, "[SceneRendererGL] GL 4.1 Core shader compiled OK\n");
  } else {
    fprintf(stderr, "[SceneRendererGL] SHADER COMPILATION FAILED\n");
  }
}

void SceneRendererGL::render(const Scene &scene, int camIndex) {
  ensureShaderCompiled();
  if (!m_data->program) return;

  const Camera &cam = scene.getCamera(camIndex);
  Mat projGL = cam.getProjectionMatrixGL();
  Mat viewGL = cam.getCSTransformationMatrixGL();

  // Set viewport to match camera aspect ratio (letterboxed in widget)
  GLint widgetVP[4];
  glGetIntegerv(GL_VIEWPORT, widgetVP);
  int ww = widgetVP[2], wh = widgetVP[3];
  const Size &chip = cam.getRenderParams().chipSize;
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
  glViewport(vpX, vpY, vpW, vpH);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
  glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(m_data->program);

  m_data->setUniformMat4(m_data->locProjectionMatrix, projGL);
  m_data->setUniformMat4(m_data->locViewMatrix, viewGL);
  glUniform1f(m_data->locAmbient, m_data->ambient);
  glUniform1f(m_data->locExposure, m_data->exposure);

  // Camera position in world space (for specular)
  Vec camPos = cam.getPosition();
  glUniform3f(m_data->locCameraPos, camPos[0], camPos[1], camPos[2]);

  // Set all active lights from Scene (world-space positions and colors)
  int numLights = 0;
  static bool lightsPrinted = false;
  for (int i = 0; i < 8; i++) {
    const auto &light = scene.getLight(i);
    if (!light.isOn()) continue;
    if (numLights >= 8) break;

    Vec wp = light.getPosition();
    auto d = light.getDiffuse();  // already [0,1] — setDiffuse divides by 255
    glUniform4f(m_data->locLightPos[numLights], wp[0], wp[1], wp[2], 1.0f);
    glUniform3f(m_data->locLightColor[numLights], d[0], d[1], d[2]);

    if (!lightsPrinted) {
      fprintf(stderr, "[SceneRendererGL] Light %d→slot %d: pos=(%.0f,%.0f,%.0f) color=(%.2f,%.2f,%.2f)\n",
              i, numLights, wp[0], wp[1], wp[2], d[0], d[1], d[2]);
    }
    numLights++;
  }
  if (!lightsPrinted && numLights > 0) {
    fprintf(stderr, "[SceneRendererGL] %d lights active\n", numLights);
    lightsPrinted = true;
  }
  glUniform1i(m_data->locNumLights, numLights);

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
    static bool firstObj = true;
    if (firstObj) {
      fprintf(stderr, "[SceneRendererGL] First object: %d indices\n", cache->numIndices);
      firstObj = false;
    }
  }

  if (cache->numIndices == 0) return;

  Mat modelMatrix = obj->getTransformation(true);
  m_data->setUniformMat4(m_data->locModelMatrix, modelMatrix);

  auto mat = obj->getMaterial();
  if (mat) {
    glUniform4f(m_data->locBaseColor, mat->baseColor[0], mat->baseColor[1],
                mat->baseColor[2], mat->baseColor[3]);
    glUniform1f(m_data->locMetallic, mat->metallic);
    glUniform1f(m_data->locRoughness, mat->roughness);
    bool hasEmissiveMap = !mat->emissiveMap.isNull();
    float em0 = hasEmissiveMap ? 0 : mat->emissive[0];
    float em1 = hasEmissiveMap ? 0 : mat->emissive[1];
    float em2 = hasEmissiveMap ? 0 : mat->emissive[2];
    glUniform4f(m_data->locEmissive, em0, em1, em2, 0);
  } else {
    glUniform4f(m_data->locBaseColor, 0.8f, 0.8f, 0.8f, 1.0f);
    glUniform1f(m_data->locMetallic, 0.0f);
    glUniform1f(m_data->locRoughness, 0.5f);
    glUniform4f(m_data->locEmissive, 0, 0, 0, 0);
  }

  glBindVertexArray(cache->vao);
  glDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  for (int c = 0; c < obj->getChildCount(); c++) {
    renderObject(obj->getChild(c), viewMatrix);
  }
}

// =====================================================================
// GLImageRenderer — fullscreen textured quad for 2D image display
// =====================================================================

static const char *QUAD_VERT = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform vec2 uScale;  // letterbox scale (1.0 = fill, <1.0 = shrink on that axis)
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos * uScale, 0.0, 1.0);
}
)";

static const char *QUAD_FRAG = R"(
#version 410 core
uniform sampler2D uTexture;
in vec2 vUV;
out vec4 FragColor;
void main() {
    FragColor = texture(uTexture, vUV);
}
)";

struct GLImageRenderer::Data {
  GLuint program = 0;
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint texture = 0;
  int texW = 0, texH = 0;
  bool ready = false;

  void init() {
    if (ready) return;
    ready = true;

    GLuint vs = compileShader(GL_VERTEX_SHADER, QUAD_VERT);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, QUAD_FRAG);
    if (vs && fs) program = linkProgram(vs, fs);
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    if (!program) {
      fprintf(stderr, "[GLImageRenderer] Shader compilation failed!\n");
      return;
    }

    // Fullscreen quad: 2 triangles, each vertex = pos(2) + uv(2)
    float quad[] = {
      -1, -1,  0, 1,   // bottom-left  (UV flipped Y: 0,1 → top of image)
       1, -1,  1, 1,   // bottom-right
       1,  1,  1, 0,   // top-right
      -1, -1,  0, 1,   // bottom-left
       1,  1,  1, 0,   // top-right
      -1,  1,  0, 0,   // top-left
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    glGenTextures(1, &texture);
    fprintf(stderr, "[GLImageRenderer] Initialized (program=%u vao=%u tex=%u)\n",
            program, vao, texture);
  }
};

GLImageRenderer::GLImageRenderer() : m_data(new Data) {}

GLImageRenderer::~GLImageRenderer() {
  if (m_data->program) glDeleteProgram(m_data->program);
  if (m_data->vao) glDeleteVertexArrays(1, &m_data->vao);
  if (m_data->vbo) glDeleteBuffers(1, &m_data->vbo);
  if (m_data->texture) glDeleteTextures(1, &m_data->texture);
  delete m_data;
}

void GLImageRenderer::render(const core::Image &img) {
  m_data->init();
  if (!m_data->program || img.isNull()) return;

  const auto &img8u = img.as<icl8u>();
  int w = img8u.getWidth(), h = img8u.getHeight(), ch = img8u.getChannels();

  // Upload image data as RGBA texture
  std::vector<icl8u> rgba(w * h * 4);
  for (int i = 0; i < w * h; i++) {
    rgba[i*4+0] = (ch > 0) ? img8u.getData(0)[i] : 0;
    rgba[i*4+1] = (ch > 1) ? img8u.getData(1)[i] : 0;
    rgba[i*4+2] = (ch > 2) ? img8u.getData(2)[i] : 0;
    rgba[i*4+3] = (ch > 3) ? img8u.getData(3)[i] : 255;
  }

  glBindTexture(GL_TEXTURE_2D, m_data->texture);
  if (w != m_data->texW || h != m_data->texH) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, rgba.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_data->texW = w;
    m_data->texH = h;
  } else {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
                    GL_UNSIGNED_BYTE, rgba.data());
  }

  // Compute letterbox scale to preserve image aspect ratio
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  float widgetW = vp[2], widgetH = vp[3];
  float imgAR = (float)w / h;
  float widgetAR = widgetW / widgetH;
  float scaleX = 1.0f, scaleY = 1.0f;
  if (widgetAR > imgAR) {
    scaleX = imgAR / widgetAR;  // pillarbox
  } else {
    scaleY = widgetAR / imgAR;  // letterbox
  }

  // Draw letterboxed quad
  glDisable(GL_DEPTH_TEST);
  glUseProgram(m_data->program);
  glUniform1i(glGetUniformLocation(m_data->program, "uTexture"), 0);
  glUniform2f(glGetUniformLocation(m_data->program, "uScale"), scaleX, scaleY);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_data->texture);

  glBindVertexArray(m_data->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glUseProgram(0);
}

} // namespace icl::geom
