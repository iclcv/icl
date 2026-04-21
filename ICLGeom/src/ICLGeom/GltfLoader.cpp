// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "stb_image.h"

#include "GltfLoader.h"
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Material.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLUtils/Size.h>

#include <cstdio>
#include <cstring>
#include <fstream>
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

/// Decode an image from glTF (embedded buffer or file reference) into Image
static Image decodeImage(const cgltf_image *img,
                         const cgltf_data *data,
                         const std::string &basePath) {
  (void)data;
  const unsigned char *rawData = nullptr;
  int rawSize = 0;
  std::vector<unsigned char> fileBuffer;

  if (img->buffer_view) {
    // Embedded image (GLB binary chunk or buffer reference)
    const cgltf_buffer_view *bv = img->buffer_view;
    if (!bv->buffer || !bv->buffer->data) return Image();
    rawData = (const unsigned char *)bv->buffer->data + bv->offset;
    rawSize = (int)bv->size;
  } else if (img->uri) {
    // External file reference
    std::string path = basePath + img->uri;
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
      fprintf(stderr, "  [texture] cannot open: %s\n", path.c_str());
      return Image();
    }
    fileBuffer.resize(f.tellg());
    f.seekg(0);
    f.read((char *)fileBuffer.data(), fileBuffer.size());
    rawData = fileBuffer.data();
    rawSize = (int)fileBuffer.size();
  }

  if (!rawData || rawSize <= 0) return Image();

  // Decode PNG/JPEG to RGBA using stb_image
  int w, h, channels;
  unsigned char *pixels = stbi_load_from_memory(rawData, rawSize, &w, &h, &channels, 4);
  if (!pixels) {
    fprintf(stderr, "  [texture] decode failed: %s\n", img->name ? img->name : "(unnamed)");
    return Image();
  }

  // Convert interleaved RGBA to ICL planar Img8u (4 channels)
  Img8u result(utils::Size(w, h), 4);
  for (int c = 0; c < 4; c++) {
    icl8u *dst = result.getData(c);
    for (int i = 0; i < w * h; i++) {
      dst[i] = pixels[i * 4 + c];
    }
  }
  stbi_image_free(pixels);

  fprintf(stderr, "  [texture] decoded %dx%d (%d ch): %s\n",
          w, h, channels, img->name ? img->name : "(unnamed)");
  return Image(result);
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

  // Occlusion map (ambient occlusion, R channel)
  if (gmat->occlusion_texture.texture && gmat->occlusion_texture.texture->image) {
    mat->occlusionMap = decodeImage(gmat->occlusion_texture.texture->image, data, basePath);
  }

  // Emissive — always store the factor. Per glTF spec: final emission = factor * texture.
  // When there's no texture, the factor alone controls uniform emission.
  float em = gmat->emissive_factor[0] + gmat->emissive_factor[1] + gmat->emissive_factor[2];
  if (em > 0.001f) {
    mat->emissive = GeomColor(gmat->emissive_factor[0],
                               gmat->emissive_factor[1],
                               gmat->emissive_factor[2], 1.0f);
  }
  if (gmat->emissive_texture.texture && gmat->emissive_texture.texture->image) {
    mat->emissiveMap = decodeImage(gmat->emissive_texture.texture->image, data, basePath);
  }

  // KHR_materials_transmission
  if (gmat->has_transmission) {
    mat->transmission = gmat->transmission.transmission_factor;
  }

  // KHR_materials_ior
  if (gmat->has_ior) {
    mat->ior = gmat->ior.ior;
  }

  // KHR_materials_volume (attenuation)
  if (gmat->has_volume) {
    mat->thicknessFactor = gmat->volume.thickness_factor;
    mat->attenuationColor = GeomColor(
      gmat->volume.attenuation_color[0],
      gmat->volume.attenuation_color[1],
      gmat->volume.attenuation_color[2], 1.0f);
    mat->attenuationDistance = gmat->volume.attenuation_distance;
  }

  if (gmat->name && gmat->name[0]) {
    mat->name = gmat->name;
  }

  fprintf(stderr, "  [material] '%s': baseColor=(%.2f,%.2f,%.2f,%.2f) metallic=%.2f roughness=%.2f"
          " transmission=%.2f ior=%.2f",
          mat->name.c_str(), mat->baseColor[0], mat->baseColor[1], mat->baseColor[2], mat->baseColor[3],
          mat->metallic, mat->roughness, mat->transmission, mat->ior);
  if (mat->attenuationDistance > 0.0f) {
    fprintf(stderr, " volume: attenColor=(%.2f,%.2f,%.2f) attenDist=%.4f thick=%.4f",
            mat->attenuationColor[0], mat->attenuationColor[1], mat->attenuationColor[2],
            mat->attenuationDistance, mat->thicknessFactor);
  }
  fprintf(stderr, " maps: base=%d normal=%d mr=%d occ=%d emissive=%d\n",
          (bool)mat->baseColorMap, (bool)mat->normalMap, (bool)mat->metallicRoughnessMap,
          (bool)mat->occlusionMap, (bool)mat->emissiveMap);

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

  // UVs
  bool hasUVs = uvAcc && uvAcc->count == posAcc->count;
  if (hasUVs) {
    auto uvs = readAccessorFloats(uvAcc);
    for (size_t i = 0; i < uvAcc->count; i++) {
      obj->addTexCoord(uvs[i*2+0], uvs[i*2+1]);
    }
  }

  // Indices → triangles
  // In glTF, vertices are pre-split at UV seams, so vertex index = texcoord index.
  bool hasNormals = normAcc && normAcc->count == posAcc->count;
  const GeomColor defaultColor(0, 100, 250, 255);

  if (prim.indices) {
    auto indices = readAccessorIndices(prim.indices);
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
      int a = indices[i], b = indices[i+1], c = indices[i+2];
      int na = hasNormals ? a : -1, nb = hasNormals ? b : -1, nc = hasNormals ? c : -1;
      int ta = hasUVs ? a : -1, tb = hasUVs ? b : -1, tc = hasUVs ? c : -1;
      obj->addTriangle(a, b, c, na, nb, nc, defaultColor, ta, tb, tc);
    }
  } else {
    // Non-indexed: every 3 vertices form a triangle
    for (size_t i = 0; i + 2 < posAcc->count; i += 3) {
      int na = hasNormals ? (int)i : -1, nb = hasNormals ? (int)(i+1) : -1, nc = hasNormals ? (int)(i+2) : -1;
      int ta = hasUVs ? (int)i : -1, tb = hasUVs ? (int)(i+1) : -1, tc = hasUVs ? (int)(i+2) : -1;
      obj->addTriangle(i, i+1, i+2, na, nb, nc, defaultColor, ta, tb, tc);
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
