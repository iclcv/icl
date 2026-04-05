// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "RaytracerTypes.h"
#include <vector>
#include <unordered_map>

namespace icl::geom {
  class Scene;
  class SceneObject;
  class Camera;
}

namespace icl::rt {

/// Tracks per-object state for dirty detection.
struct ObjectState {
  size_t vertexCount = 0;
  size_t primitiveCount = 0;
  RTMat4 lastTransform;
  int blasHandle = -1;
  bool geometryDirty = true;
  bool transformDirty = true;
};

/// Extracted geometry for one SceneObject (its BLAS data).
struct ObjectGeometry {
  std::vector<RTVertex> vertices;
  std::vector<RTTriangle> triangles;
  RTMaterial material;
  RTMat4 transform;
  RTMat4 transformInverse;
  bool geometryChanged = false;  // BLAS needs rebuild
  bool transformChanged = false; // TLAS needs rebuild
};

/// Full scene extraction result.
struct ExtractedScene {
  std::vector<ObjectGeometry> objects;
  std::vector<RTInstance> instances;
  std::vector<RTMaterial> materials;
  std::vector<RTLight> lights;
  std::vector<RTEmissiveTriangle> emissiveTriangles;
  RTRayGenParams camera;
  RTFloat4 backgroundColor;
  bool anyGeometryChanged = false;
  bool anyTransformChanged = false;
  bool lightsChanged = false;
};

/// Walks an ICL Scene, tessellates geometry to triangles, and packs into
/// GPU-ready buffers. Tracks per-object dirty state to minimize rebuilds.
class GeometryExtractor {
public:
  GeometryExtractor() = default;

  /// Extract geometry, lights, and camera from a Scene.
  /// Only re-extracts objects whose geometry or transform has changed.
  ExtractedScene extract(const geom::Scene &scene, int camIndex);

  /// Force full re-extraction next frame (geometry + transforms).
  void invalidateAll();

  /// Force only transform re-extraction (no retessellation / BLAS rebuild).
  void invalidateTransforms();

  /// Force re-extraction for a specific object.
  void invalidateObject(const geom::SceneObject *obj);

private:
  /// Extract one SceneObject (non-recursive — children are separate objects).
  void extractObject(const geom::SceneObject *obj, const RTMat4 &parentTransform,
                     ExtractedScene &result);

  /// Tessellate all primitives of one SceneObject into triangles.
  void tessellateObject(const geom::SceneObject *obj,
                        std::vector<RTVertex> &vertices,
                        std::vector<RTTriangle> &triangles,
                        RTMaterial &material);

  /// Extract lights from Scene.
  void extractLights(const geom::Scene &scene, ExtractedScene &result);

  /// Extract camera parameters.
  void extractCamera(const geom::Camera &cam, RTRayGenParams &params);

  /// Convert ICL's row-major Mat4D32f to column-major RTMat4.
  static RTMat4 matToRTMat4(const float *data);

  /// Compute a simple 4x4 inverse (for affine transforms).
  static RTMat4 invertAffine(const RTMat4 &m);

  /// Per-object dirty tracking, keyed by SceneObject pointer.
  std::unordered_map<const geom::SceneObject *, ObjectState> m_objectStates;

  /// Previous light state hash for dirty detection.
  size_t m_lastLightHash = 0;
};

} // namespace icl::rt
