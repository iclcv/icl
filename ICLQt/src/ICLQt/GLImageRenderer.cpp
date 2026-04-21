// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/GLImageRenderer.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Macros.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <cstdio>
#include <algorithm>
#include <cmath>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {

// --- GL shader helpers (local) ---

static GLuint compileShader(GLenum type, const char *src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, sizeof(log), nullptr, log);
    fprintf(stderr, "[GLImageRenderer] Shader compile error:\n%s\n", log);
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
    fprintf(stderr, "[GLImageRenderer] Program link error:\n%s\n", log);
    glDeleteProgram(prog);
    return 0;
  }
  return prog;
}

// --- Shaders ---

static const char *QUAD_VERT = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform vec2 uScale;
uniform vec2 uUVOffset;
uniform vec2 uUVScale;
out vec2 vUV;
void main() {
    vUV = uUVOffset + aUV * uUVScale;
    gl_Position = vec4(aPos * uScale, 0.0, 1.0);
}
)";

static const char *QUAD_FRAG = R"(
#version 410 core
uniform sampler2D uTexture;
uniform float uBCIScale;
uniform float uBCIBias;
in vec2 vUV;
out vec4 FragColor;
void main() {
    vec4 t = texture(uTexture, vUV);
    FragColor = vec4(clamp(t.rgb * uBCIScale + uBCIBias, 0.0, 1.0), t.a);
}
)";

// --- Data struct ---

struct GLImageRenderer::Data {
  // GL resources
  GLuint program = 0;
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint texture = 0;
  int texW = 0, texH = 0;
  bool glReady = false;
  GLint locScale = -1, locBCIScale = -1, locBCIBias = -1;
  GLint locUVOffset = -1, locUVScale = -1;

  // Stored image (guarded by mutex for thread safety)
  ImgBase *storedImage = nullptr;
  bool imageNull = true;
  mutable std::recursive_mutex imageMutex;

  // BCI
  int bci[3] = {0, 0, 0};
  float bciScale = 1.0f;
  float bciBias = 0.0f;
  bool bciDirty = true;

  // Scale mode
  scalemode sm = interpolateLIN;

  // Grid
  bool drawGrid = false;
  float gridColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  // Stats
  mutable ImageStatistics stats;
  mutable bool statsDirty = true;

  // Texture upload flag
  bool textureDirty = true;

  void initGL() {
    if (glReady) return;
    glReady = true;

    GLuint vs = compileShader(GL_VERTEX_SHADER, QUAD_VERT);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, QUAD_FRAG);
    if (vs && fs) program = linkProgram(vs, fs);
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    if (!program) {
      fprintf(stderr, "[GLImageRenderer] Shader compilation failed!\n");
      return;
    }

    locScale = glGetUniformLocation(program, "uScale");
    locUVOffset = glGetUniformLocation(program, "uUVOffset");
    locUVScale = glGetUniformLocation(program, "uUVScale");
    locBCIScale = glGetUniformLocation(program, "uBCIScale");
    locBCIBias = glGetUniformLocation(program, "uBCIBias");

    // Fullscreen quad: 2 triangles, each vertex = pos(2) + uv(2)
    float quad[] = {
      -1, -1,  0, 1,   // bottom-left  (UV flipped Y)
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
  }

  void updateBCI() {
    if (!bciDirty || imageNull) return;
    bciDirty = false;

    depth d = storedImage->getDepth();
    int ch = storedImage->getChannels();

    if (bci[0] < 0 && bci[1] < 0 && bci[2] < 0) {
      // Auto-adapt: normalize [min,max] → [0,1]
      Range64f range;
      range.minVal = std::numeric_limits<double>::max();
      range.maxVal = -std::numeric_limits<double>::max();
      for (int c = 0; c < ch; c++) {
        Range64f cr = storedImage->getMinMax(c);
        range.minVal = std::min(range.minVal, cr.minVal);
        range.maxVal = std::max(range.maxVal, cr.maxVal);
      }
      double len = std::max(1.0, range.maxVal - range.minVal);

      if (d == depth8u) {
        bciScale = (float)(255.0 / len);
        bciBias = (float)(-bciScale * range.minVal / 255.0);
      } else if (d == depth16s) {
        bciScale = (float)(32767.0 / len);
        bciBias = (float)(-bciScale * range.minVal / 255.0);
      } else {
        bciScale = (float)(255.0 / len);
        bciBias = (float)(-bciScale * range.minVal / 255.0);
        bciScale /= 255.0f;
      }
    } else {
      // Manual BCI
      bciScale = 1.0f;
      if (d == depth16s) bciScale = 127.0f;
      else if (d != depth8u) bciScale = 1.0f / 255.0f;

      bciBias = bci[0] / 255.0f;
      float c = bci[1] / 255.0f;
      if (c > 0) c *= 10.0f;
      bciScale *= (1.0f + c);
      bciBias -= c / 2.0f;
    }
  }

  void uploadTexture() {
    if (!textureDirty || imageNull) return;
    textureDirty = false;

    const ImgBase *src = storedImage;
    int w = src->getWidth(), h = src->getHeight(), ch = src->getChannels();

    // Convert planar ICL image to interleaved RGBA uint8
    std::vector<icl8u> rgba(w * h * 4);
    if (src->getDepth() == depth8u) {
      const Img<icl8u> *img8u = src->as8u();
      for (int i = 0; i < w * h; i++) {
        rgba[i*4+0] = (ch > 0) ? img8u->getData(0)[i] : 0;
        rgba[i*4+1] = (ch > 1) ? img8u->getData(1)[i] : rgba[i*4+0];
        rgba[i*4+2] = (ch > 2) ? img8u->getData(2)[i] : rgba[i*4+0];
        rgba[i*4+3] = (ch > 3) ? img8u->getData(3)[i] : 255;
      }
    } else {
      // Convert other depths to 8u (BCI shader handles range mapping)
      for (int i = 0; i < w * h; i++) {
        for (int c = 0; c < std::min(ch, 4); c++) {
          double v = 0;
          switch (src->getDepth()) {
            case depth16s: v = (double)src->as16s()->getData(c)[i]; break;
            case depth32s: v = (double)src->as32s()->getData(c)[i]; break;
            case depth32f: v = (double)src->as32f()->getData(c)[i]; break;
            case depth64f: v = (double)src->as64f()->getData(c)[i]; break;
            default: break;
          }
          // Store raw value clamped to [0,255] for uint8 texture
          // BCI shader will remap
          rgba[i*4+c] = (icl8u)std::max(0.0, std::min(255.0, v));
        }
        if (ch == 1) { rgba[i*4+1] = rgba[i*4+0]; rgba[i*4+2] = rgba[i*4+0]; }
        if (ch < 4) rgba[i*4+3] = 255;
      }
    }

    GLenum filter = (sm == interpolateNN) ? GL_NEAREST : GL_LINEAR;

    glBindTexture(GL_TEXTURE_2D, texture);
    if (w != texW || h != texH) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, rgba.data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      texW = w;
      texH = h;
    } else {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
                      GL_UNSIGNED_BYTE, rgba.data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }
  }

  void drawQuadWithCrop(float cx, float cy, float cw, float ch) {
    if (!program || imageNull) return;

    std::lock_guard<std::recursive_mutex> lock(imageMutex);
    updateBCI();
    uploadTexture();

    // Validate crop — fall back to full image if invalid
    if (std::isnan(cx) || std::isnan(cy) || std::isnan(cw) || std::isnan(ch) ||
        cw <= 0 || ch <= 0 || cx < 0 || cy < 0 || cx + cw > 1.001f || cy + ch > 1.001f) {
      cx = 0; cy = 0; cw = 1; ch = 1;
    }

    // Compute letterbox scale using effective (cropped) image AR
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    float widgetW = vp[2], widgetH = vp[3];
    if (widgetW <= 0 || widgetH <= 0) return;
    float effectiveW = texW * cw;
    float effectiveH = texH * ch;
    float imgAR = effectiveW / effectiveH;
    float widgetAR = widgetW / widgetH;
    float scaleX = 1.0f, scaleY = 1.0f;
    if (widgetAR > imgAR) {
      scaleX = imgAR / widgetAR;
    } else {
      scaleY = widgetAR / imgAR;
    }

    // Save GL state that QPainter might depend on
    GLboolean depthWasEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthWasEnabled);

    glDisable(GL_DEPTH_TEST);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    glUniform2f(locScale, scaleX, scaleY);
    glUniform2f(locUVOffset, cx, cy);
    glUniform2f(locUVScale, cw, ch);
    glUniform1f(locBCIScale, bciScale);
    glUniform1f(locBCIBias, bciBias);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
  }

  void computeStats() const {
    if (!statsDirty || imageNull) return;
    statsDirty = false;

    int ch = storedImage->getChannels();
    stats.params = ImgParams(storedImage->getSize(), ch);
    stats.time = storedImage->getTime();
    stats.d = storedImage->getDepth();
    stats.isNull = false;

    // Compute per-channel ranges
    stats.ranges.resize(ch);
    stats.globalRange.minVal = std::numeric_limits<double>::max();
    stats.globalRange.maxVal = -std::numeric_limits<double>::max();
    for (int c = 0; c < ch; c++) {
      stats.ranges[c] = storedImage->getMinMax(c);
      stats.globalRange.minVal = std::min(stats.globalRange.minVal, stats.ranges[c].minVal);
      stats.globalRange.maxVal = std::max(stats.globalRange.maxVal, stats.ranges[c].maxVal);
    }

    // Histograms (256 bins, normalized to global range)
    stats.histos.resize(ch);
    for (int c = 0; c < ch; c++) {
      stats.histos[c].assign(256, 0);
    }

    int dim = storedImage->getDim();
    double gMin = stats.globalRange.minVal;
    double gLen = std::max(1.0, stats.globalRange.maxVal - gMin);

    for (int c = 0; c < ch; c++) {
      for (int i = 0; i < dim; i++) {
        double v = 0;
        switch (storedImage->getDepth()) {
          case depth8u: v = storedImage->as8u()->getData(c)[i]; break;
          case depth16s: v = storedImage->as16s()->getData(c)[i]; break;
          case depth32s: v = storedImage->as32s()->getData(c)[i]; break;
          case depth32f: v = storedImage->as32f()->getData(c)[i]; break;
          case depth64f: v = storedImage->as64f()->getData(c)[i]; break;
        }
        int bin = (int)((v - gMin) / gLen * 255.0);
        bin = std::max(0, std::min(255, bin));
        stats.histos[c][bin]++;
      }
    }
  }
};

// --- Public API ---

GLImageRenderer::GLImageRenderer() : m_data(new Data) {}

GLImageRenderer::~GLImageRenderer() {
  if (m_data->program) glDeleteProgram(m_data->program);
  if (m_data->vao) glDeleteVertexArrays(1, &m_data->vao);
  if (m_data->vbo) glDeleteBuffers(1, &m_data->vbo);
  if (m_data->texture) glDeleteTextures(1, &m_data->texture);
  delete m_data->storedImage;
  delete m_data;
}

void GLImageRenderer::update(const ImgBase *src) {
  if (!src) {
    clear();
    return;
  }
  std::lock_guard<std::recursive_mutex> lock(m_data->imageMutex);
  if (m_data->storedImage) {
    delete m_data->storedImage;
  }
  m_data->storedImage = src->deepCopy();
  m_data->imageNull = false;
  m_data->textureDirty = true;
  m_data->bciDirty = true;
  m_data->statsDirty = true;
}

void GLImageRenderer::clear() {
  std::lock_guard<std::recursive_mutex> lock(m_data->imageMutex);
  delete m_data->storedImage;
  m_data->storedImage = nullptr;
  m_data->imageNull = true;
  m_data->textureDirty = true;
  m_data->statsDirty = true;
}

bool GLImageRenderer::isNull() const { return m_data->imageNull; }

Size GLImageRenderer::getSize() const {
  return m_data->imageNull ? Size::null : m_data->storedImage->getSize();
}

int GLImageRenderer::getWidth() const {
  return m_data->imageNull ? 0 : m_data->storedImage->getWidth();
}

int GLImageRenderer::getHeight() const {
  return m_data->imageNull ? 0 : m_data->storedImage->getHeight();
}

void GLImageRenderer::render() {
  m_data->initGL();
  m_data->drawQuadWithCrop(0, 0, 1, 1);
}

void GLImageRenderer::render(float cropX, float cropY, float cropW, float cropH) {
  m_data->initGL();
  m_data->drawQuadWithCrop(cropX, cropY, cropW, cropH);
}

void GLImageRenderer::render(const Image &img) {
  if (img.isNull()) return;
  update(img.ptr());
  render();
}

void GLImageRenderer::setScaleMode(scalemode sm) {
  if (m_data->sm != sm) {
    m_data->sm = sm;
    m_data->textureDirty = true;
  }
}

void GLImageRenderer::setBCI(int b, int c, int i) {
  if (m_data->bci[0] != b || m_data->bci[1] != c || m_data->bci[2] != i) {
    m_data->bci[0] = b;
    m_data->bci[1] = c;
    m_data->bci[2] = i;
    m_data->bciDirty = true;
  }
}

std::vector<icl64f> GLImageRenderer::getColor(int x, int y) const {
  std::lock_guard<std::recursive_mutex> lock(m_data->imageMutex);
  if (m_data->imageNull) return {};
  const ImgBase *img = m_data->storedImage;
  if (x < 0 || y < 0 || x >= img->getWidth() || y >= img->getHeight()) return {};

  int ch = img->getChannels();
  std::vector<icl64f> result(ch);
  for (int c = 0; c < ch; c++) {
    switch (img->getDepth()) {
      case depth8u: result[c] = img->as8u()->getData(c)[x + y * img->getWidth()]; break;
      case depth16s: result[c] = img->as16s()->getData(c)[x + y * img->getWidth()]; break;
      case depth32s: result[c] = img->as32s()->getData(c)[x + y * img->getWidth()]; break;
      case depth32f: result[c] = img->as32f()->getData(c)[x + y * img->getWidth()]; break;
      case depth64f: result[c] = img->as64f()->getData(c)[x + y * img->getWidth()]; break;
    }
  }
  return result;
}

const ImageStatistics &GLImageRenderer::getStats() const {
  std::lock_guard<std::recursive_mutex> lock(m_data->imageMutex);
  m_data->computeStats();
  return m_data->stats;
}

const ImgBase *GLImageRenderer::extractDisplay() const {
  std::lock_guard<std::recursive_mutex> lock(m_data->imageMutex);
  return m_data->storedImage;
}

void GLImageRenderer::setDrawGrid(bool enabled, float *color) {
  m_data->drawGrid = enabled;
  if (color) {
    for (int i = 0; i < 4; i++) m_data->gridColor[i] = color[i];
  }
}

void GLImageRenderer::setGridColor(float *color) {
  if (color) {
    for (int i = 0; i < 4; i++) m_data->gridColor[i] = color[i];
  }
}

const float *GLImageRenderer::getGridColor() const {
  return m_data->gridColor;
}

bool GLImageRenderer::getDrawGrid() const {
  return m_data->drawGrid;
}

} // namespace icl::qt
