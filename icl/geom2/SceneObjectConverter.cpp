// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SceneObjectConverter.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom/SceneObject.h>
#include <icl/geom/Primitive.h>
#include <icl/geom/Material.h>

namespace icl::geom2 {

  namespace {
    // Legacy geom colours live in [0,255]; geom2 stores/renders in [0,1].
    inline GeomColor scale01(const geom::GeomColor &c) {
      return GeomColor(c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, c[3] / 255.f);
    }
  }

  std::shared_ptr<MeshNode> meshFromSceneObject(const geom::SceneObject &so) {
    auto mesh = std::make_shared<MeshNode>();

    const std::vector<Vec> &V = so.getVertices();
    const auto &prims = so.getPrimitives();
    if (V.empty() && prims.empty()) return mesh;

    MeshData data;
    data.vertices = V;  // same underlying type (FixedColVector<float,4>)
    {
      const std::vector<Vec> &N = so.getNormals();
      if (!N.empty()) data.normals = N;
    }
    {
      const auto &T = so.getTexCoords();
      if (!T.empty()) data.uvs = T;
    }
    {
      const std::vector<geom::GeomColor> &C = so.getVertexColors();
      if (!C.empty()) {
        std::vector<GeomColor> cc(C.size());
        for (size_t i = 0; i < C.size(); ++i) cc[i] = scale01(C[i]);
        data.colors = std::move(cc);
      }
    }

    std::vector<TrianglePrimitive> tris;
    std::vector<QuadPrimitive> quads;
    std::vector<LinePrimitive> lines;

    // Representative face appearance (geom2 shades faces by node material).
    geom::GeomColor faceColor(204, 204, 204, 255);
    std::shared_ptr<geom::Material> faceMat;
    bool haveFace = false;
    auto captureFace = [&](const geom::Primitive *p) {
      if (haveFace) return;
      faceColor = p->color;
      faceMat = p->material;
      haveFace = true;
    };

    for (const geom::Primitive *p : prims) {
      if (!p) continue;
      switch (p->type) {
        case geom::Primitive::line: {
          auto *l = static_cast<const geom::LinePrimitive *>(p);
          lines.push_back(LinePrimitive{l->i(0), l->i(1), scale01(l->color)});
          break;
        }
        case geom::Primitive::triangle: {
          auto *t = static_cast<const geom::TrianglePrimitive *>(p);
          tris.push_back(TrianglePrimitive{
              {t->i(0), t->i(1), t->i(2)},
              {t->i(3), t->i(4), t->i(5)},
              {t->i(6), t->i(7), t->i(8)}});
          captureFace(p);
          break;
        }
        case geom::Primitive::quad: {
          auto *q = static_cast<const geom::QuadPrimitive *>(p);
          quads.push_back(QuadPrimitive{
              {q->i(0), q->i(1), q->i(2), q->i(3)},
              {q->i(4), q->i(5), q->i(6), q->i(7)},
              {q->i(8), q->i(9), q->i(10), q->i(11)}});
          captureFace(p);
          break;
        }
        case geom::Primitive::polygon: {
          auto *poly = static_cast<const geom::PolygonPrimitive *>(p);
          const int n = poly->getNumPoints();
          const bool hasN = poly->hasNormals();
          for (int k = 1; k + 1 < n; ++k) {  // triangle fan
            TrianglePrimitive g{{poly->getVertexIndex(0),
                                 poly->getVertexIndex(k),
                                 poly->getVertexIndex(k + 1)},
                                {-1, -1, -1},
                                {-1, -1, -1}};
            if (hasN) {
              g.n[0] = poly->getNormalIndex(0);
              g.n[1] = poly->getNormalIndex(k);
              g.n[2] = poly->getNormalIndex(k + 1);
            }
            tris.push_back(g);
          }
          captureFace(p);
          break;
        }
        default:  // texture/text/custom — skipped (geometry only)
          break;
      }
    }

    if (!tris.empty()) data.triangles = std::move(tris);
    if (!quads.empty()) data.quads = std::move(quads);
    if (!lines.empty()) data.lines = std::move(lines);

    mesh->ingest(std::move(data));

    // Appearance precedence mirrors the legacy renderer: a per-primitive
    // material overrides the object's default material, which overrides the
    // legacy per-primitive colour.
    if (faceMat) mesh->setMaterial(faceMat->deepCopy());
    else if (auto objMat = so.getMaterial()) mesh->setMaterial(objMat->deepCopy());
    else if (haveFace) mesh->setMaterial(geom::Material::fromColor(faceColor));

    return mesh;
  }

  std::shared_ptr<Node> fromSceneObject(const geom::SceneObject &so) {
    const int nChildren = so.getChildCount();

    if (nChildren == 0) {
      auto mesh = meshFromSceneObject(so);
      mesh->setTransformation(so.getTransformation(true));  // local transform
      mesh->setVisible(so.isVisible());
      return mesh;
    }

    auto group = std::make_shared<GroupNode>();
    group->setTransformation(so.getTransformation(true));
    group->setVisible(so.isVisible());

    // The object may carry its own geometry in addition to children.
    if (!so.getVertices().empty()) {
      group->addChild(meshFromSceneObject(so));  // identity transform under group
    }
    for (int i = 0; i < nChildren; ++i) {
      if (const geom::SceneObject *ch = so.getChild(i)) {
        group->addChild(fromSceneObject(*ch));
      }
    }
    return group;
  }

} // namespace icl::geom2
