// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "SceneRaytracer.h"
#include "RaytracerBackend_Cpu.h"
#ifdef ICL_HAVE_OPENCL
#include "RaytracerBackend_OpenCL.h"
#endif
#ifdef ICL_HAVE_METAL
#include "RaytracerBackend_Metal.h"
#endif
#include <ICLGeom/Scene.h>
#include <ICLUtils/Macros.h>
#include <cstring>

namespace icl::rt {

SceneRaytracer::SceneRaytracer(geom::Scene &scene, const std::string &backend)
  : m_scene(scene)
{
  if (backend == "cpu") {
    m_backend = std::make_unique<CpuRTBackend>();
  }
#ifdef ICL_HAVE_METAL
  else if (backend == "metal" || backend.empty()) {
    auto mtl = std::make_unique<MetalRTBackend>();
    if (mtl->isAvailable()) {
      m_backend = std::move(mtl);
    }
  }
#endif
#ifdef ICL_HAVE_OPENCL
  if (!m_backend && (backend == "opencl" || backend.empty())) {
    auto cl = std::make_unique<OpenCLRTBackend>();
    if (cl->isAvailable()) {
      m_backend = std::move(cl);
    }
  }
#endif
  if (!m_backend) {
    m_backend = std::make_unique<CpuRTBackend>();
  }
  DEBUG_LOG("SceneRaytracer using backend: " << m_backend->name());
}

void SceneRaytracer::render(int camIndex) {
  ExtractedScene extracted = m_extractor.extract(m_scene, camIndex);

  for (size_t i = 0; i < extracted.objects.size(); i++) {
    const auto &geo = extracted.objects[i];
    if (geo.geometryChanged && !geo.vertices.empty()) {
      m_backend->buildBLAS((int)i,
                           geo.vertices.data(), (int)geo.vertices.size(),
                           geo.triangles.data(), (int)geo.triangles.size());
    }
  }

  if (extracted.anyTransformChanged || extracted.anyGeometryChanged) {
    m_backend->buildTLAS(extracted.instances.data(), (int)extracted.instances.size());
  }

  if (extracted.lightsChanged || extracted.anyGeometryChanged) {
    m_backend->setSceneData(extracted.lights.data(), (int)extracted.lights.size(),
                            extracted.materials.data(), (int)extracted.materials.size(),
                            extracted.backgroundColor);
  }

  if (std::memcmp(&extracted.camera, &m_lastCamera, sizeof(RTRayGenParams)) != 0) {
    m_backend->resetAccumulation();
    m_lastCamera = extracted.camera;
  }
  if (extracted.anyGeometryChanged || extracted.anyTransformChanged) {
    m_backend->resetAccumulation();
  }

  // Apply render scale: render at lower resolution, backend upsamples to display res
  RTRayGenParams renderCamera = extracted.camera;
  float scale = m_backend->getRenderScale();
  if (scale < 1.0f && m_backend->getUpsamplingMethod() != UpsamplingMethod::None) {
    m_backend->setDisplaySize(renderCamera.imageWidth, renderCamera.imageHeight);
    renderCamera.imageWidth  = std::max(1, (int)(renderCamera.imageWidth * scale));
    renderCamera.imageHeight = std::max(1, (int)(renderCamera.imageHeight * scale));
  } else {
    m_backend->setDisplaySize(0, 0); // no upsampling
  }

  m_backend->render(renderCamera);
}

const core::Img8u &SceneRaytracer::getImage() const { return m_backend->readback(); }
void SceneRaytracer::invalidateAll() { m_extractor.invalidateAll(); }
void SceneRaytracer::invalidateTransforms() { m_extractor.invalidateTransforms(); }
void SceneRaytracer::invalidateObject(geom::SceneObject *obj) { m_extractor.invalidateObject(obj); }
void SceneRaytracer::setAASamples(int spp) { m_backend->setAASamples(spp); }
void SceneRaytracer::setFXAA(bool enabled) { m_backend->setFXAA(enabled); }
void SceneRaytracer::setAdaptiveAA(bool enabled, int edgeSpp) { m_backend->setAdaptiveAA(enabled, edgeSpp); }
void SceneRaytracer::setPathTracing(bool enabled) { m_backend->setPathTracing(enabled); }
int SceneRaytracer::getAccumulatedFrames() const { return m_backend->getAccumulatedFrames(); }
int SceneRaytracer::getObjectAtPixel(int x, int y) const { return m_backend->getObjectAtPixel(x, y); }
void SceneRaytracer::setTargetFrameTime(float ms) { m_backend->setTargetFrameTime(ms); }
const char *SceneRaytracer::backendName() const { return m_backend->name(); }
bool SceneRaytracer::setUpsampling(UpsamplingMethod method) { return m_backend->setUpsampling(method); }
void SceneRaytracer::setRenderScale(float scale) {
  if (scale != m_backend->getRenderScale()) {
    m_backend->setRenderScale(scale);
    m_backend->resetAccumulation();
  }
}
bool SceneRaytracer::supportsUpsampling(UpsamplingMethod method) const { return m_backend->supportsUpsampling(method); }
UpsamplingMethod SceneRaytracer::getUpsamplingMethod() const { return m_backend->getUpsamplingMethod(); }
float SceneRaytracer::getRenderScale() const { return m_backend->getRenderScale(); }

} // namespace icl::rt
