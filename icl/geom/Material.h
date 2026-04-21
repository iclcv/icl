// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <icl/core/Image.h>
#include <memory>
#include <string>

namespace icl::geom {

  /// PBR metallic-roughness material (glTF/USD-compatible)
  /** Materials can be shared across SceneObjects and primitives via shared_ptr.
      The PBR parameters map directly to glTF's metallic-roughness model.

      Rarely-used parameter groups (textures, transmission/glass) are stored
      behind shared_ptr sub-structs so that plain colored objects stay lightweight.

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

    // -- Copy semantics: non-copyable, move-only --

    Material() = default;
    Material(const Material &) = delete;
    Material &operator=(const Material &) = delete;
    Material(Material &&) = default;
    Material &operator=(Material &&) = default;

    /// Deep copy: clones sub-structs and deep-copies all Images
    std::shared_ptr<Material> deepCopy() const;

    // -- Core PBR metallic-roughness parameters (always inline) --

    GeomColor baseColor{0.78f, 0.78f, 0.78f, 1.0f};  ///< albedo in [0,1]
    float metallic = 0.0f;          ///< 0 = dielectric, 1 = metal
    float roughness = 0.5f;         ///< 0 = mirror, 1 = fully diffuse
    float reflectivity = 0.0f;      ///< explicit mirror reflections (raytracing)
    GeomColor emissive{0,0,0,1};    ///< self-illumination in [0,1]

    // -- Display hints (always inline) --

    GeomColor lineColor{0,0,0,0};   ///< wireframe color [0,1] (alpha=0 -> use baseColor)
    GeomColor pointColor{0,0,0,0};  ///< point color [0,1] (alpha=0 -> use baseColor)
    float pointSize = 3.0f;
    float lineWidth = 1.0f;
    bool smoothShading = true;
    bool doubleSided = false;

    // -- Lazy: texture maps (null for untextured objects) --

    struct TextureMaps {
      core::Image baseColorMap;          ///< albedo texture (RGB/RGBA)
      core::Image normalMap;             ///< tangent-space normal map (RGB)
      core::Image metallicRoughnessMap;  ///< G=roughness, B=metallic (glTF convention)
      core::Image emissiveMap;           ///< emission texture (RGB)
      core::Image occlusionMap;          ///< ambient occlusion (R channel, 1=fully lit)
    };
    std::shared_ptr<TextureMaps> textures;

    // -- Lazy: transmission / glass (null for opaque objects) --

    struct TransmissionParams {
      float transmission = 0.0f;            ///< 0 = opaque, 1 = fully transmissive
      float ior = 1.5f;                     ///< index of refraction (glTF default)
      GeomColor attenuationColor{1,1,1,1};  ///< volume absorption tint (white = none)
      float attenuationDistance = 0.0f;     ///< Beer-Lambert distance (0 = no attenuation)
      float thicknessFactor = 0.0f;         ///< thin-wall thickness for volume
      float alphaCutoff = 0.5f;             ///< discard fragments below this (Mask mode)
      AlphaMode alphaMode = Opaque;
    };
    std::shared_ptr<TransmissionParams> transmission;

    // -- Name for debugging/serialization --

    std::string name;

    // -- Helpers --

    /// Returns true if this material has glass/transmission behavior
    bool isTransmissive() const { return transmission && transmission->transmission > 0.001f; }

    /// Convert PBR parameters to approximate Blinn-Phong for legacy GL
    PhongParams toPhongParams() const;

    // -- Factories --

    /// Create from legacy color (in [0,255] range) + shininess + reflectivity
    static std::shared_ptr<Material> fromColor(const GeomColor &color,
                                                float shininess = 128,
                                                float reflectivity = 0);

    /// Create with separate face and wireframe colors (in [0,255] range)
    static std::shared_ptr<Material> fromColors(const GeomColor &faceColor,
                                                 const GeomColor &wireColor,
                                                 float shininess = 128);

    /// Create from Phong parameters (auto-converts to PBR)
    /** roughness ~ sqrt(2 / (shininess + 2)), metallic from specular intensity */
    static std::shared_ptr<Material> fromPhong(const GeomColor &diffuse,
                                                const GeomColor &specular,
                                                float shininess);
  };

} // namespace icl::geom
