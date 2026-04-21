// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLCore/Image.h>

#include <memory>
#include <vector>
#include <string>

namespace icl::geom {

  class Scene;
  class SceneObject;
  class Material;

  /// Modern OpenGL renderer for Scene (GL 4.1 compatible, VBO/VAO based)
  /** Replaces the legacy fixed-function pipeline with a shader-based approach.
      Supports: PBR material textures (baseColorMap), per-pixel Blinn-Phong lighting,
      up to 8 lights, shadow mapping (TODO).

      The camera projection matrix from Camera::getProjectionMatrixGL() is passed
      directly as a uniform, preserving pixel-perfect alignment with calibrated cameras.
  */
  class ICLGeom_API SceneRendererGL {
    struct Data;
    Data *m_data;

  public:
    SceneRendererGL();
    ~SceneRendererGL();

    /// Render the scene using the modern pipeline
    void render(const Scene &scene, int camIndex);

  private:
    void ensureShaderCompiled();
    void renderObject(const SceneObject *obj, const math::FixedMatrix<float,4,4> &viewMatrix);
  };

} // namespace icl::geom
