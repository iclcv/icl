// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "SceneLoader.h"

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/XML.h>
#include <ICLUtils/Macros.h>

#include <sstream>
#include <string>

namespace icl::rt {

using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;

// ---- Helpers ----

static bool parseFloats(const char *s, float *out, int count) {
  if (!s) return false;
  std::istringstream ss(s);
  for (int i = 0; i < count; i++) {
    if (!(ss >> out[i])) return false;
  }
  return true;
}

static GeomColor parseColor(const char *s, GeomColor fallback = GeomColor(200,200,200,255)) {
  float v[4] = {fallback[0], fallback[1], fallback[2], 255};
  if (s) {
    std::istringstream ss(s);
    ss >> v[0] >> v[1] >> v[2];
    // optional alpha
    if (!(ss >> v[3])) v[3] = 255;
  }
  return GeomColor(v[0], v[1], v[2], v[3]);
}

static Vec parseVec(const char *s, Vec fallback = Vec(0,0,0,1)) {
  float v[3] = {fallback[0], fallback[1], fallback[2]};
  parseFloats(s, v, 3);
  return Vec(v[0], v[1], v[2], 1);
}

// ---- Main loader ----

bool loadSceneXML(const std::string &filename, Scene &scene) {
  XMLDocument doc;
  auto result = doc.load_file(filename.c_str());
  if (!result) {
    ERROR_LOG("SceneLoader: failed to parse " << filename << ": " << result.description());
    return false;
  }

  auto root = doc.child("scene");
  if (!root) {
    ERROR_LOG("SceneLoader: no <scene> root element in " << filename);
    return false;
  }

  // ---- Camera ----
  for (auto cam_node : root.children("camera")) {
    Vec pos = parseVec(cam_node.attribute("pos").value(), Vec(500, -400, 350, 1));
    Vec dir = parseVec(cam_node.attribute("dir").value(), Vec(-0.6, 0.5, -0.35, 1));
    Vec up  = parseVec(cam_node.attribute("up").value(), Vec(0, 0, -1, 1));

    Camera cam(pos, dir, up);

    const char *res = cam_node.attribute("res").value();
    if (res && *res) {
      int w = 640, h = 480;
      if (sscanf(res, "%dx%d", &w, &h) == 2)
        cam.setResolution(Size(w, h));
    }

    scene.addCamera(cam);
  }

  // ---- Lights ----
  int lightIdx = 0;
  for (auto light_node : root.children("light")) {
    if (lightIdx >= 8) break;
    auto &sl = scene.getLight(lightIdx);
    sl.setOn(true);

    float scale = light_node.attribute("scale").as_float(1.0f);

    Vec pos = parseVec(light_node.attribute("pos").value(), Vec(200, -200, 500, 1));
    sl.setPosition(pos);

    GeomColor diff = parseColor(light_node.attribute("diffuse").value(), GeomColor(255,245,220,255));
    sl.setDiffuse(GeomColor(diff[0]*scale, diff[1]*scale, diff[2]*scale, 255));

    GeomColor amb = parseColor(light_node.attribute("ambient").value(), GeomColor(30,30,30,255));
    sl.setAmbient(GeomColor(amb[0]*scale, amb[1]*scale, amb[2]*scale, 255));

    GeomColor spec = parseColor(light_node.attribute("specular").value(), diff);
    sl.setSpecular(GeomColor(spec[0]*scale, spec[1]*scale, spec[2]*scale, 255));
    sl.setSpecularEnabled(true);

    lightIdx++;
  }

  // ---- Objects ----
  for (auto obj_node : root.children("object")) {
    std::string type = obj_node.attribute("type").as_string("cube");
    float pos[3] = {0, 0, 0};
    parseFloats(obj_node.attribute("pos").value(), pos, 3);

    GeomColor color = parseColor(obj_node.attribute("color").value());
    float reflectivity = obj_node.attribute("reflectivity").as_float(0.0f);
    float shininess = obj_node.attribute("shininess").as_float(25.0f);
    bool smooth = obj_node.attribute("smooth").as_bool(false);

    SceneObject *obj = nullptr;

    if (type == "cube") {
      float size = obj_node.attribute("size").as_float(50.0f);
      float sx = obj_node.attribute("sx").as_float(1.0f);
      float sy = obj_node.attribute("sy").as_float(1.0f);
      float sz = obj_node.attribute("sz").as_float(1.0f);
      obj = SceneObject::cube(pos[0], pos[1], pos[2], size / 2.0f);
      if (sx != 1.0f || sy != 1.0f || sz != 1.0f)
        obj->scale(sx, sy, sz);
    } else if (type == "sphere") {
      float radius = obj_node.attribute("radius").as_float(30.0f);
      int segments = obj_node.attribute("segments").as_int(20);
      obj = SceneObject::sphere(pos[0], pos[1], pos[2], radius, segments, segments);
      smooth = true; // spheres always smooth
    }

    if (!obj) {
      WARNING_LOG("SceneLoader: unknown object type '" << type << "'");
      continue;
    }

    obj->setVisible(Primitive::line, false);
    obj->setVisible(Primitive::vertex, false);
    obj->createAutoNormals(smooth);
    obj->setColor(Primitive::quad, color);
    obj->setColor(Primitive::triangle, color);
    obj->setReflectivity(reflectivity);
    obj->setShininess(shininess);

    // Rotation (degrees, applied as euler XYZ)
    float rot[3] = {0, 0, 0};
    if (parseFloats(obj_node.attribute("rot").value(), rot, 3)) {
      // Convert degrees to radians and apply
      float rx = rot[0] * 3.14159265f / 180.0f;
      float ry = rot[1] * 3.14159265f / 180.0f;
      float rz = rot[2] * 3.14159265f / 180.0f;
      obj->rotate(rx, ry, rz);
    }

    // Emission
    float emIntensity = obj_node.attribute("emissionIntensity").as_float(0.0f);
    if (emIntensity > 0) {
      GeomColor emColor = parseColor(obj_node.attribute("emission").value(), color);
      obj->setEmission(emColor, emIntensity);
    }

    scene.addObject(obj);
  }

  DEBUG_LOG("SceneLoader: loaded " << filename);
  return true;
}

} // namespace icl::rt
