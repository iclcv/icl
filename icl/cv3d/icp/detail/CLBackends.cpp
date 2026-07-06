// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// "nn.octree.cl" / "nn.color.cl" — OpenCL ICP nearest-neighbour backends:
// brute-force NN on the GPU, one work-item per query. Same exact results as the
// C++ backends (verified: identical correspondences). Brute-force O(N·M), so a GPU
// win only on LARGE clouds (crossover depends on the GPU/CPU) — opt-in, never the
// default. Self-register (only when ICL is built with OpenCL).

#include <icl/cv3d/icp/detail/ICPBackendRegistry.h>

#ifdef ICL_HAVE_OPENCL

#include <icl/utils/cl/CLProgram.h>
#include <icl/utils/Exception.h>

#include <exception>
#include <memory>
#include <vector>

namespace icl::cv3d {
  using namespace icl::utils;
  namespace {

    // One work-item per query: scan every target, keep the closest, write its
    // position. float4 matches ICP::Vec (FixedColVector<float,4>) byte-for-byte.
    const char *POS_KERNEL =
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

    // Colours travel as float4 (GeomColor = FixedColVector<float,4>) alongside the
    // positions; w2 is colorWeight². Matches ColorNN's metric exactly.
    const char *COLOR_KERNEL =
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

    // ---- "nn.octree.cl": GPU position-only NN ---------------------------------
    class CLNN : public ICP::Backend {
      // mutable: nearest() is const but must create transient CL buffers / dispatch
      mutable CLProgram m_program;
      mutable CLKernel  m_kernel;
      CLBuffer  m_targetBuf;
      int       m_numTargets = 0;
      bool      m_ok = false;
    public:
      CLNN() {
        try {
          m_program = CLProgram("gpu", POS_KERNEL);
          m_kernel  = m_program.createKernel("nn");
          m_ok = true;
        } catch (const std::exception &e) {
          WARNING_LOG("nn.octree.cl: OpenCL init failed, backend unusable: " << e.what());
        }
      }
      bool isValid() const override { return m_ok; }

      void build(const std::vector<ICP::Vec> &target) override {
        if (!m_ok) return;
        m_numTargets = (int)target.size();
        if (target.empty()) return;
        m_targetBuf = m_program.createBuffer("r", target.size() * sizeof(ICP::Vec),
                                             (const void *)target.data());
      }
      void nearest(const std::vector<ICP::Vec> &queries,
                   std::vector<ICP::Vec> &out) const override {
        out.resize(queries.size());
        if (!m_ok || m_numTargets == 0) { out = queries; return; }
        const int n = (int)queries.size();
        CLBuffer qbuf = m_program.createBuffer("r", n * sizeof(ICP::Vec), (const void *)queries.data());
        CLBuffer obuf = m_program.createBuffer("w", n * sizeof(ICP::Vec));
        m_kernel.setArgs(m_targetBuf, m_numTargets, qbuf, obuf);
        m_kernel.apply(n);
        obuf.read(out.data(), n * sizeof(ICP::Vec));
      }
    };

    // ---- "nn.color.cl": GPU colour-aware NN -----------------------------------
    class CLColorNN : public ICP::ColorBackend {
      // mutable: nearest() is const but must create transient CL buffers / dispatch
      mutable CLProgram m_program;
      mutable CLKernel  m_kernel;
      CLBuffer  m_targetBuf, m_targetColBuf;
      int       m_numTargets = 0;
      icl32f    m_colorWeight = 1.f;
      bool      m_ok = false;
      std::vector<GeomColor> m_targetCol, m_sourceCol;
    public:
      explicit CLColorNN(icl32f colorWeight = 1.0f) : m_colorWeight(colorWeight) {
        try {
          m_program = CLProgram("gpu", COLOR_KERNEL);
          m_kernel  = m_program.createKernel("nnColor");
          m_ok = true;
        } catch (const std::exception &e) {
          WARNING_LOG("nn.color.cl: OpenCL init failed, backend unusable: " << e.what());
        }
      }
      bool isValid() const override { return m_ok; }
      void setColorWeight(icl32f w) override { m_colorWeight = w; }
      icl32f getColorWeight() const override { return m_colorWeight; }
      void setTargetColors(const std::vector<GeomColor> &c) override { m_targetCol = c; }
      void setSourceColors(const std::vector<GeomColor> &c) override { m_sourceCol = c; }

      void build(const std::vector<ICP::Vec> &target) override {
        if (!m_ok) return;
        m_numTargets = (int)target.size();
        if (target.empty()) return;
        if (!m_targetCol.empty() && m_targetCol.size() != target.size()) {
          throw ICLException("nn.color.cl build: target color count != target point count");
        }
        // the kernel always reads a colour buffer; zero-fill when none supplied
        const std::vector<GeomColor> cols =
          m_targetCol.empty() ? std::vector<GeomColor>(target.size(), GeomColor(0,0,0,0))
                              : m_targetCol;
        m_targetBuf = m_program.createBuffer("r", target.size() * sizeof(ICP::Vec),
                                             (const void *)target.data());
        m_targetColBuf = m_program.createBuffer("r", cols.size() * sizeof(GeomColor),
                                                (const void *)cols.data());
      }
      void nearest(const std::vector<ICP::Vec> &queries,
                   std::vector<ICP::Vec> &out) const override {
        out.resize(queries.size());
        if (!m_ok || m_numTargets == 0) { out = queries; return; }
        const int n = (int)queries.size();
        if (!m_sourceCol.empty() && (int)m_sourceCol.size() != n) {
          throw ICLException("nn.color.cl nearest: source color count != query count");
        }
        const std::vector<GeomColor> qcols =
          m_sourceCol.empty() ? std::vector<GeomColor>(n, GeomColor(0,0,0,0)) : m_sourceCol;

        CLBuffer qbuf  = m_program.createBuffer("r", n * sizeof(ICP::Vec), (const void *)queries.data());
        CLBuffer qcbuf = m_program.createBuffer("r", n * sizeof(GeomColor), (const void *)qcols.data());
        CLBuffer obuf  = m_program.createBuffer("w", n * sizeof(ICP::Vec));
        const float w2 = m_colorWeight * m_colorWeight;
        m_kernel.setArgs(m_targetBuf, m_targetColBuf, m_numTargets, qbuf, qcbuf, w2, obuf);
        m_kernel.apply(n);
        obuf.read(out.data(), n * sizeof(ICP::Vec));
      }
    };

  } // anonymous namespace

  ICL_REGISTER_PLUGIN(icpBackendRegistry(), nn_octree_cl,
                      "nn.octree.cl",
                      [] { return std::static_pointer_cast<ICP::Backend>(
                                    std::make_shared<CLNN>()); },
                      "OpenCL brute-force position NN (opt-in, large clouds)", /*priority*/ 0);

  ICL_REGISTER_PLUGIN(icpBackendRegistry(), nn_color_cl,
                      "nn.color.cl",
                      [] { return std::static_pointer_cast<ICP::Backend>(
                                    std::make_shared<CLColorNN>()); },
                      "OpenCL brute-force colour-aware NN (opt-in, large clouds)", /*priority*/ 0);

} // namespace icl::cv3d

#endif // ICL_HAVE_OPENCL
