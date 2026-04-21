// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileWriter.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>

#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/FileWriterPluginJPEG.h>
#endif
#include <icl/io/FileWriterPluginCSV.h>

#include <algorithm>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  // -------------------------------- FileWriterPluginRegister --
  // Plugins self-register via REGISTER_FILE_WRITER_PLUGIN at static-init
  // time. Each registered callable carries its own state (per-lambda
  // function-local statics initialized on first call), so no external
  // per-extension cache is needed — a wash vs the pre-4b m_cache map.

  FileWriterPluginRegister &FileWriterPluginRegister::instance() {
    static FileWriterPluginRegister inst;
    return inst;
  }

  void FileWriterPluginRegister::registerExtension(
      const std::string &extension, Factory factory, int priority) {
    // Empty description, empty applicability; the registry's
    // KeepHighestPriority policy uses `priority` to resolve conflicts.
    instance().m_registry.registerPlugin(extension, std::move(factory),
                                         /*description*/ {}, priority);
  }

  const FileWriterPluginRegister::Factory *
  FileWriterPluginRegister::find(const std::string &ext) {
    const auto *e = instance().m_registry.get(ext);
    return e ? &e->payload : nullptr;
  }

  std::vector<std::string> FileWriterPluginRegister::extensions() {
    auto out = instance().m_registry.keys();
    std::sort(out.begin(), out.end());
    return out;
  }


  FileWriter::FileWriter(){

  }


  FileWriter::FileWriter(const std::string &filepattern):

    m_oGen(filepattern){}


  FileWriter::FileWriter(const FilenameGenerator &gen):

    m_oGen(gen){}


  FileWriter::~FileWriter(){

  }


  const FilenameGenerator &FileWriter::getFilenameGenerator() const{

    return m_oGen;
  }


  void FileWriter::write(const ImgBase *image){
    ICLASSERT_RETURN(image);
    ICLASSERT_RETURN(image->getDim());
    ICLASSERT_RETURN(image->getChannels());
    ICLASSERT_RETURN(!m_oGen.isNull());
    ICLASSERT_RETURN(m_oGen.filesLeft());

    File file(m_oGen.next());

    const auto *fn = FileWriterPluginRegister::find(toLower(file.getSuffix()));
    if (!fn) {
      ERROR_LOG("No Plugin to write files with suffix " << file.getSuffix() << " available");
      return;
    }
    (*fn)(file, image);
  }


  FileWriter &FileWriter::operator<<(const ImgBase *image){

    write(image);
    return *this;
  }


  void FileWriter::setOption(const std::string &option, const std::string &value){
    if(option == "csv:extend-file-name"){
      if(toLower(value) == "true"){
        FileWriterPluginCSV::setExtendFileName(true);
      }else if(toLower(value) == "false"){
        FileWriterPluginCSV::setExtendFileName(false);
      }else{
        ERROR_LOG("Undefined value \"" << value <<"\" for option \"" << option << "\"");
      }
    }else if(option == "jpg:quality"){
#ifdef WITH_JPEG_SUPPORT
      FileWriterPluginJPEG::setQuality(parse<int>(value));
#else
      ERROR_LOG("Unable to std::set option \"jpg:quality\" (JPEG support is currently disabled!)");
#endif
    }else{
      ERROR_LOG("Unsupported Option \"" << option << "\" (value: \"" << value << "\")");
    }

  }


  } // namespace icl::io
