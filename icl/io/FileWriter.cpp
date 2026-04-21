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

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  // -------------------------------- FileWriterPluginRegister --
  // Plugins self-register via REGISTER_FILE_WRITER_PLUGIN at static-init
  // time, but actual plugin instances are built lazily on first lookup
  // (and cached). This keeps ImageCompressor — which some plugins like
  // BICL hold internally — out of the static-init phase, so the
  // CompressionRegister is fully populated by the time any compressor
  // gets constructed.

  FileWriterPluginRegister &FileWriterPluginRegister::instance() {
    static FileWriterPluginRegister inst;
    return inst;
  }

  void FileWriterPluginRegister::registerExtension(
      const std::string &extension, Factory factory, bool overrideExisting) {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    auto it = inst.m_factories.find(extension);
    if (it != inst.m_factories.end() && !overrideExisting) {
      // Silently keep the first registration. Pre-refactor behaviour
      // was last-wins via map-assignment; we keep first-wins to make
      // build configurations deterministic. Pass overrideExisting=true
      // to opt in to last-wins (e.g. ImageMagick deferring to libpng).
      return;
    }
    inst.m_factories[extension] = std::move(factory);
    inst.m_cache.erase(extension);  // invalidate cached instance
  }

  FileWriterPlugin *FileWriterPluginRegister::getOrCreate(const std::string &ext) {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    if (auto cit = inst.m_cache.find(ext); cit != inst.m_cache.end()) return cit->second.get();
    auto fit = inst.m_factories.find(ext);
    if (fit == inst.m_factories.end()) return nullptr;
    auto plugin = (fit->second)();
    FileWriterPlugin *raw = plugin.get();
    inst.m_cache[ext] = std::move(plugin);
    return raw;
  }

  std::vector<std::string> FileWriterPluginRegister::extensions() {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    std::vector<std::string> out;
    out.reserve(inst.m_factories.size());
    for (const auto &[k, _] : inst.m_factories) out.push_back(k);
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

    FileWriterPlugin *plugin = FileWriterPluginRegister::getOrCreate(
      toLower(file.getSuffix()));
    if (!plugin) {
      ERROR_LOG("No Plugin to write files with suffix " << file.getSuffix() << " available");
      return;
    }
    plugin->write(file, image);
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