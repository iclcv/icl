// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// OpenCL nearest-neighbour backends for ICP: brute-force NN on the GPU, one
// work-item per query. CLNN accelerates the exact position-only metric of the
// default OctreeNN; CLColorNN adds the weighted colour term of ColorNN. Fresh,
// self-contained kernels. A rep-DB approximate-NN pass (skip scanning every
// target) would speed very large clouds further, but is unnecessary for
// correctness. Empty when ICL is built without OpenCL.

#include <icl/cv3d/icp/ICP.h>

#ifdef ICL_HAVE_OPENCL

#include <icl/utils/cl/CLProgram.h>
#include <icl/utils/Exception.h>
#include <exception>

namespace icl::cv3d {

  using namespace icl::utils;

  // One work-item per query: scan every target, keep the closest, write its
  // position. float4 matches ICP::Vec (FixedColVector<float,4>) byte-for-byte.
  static const char *ICP_NN_KERNEL =
    "__kernel void nn(__global const float4 *targets, int numTargets,          \n"
    "                 __global const float4 *queries, __global float4 *out){    \n"
    "  int i = get_global_id(0);                                                \n"
    "  float4 q = queries[i];                                                   \n"
    "  float best = INFINITY;                                                   \n"
    "  int bi = 0;                                                              \n"
    "  for(int j=0;j<numTargets;++j){                                           \n"
    "    float4 t = targets[j];                                                 \n"
    "    float dx=q.x-t.x, dy=q.y-t.y, dz=q.z-t.z;                              \n"
    "    float d = dx*dx + dy*dy + dz*dz;                                       \n"
    "    if(d<best){ best=d; bi=j; }                                            \n"
    "  }                                                                        \n"
    "  out[i] = targets[bi];                                                    \n"
    "}                                                                          \n";

  struct CLNN::Data {
    CLProgram program;
    CLKernel  kernel;
    CLBuffer  targetBuf;
    int       numTargets = 0;
    bool      ok = false;
  };

  CLNN::CLNN() : m_data(new Data) {
    try {
      m_data->program = CLProgram("gpu", ICP_NN_KERNEL);
      m_data->kernel  = m_data->program.createKernel("nn");
      m_data->ok = true;
    } catch (const std::exception &e) {
      WARNING_LOG("CLNN: OpenCL initialisation failed, backend unusable: " << e.what());
      m_data->ok = false;
    }
  }

  CLNN::~CLNN() {}

  bool CLNN::isValid() const { return m_data->ok; }

  void CLNN::build(const std::vector<ICP::Vec> &target) {
    if (!m_data->ok) return;
    m_data->numTargets = (int)target.size();
    if (target.empty()) return;
    m_data->targetBuf = m_data->program.createBuffer(
        "r", target.size() * sizeof(ICP::Vec), (const void *)target.data());
  }

  void CLNN::nearest(const std::vector<ICP::Vec> &queries,
                     std::vector<ICP::Vec> &out) const {
    out.resize(queries.size());
    if (!m_data->ok || m_data->numTargets == 0) {   // uninitialised / empty target
      out = queries;
      return;
    }
    const int n = (int)queries.size();
    CLBuffer qbuf = m_data->program.createBuffer("r", n * sizeof(ICP::Vec),
                                                 (const void *)queries.data());
    CLBuffer obuf = m_data->program.createBuffer("w", n * sizeof(ICP::Vec));
    m_data->kernel.setArgs(m_data->targetBuf, m_data->numTargets, qbuf, obuf);
    m_data->kernel.apply(n);
    obuf.read(out.data(), n * sizeof(ICP::Vec));
  }

  // ---- CLColorNN: GPU nearest neighbour with the weighted colour term ----------

  // Colours travel as float4 (GeomColor = FixedColVector<float,4>) alongside the
  // positions; w2 is colorWeight². Matches ColorNN's metric exactly.
  static const char *ICP_NN_COLOR_KERNEL =
    "__kernel void nnColor(__global const float4 *targets,                       \n"
    "                      __global const float4 *targetCols, int numTargets,    \n"
    "                      __global const float4 *queries,                       \n"
    "                      __global const float4 *queryCols, float w2,           \n"
    "                      __global float4 *out){                                \n"
    "  int i = get_global_id(0);                                                 \n"
    "  float4 q = queries[i];                                                    \n"
    "  float4 qc = queryCols[i];                                                 \n"
    "  float best = INFINITY;                                                    \n"
    "  int bi = 0;                                                               \n"
    "  for(int j=0;j<numTargets;++j){                                            \n"
    "    float4 t = targets[j];                                                  \n"
    "    float4 tc = targetCols[j];                                              \n"
    "    float dx=q.x-t.x, dy=q.y-t.y, dz=q.z-t.z;                               \n"
    "    float dr=qc.x-tc.x, dg=qc.y-tc.y, db=qc.z-tc.z;                         \n"
    "    float d = dx*dx + dy*dy + dz*dz + w2*(dr*dr + dg*dg + db*db);           \n"
    "    if(d<best){ best=d; bi=j; }                                             \n"
    "  }                                                                         \n"
    "  out[i] = targets[bi];                                                     \n"
    "}                                                                           \n";

  struct CLColorNN::Data {
    CLProgram program;
    CLKernel  kernel;
    CLBuffer  targetBuf, targetColBuf;
    int       numTargets = 0;
    icl32f    colorWeight = 1.f;
    bool      ok = false;
    std::vector<GeomColor> targetCol;   // parallel to the target cloud (may be empty)
    std::vector<GeomColor> sourceCol;   // parallel to the query cloud (may be empty)
  };

  CLColorNN::CLColorNN(icl32f colorWeight) : m_data(new Data) {
    m_data->colorWeight = colorWeight;
    try {
      m_data->program = CLProgram("gpu", ICP_NN_COLOR_KERNEL);
      m_data->kernel  = m_data->program.createKernel("nnColor");
      m_data->ok = true;
    } catch (const std::exception &e) {
      WARNING_LOG("CLColorNN: OpenCL initialisation failed, backend unusable: " << e.what());
      m_data->ok = false;
    }
  }

  CLColorNN::~CLColorNN() {}

  bool CLColorNN::isValid() const { return m_data->ok; }
  void CLColorNN::setColorWeight(icl32f w) { m_data->colorWeight = w; }
  icl32f CLColorNN::getColorWeight() const { return m_data->colorWeight; }
  void CLColorNN::setTargetColors(const std::vector<GeomColor> &c) { m_data->targetCol = c; }
  void CLColorNN::setSourceColors(const std::vector<GeomColor> &c) { m_data->sourceCol = c; }

  void CLColorNN::build(const std::vector<ICP::Vec> &target) {
    if (!m_data->ok) return;
    m_data->numTargets = (int)target.size();
    if (target.empty()) return;
    if (!m_data->targetCol.empty() && m_data->targetCol.size() != target.size()) {
      throw ICLException("CLColorNN::build: target color count != target point count");
    }
    // the kernel always reads a colour buffer; zero-fill when none were supplied
    // (then Δcolor == 0 for every pair, so it is a plain position NN)
    const std::vector<GeomColor> cols =
      m_data->targetCol.empty() ? std::vector<GeomColor>(target.size(), GeomColor(0,0,0,0))
                                : m_data->targetCol;
    m_data->targetBuf = m_data->program.createBuffer(
        "r", target.size() * sizeof(ICP::Vec), (const void *)target.data());
    m_data->targetColBuf = m_data->program.createBuffer(
        "r", cols.size() * sizeof(GeomColor), (const void *)cols.data());
  }

  void CLColorNN::nearest(const std::vector<ICP::Vec> &queries,
                          std::vector<ICP::Vec> &out) const {
    out.resize(queries.size());
    if (!m_data->ok || m_data->numTargets == 0) {
      out = queries;
      return;
    }
    const int n = (int)queries.size();
    if (!m_data->sourceCol.empty() && (int)m_data->sourceCol.size() != n) {
      throw ICLException("CLColorNN::nearest: source color count != query count");
    }
    const std::vector<GeomColor> qcols =
      m_data->sourceCol.empty() ? std::vector<GeomColor>(n, GeomColor(0,0,0,0))
                                : m_data->sourceCol;

    CLBuffer qbuf  = m_data->program.createBuffer("r", n * sizeof(ICP::Vec),
                                                  (const void *)queries.data());
    CLBuffer qcbuf = m_data->program.createBuffer("r", n * sizeof(GeomColor),
                                                  (const void *)qcols.data());
    CLBuffer obuf  = m_data->program.createBuffer("w", n * sizeof(ICP::Vec));
    const float w2 = m_data->colorWeight * m_data->colorWeight;
    m_data->kernel.setArgs(m_data->targetBuf, m_data->targetColBuf, m_data->numTargets,
                           qbuf, qcbuf, w2, obuf);
    m_data->kernel.apply(n);
    obuf.read(out.data(), n * sizeof(ICP::Vec));
  }

} // namespace icl::cv3d

#endif // ICL_HAVE_OPENCL
