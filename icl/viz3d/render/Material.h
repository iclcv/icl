// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/cv3d/Types.h>
#include <icl/core/Image.h>
#include <memory>
#include <string>

#ifndef ICLViz3d_API
#define ICLViz3d_API
#endif

namespace icl::viz3d {

  /// Next value from the PROCESS-WIDE texture-version counter. Used so every
  /// Material::TextureMaps (and each of its updates) carries a globally unique
  /// stamp: the viz3d renderer caches GL textures in a map keyed by the raw
  /// Material pointer, and a per-object counter would restart when a freed
  /// Material's address is reused (e.g. the plot's per-retic tick labels),
  /// aliasing the stale cache entry. A global counter never repeats.
  ICLViz3d_API unsigned int nextTextureMapsVersion();

  /// PBR metallic-roughness material (glTF/USD-compatible)
  /** Materials can be shared across SceneObjects and primitives via shared_ptr.
      The PBR parameters map directly to glTF's metallic-roughness model.

      Rarely-used parameter groups (textures, transmission/glass) are stored
      behind shared_ptr sub-structs so that plain colored objects stay lightweight.

      For backwards compatibility with ICL's legacy OpenGL renderer, toPhongParams()
      converts PBR parameters to approximate Blinn-Phong equivalents. The static
      factories fromColor() and fromPhong() create materials from legacy parameters.
  */
  class ICLViz3d_API Material {
  public:

    /// Alpha blending mode
    enum AlphaMode { Opaque, Mask, Blend };

    /// Phong parameters for legacy OpenGL rendering
    struct PhongParams {
      cv3d::GeomColor diffuse;
      cv3d::GeomColor specular;
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

    cv3d::GeomColor baseColor{0.78f, 0.78f, 0.78f, 1.0f};  ///< albedo in [0,1]
    float metallic = 0.0f;          ///< 0 = dielectric, 1 = metal
    float roughness = 0.5f;         ///< 0 = mirror, 1 = fully diffuse
    cv3d::GeomColor emissive{0,0,0,1};    ///< self-illumination in [0,1]
    // NB: mirrors/reflections are expressed the glTF way — a mirror is
    // `metallic = 1, roughness ≈ 0` (its reflection is tinted by baseColor); a
    // glossy dielectric is `metallic = 0` with low roughness. There is no separate
    // "reflectivity" knob (it duplicated metallic-roughness and each backend had
    // to interpret it ad-hoc). Every field here maps 1:1 to Filament and to
    // Cycles' Principled BSDF.

    // ---- EXTENSION IDEA: per-material reflection technique (not yet built) ----
    //
    // metallic/roughness say HOW MUCH a surface reflects; they don't say HOW the
    // real-time backend should GATHER what's reflected. Today that's fixed: the IBL
    // (sky/env) + Filament SSR, and SSR is forced on for EVERY lit material via a
    // compile-time flag in lit_pbr.mat (`reflections : screenspace`). A path tracer
    // (Cycles) just ray-traces the true environment, so it needs none of this.
    //
    // The clean extension is a per-material HINT — declarative intent, each backend
    // picking the best implementation it can; PATH TRACERS IGNORE IT:
    //
    //   enum class Reflection { Auto, Environment, ScreenSpace, Planar, Probe };
    //
    //   - Environment : IBL/cubemap only — cheap, stable, no screen-space artifacts.
    //   - ScreenSpace : force SSR (+ IBL fallback) — reflects only ON-SCREEN geometry,
    //                   so it cannot show an object's occluded side (e.g. a sphere's
    //                   underside reflected in a floor — that needs Planar/Probe).
    //   - Planar      : mirror the scene across the reflector's plane. Exact, but the
    //                   faces carrying the material MUST be coplanar. The backend
    //                   derives the plane FROM THOSE FACES (area-weighted normal +
    //                   coplanarity check + node transform): the reflector is simply
    //                   "whichever faces have this material" — NOT a special object
    //                   and NOT a scene "ground" concept. A non-coplanar mesh (a whole
    //                   cuboid, opposing normals cancel) → plane degenerates → the
    //                   backend declines and falls back to Environment (no guessing
    //                   "which face"). To make only a slab's top reflective, that top
    //                   is its own flat node (material is per-node today, not per-face).
    //   - Probe       : per-object cubemap capture — works on ANY geometry (curved,
    //                   boxy) at the cost of parallax error + a cubemap render pass.
    //
    // Filament reality: SSR opt-in is that .mat COMPILE flag, so per-material on/off
    // needs two compiled lit variants (screenspace / plain) chosen by the hint;
    // Planar needs a mirrored render-to-texture pass. Grow it incrementally: land the
    // enum + Environment/ScreenSpace first (also removes the global-SSR wart), then
    // Planar (flat reflectors), then Probe (arbitrary shapes) behind the same seam.

    // -- Display hints (always inline) --

    cv3d::GeomColor lineColor{0,0,0,0};   ///< wireframe color [0,1] (alpha=0 -> use baseColor)
    cv3d::GeomColor pointColor{0,0,0,0};  ///< point color [0,1] (alpha=0 -> use baseColor)
    float pointSize = 3.0f;
    float lineWidth = 1.0f;
    bool smoothShading = true;
    bool doubleSided = false;

    // -- Lazy: texture maps (null for untextured objects) --

    /// Texture sampling filter. Linear (default) = smooth, for photo textures.
    /// Nearest = crisp texels with no blur — e.g. a 1-texel-per-cell checkerboard
    /// where each texel must render as a hard-edged square. Honoured by both the
    /// GL renderer (GL_NEAREST) and Cycles (INTERPOLATION_CLOSEST).
    enum class TexFilter { Linear, Nearest };

    struct TextureMaps {
      core::Image baseColorMap;          ///< albedo texture (RGB/RGBA)
      core::Image normalMap;             ///< tangent-space normal map (RGB)
      core::Image metallicRoughnessMap;  ///< G=roughness, B=metallic (glTF convention)
      core::Image emissiveMap;           ///< emission texture (RGB)
      core::Image occlusionMap;          ///< ambient occlusion (R channel, 1=fully lit)
      TexFilter filter = TexFilter::Linear;  ///< sampling for all maps of this material
      /// Globally-unique stamp, refreshed whenever a map changes; the renderer
      /// re-uploads this material's GL textures only when its cached version
      /// differs (see setBaseColorMap). Global (not per-object) so a reused
      /// Material address can't alias a stale renderer cache entry.
      unsigned int version = nextTextureMapsVersion();
    };
    std::shared_ptr<TextureMaps> textures;

    // -- Lazy: transmission / glass (null for opaque objects) --

    struct TransmissionParams {
      float transmission = 0.0f;            ///< 0 = opaque, 1 = fully transmissive
      float ior = 1.5f;                     ///< index of refraction (glTF default)
      cv3d::GeomColor attenuationColor{1,1,1,1};  ///< volume absorption tint (white = none)
      float attenuationDistance = 0.0f;     ///< Beer-Lambert distance (0 = no attenuation)
      float thicknessFactor = 0.0f;         ///< thin-wall thickness for volume
      float alphaCutoff = 0.5f;             ///< discard fragments below this (Mask mode)
      AlphaMode alphaMode = Opaque;
    };
    std::shared_ptr<TransmissionParams> transmission;

    // -- Name for debugging/serialization --

    std::string name;

    // -- Helpers --

    /// Set/replace the albedo (base color) texture from an image.
    /** Allocates the texture maps on first use and bumps the texture version so
        the renderer re-uploads. **Safe to call every frame for a live/video
        texture** — the viz3d renderer then refreshes only this material's GL
        textures (via glTexSubImage2D when the size is unchanged), without
        touching geometry or other materials' caches. */
    void setBaseColorMap(const core::Image &img);

    /// Returns true if this material has glass/transmission behavior
    bool isTransmissive() const { return transmission && transmission->transmission > 0.001f; }

    /// Convert PBR parameters to approximate Blinn-Phong for legacy GL
    PhongParams toPhongParams() const;

    // -- Factories --

    /// Create from legacy color (in [0,255] range) + shininess
    static std::shared_ptr<Material> fromColor(const cv3d::GeomColor &color,
                                                float shininess = 128);

    /// Create with separate face and wireframe colors (in [0,255] range)
    static std::shared_ptr<Material> fromColors(const cv3d::GeomColor &faceColor,
                                                 const cv3d::GeomColor &wireColor,
                                                 float shininess = 128);

    /// Create from Phong parameters (auto-converts to PBR)
    /** roughness ~ sqrt(2 / (shininess + 2)), metallic from specular intensity */
    static std::shared_ptr<Material> fromPhong(const cv3d::GeomColor &diffuse,
                                                const cv3d::GeomColor &specular,
                                                float shininess);

    /// Create a matte material whose albedo is the given image texture.
    /** Convenience for image-on-a-surface use (e.g. a "screen"). Update it live
        with setBaseColorMap(). */
    static std::shared_ptr<Material> fromTexture(const core::Image &albedo);
  };

} // namespace icl::viz3d
