// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Image.h>
#include <memory>
#include <string>

namespace icl::geom {

  /// PBR metallic-roughness material (glTF/USD-compatible)
  /** Materials can be shared across SceneObjects and primitives via shared_ptr.
      The PBR parameters map directly to glTF's metallic-roughness model.

      For backwards compatibility with ICL's legacy OpenGL renderer, toPhongParams()
      converts PBR parameters to approximate Blinn-Phong equivalents. The static
      factories fromColor() and fromPhong() create materials from legacy parameters.
  */
  class ICLGeom_API Material {
  public:

    /// Alpha blending mode
    enum AlphaMode { Opaque, Mask, Blend };

    /// Phong parameters for legacy OpenGL rendering
    struct PhongParams {
      GeomColor diffuse;
      GeomColor specular;
      float shininess;
    };

    // -- PBR metallic-roughness parameters --

    GeomColor baseColor{0.78f, 0.78f, 0.78f, 1.0f};  ///< albedo in [0,1]
    float metallic = 0.0f;          ///< 0 = dielectric, 1 = metal
    float roughness = 0.5f;         ///< 0 = mirror, 1 = fully diffuse
    float reflectivity = 0.0f;      ///< explicit mirror reflections (raytracing)
    GeomColor emissive{0,0,0,1};    ///< self-illumination in [0,1]

    // -- Optional texture maps (as Image for portability and value semantics) --

    core::Image baseColorMap;          ///< albedo texture (RGB/RGBA)
    core::Image normalMap;             ///< tangent-space normal map (RGB)
    core::Image metallicRoughnessMap;  ///< G=roughness, B=metallic (glTF convention)
    core::Image emissiveMap;           ///< emission texture (RGB)
    core::Image occlusionMap;          ///< ambient occlusion (R channel, 1=fully lit)

    // -- Alpha handling --

    float alphaCutoff = 0.5f;       ///< discard fragments below this (Mask mode)
    AlphaMode alphaMode = Opaque;

    // -- Display hints --

    bool smoothShading = true;
    bool doubleSided = false;

    // -- Name for debugging/serialization --

    std::string name;

    // -- Constructors --

    Material() = default;

    /// Create from legacy color (in [0,255] range) + shininess + reflectivity
    static std::shared_ptr<Material> fromColor(const GeomColor &color,
                                                float shininess = 128,
                                                float reflectivity = 0);

    /// Create from Phong parameters (auto-converts to PBR)
    /** roughness ~ sqrt(2 / (shininess + 2)), metallic from specular intensity */
    static std::shared_ptr<Material> fromPhong(const GeomColor &diffuse,
                                                const GeomColor &specular,
                                                float shininess);

    /// Convert PBR parameters to approximate Blinn-Phong for legacy GL
    PhongParams toPhongParams() const;
  };

} // namespace icl::geom
