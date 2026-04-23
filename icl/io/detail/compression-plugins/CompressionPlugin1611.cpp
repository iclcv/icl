// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `1611` codec: Kinect-style 11-bit packing of single-channel icl16s
// images. Two quality variants — see Kinect11BitCompressor for details:
//   "1" = lossless bit-pack (clamps inputs >2047 to the 11-bit mask)
//   "0" = depth-mapping variant (applies the Kinect Z formula; LOSSY)
// Always built (no external dep beyond ICLIO itself).

#include <icl/io/detail/compression-plugins/CompressionPlugin.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/detail/compression-plugins/CompressionRegistry.h>
#include <icl/io/detail/kinect/Kinect11BitCompressor.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <vector>
#include <cstdint>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    class Plugin1611 : public CompressionPlugin {
      std::vector<icl8u> m_buf;
      int m_quality = 1;  // default to lossless bit-pack

    public:
      Plugin1611() {
        addProperty("quality", prop::Menu{"0", "1"}, "1", 0,
                    "0 = lossy depth-mapping (Kinect Z formula). "
                    "1 = lossless 11-bit bit-pack (default; clamps >2047).");
        Configurable::registerCallback([this](const Property &p){
          if (p.name == "quality") m_quality = parse<int>(p.value);
        });
      }

      std::string name() const override { return "1611"; }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        if (p->getDepth() != depth16s || p->getChannels() != 1) {
          throw ICLException("1611: only single-channel icl16s images are supported");
        }
        const int dim = p->getDim();
        const std::size_t encoded = Kinect11BitCompressor::estimate_packed_size(dim);
        m_buf.resize(encoded * sizeof(uint16_t));
        const std::uint16_t *in = reinterpret_cast<const std::uint16_t*>(p->as16s()->getData(0));
        std::uint16_t *out = reinterpret_cast<std::uint16_t*>(m_buf.data());
        if (m_quality == 1) Kinect11BitCompressor::pack16to11   (in, out, dim);
        else                Kinect11BitCompressor::pack16to11_2 (in, out, dim);
        return {m_buf.data(), m_buf.size()};
      }

      Image decompress(Bytes bytes, const ImgParams &params, depth d) override {
        if (d != depth16s || params.getChannels() != 1) {
          throw ICLException("1611: only single-channel icl16s images are supported");
        }
        Image out(params.getSize(), depth16s, 1, params.getFormat());
        out.ptr()->setROI(params.getROI());
        const int dim = out.getDim();
        const std::uint16_t *src = reinterpret_cast<const std::uint16_t*>(bytes.data);
        std::uint16_t *dst = reinterpret_cast<std::uint16_t*>(out.ptr()->as16s()->getData(0));
        if (m_quality == 1) Kinect11BitCompressor::unpack11to16   (src, dst, dim);
        else                Kinect11BitCompressor::unpack11to16_2 (src, dst, dim);
        return out;
      }

      std::string getCodecParamsString() const override { return str(m_quality); }
      void setCodecParamsString(const std::string &p) override {
        if (!p.empty()) m_quality = parse<int>(p);
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(1611, [](){
    return std::unique_ptr<CompressionPlugin>(new Plugin1611);
  })
} // namespace icl::io
