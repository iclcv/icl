// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Loader.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

// cgltf: single-header glTF parser from Cycles' bundled MaterialX
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

// stb_image: single-header image decoder from 3rdparty/
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "stb_image.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <sstream>

using namespace icl::geom;
using namespace icl::core;

namespace icl::geom2 {

  // ---- glTF helpers (adapted from geom/GltfLoader.cpp) ----

  static std::vector<float> readAccessorFloats(const cgltf_accessor *acc) {
    std::vector<float> out(acc->count * cgltf_num_components(acc->type));
    cgltf_accessor_unpack_floats(acc, out.data(), out.size());
    return out;
  }

  static std::vector<uint32_t> readAccessorIndices(const cgltf_accessor *acc) {
    std::vector<uint32_t> out(acc->count);
    for (cgltf_size i = 0; i < acc->count; i++)
      out[i] = (uint32_t)cgltf_accessor_read_index(acc, i);
    return out;
  }

  static Image decodeImage(const cgltf_image *img, const cgltf_data *data,
                            const std::string &basePath) {
    (void)data;
    const unsigned char *rawData = nullptr;
    int rawSize = 0;
    std::vector<unsigned char> fileBuffer;

    if (img->buffer_view) {
      const cgltf_buffer_view *bv = img->buffer_view;
      if (!bv->buffer || !bv->buffer->data) return Image();
      rawData = (const unsigned char *)bv->buffer->data + bv->offset;
      rawSize = (int)bv->size;
    } else if (img->uri) {
      std::string path = basePath + img->uri;
      std::ifstream f(path, std::ios::binary | std::ios::ate);
      if (!f) return Image();
      fileBuffer.resize(f.tellg());
      f.seekg(0);
      f.read((char *)fileBuffer.data(), fileBuffer.size());
      rawData = fileBuffer.data();
      rawSize = (int)fileBuffer.size();
    }

    if (!rawData || rawSize <= 0) return Image();

    int w, h, channels;
    unsigned char *pixels = stbi_load_from_memory(rawData, rawSize, &w, &h, &channels, 4);
    if (!pixels) return Image();

    Img8u result(utils::Size(w, h), 4);
    for (int c = 0; c < 4; c++) {
      icl8u *dst = result.getData(c);
      for (int i = 0; i < w * h; i++) dst[i] = pixels[i * 4 + c];
    }
    stbi_image_free(pixels);
    return Image(result);
  }

  static std::shared_ptr<Material> convertMaterial(const cgltf_material *gmat,
                                                    const cgltf_data *data,
                                                    const std::string &basePath) {
    auto mat = std::make_shared<Material>();
    mat->smoothShading = true;

    if (gmat->has_pbr_metallic_roughness) {
      const auto &pbr = gmat->pbr_metallic_roughness;
      mat->baseColor = GeomColor(pbr.base_color_factor[0], pbr.base_color_factor[1],
                                  pbr.base_color_factor[2], pbr.base_color_factor[3]);
      mat->lineColor = mat->baseColor;
      mat->pointColor = mat->baseColor;
      mat->metallic = pbr.metallic_factor;
      mat->roughness = pbr.roughness_factor;

      if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
        if (!mat->textures) mat->textures = std::make_shared<Material::TextureMaps>();
        mat->textures->baseColorMap = decodeImage(pbr.base_color_texture.texture->image, data, basePath);
      }
      if (pbr.metallic_roughness_texture.texture && pbr.metallic_roughness_texture.texture->image) {
        if (!mat->textures) mat->textures = std::make_shared<Material::TextureMaps>();
        mat->textures->metallicRoughnessMap = decodeImage(pbr.metallic_roughness_texture.texture->image, data, basePath);
      }
    }

    if (gmat->normal_texture.texture && gmat->normal_texture.texture->image) {
      if (!mat->textures) mat->textures = std::make_shared<Material::TextureMaps>();
      mat->textures->normalMap = decodeImage(gmat->normal_texture.texture->image, data, basePath);
    }
    if (gmat->occlusion_texture.texture && gmat->occlusion_texture.texture->image) {
      if (!mat->textures) mat->textures = std::make_shared<Material::TextureMaps>();
      mat->textures->occlusionMap = decodeImage(gmat->occlusion_texture.texture->image, data, basePath);
    }

    float em = gmat->emissive_factor[0] + gmat->emissive_factor[1] + gmat->emissive_factor[2];
    if (em > 0.001f) {
      mat->emissive = GeomColor(gmat->emissive_factor[0], gmat->emissive_factor[1],
                                 gmat->emissive_factor[2], 1.0f);
    }
    if (gmat->emissive_texture.texture && gmat->emissive_texture.texture->image) {
      if (!mat->textures) mat->textures = std::make_shared<Material::TextureMaps>();
      mat->textures->emissiveMap = decodeImage(gmat->emissive_texture.texture->image, data, basePath);
    }

    if (gmat->has_transmission) {
      if (!mat->transmission) mat->transmission = std::make_shared<Material::TransmissionParams>();
      mat->transmission->transmission = gmat->transmission.transmission_factor;
    }
    if (gmat->has_ior) {
      if (!mat->transmission) mat->transmission = std::make_shared<Material::TransmissionParams>();
      mat->transmission->ior = gmat->ior.ior;
    }
    if (gmat->has_volume) {
      if (!mat->transmission) mat->transmission = std::make_shared<Material::TransmissionParams>();
      mat->transmission->thicknessFactor = gmat->volume.thickness_factor;
      mat->transmission->attenuationColor = GeomColor(
          gmat->volume.attenuation_color[0], gmat->volume.attenuation_color[1],
          gmat->volume.attenuation_color[2], 1.0f);
      mat->transmission->attenuationDistance = gmat->volume.attenuation_distance;
    }

    if (gmat->name && gmat->name[0]) mat->name = gmat->name;
    return mat;
  }

  static std::shared_ptr<MeshNode> processGltfPrimitive(
      const cgltf_primitive &prim, const cgltf_data *data,
      const std::string &basePath,
      std::unordered_map<const cgltf_material*, std::shared_ptr<Material>> &matCache) {

    if (prim.type != cgltf_primitive_type_triangles) return nullptr;

    const cgltf_accessor *posAcc = nullptr, *normAcc = nullptr, *uvAcc = nullptr;
    for (cgltf_size i = 0; i < prim.attributes_count; i++) {
      if (prim.attributes[i].type == cgltf_attribute_type_position) posAcc = prim.attributes[i].data;
      else if (prim.attributes[i].type == cgltf_attribute_type_normal) normAcc = prim.attributes[i].data;
      else if (prim.attributes[i].type == cgltf_attribute_type_texcoord) uvAcc = prim.attributes[i].data;
    }
    if (!posAcc) return nullptr;

    auto mesh = std::make_shared<MeshNode>();

    // Build vertex data
    auto posFloats = readAccessorFloats(posAcc);
    std::vector<Vec> verts(posAcc->count);
    for (size_t i = 0; i < posAcc->count; i++)
      verts[i] = Vec(posFloats[i*3+0], posFloats[i*3+1], posFloats[i*3+2], 1);

    bool hasNormals = normAcc && normAcc->count == posAcc->count;
    std::optional<std::vector<Vec>> norms;
    if (hasNormals) {
      auto nf = readAccessorFloats(normAcc);
      std::vector<Vec> nv(normAcc->count);
      for (size_t i = 0; i < normAcc->count; i++)
        nv[i] = Vec(nf[i*3+0], nf[i*3+1], nf[i*3+2], 0);
      norms = std::move(nv);
    }

    bool hasUVs = uvAcc && uvAcc->count == posAcc->count;
    std::optional<std::vector<utils::Point32f>> uvData;
    if (hasUVs) {
      auto uf = readAccessorFloats(uvAcc);
      std::vector<utils::Point32f> uv(uvAcc->count);
      for (size_t i = 0; i < uvAcc->count; i++)
        uv[i] = utils::Point32f(uf[i*2+0], uf[i*2+1]);
      uvData = std::move(uv);
    }

    // Build triangle indices
    std::vector<TrianglePrimitive> tris;
    if (prim.indices) {
      auto indices = readAccessorIndices(prim.indices);
      tris.reserve(indices.size() / 3);
      for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        int a = indices[i], b = indices[i+1], c = indices[i+2];
        tris.push_back({{a, b, c},
                         {hasNormals?a:-1, hasNormals?b:-1, hasNormals?c:-1},
                         {hasUVs?a:-1, hasUVs?b:-1, hasUVs?c:-1}});
      }
    } else {
      tris.reserve(posAcc->count / 3);
      for (size_t i = 0; i + 2 < posAcc->count; i += 3) {
        int ii = (int)i;
        tris.push_back({{ii, ii+1, ii+2},
                         {hasNormals?ii:-1, hasNormals?ii+1:-1, hasNormals?ii+2:-1},
                         {hasUVs?ii:-1, hasUVs?ii+1:-1, hasUVs?ii+2:-1}});
      }
    }

    mesh->ingest({
      .vertices  = std::move(verts),
      .normals   = std::move(norms),
      .uvs       = std::move(uvData),
      .triangles = std::move(tris),
    });

    if (prim.material) {
      auto it = matCache.find(prim.material);
      if (it == matCache.end()) {
        auto mat = convertMaterial(prim.material, data, basePath);
        matCache[prim.material] = mat;
        mesh->setMaterial(mat);
      } else {
        mesh->setMaterial(it->second);
      }
    } else {
      mesh->setMaterial(Material::fromColor(geom2::GeomColor(200, 200, 200, 255)));
    }

    return mesh;
  }

  static std::vector<std::shared_ptr<MeshNode>> loadGltf(const std::string &filename) {
    std::vector<std::shared_ptr<MeshNode>> result;

    std::string basePath;
    auto lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) basePath = filename.substr(0, lastSlash + 1);

    cgltf_options options = {};
    cgltf_data *data = nullptr;
    if (cgltf_parse_file(&options, filename.c_str(), &data) != cgltf_result_success) return result;
    if (cgltf_load_buffers(&options, data, filename.c_str()) != cgltf_result_success) {
      cgltf_free(data);
      return result;
    }

    std::unordered_map<const cgltf_material*, std::shared_ptr<Material>> matCache;

    for (cgltf_size ni = 0; ni < data->nodes_count; ni++) {
      const cgltf_node *node = &data->nodes[ni];
      if (!node->mesh) continue;

      for (cgltf_size pi = 0; pi < node->mesh->primitives_count; pi++) {
        auto mesh = processGltfPrimitive(node->mesh->primitives[pi], data, basePath, matCache);
        if (!mesh) continue;

        // Apply node transform
        float m[16];
        cgltf_node_transform_world(node, m);
        Mat T;
        T(0,0)=m[0]; T(1,0)=m[1]; T(2,0)=m[2];  T(3,0)=m[12];
        T(0,1)=m[4]; T(1,1)=m[5]; T(2,1)=m[6];  T(3,1)=m[13];
        T(0,2)=m[8]; T(1,2)=m[9]; T(2,2)=m[10]; T(3,2)=m[14];
        T(0,3)=0;    T(1,3)=0;    T(2,3)=0;      T(3,3)=1;
        mesh->setTransformation(T);

        result.push_back(mesh);
      }
    }

    cgltf_free(data);
    return result;
  }

  // ---- OBJ loader (simplified, triangles only) ----

  static std::vector<std::shared_ptr<MeshNode>> loadObj(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      fprintf(stderr, "[geom2::loadFile] cannot open: %s\n", filename.c_str());
      return {};
    }

    auto mesh = std::make_shared<MeshNode>();

    // OBJ stores global vertex/normal/texcoord pools, faces reference by index.
    // We collect pools first, then build the MeshNode.
    std::vector<Vec> positions;
    std::vector<Vec> objNormals;
    std::vector<utils::Point32f> uvs;

    struct FaceVert { int v = -1, vt = -1, vn = -1; };
    struct Face { std::vector<FaceVert> verts; };
    std::vector<Face> faces;

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream ss(line);
      std::string tok;
      ss >> tok;

      if (tok == "v") {
        float x, y, z;
        ss >> x >> y >> z;
        positions.push_back(Vec(x, y, z, 1));
      } else if (tok == "vn") {
        float x, y, z;
        ss >> x >> y >> z;
        objNormals.push_back(Vec(x, y, z, 0));
      } else if (tok == "vt") {
        float u, v;
        ss >> u >> v;
        uvs.push_back(utils::Point32f(u, v));
      } else if (tok == "f") {
        Face face;
        std::string fv;
        while (ss >> fv) {
          FaceVert f;
          for (auto &c : fv) if (c == '/') c = ' ';
          std::istringstream fss(fv);
          fss >> f.v;
          if (fss.peek() != EOF) fss >> f.vt;
          if (fss.peek() != EOF) fss >> f.vn;
          if (f.v > 0) f.v--;
          if (f.vt > 0) f.vt--;
          if (f.vn > 0) f.vn--;
          face.verts.push_back(f);
        }
        faces.push_back(std::move(face));
      }
    }

    // Fan-triangulate faces into TrianglePrimitive list
    std::vector<TrianglePrimitive> tris;
    for (auto &face : faces) {
      for (size_t i = 1; i + 1 < face.verts.size(); i++) {
        tris.push_back({{face.verts[0].v, face.verts[i].v, face.verts[i+1].v},
                         {face.verts[0].vn, face.verts[i].vn, face.verts[i+1].vn},
                         {face.verts[0].vt, face.verts[i].vt, face.verts[i+1].vt}});
      }
    }

    // Bulk-ingest: moves vectors without copying
    mesh->ingest({
      .vertices  = std::move(positions),
      .normals   = objNormals.empty() ? std::nullopt : std::optional(std::move(objNormals)),
      .uvs       = uvs.empty() ? std::nullopt : std::optional(std::move(uvs)),
      .triangles = std::move(tris),
    });

    mesh->setMaterial(Material::fromColor(geom2::GeomColor(200, 200, 200, 255)));

    return {mesh};
  }

  // ---- Public API ----

  std::vector<std::shared_ptr<MeshNode>> loadFile(const std::string &filename) {
    // Dispatch by extension
    std::string ext;
    auto dot = filename.rfind('.');
    if (dot != std::string::npos) ext = filename.substr(dot);
    for (auto &c : ext) c = std::tolower(c);

    if (ext == ".glb" || ext == ".gltf") {
      return loadGltf(filename);
    } else if (ext == ".obj") {
      return loadObj(filename);
    } else {
      fprintf(stderr, "[geom2::loadFile] unsupported format: %s\n", ext.c_str());
      return {};
    }
  }

} // namespace icl::geom2
