// Offline renderer: load XML scene files → render → save PPM.
// Supports composing multiple scene files (first camera wins, objects/lights accumulate).
//
// Usage: render-offline -scene base.xml [objects.xml ...] [-o output.ppm] [-frames 64]
//        [-pt] [-denoise svgf|atrous|none] [-tonemap aces|reinhard|hable|none]
//        [-exposure 1.0] [-backend auto|metal|cpu]

#include <ICLGeom/Scene.h>
#include <ICLUtils/ProgArg.h>
#include <ICLIO/FileWriter.h>
#include <Raytracing/SceneRaytracer.h>
#include <Raytracing/SceneLoader.h>
#include <cstdio>

using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;

int main(int argc, char **argv) {
  pa_init(argc, argv,
    "-o(string=output.png) -frames(int=64) -pt -backend(string=auto) "
    "-denoise(string=none) -tonemap(string=none) -exposure(float=1.0) "
    "-scene(...)");

  // Load scene files (composable: multiple files merged)
  Scene scene;
  int numScenes = pa("-scene").n();
  if (numScenes == 0) {
    fprintf(stderr, "Usage: render-offline -scene file1.xml [file2.xml ...] [options]\n");
    return 1;
  }

  for (int i = 0; i < numScenes; i++) {
    std::string file = pa("-scene", i);
    printf("Loading: %s\n", file.c_str());
    if (!icl::rt::loadSceneXML(file, scene)) {
      fprintf(stderr, "Failed to load: %s\n", file.c_str());
      return 1;
    }
  }

  if (scene.getCameraCount() == 0) {
    fprintf(stderr, "Error: no camera defined in scene files\n");
    return 1;
  }

  // Create raytracer
  std::string backend = pa("-backend");
  if (backend == "auto") backend = "";
  icl::rt::SceneRaytracer rt(scene, backend);
  printf("Backend: %s\n", rt.backendName());

  // Path tracing
  bool pt = pa("-pt");
  rt.setPathTracing(pt);
  if (pt) rt.setTargetFrameTime(0);

  // Denoising
  std::string dn = pa("-denoise");
  if (dn == "svgf") rt.setDenoising(icl::rt::DenoisingMethod::SVGF);
  else if (dn == "atrous") rt.setDenoising(icl::rt::DenoisingMethod::ATrousWavelet);
  else if (dn == "bilateral") rt.setDenoising(icl::rt::DenoisingMethod::Bilateral);

  // Tone mapping
  std::string tm = pa("-tonemap");
  if (tm == "aces") rt.setToneMapping(icl::rt::ToneMapMethod::ACES);
  else if (tm == "reinhard") rt.setToneMapping(icl::rt::ToneMapMethod::Reinhard);
  else if (tm == "hable") rt.setToneMapping(icl::rt::ToneMapMethod::Hable);
  rt.setExposure(pa("-exposure"));

  // Render N frames
  int frames = pa("-frames");
  const auto &cam = scene.getCamera(0);
  printf("Rendering %d frames at %dx%d (pt=%s denoise=%s tonemap=%s exposure=%.1f)...\n",
         frames, cam.getRenderParams().chipSize.width, cam.getRenderParams().chipSize.height,
         pt ? "on" : "off", dn.c_str(), tm.c_str(), (float)pa("-exposure"));

  for (int i = 0; i < frames; i++) {
    rt.render(0);
    if ((i+1) % 16 == 0 || i == frames-1)
      printf("  frame %d/%d\n", i+1, frames);
  }

  // Save output
  std::string outFile = pa("-o");
  icl::io::FileWriter writer(outFile);
  writer.write(&rt.getImage());
  printf("Wrote %s\n", outFile.c_str());
  return 0;
}
