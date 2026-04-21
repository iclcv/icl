// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "SceneRaytracer.h"
#include "RaytracerBackend_Cpu.h"
#include <ICLGeom/Scene.h>
#include <ICLUtils/Macros.h>
#include <cstring>

namespace icl::rt {

SceneRaytracer::SceneRaytracer(geom::Scene &scene)
  : m_scene(scene)
  , m_backend(std::make_unique<CpuRTBackend>())
{
  // TODO: try Metal backend first when available
  DEBUG_LOG("SceneRaytracer using backend: " << m_backend->name());
}

void SceneRaytracer::render(int camIndex) {
  // Extract geometry (only dirty objects are re-extracted)
  ExtractedScene extracted = m_extractor.extract(m_scene, camIndex);

  // Update BLAS for objects with changed geometry
  for (size_t i = 0; i < extracted.objects.size(); i++) {
    const auto &geo = extracted.objects[i];
    if (geo.geometryChanged && !geo.vertices.empty()) {
      m_backend->buildBLAS((int)i,
                           geo.vertices.data(), (int)geo.vertices.size(),
                           geo.triangles.data(), (int)geo.triangles.size());
    }
  }

  // Rebuild TLAS if any transforms changed (or objects added/removed)
  if (extracted.anyTransformChanged || extracted.anyGeometryChanged) {
    m_backend->buildTLAS(extracted.instances.data(), (int)extracted.instances.size());
  }

  // Update scene data if lights/materials changed
  if (extracted.lightsChanged || extracted.anyGeometryChanged) {
    m_backend->setSceneData(extracted.lights.data(), (int)extracted.lights.size(),
                            extracted.materials.data(), (int)extracted.materials.size(),
                            extracted.backgroundColor);
  }

  // Detect camera changes → reset path tracing accumulation
  if (std::memcmp(&extracted.camera, &m_lastCamera, sizeof(RTRayGenParams)) != 0) {
    auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
    if (cpu) cpu->resetAccumulation();
    m_lastCamera = extracted.camera;
  }

  // Also reset if scene geometry changed
  if (extracted.anyGeometryChanged || extracted.anyTransformChanged) {
    auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
    if (cpu) cpu->resetAccumulation();
  }

  // Render
  m_backend->render(extracted.camera);
}

const core::Img8u &SceneRaytracer::getImage() const {
  return m_backend->readback();
}

void SceneRaytracer::invalidateAll() {
  m_extractor.invalidateAll();
}

void SceneRaytracer::invalidateObject(geom::SceneObject *obj) {
  m_extractor.invalidateObject(obj);
}

void SceneRaytracer::setAASamples(int spp) {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  if (cpu) cpu->setAASamples(spp);
}

void SceneRaytracer::setFXAA(bool enabled) {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  if (cpu) cpu->setFXAA(enabled);
}

void SceneRaytracer::setAdaptiveAA(bool enabled, int edgeSpp) {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  if (cpu) cpu->setAdaptiveAA(enabled, edgeSpp);
}

void SceneRaytracer::setPathTracing(bool enabled) {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  if (cpu) cpu->setPathTracing(enabled);
}

int SceneRaytracer::getAccumulatedFrames() const {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  return cpu ? cpu->getAccumulatedFrames() : 0;
}

int SceneRaytracer::getObjectAtPixel(int x, int y) const {
  auto *cpu = dynamic_cast<CpuRTBackend *>(m_backend.get());
  return cpu ? cpu->getObjectAtPixel(x, y) : -1;
}

const char *SceneRaytracer::backendName() const {
  return m_backend->name();
}

} // namespace icl::rt
