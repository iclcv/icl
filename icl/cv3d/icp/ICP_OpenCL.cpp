// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// OpenCL nearest-neighbour backend for ICP: brute-force NN on the GPU, one
// work-item per query. Accelerates the same exact position-only metric as the
// default OctreeNN for large clouds. A fresh, self-contained kernel (the
// preserved rep-DB approximate-NN seed in IterativeClosestPoint* is a further
// optimisation, not needed for a correct GPU backend). Empty when ICL is built
// without OpenCL.

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

} // namespace icl::cv3d

#endif // ICL_HAVE_OPENCL
