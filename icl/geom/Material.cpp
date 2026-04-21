// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/Material.h>
#include <cmath>
#include <algorithm>

namespace icl::geom {

  std::shared_ptr<Material> Material::deepCopy() const {
    auto m = std::make_shared<Material>();
    m->baseColor = baseColor;
    m->metallic = metallic;
    m->roughness = roughness;
    m->reflectivity = reflectivity;
    m->emissive = emissive;
    m->lineColor = lineColor;
    m->pointColor = pointColor;
    m->pointSize = pointSize;
    m->lineWidth = lineWidth;
    m->smoothShading = smoothShading;
    m->doubleSided = doubleSided;
    m->name = name;
    if (textures) {
      m->textures = std::make_shared<TextureMaps>();
      m->textures->baseColorMap = textures->baseColorMap.deepCopy();
      m->textures->normalMap = textures->normalMap.deepCopy();
      m->textures->metallicRoughnessMap = textures->metallicRoughnessMap.deepCopy();
      m->textures->emissiveMap = textures->emissiveMap.deepCopy();
      m->textures->occlusionMap = textures->occlusionMap.deepCopy();
    }
    if (transmission) {
      m->transmission = std::make_shared<TransmissionParams>(*transmission);
    }
    return m;
  }

  std::shared_ptr<Material> Material::fromColor(const GeomColor &color,
                                                 float shininess,
                                                 float reflectivity) {
    auto m = std::make_shared<Material>();
    GeomColor c01 = color * (1.0f / 255.0f);
    m->baseColor = c01;
    m->lineColor = c01;
    m->pointColor = c01;
    m->roughness = std::sqrt(2.0f / (shininess + 2.0f));
    m->metallic = 0.0f;
    m->reflectivity = std::max(0.0f, std::min(1.0f, reflectivity));
    return m;
  }

  std::shared_ptr<Material> Material::fromColors(const GeomColor &faceColor,
                                                   const GeomColor &wireColor,
                                                   float shininess) {
    auto m = std::make_shared<Material>();
    m->baseColor = faceColor * (1.0f / 255.0f);
    m->lineColor = wireColor * (1.0f / 255.0f);
    m->pointColor = wireColor * (1.0f / 255.0f);
    m->roughness = std::sqrt(2.0f / (shininess + 2.0f));
    m->metallic = 0.0f;
    return m;
  }

  std::shared_ptr<Material> Material::fromPhong(const GeomColor &diffuse,
                                                 const GeomColor &specular,
                                                 float shininess) {
    auto m = std::make_shared<Material>();
    GeomColor c01 = diffuse * (1.0f / 255.0f);
    m->baseColor = c01;
    m->lineColor = c01;
    m->pointColor = c01;
    m->roughness = std::sqrt(2.0f / (shininess + 2.0f));
    // heuristic: high specular luminance -> metallic
    float specLum = (specular[0] + specular[1] + specular[2]) / (3.0f * 255.0f);
    m->metallic = specLum > 0.5f ? 1.0f : 0.0f;
    return m;
  }

  Material::PhongParams Material::toPhongParams() const {
    PhongParams p;
    p.diffuse = baseColor;
    // inverse of roughness -> shininess conversion
    float r2 = roughness * roughness;
    p.shininess = std::max(1.0f, 2.0f / (r2 + 1e-4f) - 2.0f);
    p.shininess = std::min(p.shininess, 255.0f);
    // specular: metals use baseColor, dielectrics use white scaled by (1-roughness)
    float inv = 1.0f - metallic;
    p.specular = GeomColor(
      baseColor[0] * metallic + inv * (1.0f - roughness),
      baseColor[1] * metallic + inv * (1.0f - roughness),
      baseColor[2] * metallic + inv * (1.0f - roughness),
      1.0f
    );
    return p;
  }

} // namespace icl::geom
