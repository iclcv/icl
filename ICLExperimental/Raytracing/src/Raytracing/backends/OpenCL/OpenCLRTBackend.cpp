// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#ifdef ICL_HAVE_OPENCL

#include "OpenCLRTBackend.h"
#include <ICLUtils/Macros.h>
#include <fstream>
#include <sstream>
#include <chrono>

namespace icl::rt {

// Embedded kernel source (from RaytracerKernel.cl)
static std::string loadKernelSource() {
  const char *searchPaths[] = {
    "RaytracerKernel.cl",
    "../ICLExperimental/Raytracing/src/Raytracing/backends/OpenCL/RaytracerKernel.cl",
#ifdef ICL_SOURCE_DIR
    ICL_SOURCE_DIR "/ICLExperimental/Raytracing/src/Raytracing/backends/OpenCL/RaytracerKernel.cl",
#endif
    nullptr
  };
  for (int i = 0; searchPaths[i]; i++) {
    std::ifstream f(searchPaths[i]);
    if (f.good()) {
      std::stringstream ss;
      ss << f.rdbuf();
      DEBUG_LOG("Loaded OpenCL kernel from: " << searchPaths[i]);
      return ss.str();
    }
  }
  ERROR_LOG("Could not find RaytracerKernel.cl");
  return "";
}

OpenCLRTBackend::OpenCLRTBackend() {
  try {
    std::string src = loadKernelSource();
    if (src.empty()) { m_valid = false; return; }
    m_program = std::make_unique<utils::CLProgram>("gpu", src);
    m_kernel = m_program->createKernel("raytrace");
    m_ptKernel = m_program->createKernel("pathTraceKernel");
    m_valid = true;
    DEBUG_LOG("OpenCL raytracing backend initialized");
  } catch (const utils::CLException &e) {
    WARNING_LOG("OpenCL backend not available: " << e.what());
    m_valid = false;
  }
}

int OpenCLRTBackend::buildBLAS(int objectIndex,
                                const RTVertex *vertices, int numVertices,
                                const RTTriangle *triangles, int numTriangles) {
  if (objectIndex >= (int)m_blas.size()) m_blas.resize(objectIndex + 1);

  auto &entry = m_blas[objectIndex];
  entry.vertices.assign(vertices, vertices + numVertices);
  entry.triangles.assign(triangles, triangles + numTriangles);
  entry.bvh.build(entry.vertices.data(), numVertices,
                  entry.triangles.data(), numTriangles);
  entry.valid = true;
  m_sceneDirty = true;
  return objectIndex;
}

void OpenCLRTBackend::removeBLAS(int blasHandle) {
  if (blasHandle >= 0 && blasHandle < (int)m_blas.size()) {
    m_blas[blasHandle].valid = false;
    m_sceneDirty = true;
  }
}

void OpenCLRTBackend::buildTLAS(const RTInstance *instances, int numInstances) {
  m_instances.assign(instances, instances + numInstances);
  m_sceneDirty = true;
}

void OpenCLRTBackend::setSceneData(const RTLight *lights, int numLights,
                                    const RTMaterial *materials, int numMaterials,
                                    const RTFloat4 &backgroundColor) {
  m_lights.assign(lights, lights + numLights);
  m_materials.assign(materials, materials + numMaterials);
  m_bgColor = backgroundColor;
  m_sceneDirty = true;
}

void OpenCLRTBackend::flattenScene() {
  // Flatten all per-object BVH nodes, tri indices, vertices, and triangles
  // into single contiguous arrays for GPU upload.

  std::vector<BVHNode> allNodes;
  std::vector<uint32_t> allTriIndices;
  std::vector<RTVertex> allVertices;
  std::vector<RTTriangle> allTriangles;
  std::vector<FlatInstance> flatInstances;

  for (size_t i = 0; i < m_instances.size(); i++) {
    const auto &inst = m_instances[i];
    int bi = inst.blasIndex;
    if (bi < 0 || bi >= (int)m_blas.size() || !m_blas[bi].valid) continue;

    const auto &blas = m_blas[bi];
    FlatInstance fi;
    fi.transform = inst.transform;
    fi.transformInverse = inst.transformInverse;
    fi.bvhNodeOffset = (int)allNodes.size();
    fi.numBVHNodes = (int)blas.bvh.getNodes().size();
    fi.triIndexOffset = (int)allTriIndices.size();
    fi.vertexOffset = (int)allVertices.size();
    fi.triangleOffset = (int)allTriangles.size();
    fi.materialIndex = inst.materialIndex;

    // Append this object's data
    const auto &nodes = blas.bvh.getNodes();
    allNodes.insert(allNodes.end(), nodes.begin(), nodes.end());
    const auto &triIdx = blas.bvh.getTriIndices();
    allTriIndices.insert(allTriIndices.end(), triIdx.begin(), triIdx.end());
    allVertices.insert(allVertices.end(), blas.vertices.begin(), blas.vertices.end());
    allTriangles.insert(allTriangles.end(), blas.triangles.begin(), blas.triangles.end());

    flatInstances.push_back(fi);
  }

  // Upload to GPU
  if (!allNodes.empty())
    m_gpuBVHNodes = m_program->createBuffer("r", allNodes.size() * sizeof(BVHNode), allNodes.data());
  if (!allTriIndices.empty())
    m_gpuTriIndices = m_program->createBuffer("r", allTriIndices.size() * sizeof(uint32_t), allTriIndices.data());
  if (!allVertices.empty())
    m_gpuVertices = m_program->createBuffer("r", allVertices.size() * sizeof(RTVertex), allVertices.data());
  if (!allTriangles.empty())
    m_gpuTriangles = m_program->createBuffer("r", allTriangles.size() * sizeof(RTTriangle), allTriangles.data());
  if (!flatInstances.empty())
    m_gpuInstances = m_program->createBuffer("r", flatInstances.size() * sizeof(FlatInstance), flatInstances.data());
  if (!m_lights.empty())
    m_gpuLights = m_program->createBuffer("r", m_lights.size() * sizeof(RTLight), m_lights.data());
  if (!m_materials.empty())
    m_gpuMaterials = m_program->createBuffer("r", m_materials.size() * sizeof(RTMaterial), m_materials.data());

  m_instances.resize(flatInstances.size()); // update count
  m_sceneDirty = false;
  m_accumFrame = 0; // scene changed, reset accumulation
}

void OpenCLRTBackend::render(const RTRayGenParams &camera) {
  if (!m_valid) return;

  int w = camera.imageWidth;
  int h = camera.imageHeight;
  if (w <= 0 || h <= 0) return;

  // Ensure output image
  if (m_output.getWidth() != w || m_output.getHeight() != h || m_output.getChannels() != 3) {
    m_output = core::Img8u(utils::Size(w, h), core::formatRGB);
  }

  // Re-upload scene data if dirty
  if (m_sceneDirty) flattenScene();

  int n = w * h;

  // Allocate persistent GPU buffers on resize
  if (w != m_lastW || h != m_lastH) {
    m_gpuOutput = m_program->createBuffer("rw", n);
    m_gpuObjectIds = m_program->createBuffer("rw", n * sizeof(int));
    m_gpuAccumR = m_program->createBuffer("rw", n * sizeof(float));
    m_gpuAccumG = m_program->createBuffer("rw", n * sizeof(float));
    m_gpuAccumB = m_program->createBuffer("rw", n * sizeof(float));
    m_lastW = w;
    m_lastH = h;
    m_accumFrame = 0;
  }

  utils::CLBuffer gpuR = m_program->createBuffer("w", n);
  utils::CLBuffer gpuG = m_program->createBuffer("w", n);
  utils::CLBuffer gpuB = m_program->createBuffer("w", n);
  utils::CLBuffer gpuIds = m_program->createBuffer("rw", n * sizeof(int));

  const auto &Qi = camera.invViewProj;

  // Common args helper
  auto setCommonArgs = [&](utils::CLKernel &k) {
    int a = 0;
    k.setArg(a++, m_gpuBVHNodes);
    k.setArg(a++, m_gpuTriIndices);
    k.setArg(a++, m_gpuVertices);
    k.setArg(a++, m_gpuTriangles);
    k.setArg(a++, m_gpuInstances);
    k.setArg(a++, m_gpuLights);
    k.setArg(a++, m_gpuMaterials);
    return a;
  };

  if (m_pathTracing) {
    // Path tracing with GPU accumulation — always clear on frame 0
    if (m_accumFrame == 0) {
      // Ensure accum buffers exist and are zeroed
      if (!m_gpuAccumR || !m_gpuAccumG || !m_gpuAccumB) {
        m_gpuAccumR = m_program->createBuffer("rw", n * sizeof(float));
        m_gpuAccumG = m_program->createBuffer("rw", n * sizeof(float));
        m_gpuAccumB = m_program->createBuffer("rw", n * sizeof(float));
      }
      std::vector<float> zeros(n, 0);
      m_gpuAccumR.write(zeros.data(), n * sizeof(float));
      m_gpuAccumG.write(zeros.data(), n * sizeof(float));
      m_gpuAccumB.write(zeros.data(), n * sizeof(float));
    }
    // Set all args except frameNumber (last arg)
    int a = setCommonArgs(m_ptKernel);
    m_ptKernel.setArg(a++, m_gpuAccumR);
    m_ptKernel.setArg(a++, m_gpuAccumG);
    m_ptKernel.setArg(a++, m_gpuAccumB);
    m_ptKernel.setArg(a++, gpuR);
    m_ptKernel.setArg(a++, gpuG);
    m_ptKernel.setArg(a++, gpuB);
    m_ptKernel.setArg(a++, gpuIds);
    m_ptKernel.setArg(a++, camera.cameraPos.x);
    m_ptKernel.setArg(a++, camera.cameraPos.y);
    m_ptKernel.setArg(a++, camera.cameraPos.z);
    m_ptKernel.setArg(a++, Qi.cols[0][0]); m_ptKernel.setArg(a++, Qi.cols[0][1]); m_ptKernel.setArg(a++, Qi.cols[0][2]);
    m_ptKernel.setArg(a++, Qi.cols[1][0]); m_ptKernel.setArg(a++, Qi.cols[1][1]); m_ptKernel.setArg(a++, Qi.cols[1][2]);
    m_ptKernel.setArg(a++, Qi.cols[2][0]); m_ptKernel.setArg(a++, Qi.cols[2][1]); m_ptKernel.setArg(a++, Qi.cols[2][2]);
    m_ptKernel.setArg(a++, w);
    m_ptKernel.setArg(a++, h);
    m_ptKernel.setArg(a++, (int)m_instances.size());
    m_ptKernel.setArg(a++, (int)m_lights.size());
    m_ptKernel.setArg(a++, m_bgColor.x);
    m_ptKernel.setArg(a++, m_bgColor.y);
    m_ptKernel.setArg(a++, m_bgColor.z);
    int frameArgIdx = a; // remember index of frameNumber arg

    // Multi-pass: run as many passes as fit in the time budget
    auto t0 = std::chrono::steady_clock::now();
    float firstPassMs = 0;

    do {
      m_accumFrame++;
      m_ptKernel.setArg(frameArgIdx, m_accumFrame);
      m_ptKernel.apply(w, h);
      m_ptKernel.finish();

      auto now = std::chrono::steady_clock::now();
      float elapsedMs = std::chrono::duration<float, std::milli>(now - t0).count();
      if (firstPassMs == 0) firstPassMs = std::max(0.1f, elapsedMs);

      // Stop if we can't fit another pass in the remaining budget
      if (m_targetFrameMs <= 0) break; // single pass mode
      if (elapsedMs + firstPassMs > m_targetFrameMs) break;
    } while (true);
  } else {
    // Direct lighting + reflections
    int a = setCommonArgs(m_kernel);
    m_kernel.setArg(a++, gpuR);
    m_kernel.setArg(a++, gpuG);
    m_kernel.setArg(a++, gpuB);
    m_kernel.setArg(a++, gpuIds);
    m_kernel.setArg(a++, camera.cameraPos.x);
    m_kernel.setArg(a++, camera.cameraPos.y);
    m_kernel.setArg(a++, camera.cameraPos.z);
    m_kernel.setArg(a++, Qi.cols[0][0]); m_kernel.setArg(a++, Qi.cols[0][1]); m_kernel.setArg(a++, Qi.cols[0][2]);
    m_kernel.setArg(a++, Qi.cols[1][0]); m_kernel.setArg(a++, Qi.cols[1][1]); m_kernel.setArg(a++, Qi.cols[1][2]);
    m_kernel.setArg(a++, Qi.cols[2][0]); m_kernel.setArg(a++, Qi.cols[2][1]); m_kernel.setArg(a++, Qi.cols[2][2]);
    m_kernel.setArg(a++, w);
    m_kernel.setArg(a++, h);
    m_kernel.setArg(a++, (int)m_instances.size());
    m_kernel.setArg(a++, (int)m_lights.size());
    m_kernel.setArg(a++, m_bgColor.x);
    m_kernel.setArg(a++, m_bgColor.y);
    m_kernel.setArg(a++, m_bgColor.z);

    m_kernel.apply(w, h);
    m_kernel.finish();
  }

  // Read back
  gpuR.read(m_output.getData(0), n);
  gpuG.read(m_output.getData(1), n);
  gpuB.read(m_output.getData(2), n);

  // Read back object IDs for picking
  m_cpuObjectIds.resize(n);
  gpuIds.read(m_cpuObjectIds.data(), n * sizeof(int));

  // Post-processing stages (virtual — CPU fallbacks by default)
  applyDenoisingStage(m_output);
  applyUpsamplingStage(m_output, m_cpuObjectIds);
}

} // namespace icl::rt

#endif // ICL_HAVE_OPENCL
