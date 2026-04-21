/* Minimal Cycles integration test.
 * Creates a Session, builds a simple scene (ground plane + 3 spheres + light),
 * renders to PNG via a custom OutputDriver, then exits.
 * Validates that Cycles library linking works correctly. */

#include "device/device.h"
#include "scene/camera.h"
#include "scene/film.h"
#include "scene/integrator.h"
#include "scene/light.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/pass.h"
#include "scene/scene.h"
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "session/buffers.h"
#include "session/output_driver.h"
#include "session/session.h"
#include "util/log.h"
#include "util/path.h"
#include "util/unique_ptr.h"
#include "util/version.h"

#include <ICLCore/Img.h>
#include <ICLIO/FileWriter.h>

#include <cstdio>
#include <cmath>
#include <mutex>
#include <vector>

using namespace ccl;

/* Simple output driver that captures pixels into an ICL Img8u */
class ICLOutputDriver : public OutputDriver {
public:
  void write_render_tile(const Tile &tile) override {
    if (!(tile.size == tile.full_size))
      return;

    const int w = tile.size.x;
    const int h = tile.size.y;

    std::vector<float> pixels(w * h * 4);
    if (!tile.get_pass_pixels("combined", 4, pixels.data()))
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_image.setSize(icl::utils::Size(w, h));
    m_image.setChannels(3);

    icl::icl8u *r = m_image.begin(0);
    icl::icl8u *g = m_image.begin(1);
    icl::icl8u *b = m_image.begin(2);

    /* Cycles outputs bottom-up; convert to top-down + linear->sRGB gamma */
    for (int y = 0; y < h; ++y) {
      const int srcY = h - 1 - y;
      for (int x = 0; x < w; ++x) {
        const int srcIdx = (srcY * w + x) * 4;
        const int dstIdx = y * w + x;
        auto toSRGB = [](float v) -> icl::icl8u {
          v = std::max(0.0f, std::min(1.0f, v));
          return static_cast<icl::icl8u>(std::pow(v, 1.0f / 2.2f) * 255.0f + 0.5f);
        };
        r[dstIdx] = toSRGB(pixels[srcIdx + 0]);
        g[dstIdx] = toSRGB(pixels[srcIdx + 1]);
        b[dstIdx] = toSRGB(pixels[srcIdx + 2]);
      }
    }
  }

  const icl::core::Img8u &getImage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_image;
  }

private:
  mutable std::mutex m_mutex;
  icl::core::Img8u m_image;
};

/* Create a PrincipledBsdf shader */
static Shader *create_principled_shader(Scene *scene,
                                        float3 base_color,
                                        float metallic,
                                        float roughness) {
  Shader *shader = scene->create_node<Shader>();
  ShaderGraph *graph = new ShaderGraph();

  PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
  bsdf->set_base_color(base_color);
  bsdf->set_metallic(metallic);
  bsdf->set_roughness(roughness);

  graph->connect(bsdf->output("BSDF"), graph->output()->input("Surface"));

  shader->set_graph(unique_ptr<ShaderGraph>(graph));
  shader->tag_update(scene);
  return shader;
}

/* Assign a shader to all triangles of a mesh via used_shaders */
static void assign_shader(Mesh *mesh, Shader *shader) {
  /* Replace any existing shaders with just this one at index 0 */
  array<Node *> used_shaders;
  used_shaders.push_back_slow(shader);
  mesh->set_used_shaders(used_shaders);
  for (int i = 0; i < mesh->num_triangles(); ++i)
    mesh->get_shader()[i] = 0;
}

/* Build a UV sphere mesh (matching Cycles XML reader pattern) */
static Mesh *create_sphere(Scene *scene, int segments, int rings, float radius) {
  const int nv = (rings - 1) * segments + 2;
  const int nt = 2 * segments + 2 * (rings - 2) * segments;

  /* Build vertex array */
  array<float3> P;
  P.push_back_slow(make_float3(0, 0, radius)); /* Top pole */
  for (int r = 1; r < rings; ++r) {
    float phi = M_PI * float(r) / float(rings);
    for (int s = 0; s < segments; ++s) {
      float theta = 2.0f * M_PI * float(s) / float(segments);
      P.push_back_slow(make_float3(radius * sinf(phi) * cosf(theta),
                                   radius * sinf(phi) * sinf(theta),
                                   radius * cosf(phi)));
    }
  }
  P.push_back_slow(make_float3(0, 0, -radius)); /* Bottom pole */

  Mesh *mesh = scene->create_node<Mesh>();
  mesh->set_verts(P);
  mesh->resize_mesh(nv, nt);

  int *triangles = mesh->get_triangles().data();
  bool *smooth = mesh->get_smooth().data();
  int *shader_arr = mesh->get_shader().data();

  int ti = 0;
  auto addTri = [&](int a, int b, int c) {
    triangles[ti * 3 + 0] = a;
    triangles[ti * 3 + 1] = b;
    triangles[ti * 3 + 2] = c;
    smooth[ti] = true;
    shader_arr[ti] = 0;
    ti++;
  };

  /* Top cap */
  for (int s = 0; s < segments; ++s)
    addTri(0, 1 + s, 1 + (s + 1) % segments);

  /* Middle bands */
  for (int r = 0; r < rings - 2; ++r) {
    for (int s = 0; s < segments; ++s) {
      int a = 1 + r * segments + s;
      int b = 1 + r * segments + (s + 1) % segments;
      int c = 1 + (r + 1) * segments + (s + 1) % segments;
      int d = 1 + (r + 1) * segments + s;
      addTri(a, d, c);
      addTri(a, c, b);
    }
  }

  /* Bottom cap */
  int bottomPole = nv - 1;
  int lastRingStart = 1 + (rings - 2) * segments;
  for (int s = 0; s < segments; ++s)
    addTri(bottomPole, lastRingStart + (s + 1) % segments, lastRingStart + s);

  return mesh;
}

/* Build a ground plane (matching Cycles XML reader pattern) */
static Mesh *create_plane(Scene *scene, float size) {
  Mesh *mesh = scene->create_node<Mesh>();

  /* Set verts first, then resize_mesh */
  /* Ground in XZ plane (Y=0), camera Y is up */
  array<float3> P;
  P.push_back_slow(make_float3(-size, 0, -size));
  P.push_back_slow(make_float3(size, 0, -size));
  P.push_back_slow(make_float3(size, 0, size));
  P.push_back_slow(make_float3(-size, 0, size));
  mesh->set_verts(P);

  mesh->resize_mesh(4, 2);
  int *triangles = mesh->get_triangles().data();
  triangles[0] = 0; triangles[1] = 1; triangles[2] = 2;
  triangles[3] = 0; triangles[4] = 2; triangles[5] = 3;

  mesh->get_smooth()[0] = false;
  mesh->get_smooth()[1] = false;

  mesh->get_shader()[0] = 0;
  mesh->get_shader()[1] = 0;

  return mesh;
}

int main(int argc, const char **argv) {
  log_init(nullptr);
  path_init(CYCLES_INSTALL_DIR);

  printf("Cycles link test — version %s\n", CYCLES_VERSION_STRING);

  /* Parse args */
  std::string output_path = "cycles-link-test.png";
  int samples = 64;
  for (int i = 1; i < argc; ++i) {
    if (std::string("-o") == argv[i] && i + 1 < argc)
      output_path = argv[++i];
    else if (std::string("-samples") == argv[i] && i + 1 < argc)
      samples = atoi(argv[++i]);
  }

  /* Select device (prefer Metal on macOS) */
  vector<DeviceInfo> devices = Device::available_devices();
  DeviceInfo device_info = devices[0];
  for (const auto &d : devices) {
    printf("  Device: %s (%s)\n", d.description.c_str(),
           Device::string_from_type(d.type).c_str());
    if (d.type == DEVICE_METAL)
      device_info = d;
  }
  printf("Using: %s\n", device_info.description.c_str());

  /* Create session */
  SessionParams session_params;
  session_params.device = device_info;
  session_params.samples = samples;
  session_params.background = true;

  SceneParams scene_params;
  scene_params.shadingsystem = SHADINGSYSTEM_SVM;

  unique_ptr<Session> session = make_unique<Session>(session_params, scene_params);
  Scene *scene = session->scene.get();

  /* Output driver */
  ICLOutputDriver *output_driver = new ICLOutputDriver();
  session->set_output_driver(unique_ptr<OutputDriver>(output_driver));

  /* Progress callback */
  Session *session_ptr = session.get();
  session->progress.set_update_callback([session_ptr]() {
    string status, substatus;
    session_ptr->progress.get_status(status, substatus);
    double progress = session_ptr->progress.get_progress();
    printf("\rProgress %05.2f   %-40s", progress * 100.0, status.c_str());
    fflush(stdout);
  });

  /* --- Build scene --- */
  const int W = 800, H = 600;

  /* Camera */
  scene->camera->set_full_width(W);
  scene->camera->set_full_height(H);

  /* Camera: straight-on, same setup as the working triangle test */
  {
    Transform tfm = transform_translate(make_float3(0.0f, 1.0f, -4.0f));
    scene->camera->set_matrix(tfm);
    scene->camera->compute_auto_viewplane();
    scene->camera->need_flags_update = true;
    scene->camera->update(scene);
  }

  /* Integrator */
  scene->integrator->set_max_bounce(8);

  /* Film */
  scene->film->set_exposure(1.0f);

  /* Combined pass */
  Pass *pass = scene->create_node<Pass>();
  pass->set_name(ustring("combined"));
  pass->set_type(PASS_COMBINED);

  /* Background */
  {
    Shader *bg = scene->default_background;
    ShaderGraph *graph = new ShaderGraph();
    BackgroundNode *bgn = graph->create_node<BackgroundNode>();
    bgn->set_color(make_float3(0.3f, 0.5f, 0.8f));
    bgn->set_strength(1.0f);
    graph->connect(bgn->output("Background"), graph->output()->input("Surface"));
    bg->set_graph(unique_ptr<ShaderGraph>(graph));
    bg->tag_update(scene);
  }

  /* Shaders */
  Shader *ground_shader = create_principled_shader(scene, make_float3(0.4f, 0.35f, 0.3f), 0.0f, 0.8f);
  Shader *gold_shader = create_principled_shader(scene, make_float3(1.0f, 0.76f, 0.34f), 1.0f, 0.15f);
  Shader *red_shader = create_principled_shader(scene, make_float3(0.8f, 0.1f, 0.1f), 0.0f, 0.3f);
  Shader *green_shader = create_principled_shader(scene, make_float3(0.1f, 0.6f, 0.15f), 0.0f, 0.9f);

  /* Ground plane */
  {
    Mesh *mesh = create_plane(scene, 10.0f);
    assign_shader(mesh, ground_shader);
    Object *obj = scene->create_node<Object>();
    obj->set_geometry(mesh);
    obj->set_tfm(transform_identity());
  }

  /* Three PBR spheres */
  auto addSphere = [&](float3 pos, Shader *shader) {
    Mesh *mesh = create_sphere(scene, 32, 16, 1.0f);
    assign_shader(mesh, shader);
    Object *obj = scene->create_node<Object>();
    obj->set_geometry(mesh);
    obj->set_tfm(transform_translate(pos));
  };
  addSphere(make_float3(-2.5f, 1.0f, 2.0f), gold_shader);
  addSphere(make_float3(0.0f, 1.0f, 2.0f), red_shader);
  addSphere(make_float3(2.5f, 1.0f, 2.0f), green_shader);

  /* Point light */
  {
    PointLight *light = scene->create_node<PointLight>();
    light->set_strength(make_float3(800, 800, 800));
    light->set_radius(0.1f);

    Shader *light_shader = scene->create_node<Shader>();
    ShaderGraph *lgraph = new ShaderGraph();
    EmissionNode *emit = lgraph->create_node<EmissionNode>();
    emit->set_color(make_float3(1, 1, 1));
    emit->set_strength(1.0f);
    lgraph->connect(emit->output("Emission"), lgraph->output()->input("Surface"));
    light_shader->set_graph(unique_ptr<ShaderGraph>(lgraph));
    light_shader->tag_update(scene);

    array<Node *> used_shaders;
    used_shaders.push_back_slow(light_shader);
    light->set_used_shaders(used_shaders);

    Object *light_obj = scene->create_node<Object>();
    light_obj->set_geometry(light);
    light_obj->set_tfm(transform_translate(make_float3(3.0f, 5.0f, -2.0f)));
  }

  /* --- Render --- */
  BufferParams buffer_params;
  buffer_params.width = W;
  buffer_params.height = H;
  buffer_params.full_width = W;
  buffer_params.full_height = H;

  session->reset(session_params, buffer_params);
  session->start();
  session->wait();
  printf("\n");

  /* Save result */
  const icl::core::Img8u &image = output_driver->getImage();
  if (image.getWidth() > 0) {
    icl::io::FileWriter writer(output_path);
    writer.write(&image);
    printf("Saved %dx%d image to %s\n", image.getWidth(), image.getHeight(),
           output_path.c_str());
  }
  else {
    fprintf(stderr, "ERROR: No image received from Cycles!\n");
    return 1;
  }

  return 0;
}
