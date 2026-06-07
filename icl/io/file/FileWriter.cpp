// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/file/FileWriter.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Macros.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  // Plugins self-register via REGISTER_FILE_WRITER_PLUGIN at static-init
  // time. Each registered callable carries its own state (per-lambda
  // function-local statics initialized on first call), so no external
  // per-extension cache is needed.

  FileWriterRegistry& fileWriterRegistry() {
    static FileWriterRegistry inst(utils::OnDuplicate::KeepHighestPriority);
    return inst;
  }

  FileWriterConfigRegistry& fileWriterConfigRegistry() {
    static FileWriterConfigRegistry inst(utils::OnDuplicate::KeepFirst);
    return inst;
  }

  // Pull in every plugin-registered Configurable singleton as a named
  // child Configurable on this FileWriter instance.  Singletons survive
  // process lifetime; multiple FileWriters share the same children
  // (property writes go to the underlying singletons, so changing
  // "jpeg.quality" on one writer affects all future .jpg writes).
  void FileWriter::attachPluginConfigurables() {
    for (const auto &e : fileWriterConfigRegistry().entries()) {
      if (auto *cfg = e.payload()) {
        addChildConfigurable(cfg, e.key);  // key is the "jpeg"/"csv"/"png" prefix
      }
    }
  }

  FileWriter::FileWriter() {
    attachPluginConfigurables();
  }
  FileWriter::FileWriter(const std::string &filepattern) : m_oGen(filepattern) {
    attachPluginConfigurables();
  }
  FileWriter::FileWriter(const FilenameGenerator &gen)   : m_oGen(gen) {
    attachPluginConfigurables();
  }

  void FileWriter::write(const Image &image){
    ICLASSERT_RETURN(!image.isNull());
    ICLASSERT_RETURN(image.getDim());
    ICLASSERT_RETURN(image.getChannels());
    ICLASSERT_RETURN(!m_oGen.isNull());
    ICLASSERT_RETURN(m_oGen.filesLeft());

    File file(m_oGen.next());

    const auto *e = fileWriterRegistry().get(toLower(file.getSuffix()));
    if (!e) {
      ERROR_LOG("No Plugin to write files with suffix " << file.getSuffix() << " available");
      return;
    }
    e->payload(file, image.ptr());
  }

  } // namespace icl::io
