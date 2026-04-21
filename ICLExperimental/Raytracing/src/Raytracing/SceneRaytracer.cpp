// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "SceneRaytracer.h"
#include "RaytracerBackend_Cpu.h"
#ifdef ICL_HAVE_OPENCL
#include "RaytracerBackend_OpenCL.h"
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
#ifdef ICL_HAVE_OPENCL
  else if (backend == "opencl" || backend.empty()) {
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

  m_backend->render(extracted.camera);
}

const core::Img8u &SceneRaytracer::getImage() const { return m_backend->readback(); }
void SceneRaytracer::invalidateAll() { m_extractor.invalidateAll(); }
void SceneRaytracer::invalidateObject(geom::SceneObject *obj) { m_extractor.invalidateObject(obj); }
void SceneRaytracer::setAASamples(int spp) { m_backend->setAASamples(spp); }
void SceneRaytracer::setFXAA(bool enabled) { m_backend->setFXAA(enabled); }
void SceneRaytracer::setAdaptiveAA(bool enabled, int edgeSpp) { m_backend->setAdaptiveAA(enabled, edgeSpp); }
void SceneRaytracer::setPathTracing(bool enabled) { m_backend->setPathTracing(enabled); }
int SceneRaytracer::getAccumulatedFrames() const { return m_backend->getAccumulatedFrames(); }
int SceneRaytracer::getObjectAtPixel(int x, int y) const { return m_backend->getObjectAtPixel(x, y); }
void SceneRaytracer::setTargetFrameTime(float ms) { m_backend->setTargetFrameTime(ms); }
const char *SceneRaytracer::backendName() const { return m_backend->name(); }

} // namespace icl::rt
