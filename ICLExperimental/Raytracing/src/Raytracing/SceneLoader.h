// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <string>

namespace icl::geom { class Scene; }

namespace icl::rt {

/// Load a scene description from an XML file into an ICL Scene.
///
/// Format:
/// ```xml
/// <scene>
///   <camera pos="500 -400 350" dir="-0.6 0.5 -0.35" up="0 0 -1" res="640x480"/>
///   <light pos="200 -200 500" diffuse="255 245 220" ambient="35 33 30"
///          specular="255 245 220" scale="1.0"/>
///   <object type="cube" pos="0 0 0" size="50" color="200 60 60"
///           reflectivity="0.3" emission="255 230 180" emissionIntensity="1.5"/>
///   <object type="sphere" pos="0 0 50" radius="35" segments="20"
///           color="255 220 150" smooth="true"/>
/// </scene>
/// ```
///
/// Supported object types: cube, sphere
/// Future: gltf (via type="gltf" file="model.gltf")
///
/// Returns true on success.
bool loadSceneXML(const std::string &filename, geom::Scene &scene);

} // namespace icl::rt
