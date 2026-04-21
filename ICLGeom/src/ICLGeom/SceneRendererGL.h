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

  /// Modern OpenGL 4.1 Core renderer for Scene (VAO/VBO/shader based)
  /** Replaces the legacy fixed-function pipeline with a shader-based approach.
      Requires GL 4.1 Core Profile (set QSurfaceFormat before QApplication).

      Supports: PBR material properties, per-pixel Blinn-Phong lighting,
      up to 8 lights, baseColorMap textures.

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

    /// Set exposure multiplier (default 1.0)
    void setExposure(float e);

    /// Set ambient light level (0-1, default 0.1)
    void setAmbient(float a);

    /// Set debug mode: 0=shaded, 1=normals, 2=albedo, 3=UVs, 4=lighting only
    void setDebugMode(int mode);

    /// Set environment light multiplier (default 1.5)
    void setEnvMultiplier(float m);
    float getEnvMultiplier() const;

    /// Set direct light multiplier (default 1.0)
    void setDirectMultiplier(float m);
    float getDirectMultiplier() const;

    /// Render scene to an offscreen image (creates temporary FBO)
    core::Image renderToImage(const Scene &scene, int camIndex, int width, int height);

  private:
    void ensureShaderCompiled();
    void renderObject(const SceneObject *obj, const math::FixedMatrix<float,4,4> &viewMatrix);
    void renderObjectShadow(const SceneObject *obj);
  };

  /// Fullscreen textured quad renderer for displaying 2D images in GL 4.1 Core
  /** Used by the Cycles viewer pane to display raytraced output without legacy GL. */
  class ICLGeom_API GLImageRenderer {
    struct Data;
    Data *m_data;
  public:
    GLImageRenderer();
    ~GLImageRenderer();

    /// Upload and render an image as a fullscreen quad
    void render(const core::Image &img);
  };

} // namespace icl::geom
