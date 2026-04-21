// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <unordered_map>
#include <unordered_set>

// Forward declarations — Cycles types hidden behind pointers
namespace ccl {
  class Scene;
  class Geometry;
  class Mesh;
  class Object;
  class PointCloud;
  class Shader;
}

namespace icl::geom {
  class Scene;
  class SceneObject;
  class Camera;
  class Material;
}

namespace icl::rt {

/// Synchronizes an ICL Scene to a Cycles Scene.
///
/// Walks the ICL scene graph, creates/updates Cycles meshes, objects,
/// shaders, camera, and lights. Tracks per-object state to support
/// incremental updates (only changed objects are re-synchronized).
class SceneSynchronizer {
public:
  SceneSynchronizer() = default;

  /// Full synchronization: walk ICL scene, create/update Cycles scene nodes.
  /// @param iclScene  The ICL scene to read from.
  /// @param camIndex  Which ICL camera to use.
  /// @param cclScene  The Cycles scene to write to.
  /// @param sceneScale Scale factor for positions (default 0.001 for mm→m).
  /// Sync result indicating what changed.
  enum class SyncResult { NoChange, TransformOnly, GeometryChanged };

  /// @return what changed in the scene.
  SyncResult synchronize(const geom::Scene &iclScene, int camIndex,
                          ccl::Scene *cclScene, float sceneScale = 0.001f);

  /// Force full re-extraction next frame.
  void invalidateAll();

  /// Force only transform re-extraction (no retessellation).
  void invalidateTransforms();

  /// Force re-extraction for a specific object.
  void invalidateObject(const geom::SceneObject *obj);

  /// Check if any object has pending dirty flags (without consuming them).
  bool hasPendingChanges() const;

private:
  /// Per-object tracking entry.
  struct ObjectEntry {
    const geom::SceneObject *iclObj = nullptr;
    ccl::Geometry *geometry = nullptr;  ///< Mesh or PointCloud, owned by Cycles scene
    ccl::Object *object = nullptr;
    ccl::Shader *shader = nullptr;
    size_t vertexCount = 0;
    size_t primitiveCount = 0;
    bool geometryDirty = true;
    bool transformDirty = true;
    bool visited = false;  ///< Set during sync, cleared after; detects removed objects.
    bool isSphere = false; ///< True if rendered as analytic sphere (PointCloud)
  };

  /// Walk the ICL scene graph recursively.
  void walkObject(const geom::SceneObject *obj,
                  ccl::Scene *cclScene,
                  float sceneScale,
                  bool &anyGeomChanged,
                  bool &anyTransformChanged);

  /// Create or update a Cycles Mesh from ICL SceneObject geometry.
  void syncGeometry(ObjectEntry &entry, ccl::Scene *cclScene, float sceneScale);

  /// Create or update a Cycles Shader from ICL Material.
  void syncMaterial(ObjectEntry &entry, ccl::Scene *cclScene);

  /// Update the Cycles Object transform from ICL SceneObject.
  void syncTransform(ObjectEntry &entry, float sceneScale);

  /// Sync ICL Camera to Cycles Camera.
  void syncCamera(const geom::Camera &cam, ccl::Scene *cclScene, float sceneScale);

  /// Sync ICL SceneLights to Cycles Lights.
  void syncLights(const geom::Scene &iclScene, ccl::Scene *cclScene, float sceneScale);

  /// Remove Cycles nodes for objects that are no longer in the ICL scene.
  void removeStaleObjects(ccl::Scene *cclScene, bool &anyChanged);

  std::unordered_map<const geom::SceneObject *, ObjectEntry> m_entries;

  /// Previous light state hash for dirty detection.
  size_t m_lastLightHash = 0;
  bool m_lightsCreated = false;
};

} // namespace icl::rt
