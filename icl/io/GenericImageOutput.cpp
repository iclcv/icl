// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/GenericImageOutput.h>
#include <icl/io/FileWriter.h>  // built-in "file" backend registration

#include <icl/utils/StringUtils.h>
#include <icl/utils/TextTable.h>

#include <cstdlib>
#include <iostream>
#include <string>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {

  ImageOutputRegistry& imageOutputRegistry() {
    static ImageOutputRegistry reg(utils::OnDuplicate::KeepFirst);
    return reg;
  }

  GenericImageOutput::GenericImageOutput(const std::string &type, const std::string &description){
    init(type,description);
  }

  GenericImageOutput::GenericImageOutput(const ProgArg &pa){
    init(pa);
  }

  void GenericImageOutput::init(const ProgArg &pa){
    init(*pa, utils::pa(pa.getID(), 1));
  }

  void GenericImageOutput::release(){
    impl = {};
  }

  void GenericImageOutput::send(const core::Image &image) {
    if (impl) {
      impl(image);
    } else {
      ERROR_LOG("unable to send image with a NULL output");
    }
  }

  void GenericImageOutput::init(const std::string &type, const std::string &description){
    impl = {};
    this->type        = type;
    this->description = description;

    // strip leading "<type>=" from description if present
    std::string d = description;
    if (d.substr(0, type.length() + 1) == type + "=") {
      d = d.substr(type.length() + 1);
    }

    if (type == "list") {
      const auto entries = imageOutputRegistry().entries();
      TextTable t(4, static_cast<int>(entries.size()) + 1, 50);
      t(0,0) = "nr";
      t(1,0) = "id";
      t(2,0) = "parameter";
      t(3,0) = "explanation";
      int i = 1;
      for (const auto &e : entries) {
        auto parts = tok(e.description, "~");
        t(0,i) = str(i - 1);
        t(1,i) = e.key;
        t(2,i) = parts.size() > 0 ? parts[0] : std::string();
        t(3,i) = parts.size() > 1 ? parts[1] : std::string();
        ++i;
      }
      std::cout << "Supported Image Output Devices:\n\n" << t << std::endl;
      std::terminate();
    }

    const auto *entry = imageOutputRegistry().get(type);
    if (!entry) {
      ERROR_LOG("unable to instantiate GenericImageOutput with type \""
                << type << "\" and params \"" << d << "\"");
      return;
    }
    try {
      impl = entry->payload(d);
    } catch (const std::exception &ex) {
      ERROR_LOG("Unable to construct GenericImageOutput of type \""
                << type << "\" with params \"" << d << "\": " << ex.what());
      impl = {};
    }
  }

  } // namespace icl::io

// ----- built-in backends that don't have their own .cpp -----------------

// "null" — discard every image.
REGISTER_IMAGE_OUTPUT(null_sink, "null",
  ([](const std::string&) -> icl::io::ImageOutputFn {
    return [](const icl::core::Image&) {};
  }),
  "(ignored)~Null output, discards every frame")

// "file" — forward to FileWriter. Lives here (rather than in FileWriter.cpp)
// to keep FileWriter.cpp independent of GenericImageOutput.
REGISTER_IMAGE_OUTPUT(file_sink, "file",
  ([](const std::string &params) -> icl::io::ImageOutputFn {
    auto w = std::make_shared<icl::io::FileWriter>(params);
    return [w](const icl::core::Image &img) { w->write(img.ptr()); };
  }),
  "File Pattern~File Writer (suffix dispatches to the appropriate file-writer plugin)")
