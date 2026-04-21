// Raytracing backend benchmark — compares CPU, OpenCL, and Metal RT
// Usage: ./benchmark-raytracer [resolution] [num_objects]
//   defaults: 1280x720, 20 objects

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <Raytracing/SceneRaytracer.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace std::chrono;

// Simple LCG for deterministic scene generation
static uint32_t rngState = 42;
static float randf(float lo, float hi) {
  rngState = rngState * 1103515245u + 12345u;
  float t = float((rngState >> 8) & 0xFFFF) / 65536.0f;
  return lo + t * (hi - lo);
}

static void buildScene(Scene &scene, int numObjects) {
  // Camera
  Camera cam(Vec(0, -600, 300, 1), Vec(0, 0.6, -0.3, 1).normalized(),
             Vec(0, 0, 1, 1));
  cam.setResolution(Size(1280, 720)); // overridden by caller
  scene.addCamera(cam);

  // Ground plane (large flat box)
  SceneObject *ground = SceneObject::cube(0, 0, -15, 600);
  ground->scale(3.0, 3.0, 0.05);
  ground->setColor(Primitive::quad, GeomColor(180, 180, 170, 255));
  ground->setVisible(Primitive::line, false);
  ground->setVisible(Primitive::vertex, false);
  ground->createAutoNormals(true);
  scene.addObject(ground);

  // Scatter objects
  rngState = 42;
  for (int i = 0; i < numObjects; i++) {
    float x = randf(-250, 250);
    float y = randf(-250, 250);
    float sz = randf(20, 60);
    int type = (int)randf(0, 3);

    SceneObject *obj;
    if (type == 0) {
      obj = SceneObject::cube(x, y, sz / 2, sz);
    } else if (type == 1) {
      obj = SceneObject::sphere(x, y, sz / 2, sz / 2, 16, 16);
    } else {
      obj = SceneObject::sphere(x, y, sz / 2, sz / 2, 8, 8);
      obj->scale(1.0, 1.0, sz / (sz / 2)); // elongate into a cylinder-ish shape
    }
    GeomColor c(randf(40, 240), randf(40, 240), randf(40, 240), 255);
    obj->setColor(Primitive::quad, c);
    obj->setColor(Primitive::triangle, c);
    obj->setVisible(Primitive::line, false);
    obj->setVisible(Primitive::vertex, false);
    obj->createAutoNormals(true);
    if (randf(0, 1) > 0.6f)
      obj->setReflectivity(randf(0.1f, 0.6f), true);
    scene.addObject(obj);
  }

  // Lights
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(400, -300, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 255, 255, 255));
  scene.getLight(0).setAmbient(GeomColor(30, 30, 30, 255));
  scene.getLight(0).setSpecular(GeomColor(255, 255, 255, 255));

  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 400, 400, 1));
  scene.getLight(1).setDiffuse(GeomColor(180, 180, 200, 255));
  scene.getLight(1).setAmbient(GeomColor(0, 0, 0, 255));
}

struct BenchResult {
  std::string backend;
  bool available;
  double firstFrameMs;  // includes BLAS/TLAS build
  double directAvgMs;   // avg of steady-state direct lighting frames
  double ptAvgMs;       // avg of path tracing passes
  int ptPasses;
};

static BenchResult benchBackend(Scene &scene, const std::string &name,
                                int w, int h, int warmup, int frames,
                                int ptFrames) {
  BenchResult r;
  r.backend = name;
  r.available = false;
  r.firstFrameMs = r.directAvgMs = r.ptAvgMs = 0;
  r.ptPasses = 0;

  // Set resolution
  Camera cam = scene.getCamera(0);
  cam.setResolution(Size(w, h));
  scene.getCamera(0) = cam;

  icl::rt::SceneRaytracer rt(scene, name);
  if (std::string(rt.backendName()).find("CPU") != std::string::npos &&
      name != "cpu") {
    // Fell back to CPU — requested backend not available
    printf("  %-20s  NOT AVAILABLE (fell back to CPU)\n", name.c_str());
    return r;
  }
  r.available = true;
  r.backend = rt.backendName();

  // First frame (cold — includes accel struct build)
  auto t0 = high_resolution_clock::now();
  rt.render(0);
  auto t1 = high_resolution_clock::now();
  r.firstFrameMs = duration<double, std::milli>(t1 - t0).count();

  // Warmup
  for (int i = 0; i < warmup; i++) rt.render(0);

  // Direct lighting benchmark
  auto tStart = high_resolution_clock::now();
  for (int i = 0; i < frames; i++) rt.render(0);
  auto tEnd = high_resolution_clock::now();
  r.directAvgMs = duration<double, std::milli>(tEnd - tStart).count() / frames;

  // Path tracing benchmark
  rt.setPathTracing(true);
  rt.invalidateAll(); // forces accumulation reset

  tStart = high_resolution_clock::now();
  for (int i = 0; i < ptFrames; i++) rt.render(0);
  tEnd = high_resolution_clock::now();
  r.ptPasses = rt.getAccumulatedFrames();
  r.ptAvgMs = duration<double, std::milli>(tEnd - tStart).count() / ptFrames;

  return r;
}

// Simulate physics: move objects each frame (transform-only changes)
static double benchDynamic(Scene &scene, const std::string &name,
                           int w, int h, int frames) {
  Camera cam = scene.getCamera(0);
  cam.setResolution(Size(w, h));
  scene.getCamera(0) = cam;

  icl::rt::SceneRaytracer rt(scene, name);
  if (std::string(rt.backendName()).find("CPU") != std::string::npos &&
      name != "cpu")
    return -1;

  // First frame to build everything
  rt.render(0);

  // Warmup with movement
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < scene.getObjectCount(); j++) {
      scene.getObject(j)->translate(0, 0, 0.1f);
    }
    rt.invalidateTransforms();
    rt.render(0);
  }

  // Measure
  auto t0 = high_resolution_clock::now();
  for (int i = 0; i < frames; i++) {
    // Move every object slightly (simulates physics transforms)
    float dz = (i % 2 == 0) ? 0.1f : -0.1f;
    for (int j = 1; j < scene.getObjectCount(); j++) {
      scene.getObject(j)->translate(0, 0, dz);
    }
    rt.invalidateTransforms();
    rt.render(0);
  }
  auto t1 = high_resolution_clock::now();
  return duration<double, std::milli>(t1 - t0).count() / frames;
}

int main(int argc, char **argv) {
  int w = 1280, h = 720;
  int numObjects = 20;
  if (argc > 1) {
    if (sscanf(argv[1], "%dx%d", &w, &h) != 2) {
      w = atoi(argv[1]); h = w * 9 / 16;
    }
  }
  if (argc > 2) numObjects = atoi(argv[2]);

  printf("=== Raytracing Backend Benchmark ===\n");
  printf("Resolution: %dx%d  Objects: %d\n\n", w, h, numObjects);

  Scene scene;
  buildScene(scene, numObjects);

  // Count triangles (approximate — render once to trigger extraction)
  {
    icl::rt::SceneRaytracer tmp(scene, "cpu");
    tmp.render(0);
  }

  const int warmup = 3;
  const int frames = 20;
  const int ptFrames = 10;

  printf("Warmup: %d frames, Measure: %d direct + %d path-tracing frames\n\n",
         warmup, frames, ptFrames);

  std::vector<BenchResult> results;
  const char *backends[] = {"cpu", "opencl", "metal"};

  for (const char *b : backends) {
    printf("Benchmarking: %s ...\n", b);
    results.push_back(benchBackend(scene, b, w, h, warmup, frames, ptFrames));
  }

  // Results table
  printf("\n");
  printf("%-28s %10s %12s %12s %10s\n",
         "Backend", "1st frame", "Direct avg", "PT avg", "PT passes");
  printf("%-28s %10s %12s %12s %10s\n",
         "---", "---", "---", "---", "---");

  for (const auto &r : results) {
    if (!r.available) {
      printf("%-28s %10s\n", r.backend.c_str(), "N/A");
      continue;
    }
    printf("%-28s %8.1f ms %10.1f ms %10.1f ms %10d\n",
           r.backend.c_str(), r.firstFrameMs, r.directAvgMs,
           r.ptAvgMs, r.ptPasses);
  }

  // Speedup comparison
  printf("\n");
  const BenchResult *cpuRes = nullptr;
  for (const auto &r : results)
    if (r.available && r.backend.find("CPU") != std::string::npos) cpuRes = &r;

  if (cpuRes) {
    printf("Speedup vs CPU (static):\n");
    for (const auto &r : results) {
      if (!r.available || &r == cpuRes) continue;
      printf("  %-26s  direct: %.1fx  path-tracing: %.1fx\n",
             r.backend.c_str(),
             cpuRes->directAvgMs / r.directAvgMs,
             cpuRes->ptAvgMs / r.ptAvgMs);
    }
  }

  // Dynamic benchmark (transform-only changes every frame — simulates physics)
  printf("\n--- Dynamic scene (transforms change every frame) ---\n");
  printf("%-28s %12s\n", "Backend", "Avg frame");
  printf("%-28s %12s\n", "---", "---");

  std::vector<std::pair<std::string, double>> dynResults;
  const int dynFrames = 30;

  for (const char *b : backends) {
    Scene dynScene;
    buildScene(dynScene, numObjects);
    double ms = benchDynamic(dynScene, b, w, h, dynFrames);
    std::string bname = b;
    if (ms < 0) {
      printf("%-28s %12s\n", b, "N/A");
    } else {
      printf("%-28s %10.1f ms\n", b, ms);
    }
    dynResults.push_back({b, ms});
  }

  double cpuDyn = -1;
  for (const auto &[n, ms] : dynResults)
    if (n == "cpu" && ms > 0) cpuDyn = ms;

  if (cpuDyn > 0) {
    printf("\nSpeedup vs CPU (dynamic):\n");
    for (const auto &[n, ms] : dynResults) {
      if (ms <= 0 || n == "cpu") continue;
      printf("  %-26s  %.1fx\n", n.c_str(), cpuDyn / ms);
    }
  }

  printf("\nDone.\n");
  return 0;
}
