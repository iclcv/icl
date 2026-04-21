// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "GltfLoader.h"
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Material.h>
#include <ICLCore/Img.h>

#include <cstdio>
#include <cstring>
#include <unordered_map>

using namespace icl::geom;
using namespace icl::core;

namespace icl::rt {

/// Read an accessor's float data into a flat vector
static std::vector<float> readAccessorFloats(const cgltf_accessor *acc) {
  std::vector<float> out(acc->count * cgltf_num_components(acc->type));
  cgltf_accessor_unpack_floats(acc, out.data(), out.size());
  return out;
}

/// Read an accessor's index data
static std::vector<uint32_t> readAccessorIndices(const cgltf_accessor *acc) {
  std::vector<uint32_t> out(acc->count);
  for (cgltf_size i = 0; i < acc->count; i++) {
    out[i] = (uint32_t)cgltf_accessor_read_index(acc, i);
  }
  return out;
}

/// Decode an image from glTF (embedded or file reference) into Img8u
static std::shared_ptr<ImgBase> decodeImage(const cgltf_image *img,
                                             const cgltf_data *data,
                                             const std::string &basePath) {
  // TODO: decode PNG/JPEG from buffer_view or URI
  // For now, return null — textures will be added in Phase 2
  (void)img; (void)data; (void)basePath;
  return nullptr;
}

/// Convert a glTF material to ICL Material
static std::shared_ptr<Material> convertMaterial(const cgltf_material *gmat,
                                                  const cgltf_data *data,
                                                  const std::string &basePath) {
  auto mat = std::make_shared<Material>();
  mat->smoothShading = true;

  if (gmat->has_pbr_metallic_roughness) {
    const auto &pbr = gmat->pbr_metallic_roughness;
    mat->baseColor = GeomColor(pbr.base_color_factor[0],
                                pbr.base_color_factor[1],
                                pbr.base_color_factor[2],
                                pbr.base_color_factor[3]);
    mat->metallic = pbr.metallic_factor;
    mat->roughness = pbr.roughness_factor;

    // Texture references (decoded in Phase 2)
    if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
      mat->baseColorMap = decodeImage(pbr.base_color_texture.texture->image, data, basePath);
    }
    if (pbr.metallic_roughness_texture.texture && pbr.metallic_roughness_texture.texture->image) {
      mat->metallicRoughnessMap = decodeImage(pbr.metallic_roughness_texture.texture->image, data, basePath);
    }
  }

  // Normal map
  if (gmat->normal_texture.texture && gmat->normal_texture.texture->image) {
    mat->normalMap = decodeImage(gmat->normal_texture.texture->image, data, basePath);
  }

  // Emissive
  float em = gmat->emissive_factor[0] + gmat->emissive_factor[1] + gmat->emissive_factor[2];
  if (em > 0.001f) {
    mat->emissive = GeomColor(gmat->emissive_factor[0],
                               gmat->emissive_factor[1],
                               gmat->emissive_factor[2], 1.0f);
  }

  if (gmat->name && gmat->name[0]) {
    mat->name = gmat->name;
  }

  return mat;
}

/// Process a single glTF mesh primitive into a SceneObject
static std::shared_ptr<SceneObject> processPrimitive(
    const cgltf_primitive &prim,
    const cgltf_data *data,
    const std::string &basePath,
    std::unordered_map<const cgltf_material*, std::shared_ptr<Material>> &matCache) {

  if (prim.type != cgltf_primitive_type_triangles) return nullptr;

  // Find attributes
  const cgltf_accessor *posAcc = nullptr, *normAcc = nullptr, *uvAcc = nullptr;
  for (cgltf_size i = 0; i < prim.attributes_count; i++) {
    if (prim.attributes[i].type == cgltf_attribute_type_position)
      posAcc = prim.attributes[i].data;
    else if (prim.attributes[i].type == cgltf_attribute_type_normal)
      normAcc = prim.attributes[i].data;
    else if (prim.attributes[i].type == cgltf_attribute_type_texcoord)
      uvAcc = prim.attributes[i].data;
  }

  if (!posAcc) return nullptr;

  auto obj = std::make_shared<SceneObject>();
  obj->setVisible(Primitive::line, false);
  obj->setVisible(Primitive::vertex, false);

  // Vertices
  auto positions = readAccessorFloats(posAcc);
  for (size_t i = 0; i < posAcc->count; i++) {
    obj->addVertex(Vec(positions[i*3+0], positions[i*3+1], positions[i*3+2], 1));
  }

  // Normals
  if (normAcc && normAcc->count == posAcc->count) {
    auto normals = readAccessorFloats(normAcc);
    for (size_t i = 0; i < normAcc->count; i++) {
      obj->addNormal(Vec(normals[i*3+0], normals[i*3+1], normals[i*3+2], 1));
    }
  }

  // UVs (stored for future texture support)
  if (uvAcc && uvAcc->count == posAcc->count) {
    auto uvs = readAccessorFloats(uvAcc);
    for (size_t i = 0; i < uvAcc->count; i++) {
      obj->addTexCoord(uvs[i*2+0], uvs[i*2+1]);
    }
  }

  // Indices → triangles
  if (prim.indices) {
    auto indices = readAccessorIndices(prim.indices);
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
      int a = indices[i], b = indices[i+1], c = indices[i+2];
      if (normAcc && normAcc->count == posAcc->count) {
        obj->addTriangle(a, b, c, a, b, c);  // with normal indices
      } else {
        obj->addTriangle(a, b, c);
      }
    }
  } else {
    // Non-indexed: every 3 vertices form a triangle
    for (size_t i = 0; i + 2 < posAcc->count; i += 3) {
      if (normAcc) {
        obj->addTriangle(i, i+1, i+2, i, i+1, i+2);
      } else {
        obj->addTriangle(i, i+1, i+2);
      }
    }
  }

  // Material
  if (prim.material) {
    auto it = matCache.find(prim.material);
    if (it == matCache.end()) {
      auto mat = convertMaterial(prim.material, data, basePath);
      matCache[prim.material] = mat;
      obj->setMaterial(mat);
    } else {
      obj->setMaterial(it->second);
    }
  } else {
    auto mat = Material::fromColor(GeomColor(200, 200, 200, 255));
    mat->roughness = 0.5f;
    mat->smoothShading = true;
    obj->setMaterial(mat);
  }

  return obj;
}

/// Apply a glTF node's transform to a SceneObject
static void applyNodeTransform(const cgltf_node *node, SceneObject *obj) {
  float m[16];
  cgltf_node_transform_world(node, m);

  // glTF uses column-major, ICL uses row-major
  Mat T;
  T(0,0) = m[0]; T(1,0) = m[1]; T(2,0) = m[2];  T(3,0) = m[12];
  T(0,1) = m[4]; T(1,1) = m[5]; T(2,1) = m[6];  T(3,1) = m[13];
  T(0,2) = m[8]; T(1,2) = m[9]; T(2,2) = m[10]; T(3,2) = m[14];
  T(0,3) = 0;    T(1,3) = 0;    T(2,3) = 0;      T(3,3) = 1;
  obj->setTransformation(T);
}

std::vector<std::shared_ptr<SceneObject>>
loadGltf(const std::string &filename, Scene &scene) {
  std::vector<std::shared_ptr<SceneObject>> result;

  // Extract base path for relative texture references
  std::string basePath;
  auto lastSlash = filename.find_last_of("/\\");
  if (lastSlash != std::string::npos) basePath = filename.substr(0, lastSlash + 1);

  // Parse
  cgltf_options options = {};
  cgltf_data *data = nullptr;
  cgltf_result res = cgltf_parse_file(&options, filename.c_str(), &data);
  if (res != cgltf_result_success) {
    fprintf(stderr, "glTF parse error: %d\n", (int)res);
    return result;
  }

  // Load buffers (needed for vertex/index data in .gltf files; .glb has inline)
  res = cgltf_load_buffers(&options, data, filename.c_str());
  if (res != cgltf_result_success) {
    fprintf(stderr, "glTF buffer load error: %d\n", (int)res);
    cgltf_free(data);
    return result;
  }

  fprintf(stderr, "  glTF: %zu meshes, %zu materials, %zu textures, %zu nodes\n",
          data->meshes_count, data->materials_count, data->textures_count, data->nodes_count);

  // Material cache
  std::unordered_map<const cgltf_material*, std::shared_ptr<Material>> matCache;

  // Process all nodes
  for (cgltf_size ni = 0; ni < data->nodes_count; ni++) {
    const cgltf_node *node = &data->nodes[ni];
    if (!node->mesh) continue;

    const cgltf_mesh *mesh = node->mesh;
    for (cgltf_size pi = 0; pi < mesh->primitives_count; pi++) {
      auto obj = processPrimitive(mesh->primitives[pi], data, basePath, matCache);
      if (!obj) continue;

      applyNodeTransform(node, obj.get());

      scene.addObject(obj.get());
      result.push_back(obj);

      fprintf(stderr, "  Mesh '%s' prim %zu: %zu verts, %zu prims\n",
              mesh->name ? mesh->name : "(unnamed)", pi,
              obj->getVertices().size(), obj->getPrimitives().size());
    }
  }

  cgltf_free(data);
  return result;
}

} // namespace icl::rt
