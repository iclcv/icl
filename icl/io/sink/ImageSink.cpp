// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/sink/ImageSink.h>
#include <icl/io/detail/SinkBackend.h>
#include <icl/io/detail/FileWriter.h>  // built-in "file" backend
#include <icl/io/detail/BackendListing.h>

#include <icl/utils/StringUtils.h>
#include <icl/utils/Exit.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {

  SinkBackendRegistry& sinkBackendRegistry() {
    static SinkBackendRegistry reg(utils::OnDuplicate::KeepFirst);
    return reg;
  }

  ImageSink::ImageSink(const std::string &type, const std::string &description){
    init(type,description);
  }

  ImageSink::ImageSink(const ProgArg &pa){
    init(pa);
  }

  void ImageSink::init(const ProgArg &pa){
    init(*pa, utils::pa(pa.getID(), 1));
  }

  void ImageSink::release(){
    if (m_backend) removeChildConfigurable(m_backend.get());
    m_backend.reset();
  }

  void ImageSink::send(const core::Image &image) {
    if (m_backend) {
      m_backend->send(image);
    } else {
      ERROR_LOG("unable to send image with a NULL sink");
    }
  }

  void ImageSink::init(const std::string &type, const std::string &description){
    release();
    this->type        = type;
    this->description = description;

    // strip leading "<type>=" from description if present
    std::string d = description;
    if (d.substr(0, type.length() + 1) == type + "=") {
      d = d.substr(type.length() + 1);
    }

    if (type == "list") {
      std::vector<std::pair<std::string,std::string>> entries;
      for (const auto &e : sinkBackendRegistry().entries()) {
        entries.emplace_back(e.key, e.description);
      }
      detail::printBackendTable("Supported Image Output Devices:", entries);
      utils::exit(0);
    }

    const auto *entry = sinkBackendRegistry().get(type);
    if (!entry) {
      ERROR_LOG("unable to instantiate ImageSink with type \""
                << type << "\" and params \"" << d << "\"");
      return;
    }
    try {
      m_backend = entry->payload(d);
      // Forward the backend's tunables as our own properties (empty prefix
      // flattens them, so e.g. "compression.mode" / "jpeg.quality" resolve
      // directly on the ImageSink).
      if (m_backend) addChildConfigurable(m_backend.get(), "");
    } catch (const std::exception &ex) {
      ERROR_LOG("Unable to construct ImageSink of type \""
                << type << "\" with params \"" << d << "\": " << ex.what());
      m_backend.reset();
    }
  }

  // ----- built-in backends that don't have their own .cpp ----------------
  namespace {
    /// "null" — discard every image.
    class NullSink : public SinkBackend {
      public:
      void send(const core::Image&) override {}
    };

    /// "file" — forward to FileWriter (suffix dispatches to the file-writer
    /// plugin).  Wraps FileWriter so the writer's tunables (jpeg.quality,
    /// png.compression-level, …) surface as sink properties.
    class FileSink : public SinkBackend {
      FileWriter m_writer;
      public:
      explicit FileSink(const std::string &pattern) : m_writer(pattern) {
        addChildConfigurable(&m_writer, "");
      }
      void send(const core::Image &image) override { m_writer.write(image); }
    };
  }

  } // namespace icl::io

REGISTER_SINK_BACKEND(null_sink, "null",
  ([](const std::string&) -> std::shared_ptr<icl::io::SinkBackend> {
    return std::make_shared<icl::io::NullSink>();
  }),
  "(ignored)~Null output, discards every frame")

REGISTER_SINK_BACKEND(file_sink, "file",
  ([](const std::string &params) -> std::shared_ptr<icl::io::SinkBackend> {
    return std::make_shared<icl::io::FileSink>(params);
  }),
  "File Pattern~File Writer (suffix dispatches to the appropriate file-writer plugin)")
