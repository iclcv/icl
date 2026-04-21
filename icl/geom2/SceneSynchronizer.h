// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <unordered_map>

namespace ccl {
  class Scene;
  class Geometry;
  class Mesh;
  class Object;
  class PointCloud;
  class Shader;
}

namespace icl::geom { class Camera; class Material; }

namespace icl::geom2 {

  class Node;
  class GeometryNode;
  class SphereNode;
  class LightNode;
  class GroupNode;
  class Scene2;

  /// Synchronizes a geom2 scene graph to a Cycles Scene.
  ///
  /// Walks the geom2 node tree, creates/updates Cycles meshes, objects,
  /// shaders, camera, and lights. Tracks per-node state to support
  /// incremental updates (only changed nodes are re-synchronized).
  class SceneSynchronizer {
  public:
    SceneSynchronizer() = default;

    enum class SyncResult { NoChange, TransformOnly, GeometryChanged };

    SyncResult synchronize(const Scene2 &scene, int camIndex,
                            ccl::Scene *cclScene, float sceneScale = 0.001f);

    void invalidateAll();
    void invalidateTransforms();
    void invalidateNode(const Node *node);
    bool hasPendingChanges() const;
    void invalidateLights() { m_lightsCreated = false; }

    /// Set background environment strength (0..1). Applied on next sync.
    void setBackgroundStrength(float s) { m_backgroundStrength = s; }

  private:
    struct ObjectEntry {
      const GeometryNode *geomNode = nullptr;
      ccl::Geometry *geometry = nullptr;
      ccl::Object *object = nullptr;
      ccl::Shader *shader = nullptr;
      size_t vertexCount = 0;
      size_t primitiveCount = 0;
      bool geometryDirty = true;
      bool transformDirty = true;
      bool visited = false;
    };

    void walkNode(const Node *node, ccl::Scene *cclScene, float sceneScale,
                  bool &anyGeomChanged, bool &anyTransformChanged);
    void syncGeometry(ObjectEntry &entry, ccl::Scene *cclScene, float sceneScale);
    void syncMaterial(ObjectEntry &entry, ccl::Scene *cclScene);
    void syncTransform(ObjectEntry &entry, ccl::Scene *cclScene, float sceneScale);
    void syncCamera(const geom::Camera &cam, ccl::Scene *cclScene, float sceneScale);
    void syncLights(const Scene2 &scene, ccl::Scene *cclScene, float sceneScale);
    void removeStaleNodes(ccl::Scene *cclScene, bool &anyChanged);

    std::unordered_map<const GeometryNode *, ObjectEntry> m_entries;
    bool m_lightsCreated = false;
    float m_backgroundStrength = 0.8f;
    void *m_bgNode = nullptr;  // BackgroundNode*, avoid Cycles header dependency
  };

} // namespace icl::geom2
