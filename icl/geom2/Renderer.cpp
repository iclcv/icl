// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Renderer.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
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
uniform vec4 uEmissive;
uniform float uAmbient;
uniform float uExposure;
uniform vec3 uCameraPos;
uniform int uNumLights;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec3 uLightColor[MAX_LIGHTS];
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 albedo = uBaseColor.rgb;

    vec3 color = albedo * uAmbient;  // ambient

    float shininess = 2.0 / (uRoughness * uRoughness + 0.0001) - 2.0;
    shininess = clamp(shininess, 1.0, 256.0);

    for (int i = 0; i < uNumLights && i < MAX_LIGHTS; i++) {
        vec3 L = normalize(uLightPos[i] - vWorldPos);
        float NdotL = max(dot(N, L), 0.0);

        // Diffuse
        color += albedo * NdotL * (1.0 - uMetallic) * uLightColor[i];

        // Blinn-Phong specular
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), shininess);
        color += vec3(spec) * mix(vec3(0.04), albedo, uMetallic) * uLightColor[i];
    }

    // Fallback: if no lights, use a default directional light
    if (uNumLights == 0) {
        vec3 L = normalize(vec3(0.5, 1.0, 0.3));
        float NdotL = max(dot(N, L), 0.0);
        color += albedo * NdotL * (1.0 - uMetallic);
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), shininess);
        color += vec3(spec) * mix(vec3(0.04), albedo, uMetallic);
    }

    color += uEmissive.rgb;

    // Tone mapping
    color = vec3(1.0) - exp(-color * uExposure);

    FragColor = vec4(color, uBaseColor.a);
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

  // ---- Renderer Data ----

  struct LightInfo {
    float pos[3];
    float color[3];
  };

  struct Renderer::Data {
    GLuint pbrProgram = 0;
    GLuint unlitProgram = 0;
    bool shaderReady = false;
    float exposure = 1.0f;
    float ambient = 0.15f;
    Mat currentProjection;
    Mat currentView;

    // PBR uniform locations
    GLint locModel = -1, locView = -1, locProj = -1;
    GLint locBaseColor = -1, locMetallic = -1, locRoughness = -1;
    GLint locEmissive = -1, locAmbient = -1, locExposure = -1;
    GLint locCameraPos = -1;
    GLint locNumLights = -1;
    GLint locLightPos[8] = {};
    GLint locLightColor[8] = {};

    // Unlit uniform locations
    GLint locUnlitMVP = -1, locUnlitPointSize = -1;

    std::unordered_map<const GeometryNode*, std::unique_ptr<GeomCache>> cache;

    // Lights collected during traversal
    std::vector<LightInfo> lights;
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
  Renderer::~Renderer() = default;

  void Renderer::setExposure(float e) { m_data->exposure = e; }
  void Renderer::setAmbient(float a) { m_data->ambient = a; }
  void Renderer::invalidateCache() { m_data->cache.clear(); }

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
      m_data->locEmissive = glGetUniformLocation(m_data->pbrProgram, "uEmissive");
      m_data->locAmbient = glGetUniformLocation(m_data->pbrProgram, "uAmbient");
      m_data->locExposure = glGetUniformLocation(m_data->pbrProgram, "uExposure");
      m_data->locCameraPos = glGetUniformLocation(m_data->pbrProgram, "uCameraPos");
      m_data->locNumLights = glGetUniformLocation(m_data->pbrProgram, "uNumLights");
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

    if (m_data->pbrProgram) {
      fprintf(stderr, "[geom2::Renderer] Shaders compiled OK\n");
    }
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
      float intensity = light->getIntensity();
      lights.push_back({{t(3,0), t(3,1), t(3,2)},
                         {c[0]*intensity, c[1]*intensity, c[2]*intensity}});
    }
    if (auto *group = dynamic_cast<GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        collectLights(group->getChild(i), lights);
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glUseProgram(m_data->pbrProgram);
    setUniformMat4(m_data->locProj, projectionMatrix);
    setUniformMat4(m_data->locView, viewMatrix);
    glUniform1f(m_data->locAmbient, m_data->ambient);
    glUniform1f(m_data->locExposure, m_data->exposure);

    // Camera position from view matrix (inverse translation)
    // view = [R|t], camPos = -R^T * t. For orthonormal R: R^T = R^-1.
    // The 4th column of the inverse view is the camera position.
    // Approximate: last row of ICL row-major view matrix has translation.
    float camX = -(viewMatrix(0,0)*viewMatrix(3,0) + viewMatrix(0,1)*viewMatrix(3,1) + viewMatrix(0,2)*viewMatrix(3,2));
    float camY = -(viewMatrix(1,0)*viewMatrix(3,0) + viewMatrix(1,1)*viewMatrix(3,1) + viewMatrix(1,2)*viewMatrix(3,2));
    float camZ = -(viewMatrix(2,0)*viewMatrix(3,0) + viewMatrix(2,1)*viewMatrix(3,1) + viewMatrix(2,2)*viewMatrix(3,2));
    glUniform3f(m_data->locCameraPos, camX, camY, camZ);

    // Upload lights
    int numLights = std::min((int)m_data->lights.size(), 8);
    glUniform1i(m_data->locNumLights, numLights);
    for (int i = 0; i < numLights; i++) {
      glUniform3fv(m_data->locLightPos[i], 1, m_data->lights[i].pos);
      glUniform3fv(m_data->locLightColor[i], 1, m_data->lights[i].color);
    }

    for (auto &node : nodes) {
      renderNode(node.get(), viewMatrix);
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
    else if (auto *geom = dynamic_cast<GeometryNode*>(node)) {
      auto &cache = m_data->cache[geom];
      if (!cache) {
        cache = std::make_unique<GeomCache>();
        cache->build(geom);
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
        glUniform4f(m_data->locEmissive, mat->emissive[0], mat->emissive[1],
                    mat->emissive[2], 0);
      } else {
        glUniform4f(m_data->locBaseColor, 0.8f, 0.8f, 0.8f, 1.0f);
        glUniform1f(m_data->locMetallic, 0.0f);
        glUniform1f(m_data->locRoughness, 0.5f);
        glUniform4f(m_data->locEmissive, 0, 0, 0, 0);
      }

      // Draw triangles (PBR lit)
      if (cache->numTriIndices > 0) {
        glBindVertexArray(cache->triVao);
        glDrawElements(GL_TRIANGLES, cache->numTriIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
      }

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
