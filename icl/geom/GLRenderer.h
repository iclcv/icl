// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <icl/math/FixedMatrix.h>
#include <icl/core/Image.h>

#include <memory>
#include <vector>
#include <string>

namespace icl::qt { class ICLDrawWidget3D; }

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
  class ICLGeom_API GLRenderer {
    struct Data;
    Data *m_data;

  public:
    GLRenderer();
    ~GLRenderer();

    /// Render the scene using the modern pipeline
    void render(const Scene &scene, int camIndex);

    /// Render with zoom/viewport from the widget (reads imageRect, fitMode, DPR)
    void render(const Scene &scene, int camIndex, qt::ICLDrawWidget3D *widget);

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

    /// Enable/disable screen-space reflections (default: enabled)
    void setSSREnabled(bool enabled);
    bool getSSREnabled() const;

    /// Enable overlay mode: no sky, transparent clear, alpha-blended blit
    void setHideSky(bool enabled);

    /// Set overlay alpha (0-1, default 1.0). Only used in overlay mode.
    void setOverlayAlpha(float alpha);

    /// Render scene to an offscreen image (creates temporary FBO)
    core::Image renderToImage(const Scene &scene, int camIndex, int width, int height);

  private:
    void renderWithViewport(const Scene &scene, int camIndex,
                            int vpX, int vpY, int vpW, int vpH);
    void ensureShaderCompiled();
    void renderObject(const SceneObject *obj, const math::FixedMatrix<float,4,4> &viewMatrix);
    void renderObjectShadow(const SceneObject *obj);
  };

} // namespace icl::geom
