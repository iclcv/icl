// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/CheckerboardNode.h>
#include <icl/viz3d/Primitive.h>
#include <icl/geom/Material.h>
#include <icl/core/Img.h>
#include <algorithm>

namespace icl::viz3d {

  using namespace icl::core;
  using namespace icl::utils;

  // One texel per cell: a (cols+2)×(rows+2) RGB image — white everywhere (the
  // 1-cell quiet-zone border), with the inner cols×rows black/white checker.
  static Img8u makeTexture(int cols, int rows) {
    const int W = cols + 2, H = rows + 2;
    Img8u t(Size(W, H), formatRGB);
    for (int c = 0; c < 3; ++c) std::fill(t.begin(c), t.end(c), (icl8u)255);
    for (int j = 0; j < rows; ++j)
      for (int i = 0; i < cols; ++i)
        if ((i + j) & 1) {
          const int idx = (1 + j) * W + (1 + i);
          for (int c = 0; c < 3; ++c) t.begin(c)[idx] = 0;
        }
    return t;
  }

  std::shared_ptr<CheckerboardNode>
  CheckerboardNode::create(int cols, int rows, float widthMM) {
    auto n = std::make_shared<CheckerboardNode>();
    n->m_cols = cols; n->m_rows = rows; n->m_width = widthMM;
    n->rebuild();
    return n;
  }

  // setCells/setWidth are safe to call from any thread on a node that's already
  // in a scene: ScopedEdit locks the scene around the rebuild and marks it
  // changed on exit (renderer + Cycles re-sync). Idempotent — a no-op when the
  // geometry is unchanged — so callers can drive it straight from a UI value.
  void CheckerboardNode::setCells(int cols, int rows) {
    if (cols == m_cols && rows == m_rows) return;
    ScopedEdit edit(this);
    m_cols = cols; m_rows = rows;
    rebuild();
  }

  void CheckerboardNode::setWidth(float widthMM) {
    if (widthMM == m_width) return;
    ScopedEdit edit(this);
    m_width = widthMM;
    rebuild();
  }

  void CheckerboardNode::rebuild() {
    m_height = m_width * (m_rows + 2) / float(m_cols + 2);   // keep cells square
    const float W = m_width, H = m_height;

    clearGeometry();
    addVertex(Vec(-W/2,  H/2, 0, 1));   // TL  (UV 0,0)
    addVertex(Vec( W/2,  H/2, 0, 1));   // TR  (UV 1,0)
    addVertex(Vec( W/2, -H/2, 0, 1));   // BR  (UV 1,1)
    addVertex(Vec(-W/2, -H/2, 0, 1));   // BL  (UV 0,1)
    for (int i = 0; i < 4; ++i) addNormal(Vec(0, 0, 1, 1));
    addTexCoord(0, 0); addTexCoord(1, 0); addTexCoord(1, 1); addTexCoord(0, 1);
    addQuad(0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3);

    // Reuse the existing material across rebuilds (stable pointer): updating the
    // base-color map in place bumps its texture version so the renderer re-uploads,
    // without the freed-then-reused-Material* stale-cache hazard a fresh material
    // would risk on a setCells() rebuild.
    auto mat = getMaterial();
    if (!mat) {
      mat = geom::Material::fromColor(geom::GeomColor(255, 255, 255, 255));
      mat->roughness = 1.0f;   // matte paper — no specular hot-spot over the corners
      mat->metallic  = 0.0f;
      setMaterial(mat);
    }
    mat->setBaseColorMap(Image(makeTexture(m_cols, m_rows)));
    mat->textures->filter = geom::Material::TexFilter::Nearest;   // crisp 1-texel cells
    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

  std::vector<Vec> CheckerboardNode::innerCorners() const {
    std::vector<Vec> out;
    out.reserve((m_cols - 1) * (m_rows - 1));
    const float W = m_width, H = m_height;
    const float CW = m_cols + 2, CH = m_rows + 2;
    for (int j = 1; j < m_rows; ++j)
      for (int i = 1; i < m_cols; ++i) {
        const float u = (1.f + i) / CW, v = (1.f + j) / CH;
        out.push_back(Vec((u - 0.5f) * W, (0.5f - v) * H, 0.f, 1.f));
      }
    return out;
  }

} // namespace icl::viz3d
