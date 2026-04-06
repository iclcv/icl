// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/Material.h>
#include <cmath>
#include <algorithm>

namespace icl::geom {

  std::shared_ptr<Material> Material::fromColor(const GeomColor &color,
                                                 float shininess,
                                                 float reflectivity) {
    auto m = std::make_shared<Material>();
    m->baseColor = color * (1.0f / 255.0f);
    m->roughness = std::sqrt(2.0f / (shininess + 2.0f));
    m->metallic = 0.0f;
    m->reflectivity = std::max(0.0f, std::min(1.0f, reflectivity));
    return m;
  }

  std::shared_ptr<Material> Material::fromPhong(const GeomColor &diffuse,
                                                 const GeomColor &specular,
                                                 float shininess) {
    auto m = std::make_shared<Material>();
    m->baseColor = diffuse * (1.0f / 255.0f);
    m->roughness = std::sqrt(2.0f / (shininess + 2.0f));
    // heuristic: high specular luminance → metallic
    float specLum = (specular[0] + specular[1] + specular[2]) / (3.0f * 255.0f);
    m->metallic = specLum > 0.5f ? 1.0f : 0.0f;
    return m;
  }

  Material::PhongParams Material::toPhongParams() const {
    PhongParams p;
    p.diffuse = baseColor;
    // inverse of roughness → shininess conversion
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
