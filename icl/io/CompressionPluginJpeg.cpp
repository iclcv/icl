// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `jpeg` codec: lossy JPEG via libjpeg. Built only when ICL_HAVE_LIBJPEG.
// Supports icl8u multi-channel (1ch / 3ch). decompress() returns whatever
// libjpeg's color depth/format reports — the facade reshapes via the
// envelope's image params if needed (typically a no-op for jpeg
// roundtrips, since jpeg encodes shape implicitly).

#ifdef ICL_HAVE_LIBJPEG

#include <icl/io/CompressionPlugin.h>
#include <icl/io/CompressionRegistry.h>
#include <icl/io/JPEGEncoder.h>
#include <icl/io/JPEGDecoder.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <memory>
#include <vector>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    class JpegPlugin : public CompressionPlugin {
      std::unique_ptr<JPEGEncoder> m_encoder;
      ImgBase *m_decodeBuf = nullptr;
      int m_quality = 90;

    public:
      JpegPlugin() {
        addProperty("quality", "range", "[1,100]:1", "90", 0,
                    "JPEG quality (1=worst/smallest, 100=best/largest). "
                    "Defaults to 90; values around 70 are typically a good "
                    "size/quality tradeoff for natural images.");
        Configurable::registerCallback([this](const Property &p){
          if (p.name == "quality") m_quality = parse<int>(p.value);
        });
      }

      ~JpegPlugin() override { ICL_DELETE(m_decodeBuf); }

      std::string name() const override { return "jpeg"; }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        if (p->getDepth() != depth8u) {
          throw ICLException("jpeg: only icl8u images are supported");
        }
        if (!m_encoder) m_encoder.reset(new JPEGEncoder(m_quality));
        m_encoder->setQuality(m_quality);
        const JPEGEncoder::EncodedData &j = m_encoder->encode(p);
        return {j.bytes, static_cast<std::size_t>(j.len)};
      }

      Image decompress(Bytes bytes,
                       const ImgParams &/*params*/, depth /*d*/) override {
        // libjpeg recovers width/height/channels from the stream itself —
        // we ignore the envelope's `params` for JPEG since they always
        // agree by construction (sender encoded that exact image).
        JPEGDecoder::decode(bytes.data,
                            static_cast<unsigned int>(bytes.len),
                            &m_decodeBuf);
        // Hand the decoded ImgBase* to an Image (deep-copy semantics —
        // the consumer can keep this Image even past our next decompress()).
        return Image(m_decodeBuf->deepCopy());
      }

      std::string getCodecParamsString() const override { return str(m_quality); }
      void setCodecParamsString(const std::string &p) override {
        if (!p.empty()) m_quality = parse<int>(p);
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(jpeg, [](){
    return std::unique_ptr<CompressionPlugin>(new JpegPlugin);
  })
} // namespace icl::io

#endif // ICL_HAVE_LIBJPEG
