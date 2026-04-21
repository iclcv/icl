// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SceneSynchronizer.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Primitive.h>
#include <icl/geom/Camera.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

// Qt/Cycles macro conflicts
#undef emit
#undef ERROR
#undef LOG_LEVEL

#include <icl/core/CCFunctions.h>

#include "scene/scene.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/pointcloud.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "scene/camera.h"
#include "scene/light.h"
#include "scene/background.h"
#include "scene/integrator.h"
#include "scene/image.h"
#include "scene/image_loader.h"
#include "util/image_metadata.h"
#include "util/transform.h"
#include "util/unique_ptr.h"
#include "kernel/types.h"

#include <cmath>
#include <cstring>

using namespace ccl;

namespace icl::geom2 {

  // ---- Transform helpers ----

  static Transform nodeTransformToCycles(const Node *node, float scale) {
    Transform tfm = transform_identity();
    if (node->hasTransformation(true)) {
      auto m = node->getTransformation(true);
      const float *d = m.data();
      // ICL row-major 4x4 -> Cycles Transform (row-major 4x3, rows=x,y,z)
      tfm.x = make_float4(d[0], d[1], d[2], d[3] * scale);
      tfm.y = make_float4(d[4], d[5], d[6], d[7] * scale);
      tfm.z = make_float4(d[8], d[9], d[10], d[11] * scale);
    }
    return tfm;
  }

  // ---- ICLImageLoader: bridges ICL Image to Cycles ImageManager ----

  class ICLImageLoader : public ImageLoader {
    core::Image m_img;
    std::string m_name;

  public:
    ICLImageLoader(const core::Image &img, const std::string &name)
      : m_img(img), m_name(name) {}

    bool load_metadata(ImageMetaData &meta) override {
      if (m_img.isNull()) return false;
      meta.width = m_img.getWidth();
      meta.height = m_img.getHeight();
      meta.channels = std::min(m_img.getChannels(), 4);
      meta.type = IMAGE_DATA_TYPE_BYTE4;
      meta.colorspace_file_hint = "sRGB";
      meta.finalize();
      return true;
    }

    bool load_pixels(const ImageMetaData &meta, void *pixels) override {
      if (m_img.isNull()) return false;
      const int w = m_img.getWidth();
      const int h = m_img.getHeight();
      const int ch = m_img.getChannels();
      uchar *dst = (uchar *)pixels;

      const auto &img8u = m_img.as<icl8u>();
      if (ch == 4) {
        core::planarToInterleaved(&img8u, dst);
      } else {
        std::memset(dst, 255, w * h * 4);
        for (int c = 0; c < ch; c++) {
          const icl8u *src = img8u.getData(c);
          for (int i = 0; i < w * h; i++)
            dst[i * 4 + c] = src[i];
        }
      }
      meta.conform_pixels(pixels);
      return true;
    }

    string name() const override { return m_name; }

    bool equals(const ImageLoader &other) const override {
      const auto *o = dynamic_cast<const ICLImageLoader *>(&other);
      return o && o->m_img.ptr() == m_img.ptr();
    }
  };

  // ---- Material/shader creation ----

  static ImageTextureNode *createImageTexNode(ShaderGraph *graph, ccl::Scene *scene,
                                               const core::Image &img,
                                               const std::string &name,
                                               bool isLinear = false) {
    auto *tex = graph->create_node<ImageTextureNode>();
    tex->set_filename(ustring("icl_inline_" + name));
    tex->set_interpolation(INTERPOLATION_LINEAR);
    tex->set_extension(EXTENSION_REPEAT);
    tex->set_colorspace(ustring(isLinear ? "scene_linear" : "sRGB"));
    tex->set_alpha_type(IMAGE_ALPHA_AUTO);

    ImageParams params;
    params.interpolation = INTERPOLATION_LINEAR;
    params.extension = EXTENSION_REPEAT;
    params.colorspace = ustring(isLinear ? "scene_linear" : "sRGB");

    auto loader = make_unique<ICLImageLoader>(img, name);
    tex->handle = scene->image_manager->add_image(std::move(loader), params);
    return tex;
  }

  static Shader *createDefaultShader(ccl::Scene *scene) {
    Shader *shader = scene->create_node<Shader>();
    ShaderGraph *graph = new ShaderGraph();

    PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
    bsdf->set_base_color(make_float3(0.8f, 0.8f, 0.8f));
    bsdf->set_roughness(0.5f);

    graph->connect(bsdf->output("BSDF"), graph->output()->input("Surface"));

    shader->set_graph(unique_ptr<ShaderGraph>(graph));
    shader->tag_update(scene);
    return shader;
  }

  static Shader *createPrincipledShader(ccl::Scene *scene, const geom::Material *mat) {
    Shader *shader = scene->create_node<Shader>();
    ShaderGraph *graph = new ShaderGraph();

    PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
    bsdf->set_base_color(make_float3(mat->baseColor[0], mat->baseColor[1], mat->baseColor[2]));
    bsdf->set_metallic(mat->metallic);
    bsdf->set_roughness(mat->roughness);

    // Shared UV node for all texture lookups
    TextureCoordinateNode *texCoordNode = nullptr;
    auto getUV = [&]() -> ShaderOutput * {
      if (!texCoordNode)
        texCoordNode = graph->create_node<TextureCoordinateNode>();
      return texCoordNode->output("UV");
    };

    // Base color texture
    if (mat->textures && !mat->textures->baseColorMap.isNull()) {
      auto *tex = createImageTexNode(graph, scene, mat->textures->baseColorMap,
                                      mat->name + "_baseColor");
      graph->connect(getUV(), tex->input("Vector"));
      graph->connect(tex->output("Color"), bsdf->input("Base Color"));
      auto alphaMode = mat->transmission ? mat->transmission->alphaMode : geom::Material::Opaque;
      if (alphaMode != geom::Material::Opaque)
        graph->connect(tex->output("Alpha"), bsdf->input("Alpha"));
    }

    // Metallic-roughness texture (glTF: G=roughness, B=metallic)
    if (mat->textures && !mat->textures->metallicRoughnessMap.isNull()) {
      auto *tex = createImageTexNode(graph, scene, mat->textures->metallicRoughnessMap,
                                      mat->name + "_metalRough", true);
      graph->connect(getUV(), tex->input("Vector"));
      auto *sep = graph->create_node<SeparateColorNode>();
      graph->connect(tex->output("Color"), sep->input("Color"));
      graph->connect(sep->output("Blue"), bsdf->input("Metallic"));
      graph->connect(sep->output("Green"), bsdf->input("Roughness"));
    }

    // Normal map
    if (mat->textures && !mat->textures->normalMap.isNull()) {
      auto *tex = createImageTexNode(graph, scene, mat->textures->normalMap,
                                      mat->name + "_normal", true);
      graph->connect(getUV(), tex->input("Vector"));
      auto *nmap = graph->create_node<NormalMapNode>();
      nmap->set_space(NODE_NORMAL_MAP_TANGENT);
      nmap->set_strength(1.0f);
      graph->connect(tex->output("Color"), nmap->input("Color"));
      graph->connect(nmap->output("Normal"), bsdf->input("Normal"));
    }

    ShaderNode *surfaceNode = bsdf;
    const char *surfaceOutput = "BSDF";

    // Emissive
    bool hasEmissiveMap = mat->textures && !mat->textures->emissiveMap.isNull();
    float emStrength = (mat->emissive[0] + mat->emissive[1] + mat->emissive[2]) / 3.0f;
    if (emStrength > 0.001f || hasEmissiveMap) {
      bsdf->set_emission_color(make_float3(mat->emissive[0], mat->emissive[1], mat->emissive[2]));
      bsdf->set_emission_strength(1.0f);
      if (hasEmissiveMap) {
        auto *tex = createImageTexNode(graph, scene, mat->textures->emissiveMap,
                                        mat->name + "_emissive");
        graph->connect(getUV(), tex->input("Vector"));
        graph->connect(tex->output("Color"), bsdf->input("Emission Color"));
      }
    }

    // Reflectivity: mix in a glossy mirror layer
    if (mat->reflectivity > 0.001f) {
      auto *glossy = graph->create_node<GlossyBsdfNode>();
      glossy->set_roughness(mat->roughness * 0.1f);  // near-mirror
      glossy->set_color(make_float3(1, 1, 1));
      // Share normal map with glossy node
      if (mat->textures && !mat->textures->normalMap.isNull()) {
        auto *nInput = bsdf->input("Normal");
        if (nInput && nInput->link)
          graph->connect(nInput->link, glossy->input("Normal"));
      }

      auto *mixRefl = graph->create_node<MixClosureNode>();
      mixRefl->set_fac(mat->reflectivity);
      graph->connect(surfaceNode->output(surfaceOutput), mixRefl->input("Closure1"));
      graph->connect(glossy->output("BSDF"), mixRefl->input("Closure2"));
      surfaceNode = mixRefl;
      surfaceOutput = "Closure";
    }

    // Transmission (glass)
    if (mat->isTransmissive()) {
      auto *glass = graph->create_node<GlassBsdfNode>();
      glass->set_roughness(0.0f);
      glass->set_IOR(mat->transmission ? mat->transmission->ior : 1.5f);
      glass->set_color(make_float3(mat->baseColor[0], mat->baseColor[1], mat->baseColor[2]));
      if (mat->textures && !mat->textures->baseColorMap.isNull()) {
        auto *bcInput = bsdf->input("Base Color");
        if (bcInput && bcInput->link)
          graph->connect(bcInput->link, glass->input("Color"));
      }
      if (mat->textures && !mat->textures->normalMap.isNull()) {
        auto *nInput = bsdf->input("Normal");
        if (nInput && nInput->link)
          graph->connect(nInput->link, glass->input("Normal"));
      }

      auto *mix = graph->create_node<MixClosureNode>();
      mix->set_fac(mat->transmission ? mat->transmission->transmission : 0.0f);
      graph->connect(surfaceNode->output(surfaceOutput), mix->input("Closure1"));
      graph->connect(glass->output("BSDF"), mix->input("Closure2"));
      graph->connect(mix->output("Closure"), graph->output()->input("Surface"));
    } else {
      graph->connect(surfaceNode->output(surfaceOutput), graph->output()->input("Surface"));
    }

    // Volume absorption for colored glass
    if (mat->isTransmissive() && mat->transmission) {
      float attDist = mat->transmission->attenuationDistance;
      if (attDist > 0.0f && attDist < 1e10f) {
        auto *absNode = graph->create_node<AbsorptionVolumeNode>();
        const auto &attCol = mat->transmission->attenuationColor;
        absNode->set_color(make_float3(attCol[0], attCol[1], attCol[2]));
        absNode->set_density(1.0f / attDist);
        graph->connect(absNode->output("Volume"), graph->output()->input("Volume"));
      }
    }

    shader->set_graph(unique_ptr<ShaderGraph>(graph));
    shader->tag_update(scene);
    return shader;
  }

  // ---- Tessellation: geom2 primitives -> Cycles Mesh ----
  //
  // Follows the same pattern as the working geom SceneSynchronizer:
  // resize_mesh(), fill via direct array access, set shader indices,
  // then tag all modified arrays.

  static void tessellateToMesh(const GeometryNode *node, ccl::Mesh *mesh, float scale) {
    const auto &srcVerts = node->getVertices();
    const auto &texCoords = node->getTexCoords();

    if (srcVerts.empty()) return;

    // Count triangles
    int numTris = (int)node->getTriangles().size();
    numTris += (int)node->getQuads().size() * 2;

    // Set vertices (applying scale)
    array<float3> P;
    for (const auto &v : srcVerts)
      P.push_back_slow(make_float3(v[0] * scale, v[1] * scale, v[2] * scale));
    mesh->set_verts(P);

    // Allocate triangle storage
    mesh->resize_mesh(srcVerts.size(), numTris);
    int *triangles = mesh->get_triangles().data();
    bool *smooth = mesh->get_smooth().data();

    bool hasUVs = !texCoords.empty();
    auto mat = node->getMaterial();
    bool useSmooth = mat && mat->smoothShading;
    int ti = 0;

    // Collect per-corner UV indices alongside triangles
    std::vector<int> uvCornerIndices;
    if (hasUVs) uvCornerIndices.reserve(numTris * 3);

    for (const auto &t : node->getTriangles()) {
      triangles[ti * 3 + 0] = t.v[0];
      triangles[ti * 3 + 1] = t.v[1];
      triangles[ti * 3 + 2] = t.v[2];
      smooth[ti] = useSmooth;
      mesh->get_shader()[ti] = 0;
      if (hasUVs) {
        uvCornerIndices.push_back(t.t[0]);
        uvCornerIndices.push_back(t.t[1]);
        uvCornerIndices.push_back(t.t[2]);
      }
      ti++;
    }
    for (const auto &q : node->getQuads()) {
      // Quad -> 2 triangles
      triangles[ti * 3 + 0] = q.v[0];
      triangles[ti * 3 + 1] = q.v[1];
      triangles[ti * 3 + 2] = q.v[2];
      smooth[ti] = useSmooth;
      mesh->get_shader()[ti] = 0;
      if (hasUVs) {
        uvCornerIndices.push_back(q.t[0]);
        uvCornerIndices.push_back(q.t[1]);
        uvCornerIndices.push_back(q.t[2]);
      }
      ti++;

      triangles[ti * 3 + 0] = q.v[0];
      triangles[ti * 3 + 1] = q.v[2];
      triangles[ti * 3 + 2] = q.v[3];
      smooth[ti] = useSmooth;
      mesh->get_shader()[ti] = 0;
      if (hasUVs) {
        uvCornerIndices.push_back(q.t[0]);
        uvCornerIndices.push_back(q.t[2]);
        uvCornerIndices.push_back(q.t[3]);
      }
      ti++;
    }

    // Upload per-corner UV coordinates as ATTR_STD_UV
    if (hasUVs && (int)uvCornerIndices.size() == numTris * 3) {
      Attribute *uv_attr = mesh->attributes.add(ATTR_STD_UV, ustring("UVMap"));
      float2 *uv_data = uv_attr->data_float2();
      for (int i = 0; i < numTris * 3; i++) {
        int idx = uvCornerIndices[i];
        if (idx >= 0 && idx < (int)texCoords.size()) {
          uv_data[i] = make_float2(texCoords[idx].x, texCoords[idx].y);
        } else {
          uv_data[i] = make_float2(0.0f, 0.0f);
        }
      }
    }

    // Tag mesh as modified so Cycles processes the new data.
    mesh->tag_verts_modified();
    mesh->tag_triangles_modified();
    mesh->tag_shader_modified();
    mesh->tag_smooth_modified();
  }

  // ---- SceneSynchronizer implementation ----

  SceneSynchronizer::SyncResult
  SceneSynchronizer::synchronize(const Scene2 &scene, int camIndex,
                                  ccl::Scene *cclScene, float sceneScale) {
    bool anyGeomChanged = false, anyTransformChanged = false;

    // Mark all entries as unvisited
    for (auto &[_, entry] : m_entries)
      entry.visited = false;

    // Walk scene graph
    for (int i = 0; i < scene.getNodeCount(); i++)
      walkNode(scene.getNode(i), cclScene, sceneScale, anyGeomChanged, anyTransformChanged);

    // Remove stale objects
    bool removedAny = false;
    removeStaleNodes(cclScene, removedAny);
    if (removedAny) anyGeomChanged = true;

    // Sync camera
    if (camIndex >= 0 && camIndex < scene.getCameraCount())
      syncCamera(scene.getCamera(camIndex), cclScene, sceneScale);

    // Sync lights
    syncLights(scene, cclScene, sceneScale);

    // Background: create once, then update strength in-place
    float bgStrength = m_backgroundStrength;
    if (!m_bgNode) {
      // First time: build gradient background shader graph
      Shader *bg = cclScene->default_background;
      ShaderGraph *graph = new ShaderGraph();

      auto *bgn = graph->create_node<BackgroundNode>();
      bgn->set_strength(bgStrength);

      // Gradient: ray direction Y -> blend ground/horizon/zenith
      // Identical to geom SceneSynchronizer using Sky default colors.
      // Negate Y: Cycles background Normal points inward
      auto *geomNode = graph->create_node<ccl::GeometryNode>();
      auto *sepXYZ = graph->create_node<SeparateXYZNode>();
      graph->connect(geomNode->output("Normal"), sepXYZ->input("Vector"));
      auto *flipY = graph->create_node<MathNode>();
      flipY->set_math_type(NODE_MATH_MULTIPLY);
      flipY->set_value2(-1.0f);
      graph->connect(sepXYZ->output("Y"), flipY->input("Value1"));

      // Sky colors (exact match: geom::Sky default gradient)
      auto *zenithCol = graph->create_node<ColorNode>();
      zenithCol->set_value(make_float3(0.55f, 0.65f, 0.85f));
      auto *horizCol = graph->create_node<ColorNode>();
      horizCol->set_value(make_float3(0.95f, 0.93f, 0.90f));
      auto *groundCol = graph->create_node<ColorNode>();
      groundCol->set_value(make_float3(0.30f, 0.27f, 0.25f));

      // Upper hemisphere: tUp = pow(max(Y, 0), 0.4)
      auto *clampY = graph->create_node<MathNode>();
      clampY->set_math_type(NODE_MATH_MAXIMUM);
      clampY->set_value2(0.0f);
      graph->connect(flipY->output("Value"), clampY->input("Value1"));

      auto *powNode = graph->create_node<MathNode>();
      powNode->set_math_type(NODE_MATH_POWER);
      powNode->set_value2(0.4f);  // horizonSharpness
      graph->connect(clampY->output("Value"), powNode->input("Value1"));

      // skyAbove = mix(horizon, zenith, tUp)
      auto *mixUp = graph->create_node<MixColorNode>();
      mixUp->set_blend_type(NODE_MIX_BLEND);
      graph->connect(horizCol->output("Color"), mixUp->input("A"));
      graph->connect(zenithCol->output("Color"), mixUp->input("B"));
      graph->connect(powNode->output("Value"), mixUp->input("Factor"));

      // Lower hemisphere: tDown = clamp(-flippedY * 3, 0, 1)
      auto *negY = graph->create_node<MathNode>();
      negY->set_math_type(NODE_MATH_MULTIPLY);
      negY->set_value2(-1.0f);
      graph->connect(flipY->output("Value"), negY->input("Value1"));

      auto *mulThree = graph->create_node<MathNode>();
      mulThree->set_math_type(NODE_MATH_MULTIPLY);
      mulThree->set_value2(3.0f);
      mulThree->set_use_clamp(true);
      graph->connect(negY->output("Value"), mulThree->input("Value1"));

      // skyBelow = mix(horizon, ground, tDown)
      auto *mixDown = graph->create_node<MixColorNode>();
      mixDown->set_blend_type(NODE_MIX_BLEND);
      graph->connect(horizCol->output("Color"), mixDown->input("A"));
      graph->connect(groundCol->output("Color"), mixDown->input("B"));
      graph->connect(mulThree->output("Value"), mixDown->input("Factor"));

      // Select above/below: isAbove = (flippedY > 0) ? 1 : 0
      auto *isAbove = graph->create_node<MathNode>();
      isAbove->set_math_type(NODE_MATH_GREATER_THAN);
      isAbove->set_value2(0.0f);
      graph->connect(flipY->output("Value"), isAbove->input("Value1"));

      // finalColor = mix(skyBelow, skyAbove, isAbove)
      auto *mixFinal = graph->create_node<MixColorNode>();
      mixFinal->set_blend_type(NODE_MIX_BLEND);
      graph->connect(mixDown->output("Result"), mixFinal->input("A"));
      graph->connect(mixUp->output("Result"), mixFinal->input("B"));
      graph->connect(isAbove->output("Value"), mixFinal->input("Factor"));

      graph->connect(mixFinal->output("Result"), bgn->input("Color"));
      graph->connect(bgn->output("Background"), graph->output()->input("Surface"));

      m_bgNode = static_cast<void*>(bgn);

      bg->set_graph(unique_ptr<ShaderGraph>(graph));
      bg->tag_update(cclScene);
    } else {
      // Update strength in-place (no shader graph rebuild)
      auto *bgn = static_cast<BackgroundNode*>(m_bgNode);
      if (bgn->get_strength() != bgStrength) {
        bgn->set_strength(bgStrength);
        cclScene->default_background->tag_update(cclScene);
      }
    }

    if (anyGeomChanged) return SyncResult::GeometryChanged;
    if (anyTransformChanged) return SyncResult::TransformOnly;
    return SyncResult::NoChange;
  }

  void SceneSynchronizer::walkNode(const Node *node, ccl::Scene *cclScene,
                                    float sceneScale,
                                    bool &anyGeomChanged, bool &anyTransformChanged) {
    if (!node || !node->isVisible()) return;

    if (auto *group = dynamic_cast<const GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        walkNode(group->getChild(i), cclScene, sceneScale, anyGeomChanged, anyTransformChanged);
    }
    else if (auto *geom = dynamic_cast<const GeometryNode*>(node)) {
      auto &entry = m_entries[geom];
      entry.geomNode = geom;
      entry.visited = true;

      // Check for changes
      size_t vc = geom->getVertices().size();
      size_t tc = geom->getTriangles().size() + geom->getQuads().size();
      bool geomDirty = entry.geometryDirty || (vc != entry.vertexCount) || (tc != entry.primitiveCount);

      if (!entry.geometry || geomDirty) {
        syncGeometry(entry, cclScene, sceneScale);
        syncMaterial(entry, cclScene);
        entry.vertexCount = vc;
        entry.primitiveCount = tc;
        entry.geometryDirty = false;
        anyGeomChanged = true;
      }

      if (entry.transformDirty || geomDirty) {
        syncTransform(entry, cclScene, sceneScale);
        entry.transformDirty = false;
        anyTransformChanged = true;
      }
    }
    // LightNodes handled separately in syncLights()
  }

  void SceneSynchronizer::syncGeometry(ObjectEntry &entry, ccl::Scene *cclScene,
                                        float sceneScale) {
    if (!entry.geometry) {
      auto *mesh = cclScene->create_node<ccl::Mesh>();
      entry.geometry = mesh;
      entry.object = cclScene->create_node<ccl::Object>();
      entry.object->set_geometry(mesh);
    }
    tessellateToMesh(entry.geomNode, static_cast<ccl::Mesh*>(entry.geometry), sceneScale);
  }

  void SceneSynchronizer::syncMaterial(ObjectEntry &entry, ccl::Scene *cclScene) {
    // Remove old shader if exists
    if (entry.shader) {
      cclScene->delete_node(entry.shader);
      entry.shader = nullptr;
    }

    auto mat = entry.geomNode->getMaterial();
    if (mat) {
      entry.shader = createPrincipledShader(cclScene, mat.get());
    } else {
      entry.shader = createDefaultShader(cclScene);
    }

    // Assign shader to geometry
    array<ccl::Node*> used_shaders;
    used_shaders.push_back_slow(entry.shader);
    entry.geometry->set_used_shaders(used_shaders);
  }

  void SceneSynchronizer::syncTransform(ObjectEntry &entry, ccl::Scene *cclScene,
                                         float sceneScale) {
    Transform tfm = nodeTransformToCycles(entry.geomNode, sceneScale);
    entry.object->set_tfm(tfm);
    entry.object->tag_update(cclScene);
  }

  void SceneSynchronizer::syncCamera(const geom::Camera &cam, ccl::Scene *cclScene,
                                      float sceneScale) {
    ccl::Camera *cclCam = cclScene->camera;

    // Resolution
    auto rp = cam.getRenderParams();
    int w = rp.chipSize.width, h = rp.chipSize.height;
    if (w > 0 && h > 0) {
      cclCam->set_full_width(w);
      cclCam->set_full_height(h);
    }

    // Camera position and orientation
    auto pos = cam.getPosition();
    auto norm = cam.getNorm();
    auto up = cam.getUp();

    float3 camPos = make_float3(pos[0]*sceneScale, pos[1]*sceneScale, pos[2]*sceneScale);
    float3 forward = normalize(make_float3(norm[0], norm[1], norm[2]));

    // ICL camera axes: horiz = cross(up, norm), up, norm.
    float3 iclUp = normalize(make_float3(up[0], up[1], up[2]));
    float3 horiz = cross(iclUp, forward);
    float horizLen = len(horiz);
    if (horizLen < 1e-6f) {
      horiz = make_float3(1, 0, 0);
      if (fabsf(dot(forward, horiz)) > 0.9f)
        horiz = make_float3(0, 1, 0);
      horiz = normalize(cross(horiz, forward));
    } else {
      horiz = horiz * (1.0f / horizLen);
    }
    // Reorthogonalize up
    float3 upVec = cross(forward, horiz);

    // Cycles Transform: row-stored, columns are basis vectors.
    //   column 0 =  horiz  (ICL image +X)
    //   column 1 = -upVec  (negated: ICL up = visual down in Cycles)
    //   column 2 =  forward (view direction)
    //   column 3 =  camPos
    Transform tfm;
    tfm.x = make_float4( horiz.x, -upVec.x, forward.x, camPos.x);
    tfm.y = make_float4( horiz.y, -upVec.y, forward.y, camPos.y);
    tfm.z = make_float4( horiz.z, -upVec.z, forward.z, camPos.z);

    cclCam->set_matrix(tfm);
    cclCam->set_camera_type(CAMERA_PERSPECTIVE);

    // Vertical FOV: fov = 2 * atan(h / (2 * f * my))
    float f = cam.getFocalLength();
    float my = cam.getSamplingResolutionY();
    if (f > 0 && my > 0 && h > 0) {
      float fov = 2.0f * std::atan(float(h) / (2.0f * f * my));
      fov = std::max(0.087f, std::min(2.97f, fov));
      cclCam->set_fov(fov);
    }

    // Clip planes
    float nearClip = rp.clipZNear * sceneScale;
    float farClip = rp.clipZFar * sceneScale;
    if (nearClip <= 0) nearClip = 0.01f;
    if (farClip <= nearClip) farClip = nearClip * 10000.0f;
    if (farClip / nearClip > 1e6f) farClip = nearClip * 1e6f;
    cclCam->set_nearclip(nearClip);
    cclCam->set_farclip(farClip);

    cclCam->compute_auto_viewplane();
    cclCam->need_flags_update = true;
  }

  void SceneSynchronizer::syncLights(const Scene2 &scene, ccl::Scene *cclScene,
                                      float sceneScale) {
    if (m_lightsCreated) return;
    m_lightsCreated = true;

    for (int i = 0; i < scene.getLightCount(); i++) {
      const auto *light = scene.getLight(i);
      if (!light || !light->isVisible()) continue;

      auto c = light->getColor();
      float intensity = light->getIntensity();

      auto t = light->getTransformation(true);
      // Translation in column 3 of the 4x4 transform (row-major, (row,col) convention)
      float3 pos = make_float3(t(0,3)*sceneScale, t(1,3)*sceneScale, t(2,3)*sceneScale);

      // Physical inverse-square falloff calibration.
      // The geom version's SceneLight::getDiffuse() returns 0-1 range colors,
      // but the sync divides by 255 again — an accidental double-normalization.
      // The 300.0 factor was calibrated with that double /255 baked in.
      // Since LightNode::getColor() returns 0-255 range (single /255 below),
      // we divide the calibration by 255 to match: 300/255 ≈ 1.176.
      float typicalDist = 500.0f * sceneScale;
      float physIntensity = intensity * (300.0f / 255.0f) * typicalDist * typicalDist;

      PointLight *cclLight = cclScene->create_node<PointLight>();
      cclLight->set_strength(make_float3(
          c[0] / 255.0f * physIntensity,
          c[1] / 255.0f * physIntensity,
          c[2] / 255.0f * physIntensity));
      cclLight->set_radius(0.1f * sceneScale);

      // Emission shader for light
      Shader *lightShader = cclScene->create_node<Shader>();
      ShaderGraph *graph = new ShaderGraph();
      EmissionNode *emNode = graph->create_node<EmissionNode>();
      emNode->set_color(make_float3(1, 1, 1));
      emNode->set_strength(1.0f);
      graph->connect(emNode->output("Emission"), graph->output()->input("Surface"));
      lightShader->set_graph(unique_ptr<ShaderGraph>(graph));
      lightShader->tag_update(cclScene);

      array<ccl::Node *> used_shaders;
      used_shaders.push_back_slow(lightShader);
      cclLight->set_used_shaders(used_shaders);

      // Object for the light (hidden from camera to avoid visible sphere)
      ccl::Object *lightObj = cclScene->create_node<ccl::Object>();
      lightObj->set_geometry(cclLight);
      lightObj->set_tfm(transform_translate(pos));
      lightObj->set_visibility(PATH_RAY_ALL_VISIBILITY & ~PATH_RAY_CAMERA);
    }
  }

  void SceneSynchronizer::removeStaleNodes(ccl::Scene *cclScene, bool &anyChanged) {
    std::vector<const GeometryNode *> toRemove;
    for (auto &[ptr, entry] : m_entries) {
      if (!entry.visited) {
        if (entry.object) cclScene->delete_node(entry.object);
        if (entry.geometry) cclScene->delete_node(entry.geometry);
        if (entry.shader) cclScene->delete_node(entry.shader);
        toRemove.push_back(ptr);
        anyChanged = true;
      }
    }
    for (auto *ptr : toRemove)
      m_entries.erase(ptr);
  }

  void SceneSynchronizer::invalidateAll() {
    for (auto &[_, entry] : m_entries) {
      entry.geometryDirty = true;
      entry.transformDirty = true;
    }
    // Note: don't reset m_lightsCreated - lights don't need rebuild on geometry change
  }

  void SceneSynchronizer::invalidateTransforms() {
    for (auto &[_, entry] : m_entries)
      entry.transformDirty = true;
  }

  void SceneSynchronizer::invalidateNode(const Node *node) {
    if (auto *geom = dynamic_cast<const GeometryNode*>(node)) {
      auto it = m_entries.find(geom);
      if (it != m_entries.end()) {
        it->second.geometryDirty = true;
        it->second.transformDirty = true;
      }
    }
  }

  bool SceneSynchronizer::hasPendingChanges() const {
    for (auto &[_, entry] : m_entries)
      if (entry.geometryDirty || entry.transformDirty) return true;
    return false;
  }

} // namespace icl::geom2
